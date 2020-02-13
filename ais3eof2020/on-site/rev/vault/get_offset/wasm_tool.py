# TODO:
#  - name locations
#  - mark data xref to memory load/store
#  - mark xref to imports
#  - compute stack deltas
#  - add entry point for start function (need to see an example)

# stdlib
import logging
import functools

# from pip
import wasm
import wasm.wasmtypes
#  import netnode

# from IDA
#  import idc
#  import idaapi
#  import idautils

# from this project
import idawasm.const
import idawasm.common
#  import idawasm.analysis.llvm


logger = logging.getLogger(__name__)

class WWW:
    # processor ID for the wasm disassembler.
    # I made this number up.
    id = 0x8069
#      flag = idaapi.PR_USE32 | idaapi.PR_RNAMESOK | idaapi.PRN_HEX | idaapi.PR_NO_SEGMOVE
    cnbits = 8
    dnbits = 8
    psnames = ['wasm']
    plnames = ['WebAssembly']
    segreg_size = 0
    tbyte_size = 0
    assembler = {
#          'flag': idaapi.ASH_HEXF3 | idaapi.AS_UNEQU | idaapi.AS_COLON | idaapi.ASB_BINF4 | idaapi.AS_N2CHR,
        'uflag': 0,
        'name': "WebAssembly assembler",
        'origin': "org",
        'end': "end",
        'cmnt': ";;",
        'ascsep': "\"",
        'accsep': "'",
        'esccodes': "\"'",
        'a_ascii': "db",
        'a_byte': "db",
        'a_word': "dw",
        'a_dword': "dd",
        'a_qword': "dq",
        'a_oword': "xmmword",
        'a_float': "dd",
        'a_double': "dq",
        'a_tbyte': "dt",
        'a_dups': "#d dup(#v)",
        'a_bss': "%s dup ?",
        'a_seg': "seg",
        'a_curip': "$",
        'a_public': "public",
        'a_weak': "weak",
        'a_extrn': "extrn",
        'a_comdef': "",
        'a_align': "align",
        'lbrace': "(",
        'rbrace': ")",
        'a_mod': "%",
        'a_band': "&",
        'a_bor': "|",
        'a_xor': "^",
        'a_bnot': "~",
        'a_shl': "<<",
        'a_shr': ">>",
        'a_sizeof_fmt': "size %s",
    }


    def _get_section(self, section_id):
        '''
        fetch the section with the given id.

        Args:
          section_id (int): the section id.

        Returns:
          wasm.Structure: the section.

        Raises:
          KeyError: if the section is not found.
        '''
        for i, section in enumerate(self.sections):
            if i == 0:
                continue

            if section.data.id != section_id:
                continue

            return section

        raise KeyError(section_id)

    def _get_section_offset(self, section_id):
        '''
        fetch the file offset of the given section.

        Args:
          section_id (int): the section id.

        Returns:
          int: the offset of the section.

        Raises:
          KeyError: if the section is not found.
        '''
        p = 0
        for i, section in enumerate(self.sections):
            if i == 0:
                p += idawasm.common.size_of(section.data)
                continue

            if section.data.id != section_id:
                p += idawasm.common.size_of(section.data)
                continue

            return p

        raise KeyError(section_id)

    def _compute_function_branch_targets(self, offset, code):
        '''
        compute branch targets for the given code segment.

        we can do it in a single pass:
        scan instructions, tracking new blocks, and maintaining a stack of nested blocks.
        when we hit a branch instruction, use the stack to resolve the branch target.
        the branch target will always come from the enclosing scope.

        Args:
          offset (int): offset of the given code segment.
          code (bytes): raw bytecode.

        Returns:
          Dict[int, Dict[int, int]]: map from instruction addresses to map from relative depth to branch target address.
        '''
        # map from virtual address to map from relative depth to virtual address
        branch_targets = {}
        # map from block index to block instance, with fields including `offset` and `depth`
        blocks = {}
        # stack of block indexes
        block_stack = []
        p = offset

        for bc in wasm.decode.decode_bytecode(code):
            if bc.op.id in {wasm.opcodes.OP_BLOCK, wasm.opcodes.OP_LOOP, wasm.opcodes.OP_IF}:
                # enter a new block, so capture info, and push it onto the current depth stack
                block_index = len(blocks)
                block = {
                    'index': block_index,
                    'offset': p,
                    'depth': len(block_stack),
                    'type': {
                        wasm.opcodes.OP_BLOCK: 'block',
                        wasm.opcodes.OP_LOOP: 'loop',
                        wasm.opcodes.OP_IF: 'if',
                    }[bc.op.id],
                }
                blocks[block_index] = block
                block_stack.insert(0, block_index)
                branch_targets[p] = {
                    # reference to block that is starting
                    'block': block
                }

            elif bc.op.id in {wasm.opcodes.OP_END}:
                if len(block_stack) == 0:
                    # end of function
                    branch_targets[p] = {
                        'block': {
                            'type': 'function',
                            'offset': offset,     # start of function
                            'end_offset': p,      # end of function
                            'depth': 0,           # top level always has depth 0
                        }
                    }
                    break

                # leaving a block, so pop from the depth stack
                block_index = block_stack.pop(0)
                block = blocks[block_index]
                block['end_offset'] = p + bc.len
                branch_targets[p] = {
                    # reference to block that is ending
                    'block': block
                }

            elif bc.op.id in {wasm.opcodes.OP_BR, wasm.opcodes.OP_BR_IF}:
                block_index = block_stack[bc.imm.relative_depth]
                block = blocks[block_index]
                branch_targets[p] = {
                    bc.imm.relative_depth: block
                }

            elif bc.op.id in {wasm.opcodes.OP_ELSE}:
                # TODO: not exactly sure of the semantics here
#                  raise NotImplementedError('else')
                pass

            elif bc.op.id in {wasm.opcodes.OP_BR_TABLE}:
                # TODO: not exactly sure what one of these looks like yet.
#                  raise NotImplementedError('br table')
                pass
                # probably will populate `branch_targets` with multiple entries

            p += bc.len

        return branch_targets

    def _compute_branch_targets(self):
        branch_targets = {}

        code_section = self._get_section(wasm.wasmtypes.SEC_CODE)
        pcode_section = self._get_section_offset(wasm.wasmtypes.SEC_CODE)

        ppayload = pcode_section + idawasm.common.offset_of(code_section.data, 'payload')
        pbody = ppayload + idawasm.common.offset_of(code_section.data.payload, 'bodies')
        for body in code_section.data.payload.bodies:
            pcode = pbody + idawasm.common.offset_of(body, 'code')
            branch_targets.update(self._compute_function_branch_targets(pcode, body.code))
            pbody += idawasm.common.size_of(body)

        return branch_targets

    def _parse_types(self):
        '''
        parse the type entries.

        Returns:
          List[Dict[str, Any]]: list if type descriptors, each which hash:
            - form
            - param_count
            - param_types
            - return_count
            - return_type
        '''
        type_section = self._get_section(wasm.wasmtypes.SEC_TYPE)
        return idawasm.common.struc_to_dict(type_section.data.payload.entries)

    def _parse_globals(self):
        '''
        parse the global entries.

        Returns:
          Dict[int, Dict[str, any]]: from global index to dict with keys `offset` and `type`.
        '''
        globals_ = {}
        global_section = self._get_section(wasm.wasmtypes.SEC_GLOBAL)
        pglobal_section = self._get_section_offset(wasm.wasmtypes.SEC_GLOBAL)

        ppayload = pglobal_section + idawasm.common.offset_of(global_section.data, 'payload')
        pglobals = ppayload + idawasm.common.offset_of(global_section.data.payload, 'globals')
        pcur = pglobals
        for i, body in enumerate(global_section.data.payload.globals):
            pinit = pcur + idawasm.common.offset_of(body, 'init')
            ctype = idawasm.const.WASM_TYPE_NAMES[body.type.content_type]
            globals_[i] = {
                'index': i,
                'offset': pinit,
                'type': ctype,
            }
            pcur += idawasm.common.size_of(body)
        return globals_

    def _parse_imported_functions(self):
        '''
        parse the import entries for functions.
        useful for recovering function names.

        Returns:
          Dict[int, Dict[str, any]]: from function index to dict with keys `index`, `module`, and `name`.
        '''
        functions = {}
        import_section = self._get_section(wasm.wasmtypes.SEC_IMPORT)
        type_section = self._get_section(wasm.wasmtypes.SEC_TYPE)

        function_index = 0
        for entry in import_section.data.payload.entries:
            if entry.kind != idawasm.const.WASM_EXTERNAL_KIND_FUNCTION:
                continue

            type_index = entry.type.type
            ftype = type_section.data.payload.entries[type_index]

            functions[function_index] = {
                'index': function_index,
                'module': entry.module_str.tobytes().decode('utf-8'),
                'name': entry.field_str.tobytes().decode('utf-8'),
                'type': idawasm.common.struc_to_dict(ftype),
                'imported': True,
                # TODO: not sure if an import can be exported.
                'exported': False,
            }

            function_index += 1

        return functions

    def _parse_exported_functions(self):
        '''
        parse the export entries for functions.
        useful for recovering function names.

        Returns:
          Dict[int, Dict[str, any]]: from function index to dict with keys `index` and `name`.
        '''
        functions = {}
        export_section = self._get_section(wasm.wasmtypes.SEC_EXPORT)
        for entry in export_section.data.payload.entries:
            if entry.kind != idawasm.const.WASM_EXTERNAL_KIND_FUNCTION:
                continue

            functions[entry.index] = {
                'index': entry.index,
                'name': entry.field_str.tobytes().decode('utf-8'),
                'exported': True,
                # TODO: not sure if an export can be imported.
                'imported': False,
            }

        return functions

    def _parse_functions(self):
        imported_functions = self._parse_imported_functions()
        exported_functions = self._parse_exported_functions()

        functions = dict(imported_functions)

        function_section = self._get_section(wasm.wasmtypes.SEC_FUNCTION)
        code_section = self._get_section(wasm.wasmtypes.SEC_CODE)
        pcode_section = self._get_section_offset(wasm.wasmtypes.SEC_CODE)
        type_section = self._get_section(wasm.wasmtypes.SEC_TYPE)

        payload = code_section.data.payload
        ppayload = pcode_section + idawasm.common.offset_of(code_section.data, 'payload')
        pbody = ppayload + idawasm.common.offset_of(payload, 'bodies')
        for i in range(code_section.data.payload.count):
            function_index = len(imported_functions) + i
            body = code_section.data.payload.bodies[i]
            type_index = function_section.data.payload.types[i]
            ftype = type_section.data.payload.entries[type_index]

            local_types = []
            for locals_group in body.locals:
                ltype = locals_group.type
                for j in range(locals_group.count):
                    local_types.append(ltype)

            if function_index in exported_functions:
                name = exported_functions[function_index]['name']
                is_exported = True
            else:
                name = '$func%d' % (function_index)
                is_exported = False

            functions[function_index] = {
                'index': function_index,
                'name': name,
                'offset': pbody + idawasm.common.offset_of(body, 'code'),
                'type': idawasm.common.struc_to_dict(ftype),
                'exported': is_exported,
                'imported': False,
                'local_types': local_types,
                'size': idawasm.common.size_of(body, 'code'),
            }

            pbody += idawasm.common.size_of(body)

        return functions

    def _render_type(self, type_, name=None):
        if name is None:
            name = ''
        else:
            name = ' ' + name

        params = []
        if type_['param_count'] > 0:
            for i, param in enumerate(type_['param_types']):
                params.append(' (param $param%d %s)' % (i, idawasm.const.WASM_TYPE_NAMES[param]))
        sparam = ''.join(params)

        if type_['return_count'] == 0:
            sresult = ''
        elif type_['return_count'] == 1:
            sresult = ' (result %s)' % (idawasm.const.WASM_TYPE_NAMES[type_['return_type']])
        else:
            raise NotImplementedError('multiple return values')

        return '(func%s%s%s)' % (name, sparam, sresult)

    def _render_function_prototype(self, function):
        if function.get('imported'):
            name = '$import%d' % (function['index'])
            signature = self._render_type(function['type'], name=name)
            return '(import "%s" "%s" %s)' % (function['module'],
                                              function['name'],
                                              signature)
        else:
            return self._render_type(function['type'], name=function['name'])

    def load(self):
        '''
        load the state of the processor and analysis from the segments.

        the processor object may not be re-created, so we do our initializiation here.
        initialize the following fields:

          - self.buf
          - self.sections
          - self.functions
          - self.function_offsets
          - self.function_ranges
          - self.globals
          - self.branch_targets
        '''
        logger.info('parsing sections')
#          buf = []
#          for ea in idautils.Segments():
#              # assume all the segments are contiguous, which is what our loader does
#              buf.append(idc.GetManyBytes(idc.SegStart(ea), idc.SegEnd(ea) - idc.SegStart(ea)))

#          self.buf = b''.join(buf)
        self.sections = list(wasm.decode.decode_module(self.buf))

        logger.info('parsing types')
        self.types = self._parse_types()

        logger.info('parsing globals')
        self.globals = self._parse_globals()

        logger.info('parsing functions')
        self.functions = self._parse_functions()

        # map from function offset to function object
        self.function_offsets = {f['offset']: f for f in self.functions.values() if 'offset' in f}

        # map from (function start, function end) to function object
        self.function_ranges = {
            (f['offset'], f['offset'] + f['size']): f
            for f in self.functions.values()
            if 'offset' in f
        }

        logger.info('computing branch targets')
        self.branch_targets = self._compute_branch_targets()

        self.deferred_noflows = {}
        self.deferred_flows = {}

#          for function in self.functions.values():
#              name = function['name'].encode('utf-8')
#              if 'offset' in function:
#                  idc.MakeName(function['offset'], name)
#                  # notify_emu will be invoked from here.
#                  idc.MakeCode(function['offset'])
#                  idc.MakeFunction(function['offset'], function['offset'] + function['size'])

#              if function.get('exported'):
#                  # TODO: this should really be done in the loader.
#                  # though, at the moment, we do a lot more analysis here in the processor.
#                  idc.add_entry(function['index'], function['offset'], name, True)


    def _get_function(self, ea):
        '''
        fetch the function object that contains the given address.
        '''
        # warning: O(#funcs) scan here, called in a tight loop (render operand).
        for (start, end), f in self.function_ranges.items():
            if start <= ea < end:
                return f
        raise KeyError(ea)

    def init_instructions(self):
        # Now create an instruction table compatible with IDA processor module requirements
        self.insns = {}
        for i, op in enumerate(wasm.opcodes.OPCODES):
            self.insns[op.id] = {
                # the opcode byte
                'opcode': op.id,
                # the IDA constant for this instruction
                'id': i,
                # danger: this must be an ASCII-encoded byte string, *not* unicode!
                'name': op.mnemonic.encode('ascii'),
                'feature': op.flags,
                'cmt': idawasm.const.WASM_OPCODE_DESCRIPTIONS.get(op.id),
            }
            clean_mnem = op.mnemonic.encode('ascii').replace(b'.', b'_').replace(b'/', b'_').upper()
            # the itype constant value must be contiguous, which sucks, because its not the op.id value.
            setattr(self, 'itype_' + clean_mnem.decode(), i)

        # Array of instructions
        # the index into this array apparently must match the `self.itype_*`.
        self.instruc = list(sorted(self.insns.values(), key=lambda i: i['id']))

        self.instruc_start = 0
        self.instruc_end = len(self.instruc)
        self.icode_return = self.itype_RETURN

    def init_registers(self):
        """This function parses the register table and creates corresponding ireg_XXX constants"""

        # Registers definition
        # for wasm, "registers" are local variables.
        self.reg_names = []

        # we'd want to scan the module and pick the max number of parameters,
        # however, the data isn't available yet,
        # so we pick a scary large number.
        #
        # note: IDA reg_t size is 16-bits
        MAX_LOCALS = 0x1000
        for i in range(MAX_LOCALS):
            self.reg_names.append("$local%d" % (i))

        # we'd want to scan the module and pick the max number of parameters,
        # however, the data isn't available yet,
        # so we pick a scary large number.
        MAX_PARAMS = 0x1000
        for i in range(MAX_PARAMS):
            self.reg_names.append("$param%d" % (i))

        # these are fake, "virtual" registers.
        # req'd for IDA, apparently.
        # (not actually used in wasm)
        self.reg_names.append("SP")
        self.reg_names.append("CS")
        self.reg_names.append("DS")

        # Create the ireg_XXXX constants.
        # for wasm, will look like: ireg_LOCAL0, ireg_PARAM0
        for i in range(len(self.reg_names)):
            setattr(self, 'ireg_' + self.reg_names[i].replace('$', ''), i)

        # Segment register information (use virtual CS and DS registers if your
        # processor doesn't have segment registers):
        # (not actually used in wasm)
#          self.reg_first_sreg = self.ireg_CS
#          self.reg_last_sreg = self.ireg_DS

        # number of CS register
        # (not actually used in wasm)
#          self.reg_code_sreg = self.ireg_CS

        # number of DS register
        # (not actually used in wasm)

    def __init__(self, filename):
        # this is called prior to loading a binary, so don't read from the database here.
#          idaapi.processor_t.__init__(self)
        self.PTRSZ = 4  # Assume PTRSZ = 4 by default
        self.init_instructions()
        self.init_registers()

        # these will be populated by `notify_newfile`
#          self.buf = b''
        self.buf = open(filename, 'rb').read()
        # ordered list of wasm section objects
        self.sections = []
        # map from function index to function object
        self.functions = {}
        # map from virtual address to function object
        self.function_offsets = {}
        # map from (va-start, va-end) to function object
        self.function_ranges = {}
        # map from global index to global object
        self.globals = {}
        # map from va to map from relative depth to va
        self.branch_targets = {}
        # list of type descriptors
        self.types = []

        # map from address to list of cref arguments.
        # used by `notify_emu`.
        self.deferred_flows = {}

        # set of addresses which should not flow.
        # map from address to True.
        # used by `notify_emu`.
        self.deferred_noflows = {}

w = WWW('vault.wasm')
w.load()
import pdb; pdb.set_trace()
for f in w.functions.values():
    try:
        print(f['name'], hex(f['offset']))
    except KeyError:
        pass

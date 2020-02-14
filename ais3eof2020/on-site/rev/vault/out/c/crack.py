import angr
import claripy

angr.l.setLevel('DEBUG')
p = angr.Project('./vault', load_options={"auto_load_libs": False})

args = claripy.BVS('args', 8*15)
initial_state = p.factory.entry_state(args=[p.filename, args], add_options={'BYPASS_UNSUPPORTED_SYSCALL'})

for i, b in enumerate(args.chop(8)):
    initial_state.add_constraints(b >= 0x21, b <= 0x7e)

pg = p.factory.simulation_manager(initial_state)
pg.explore(find=[0x4022cb], avoid=[])
print(pg)
print(pg.found[0].solver.eval(args, cast_to=bytes))
import angr
import claripy

angr.l.setLevel('DEBUG')
p = angr.Project('./vault', load_options={"auto_load_libs": False})

args = claripy.BVS('args', 8*15)
initial_state = p.factory.entry_state(args=[p.filename, args], add_options={'BYPASS_UNSUPPORTED_SYSCALL'})

for i, b in enumerate(args.chop(8)):
    initial_state.add_constraints(
        ( (ord('a') <= b) & (b <= ord('z')) )
        | ( (ord('A') <= b) & (b <= ord('Z')) )
        | ( (ord('0') <= b) & (b <= ord('9')) ))

pg = p.factory.simulation_manager(initial_state)
pg.explore(find=[0x804c6b3], avoid=[])
print(pg)
for i in range(len(pg.found)):
    print(pg.found[i].solver.eval(args, cast_to=bytes))
# mtQQXXqN7iUqFi1
# EOF{h0ly_3gg!th1s_1s_webassembly!!}

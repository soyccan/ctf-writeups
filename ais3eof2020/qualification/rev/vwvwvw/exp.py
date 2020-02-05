import sys
sys.path.append('/home/soyccan/.local/share/virtualenvs/vwvwvw-SJfVni3y/lib/python3.6/site-packages/')

import angr
import claripy

angr.l.setLevel('DEBUG')
p = angr.Project('./verify', load_options={"auto_load_libs": False})

args = claripy.BVS('args', 8*24)
initial_state = p.factory.entry_state(args=[p.filename, args], add_options={'BYPASS_UNSUPPORTED_SYSCALL'})

prefix = 'FLAG{'
for i, b in enumerate(args.chop(8)):
    initial_state.add_constraints(b >= 0x21, b <= 0x7e)
#    if i < len(prefix):
#        initial_state.add_constraints(b == prefix[i])
#    if i == 23:
#        initial_state.add_constraints(b == '}')


pg = p.factory.simulation_manager(initial_state)
pg.explore(find=[0x4022cb], avoid=[])
print(pg)

#print(pg.found[0].posix.dumps(0).strip('\0\n'))
#args_str = pg.found[0].state.se.any_str(args)
#stdin = f.state.posix.dumps(0)
#ans = pg.found[0].state.se._solver.result.model
#print(ans)
print(pg.found[0].solver.eval(args, cast_to=bytes))


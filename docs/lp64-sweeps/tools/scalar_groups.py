"""Group the hits classify.py cannot judge, so they can be read by hand.

classify.py only rules on casts whose base is a struct it has layouts for.
Everything else -- a base typed char*, float*, long long int*, or nothing the
compiler could report -- is still a raw x86 offset and still has to be read,
just not mechanically. Grouping by file, then by (base type, cast, base),
turns ~1100 scattered lines into a few hundred decisions.

Writes scalar_groups.txt. Run join.py first.
"""
import collections
import lp64env as env

rows = [l.rstrip('\n').split('\t') for l in open(env.work('joined.tsv'))]

bad = {'centity_s*', 'flameGeneric_s*', 'GfxStaticModelDrawStream*'}
groups = collections.OrderedDict()
for f, ln, typ, cast, base, n, off, code in rows:
    key = (f, typ, cast, base)
    g = groups.setdefault(key, {'n': [], 'eg': (ln, code)})
    g['n'].append(int(n))

out = []
byfile = collections.OrderedDict()
for (f, typ, cast, base), g in groups.items():
    byfile.setdefault(f, []).append((typ, cast, base, g))

for f in sorted(byfile):
    out.append('== ' + f)
    for typ, cast, base, g in sorted(byfile[f], key=lambda x: -len(x[3]['n'])):
        ns = sorted(set(g['n']))
        ln, code = g['eg']
        out.append('  [%3d] %-18s (%s)%s +%s  e.g. :%s %s'
                   % (len(g['n']), typ if typ != '?' else '?', cast, base, ns, ln, code.strip()))

dest = env.work('scalar_groups.txt')
open(dest, 'w').write('\n'.join(out) + '\n')
print('groups', len(groups), 'across', len(byfile), 'files ->', dest)

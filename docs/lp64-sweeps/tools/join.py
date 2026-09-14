"""Stage 3: attach each hit's compiler-reported base type.

typeprobe.py reports one type per probed expression as file/line/column. The
scan does not record a column, so the two are matched positionally: the Nth
type reported on a line belongs to the Nth hit scanned on that line. Hits the
compiler never reported on (a line the probe build could not parse, a header
included nowhere) get '?' and are skipped by classify.py.

Writes joined.tsv: file, line, base type, cast, base, N, x86 offset, code.
"""
import collections
import lp64env as env

hits = [l.rstrip('\n').split('\t') for l in open(env.work('hits_built.tsv'))]
types = [l.rstrip('\n').split('\t') for l in open(env.work('types.tsv'))]

bycol = collections.defaultdict(list)
for f, ln, col, t in types:
    bycol[(f, ln)].append((int(col), t))
for k in bycol:
    bycol[k].sort()

seen = collections.Counter()
rows = []
unknown = 0
for f, ln, cast, base, n, off, code in hits:
    k = (f, ln)
    i = seen[k]
    seen[k] += 1
    cand = bycol.get(k, [])
    t = cand[i][1] if i < len(cand) else '?'
    if t == '?':
        unknown += 1
    rows.append((f, ln, t, cast, base, n, off, code))

path = env.work('joined.tsv')
with open(path, 'w') as fh:
    for r in rows:
        fh.write('\t'.join(str(x) for x in r) + '\n')
print('joined', len(rows), 'hits,', unknown, 'without a reported type ->', path)
c = collections.Counter(r[2] for r in rows)
for k, v in c.most_common(15):
    print(f'  {v:5d} {k}')

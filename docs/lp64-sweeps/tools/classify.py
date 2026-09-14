import dwarf, collections, re
import lp64env as env

L = dwarf.load(env.work('layouts_lp64.o'))
S = dwarf.load(env.work('layouts_ilp32.o'))
sl = dwarf.structs(L); ss = dwarf.structs(S)
leaves = {}
def lv(name):
    if name not in leaves:
        leaves[name] = (list(dwarf.flatten(S, ss[name])), list(dwarf.flatten(L, sl[name])))
    return leaves[name]

SZ86 = {'unsigned int': 4, 'int': 4, '_DWORD': 4, 'float': 4, '_BYTE': 1, 'unsigned __int8': 1,
        '_WORD': 2, 'unsigned __int16': 2, '__int16': 2, '_QWORD': 8, 'char': 1}
def csize(cast, abi):
    if cast.endswith('**'):
        return 4 if abi == 32 else 8
    return SZ86[cast.rstrip('*').strip()]

def what(leaf, off, width):
    if not leaf:
        return f'@{off} (outside struct)'
    o, s, p, t = leaf
    rel = off - o
    part = '' if rel == 0 and width == s else f' +{rel} (w{width} of {s})'
    return f'{p} @{o}{part}'

rows = [l.rstrip('\n').split('\t') for l in open(env.work('joined.tsv'))]
out = collections.OrderedDict()
for f, ln, t, cast, base, n, off, code in rows:
    st = t.replace('const ', '').rstrip('*').strip()
    if st not in ss or t.count('*') != 1:
        continue
    n = int(n)
    o86 = n * csize(cast, 32); o64 = n * csize(cast, 64)
    l86, l64 = lv(st)
    a = dwarf.leaf_at(l86, o86); b = dwarf.leaf_at(l64, o64)
    same = a and b and a[2] == b[2] and (o86 - a[0]) == (o64 - b[0])
    key = (st, cast, n)
    out.setdefault(key, {'same': same, 'x86': what(a, o86, csize(cast, 32)),
                         'lp64': what(b, o64, csize(cast, 64)), 'where': []})
    out[key]['where'].append(f'{f}:{ln}')

for (st, cast, n), v in out.items():
    tag = 'OK ' if v['same'] else 'BAD'
    print(f"{tag} {st} *(({cast}){'x'}+{n})  x86: {v['x86']}  |  LP64: {v['lp64']}  [{len(v['where'])}]")
    if not v['same']:
        for w in v['where']:
            print('      ', w)

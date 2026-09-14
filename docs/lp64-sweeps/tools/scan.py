"""Extract *((T *)base + N) / *((T **)base + N) reads from the tree.

Writes every hit as TSV: file, line, cast, base, N, x86 offset, code.
Base is the expression between the cast and '+'. The x86 offset assumes the
cast type's x86 size (pointer = 4).
"""
import os, re, sys, collections
import lp64env as env

ROOT = env.repo('src')
SKIP = {'tracy', 'nvapi', 'PhysX', 'physx', 'bink', 'libs', 'zlib', 'jpeg',
        'groupvoice', 'mjpeg', 'steam', 'DW', 'CubeMapGenLib', 'xenon', 'nx'}

X86 = {
    'unsigned int': 4, 'int': 4, '_DWORD': 4, 'DWORD': 4, '__int32': 4,
    'unsigned __int32': 4, 'float': 4, 'double': 8, '_BYTE': 1, 'char': 1,
    'unsigned char': 1, 'unsigned __int8': 1, '__int8': 1, 'bool': 1,
    '_WORD': 2, '__int16': 2, 'unsigned __int16': 2, 'short': 2,
    'unsigned short': 2, '__int64': 8, 'unsigned __int64': 8, '_QWORD': 8,
}

# *( (cast) base + N )  -- base: identifier chain with -> . [] & and simple parens
PAT = re.compile(
    r'\*\s*\(\s*\(\s*(?:const\s+)?(?P<cast>[A-Za-z_][A-Za-z_0-9 ]*?)\s*(?P<stars>\*+)\s*\)\s*'
    r'(?P<base>&?\(?[A-Za-z_][A-Za-z_0-9]*(?:(?:->|\.)[A-Za-z_][A-Za-z_0-9]*|\[[^\]]*\])*\)?)'
    r'\s*\+\s*(?P<n>\d+|0x[0-9A-Fa-f]+)\s*\)')

out = []
for dp, dns, fns in os.walk(ROOT):
    dns[:] = [d for d in dns if d not in SKIP]
    for fn in fns:
        if not fn.endswith(('.cpp', '.c', '.h')):
            continue
        path = os.path.join(dp, fn)
        with open(path, encoding='utf-8', errors='replace') as f:
            for ln, line in enumerate(f, 1):
                s = line.strip()
                if s.startswith('//'):
                    continue
                for m in PAT.finditer(line):
                    cast = m.group('cast').strip()
                    stars = len(m.group('stars'))
                    n = int(m.group('n'), 0)
                    if stars >= 2:
                        esz = 4                      # pointer on x86
                    else:
                        esz = X86.get(cast)
                    off = n * esz if esz else None
                    out.append((os.path.relpath(path, env.REPO).replace(os.sep, '/'), ln, cast + '*' * stars,
                                m.group('base'), n, off, s[:160]))

dest = sys.argv[1] if len(sys.argv) > 1 else env.work('hits.tsv')
with open(dest, 'w') as f:
    for r in out:
        f.write('\t'.join(str(x) for x in r) + '\n')

print('hits', len(out), '->', dest)
c = collections.Counter(r[0] for r in out)
print('files', len(c))
for k, v in c.most_common(25):
    print(f'  {v:4d} {k}')
cc = collections.Counter(r[2] for r in out)
print('casts:', cc.most_common(20))

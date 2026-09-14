"""Compare the decompiler's own xref signatures against the real declarations.

The decompiler left cross-reference comments all over the headers:

    // XREF: UI_RunMenuScript(int,int,char const * *,char const *)+5FE/r

Those carry the *original* parameter list, independently of whatever the
decompiler managed to recover for the function itself. Where the two disagree,
the declaration lost something -- a fused pair read as one __int64, or a dropped
trailing parameter.

Prints every function declared with fewer parameters than its xref signature,
widest gap first, and flags the ones whose declaration contains a 64-bit
parameter -- those are the fusions, and the ones that bite on LP64.
"""
import os
import re

import lp64env as env

SKIP = {'tracy', 'nvapi', 'PhysX', 'physx', 'bink', 'libs', 'zlib', 'jpeg',
        'DW', 'CubeMapGenLib', 'xenon', 'groupvoice', 'steam', 'mjpeg'}

# name(args) followed by IDA's +offset/rw or :$LN label
XREF = re.compile(r'\b([A-Za-z_]\w*)\(([^()]*(?:\([^()]*\)[^()]*)*)\)\s*(?:\+[0-9A-Fa-f]+|:\$LN)')
DECL = re.compile(r'^[A-Za-z_][\w \*]*?\b(?:__cdecl|__stdcall|__fastcall)?\s*\**'
                  r'([A-Za-z_]\w*)\s*\(([^;{]*)\)\s*;')
KEYWORDS = {'if', 'for', 'while', 'sizeof', 'switch', 'return'}


def headers():
    for dp, dns, fns in os.walk(env.repo('src')):
        dns[:] = [d for d in dns if d not in SKIP]
        for fn in fns:
            if fn.endswith('.h'):
                yield os.path.join(dp, fn)


def params(arglist):
    if arglist.strip() in ('', 'void'):
        return []
    return [a.strip() for a in arglist.split(',') if a.strip()]


original = {}
declared = {}
for path in headers():
    rel = os.path.relpath(path, env.REPO).replace(os.sep, '/')
    for ln, line in enumerate(open(path, encoding='utf-8', errors='replace'), 1):
        s = line.strip()
        if s.startswith('//'):
            for m in XREF.finditer(s):
                name, args = m.group(1), m.group(2).strip()
                if name in KEYWORDS or not args or args == 'void':
                    continue
                original.setdefault(name, set()).add(tuple(params(args)))
        else:
            m = DECL.match(s)
            if m:
                declared.setdefault(m.group(1), (params(m.group(2)), f'{rel}:{ln}'))

rows = []
for name, variants in original.items():
    if name not in declared:
        continue
    dps, where = declared[name]
    if not dps:                      # a 0-arg declaration is usually a stub, not a loss
        continue
    widest = max(variants, key=len)
    if len(widest) > len(dps):
        rows.append((len(widest) - len(dps), name, dps, list(widest), where))

rows.sort(key=lambda r: (-r[0], r[1]))
print(f'{len(original)} functions with xref signatures, {len(declared)} declarations')
print(f'{len(rows)} declared with fewer parameters than their xref\n')
for gap, name, dps, ops, where in rows:
    fused = any('__int64' in p or 'long long' in p for p in dps)
    print(f'-{gap}  {name}{"   <-- FUSED (64-bit param)" if fused else ""}')
    print(f'      decl ({len(dps)}): {", ".join(dps)}')
    print(f'      xref ({len(ops)}): {", ".join(ops)}')
    print(f'      {where}')

print('\nNote: a gap is a lead, not a verdict. Check that the xref actually names '
      'this function\n(same-name collisions happen) and that the body uses what '
      'the declaration dropped.')

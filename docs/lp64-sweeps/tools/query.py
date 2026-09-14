import dwarf, re, subprocess
import lp64env as env

L = dwarf.load(env.work('layouts_lp64.o'))
S = dwarf.load(env.work('layouts_ilp32.o'))
sl = dwarf.structs(L); ss = dwarf.structs(S)
cache = {}
def lv(st):
    if st not in cache:
        cache[st] = (list(dwarf.flatten(S, ss[st])), list(dwarf.flatten(L, sl[st])))
    return cache[st]

def member_off(st, path, abi):
    lv86, lv64 = lv(st)
    leaves = lv86 if abi == 32 else lv64
    for o, s, p, t in leaves:
        if p == path or p.startswith(path + '.') or p.startswith(path + '['):
            return o
    raise KeyError(f'{st}.{path}')

# validate ILP32 == x86 sizeof comments
hdr = subprocess.run(['grep', '-rhoE', r'^(struct|union) (__declspec\(align\([0-9]+\)\) )?[A-Za-z_0-9]+ // sizeof=0x[0-9A-Fa-f]+',
                      env.repo('src'), '--include=*.h'], capture_output=True, text=True).stdout
x86size = {}
for line in hdr.splitlines():
    m = re.match(r'^(?:struct|union) (?:__declspec\(align\(\d+\)\) )?(\w+) // sizeof=(0x[0-9A-Fa-f]+)', line)
    if m: x86size.setdefault(m.group(1), int(m.group(2), 16))

def check(st):
    a = dwarf.size(S, ss[st]); b = dwarf.size(L, sl[st])
    ok = x86size.get(st)
    return f'{st}: ilp32={a} x86comment={ok} lp64={b} {"OK" if ok == a else "!!"}'

def q(st, n, c86, c64, base=None, w86=None, w64=None):
    b86 = member_off(st, base, 32) if base else 0
    b64 = member_off(st, base, 64) if base else 0
    o86 = b86 + n * c86; o64 = b64 + n * c64
    lv86, lv64 = lv(st)
    a = dwarf.leaf_at(lv86, o86); b = dwarf.leaf_at(lv64, o64)
    def fmt(leaf, off, w):
        if not leaf: return f'@{off} fuori dalla struct'
        o, s, p, t = leaf
        rel = off - o
        return p + (f' (+{rel})' if rel else '') + (f' [w{w}/{s}]' if w and w != s else '') + f' @{off}'
    ok = a and b and a[2] == b[2] and (o86 - a[0]) == (o64 - b[0])
    return ('OK ' if ok else 'BAD'), fmt(a, o86, w86 or c86), fmt(b, o64, w64 or c64)

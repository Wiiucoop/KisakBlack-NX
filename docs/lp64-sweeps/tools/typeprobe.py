"""Stage A: learn the static type of every base expression with the compiler.

Each hit *((T *)base + N) is rewritten, in a scratch copy of its source file,
to *((sizeof(NxProbeT<decltype(base)>), 0), (T *)base + N). NxProbeT is never
defined, so the compiler reports the type on that exact line. Line numbers
are preserved. Output: types.tsv  file line col-index type
"""
import os, re, subprocess, sys, collections
from concurrent.futures import ThreadPoolExecutor
import lp64env as env

SCR = env.WORK
REPO = env.REPO
CXX = [env.CXX]
# -w would hide the very diagnostic this stage reads back; -g only slows it.
BASE = (env.flag('CXX_DEFINES') + env.flag('CXX_INCLUDES')
        + [f for f in env.flag('CXX_FLAGS') if f not in ('-w', '-g')])

PAT = re.compile(
    r'\*\s*\(\s*\(\s*(?:const\s+)?(?P<cast>[A-Za-z_][A-Za-z_0-9 ]*?)\s*(?P<stars>\*+)\s*\)\s*'
    r'(?P<base>&?\(?[A-Za-z_][A-Za-z_0-9]*(?:(?:->|\.)[A-Za-z_][A-Za-z_0-9]*|\[[^\]]*\])*\)?)'
    r'\s*\+\s*(?P<n>\d+|0x[0-9A-Fa-f]+)\s*\)')

rows = [l.rstrip('\n').split('\t') for l in open(os.path.join(SCR, 'hits_built.tsv'))]
files = sorted({r[0] for r in rows if r[0].endswith('.cpp')})

PRE = ('template<class T> struct NxProbeT;\n'
       '#define NXP(e) (sizeof(NxProbeT<decltype(e)>), 0)\n')
PREH = env.work('nxprobe_pre.h')
open(PREH, 'w').write(PRE)

def run(rel):
    src = os.path.join(REPO, rel)
    dst = env.work('probe_src', rel)
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    lines = open(src, encoding='utf-8', errors='replace').read().split('\n')
    for i, line in enumerate(lines):
        if '+' not in line or '*' not in line:
            continue
        def sub(m):
            return ('*((sizeof(NxProbeT<decltype(%s)>), 0), (%s%s)%s + %s)'
                    % (m.group('base'), m.group('cast'), m.group('stars'), m.group('base'), m.group('n')))
        if not line.strip().startswith('//'):
            lines[i] = PAT.sub(sub, line)
    open(dst, 'w', encoding='utf-8').write('\n'.join(lines))
    cmd = CXX + BASE + ['-include', PREH, '-iquote', os.path.dirname(src), '-fsyntax-only', '-fmax-errors=0',
                        '-fno-diagnostics-show-caret', '-x', 'c++', dst]
    p = subprocess.run(cmd, capture_output=True, text=True)
    return rel, p.stderr

res = {}
with ThreadPoolExecutor(8) as ex:
    for rel, err in ex.map(run, files):
        res[rel] = err

pat = re.compile(r'^(?P<f>.*probe_src[^:]*):(?P<l>\d+):(?P<c>\d+): error: invalid application of .sizeof. to incomplete type .NxProbeT<(?P<t>.*)>.\s*$')
out = []
other = collections.Counter()
for rel, err in res.items():
    for line in err.splitlines():
        m = pat.match(line)
        if m:
            out.append((rel, m.group('l'), m.group('c'), m.group('t')))
        elif ': error:' in line and 'probe_src' in line:
            other[rel] += 1
with open(env.work('types.tsv'), 'w') as f:
    for r in out:
        f.write('\t'.join(r) + '\n')
print('type reports', len(out), 'for', len(rows), 'hits')
print('files with other errors:', dict(other.most_common(15)))

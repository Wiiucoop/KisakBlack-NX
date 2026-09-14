"""For every literal-size qsort site, show the nearest declaration of its base.

That declaration names the element type, which is what decides whether the
literal is still correct on LP64: a type with no pointer keeps its x86 size,
anything else has to become sizeof(T). Run qsort_scan.py first.
"""
import io, os, re
import lp64env as env

rows = [l.rstrip('\n').split('\t') for l in open(env.work('qsort.tsv'))]
sites = [r for r in rows if not r[4].startswith('sizeof')]

for f, ln, base, count, size, cmp_ in sites:
    ln = int(ln)
    s = io.open(os.path.join(env.REPO, f), encoding='utf-8', errors='replace').read().split('\n')
    name = base.split('->')[-1].split('.')[-1].split('[')[0].lstrip('&')
    decl = ''
    for i in range(min(ln - 1, len(s) - 1), max(0, ln - 260), -1):
        if re.search(r'[A-Za-z_]\w*\s+\**' + re.escape(name) + r'\b', s[i]):
            decl = s[i].strip()
            break
    print('%-46s:%-6d size=%-8s %-34s | %s' % (f, ln, size, name, decl[:90]))

"""Census of qsort() calls whose element size is a literal instead of sizeof().

The decompiler turned every `sizeof(T)` argument into the x86 constant it
evaluated to. Those constants are correct only while T has no pointer in it,
so each one is a candidate LP64 defect: qsort strides by the x86 size over an
array laid out at the LP64 size.

Writes qsort.tsv (file, line, base, count, size, cmp) and prints the calls
whose size argument is not a sizeof() expression.
"""
import io, os, re, collections
import lp64env as env

rows = []
for root, _, fs in os.walk(env.repo('src')):
    for fn in fs:
        if not fn.endswith(('.cpp', '.h')):
            continue
        p = os.path.join(root, fn)
        s = io.open(p, encoding='utf-8', errors='replace').read()
        for m in re.finditer(r'\bqsort\s*\(', s):
            i = m.end()
            depth, j = 1, i
            while j < len(s) and depth:
                if s[j] == '(':
                    depth += 1
                elif s[j] == ')':
                    depth -= 1
                j += 1
            call = s[i:j - 1]
            parts, depth, cur = [], 0, ''
            for ch in call:
                if ch == '(':
                    depth += 1
                if ch == ')':
                    depth -= 1
                if ch == ',' and depth == 0:
                    parts.append(cur)
                    cur = ''
                else:
                    cur += ch
            parts.append(cur)
            parts = [' '.join(x.split()) for x in parts]
            line = s[:m.start()].count('\n') + 1
            if len(parts) >= 3:
                rel = os.path.relpath(p, env.REPO).replace(os.sep, '/')
                rows.append((rel, line, parts[0], parts[1], parts[2],
                             parts[3] if len(parts) > 3 else ''))

dest = env.work('qsort.tsv')
with open(dest, 'w') as f:
    for r in rows:
        f.write('\t'.join(str(x) for x in r) + '\n')

const = [r for r in rows if not r[4].startswith('sizeof')]
print('total qsort:', len(rows), ' with non-sizeof size:', len(const), '->', dest)
print()
for r in sorted(const):
    print('%s:%d  size=%-8s base=%-46s cmp=%s' % (r[0], r[1], r[4], r[2][:46], r[5][:50]))

"""Stage 0: list the translation units the Switch build actually compiles.

The scan walks all of src/, but plenty of it is dead on this target. Anything
with no object file under the CMake target directory never reaches the binary,
so its hits are noise. Writes built.txt: one path per line, no extension.
"""
import os
import lp64env as env

objdir = env.repo('build-nx/CMakeFiles/KisakBlack.dir')
out = []
for dp, _, fns in os.walk(objdir):
    for fn in fns:
        if not fn.endswith('.o'):
            continue
        rel = os.path.relpath(os.path.join(dp, fn), objdir).replace(os.sep, '/')
        rel = rel[:-2]
        if rel.startswith('src/'):
            out.append(rel)

path = env.work('built.txt')
with open(path, 'w') as f:
    f.write('\n'.join(sorted(out)) + '\n')
print('built TUs under src/:', len(out), '->', path)

# Drop hits in TUs that never reach the binary.
keep = set(out)
rows = list(open(env.work('hits.tsv')))
kept = [l for l in rows if l.split('\t', 1)[0].rsplit('.', 1)[0] in keep]
dst = env.work('hits_built.tsv')
with open(dst, 'w') as f:
    f.writelines(kept)
print(f'hits {len(rows)} -> {len(kept)} in built TUs ->', dst)

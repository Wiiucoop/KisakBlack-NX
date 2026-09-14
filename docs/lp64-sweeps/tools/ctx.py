"""For each file:line site, show where the base variable gets its value."""
import os, re, sys
import lp64env as env
sites = [s.split('@') for s in sys.argv[1:]]   # file:line@var
for spec in sites:
    fl, var = spec[0], spec[1]
    f, ln = fl.rsplit(':', 1); ln = int(ln)
    lines = open(REPO + f, encoding='utf-8', errors='replace').read().split('\n')
    root = re.escape(var.split('->')[0].split('.')[0].lstrip('&'))
    # function start: last line at column 0 that looks like a definition
    start = ln - 1
    while start > 0 and not re.match(r'^[A-Za-z_].*\(.*', lines[start]):
        start -= 1
    print(f'== {f}:{ln}  [{var}]  fn: {lines[start].strip()[:140]}')
    shown = 0
    for i in range(start, ln):
        if re.search(r'\b' + root + r'\b', lines[i]) and (
                re.search(r'\b' + root + r'\s*=[^=]', lines[i]) or re.search(r'[\*&]\s*' + root + r'\s*[;,)\[]', lines[i])
                or re.search(r'\b' + root + r';\s*//', lines[i])):
            print(f'   {i+1}: {lines[i].strip()[:170]}')
            shown += 1
            if shown > 6:
                break
    print(f'  >{ln}: {lines[ln-1].strip()[:170]}')

"""Flatten struct layouts from readelf's DWARF dump into leaf fields."""
import re, subprocess
import lp64env as env

READELF = env.READELF

def load(obj):
    txt = subprocess.run([READELF, '--debug-dump=info', obj], capture_output=True, text=True).stdout
    dies, stack = {}, []
    cur = None
    for line in txt.splitlines():
        m = re.match(r'\s*<(\d+)><([0-9a-f]+)>: Abbrev Number: (\d+)(?: \((DW_TAG_\w+)\))?', line)
        if m:
            depth, off, abbrev, tag = int(m.group(1)), int(m.group(2), 16), int(m.group(3)), m.group(4)
            if abbrev == 0:
                continue
            cur = {'tag': tag, 'off': off, 'children': [], 'attr': {}}
            dies[off] = cur
            while len(stack) > depth:
                stack.pop()
            if stack:
                stack[-1]['children'].append(cur)
            stack.append(cur)
            continue
        m = re.match(r'\s*<[0-9a-f]+>\s+(DW_AT_\w+)\s*:\s*(.*)$', line)
        if m and cur is not None:
            k, v = m.group(1), m.group(2).strip()
            if k in ('DW_AT_name',):
                v = v.split(': ')[-1] if 'indirect string' in v else v
            cur['attr'][k] = v
    return dies

def ref(v):
    m = re.search(r'<0x([0-9a-f]+)>', v)
    return int(m.group(1), 16) if m else None

def strip(dies, d):
    while d and d['tag'] in ('DW_TAG_typedef', 'DW_TAG_const_type', 'DW_TAG_volatile_type'):
        t = d['attr'].get('DW_AT_type')
        d = dies.get(ref(t)) if t else None
    return d

def size(dies, d):
    d = strip(dies, d)
    if d is None:
        return 0
    if 'DW_AT_byte_size' in d['attr']:
        return int(d['attr']['DW_AT_byte_size'], 0)
    if d['tag'] == 'DW_TAG_array_type':
        n = 1
        for c in d['children']:
            if c['tag'] == 'DW_TAG_subrange_type':
                ub = c['attr'].get('DW_AT_upper_bound')
                cnt = c['attr'].get('DW_AT_count')
                n *= (int(ub, 0) + 1) if ub else (int(cnt, 0) if cnt else 0)
        return n * size(dies, dies.get(ref(d['attr']['DW_AT_type'])))
    return 0

def typename(dies, d):
    d0 = d
    d = strip(dies, d)
    if d is None:
        return 'void'
    if d['tag'] == 'DW_TAG_pointer_type':
        t = d['attr'].get('DW_AT_type')
        return (typename(dies, dies.get(ref(t))) if t else 'void') + '*'
    return d['attr'].get('DW_AT_name', d['tag'].replace('DW_TAG_', ''))

def flatten(dies, d, base=0, path=''):
    """Yield (offset, size, path, typename) leaves."""
    d = strip(dies, d)
    if d is None:
        return
    if d['tag'] in ('DW_TAG_structure_type', 'DW_TAG_class_type'):
        for c in d['children']:
            if c['tag'] != 'DW_TAG_member':
                continue
            loc = c['attr'].get('DW_AT_data_member_location', '0')
            o = int(re.findall(r'-?\d+', loc)[0]) if re.findall(r'-?\d+', loc) else 0
            t = dies.get(ref(c['attr']['DW_AT_type']))
            nm = c['attr'].get('DW_AT_name', '?')
            yield from flatten(dies, t, base + o, (path + '.' if path else '') + nm)
    elif d['tag'] == 'DW_TAG_union_type':
        # a union is one leaf covering its whole size; members listed by name
        names = [c['attr'].get('DW_AT_name', '?') for c in d['children'] if c['tag'] == 'DW_TAG_member']
        yield (base, size(dies, d), path + '{' + '|'.join(names[:4]) + '}', 'union')
    elif d['tag'] == 'DW_TAG_array_type':
        et = dies.get(ref(d['attr']['DW_AT_type']))
        es = size(dies, et)
        total = size(dies, d)
        n = total // es if es else 0
        ets = strip(dies, et)
        if ets and ets['tag'] in ('DW_TAG_structure_type', 'DW_TAG_class_type', 'DW_TAG_union_type') and n <= 64:
            for i in range(n):
                yield from flatten(dies, et, base + i * es, f'{path}[{i}]')
        else:
            yield (base, total, f'{path}[{n}]', typename(dies, et) + f' x{es}')
    else:
        yield (base, size(dies, d), path, typename(dies, d))

def structs(dies):
    out = {}
    for d in dies.values():
        if d['tag'] in ('DW_TAG_structure_type', 'DW_TAG_class_type', 'DW_TAG_union_type') \
                and 'DW_AT_name' in d['attr'] and 'DW_AT_declaration' not in d['attr'] and d['children']:
            out.setdefault(d['attr']['DW_AT_name'], d)
    return out

def leaf_at(leaves, off):
    for o, s, p, t in leaves:
        if o <= off < o + max(s, 1):
            return (o, s, p, t)
    return None

// layoutgen.cpp -- builds layout_gen.h, the x86 -> LP64 field maps convert.cpp
// uses for the big map assets. Host tool (msys2 UCRT64 g++).
//
//   layoutgen probe <structs.txt> <probe.cpp> <members.tsv> <header|@list>...
//       Parse the named structs out of the engine headers and write a probe
//       source with an offsetof/sizeof constant per member, plus the member
//       list the emit step needs.
//   layoutgen emit <members.tsv> <lp64.txt> <ilp32.txt> <layout_gen.h>
//       Join the probe's values under both ABIs (see layout.sh) into, per
//       struct: its two sizes, an X_/L_ constant pair for every pointer (also
//       those nested in member structs), and the spans of pointer-free bytes
//       that copy across unchanged.
//
// The ILP32 compile stands in for the MSVC x86 build that wrote the
// fastfiles. A member that is not a pointer but differs in size between the
// two is an error: it holds a pointer the parser cannot see (a union of
// pointers, say) and needs listing in structs.txt as "opaque" so the
// transcoder deals with it by hand.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct Member {
    std::string name;       // declarator name
    std::string type;       // base type identifier, for recursion
    bool ptr = false;
};
struct StructDef {
    std::string name;
    std::vector<Member> members;
};

static std::string readAll(const char *path) {
    std::ifstream f(path, std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static std::string stripComments(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '/' && i + 1 < s.size() && s[i + 1] == '/') {
            while (i < s.size() && s[i] != '\n') ++i;
            out.push_back('\n');
        } else if (s[i] == '/' && i + 1 < s.size() && s[i + 1] == '*') {
            i += 2;
            while (i + 1 < s.size() && !(s[i] == '*' && s[i + 1] == '/')) ++i;
            ++i;
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

static std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// One member statement (text before its ';') -> Member. False for things
// that are not data members: methods, constructors, static members.
static bool parseMember(std::string st, Member &m) {
    st = trim(st);
    if (st.empty()) return false;
    if (st.rfind("static ", 0) == 0 || st.rfind("using ", 0) == 0 ||
        st.rfind("typedef ", 0) == 0 || st.rfind("friend ", 0) == 0)
        return false;
    bool fnPtr = st.find("(*") != std::string::npos || st.find("( *") != std::string::npos;
    if (st.find('(') != std::string::npos && !fnPtr) return false;   // method
    if (st.find(':') != std::string::npos && st.find("::") == std::string::npos)
        return false;                                                // bitfield: blob types only
    if (st.find(',') != std::string::npos) {
        fprintf(stderr, "layoutgen: multiple declarators not handled: %s\n", st.c_str());
        exit(2);
    }
    // Drop array dimensions from the end.
    std::string d = st;
    while (!d.empty() && d.back() == ']') {
        size_t o = d.rfind('[');
        d = trim(d.substr(0, o));
    }
    if (fnPtr) {
        // float (*hullPoints)[2]  ->  name inside the parentheses
        std::smatch mm;
        static const std::regex re(R"(\(\s*\*\s*(\w+)\s*\))");
        if (!std::regex_search(st, mm, re)) return false;
        m.name = mm[1];
        m.ptr = true;
        m.type = "";
        return true;
    }
    size_t e = d.size();
    size_t b = e;
    while (b > 0 && (isalnum((unsigned char)d[b - 1]) || d[b - 1] == '_')) --b;
    if (b == e) return false;
    m.name = d.substr(b, e - b);
    std::string type = trim(d.substr(0, b));
    m.ptr = type.find('*') != std::string::npos || type.find('&') != std::string::npos;
    // Base type: last identifier of the type text, ignoring qualifiers.
    std::string t = type;
    for (char &c : t) if (c == '*' || c == '&') c = ' ';
    std::istringstream is(t);
    std::string w, last;
    while (is >> w) {
        if (w == "const" || w == "volatile" || w == "struct" || w == "union" ||
            w == "enum" || w == "unsigned" || w == "signed")
            continue;
        last = w;
    }
    size_t cc = last.rfind("::");
    m.type = cc == std::string::npos ? last : last.substr(cc + 2);
    return !m.name.empty();
}
// Parse one aggregate body (the text between its braces) into data members.
// Members of an anonymous struct or union are the enclosing type's own in
// C++ (ent->index reaches into gentity_s's anonymous union), so they are
// parsed in place; a named nested aggregate is one blob member; a method's
// body is skipped with it.
static void parseBody(const std::string &body, const std::string &name, StructDef &out) {
    int depth = 0;
    std::string cur, nested;
    for (size_t i = 0; i < body.size(); ++i) {
        char c = body[i];
        if (c == '{') {
            if (depth == 0) nested.clear();
            else nested.push_back(c);
            ++depth;
            continue;
        }
        if (c == '}') {
            --depth;
            if (depth > 0) { nested.push_back(c); continue; }
            if (cur.find('(') != std::string::npos) { cur.clear(); nested.clear(); continue; }  // method
            cur += " __NESTED__ ";
            continue;
        }
        if (depth > 0) { nested.push_back(c); continue; }
        if (c == ';') {
            std::string st = trim(cur);
            cur.clear();
            if (st.find("__NESTED__") != std::string::npos) {
                std::string tail = trim(st.substr(st.find("__NESTED__") + 10));
                if (!tail.empty()) {
                    Member m;
                    m.name = tail;
                    out.members.push_back(m);
                } else {
                    parseBody(nested, name, out);   // anonymous: its members are ours
                }
                nested.clear();
                continue;
            }
            Member m;
            if (parseMember(st, m)) out.members.push_back(m);
            continue;
        }
        if (c == ':' && i + 1 < body.size() && body[i + 1] != ':' && i > 0 && body[i - 1] != ':') {
            std::string w = trim(cur);   // access specifier ("public:")
            if (w == "public" || w == "private" || w == "protected") { cur.clear(); continue; }
        }
        cur.push_back(c);
    }
}

// Find `struct|union [__declspec(...)] Name` followed by a body, and parse
// the body's data members.
static bool findStruct(const std::string &src, const std::string &name, StructDef &out) {
    std::regex re("(struct|union)\\s+(__declspec\\s*\\(\\s*align\\s*\\(\\s*\\d+\\s*\\)\\s*\\)\\s+)?" +
                  name + "\\b[^;{()]*\\{");
    std::smatch mm;
    if (!std::regex_search(src.begin(), src.end(), mm, re)) return false;
    size_t pos = (size_t)(mm[0].second - src.begin());   // just past '{'
    int depth = 1;
    size_t end = pos;
    for (; end < src.size() && depth > 0; ++end) {
        if (src[end] == '{') ++depth;
        else if (src[end] == '}') --depth;
    }
    out.name = name;
    parseBody(src.substr(pos, end - 1 - pos), name, out);
    return true;
}

static std::string mangle(const std::string &s) {
    std::string o;
    for (char c : s) o.push_back(isalnum((unsigned char)c) ? c : '_');
    return o;
}

static int cmdProbe(int argc, char **argv) {
    // argv: structs.txt probe.cpp members.tsv headers...
    std::vector<std::string> names;
    std::set<std::string> opaque;
    std::vector<std::string> includes;
    {
        std::ifstream f(argv[0]);
        std::string line;
        while (std::getline(f, line)) {
            line = trim(line.substr(0, line.find('#')));
            if (line.empty()) continue;
            std::istringstream is(line);
            std::string a, b;
            is >> a >> b;
            if (a == "include") includes.push_back(b);
            else if (a == "opaque") opaque.insert(b);
            else names.push_back(a);
        }
    }
    std::vector<std::string> paths;
    for (int i = 3; i < argc; ++i) {
        if (argv[i][0] != '@') { paths.push_back(argv[i]); continue; }
        std::ifstream f(argv[i] + 1);   // @file: one header path per line
        std::string line;
        while (std::getline(f, line)) if (!trim(line).empty()) paths.push_back(trim(line));
    }
    std::vector<std::string> srcs;
    for (const std::string &path : paths) {
        std::string s = readAll(path.c_str());
        bool wanted = false;   // std::regex is slow; only keep headers that could match
        for (auto &n : names) if (s.find(n) != std::string::npos) { wanted = true; break; }
        if (wanted) srcs.push_back(stripComments(s));
    }

    std::ofstream probe(argv[1]), tsv(argv[2]);
    probe << "// Generated by layoutgen probe -- do not edit.\n#include <stddef.h>\n";
    for (auto &inc : includes) probe << "#include <" << inc << ">\n";
    probe << "#define K(n, v) extern \"C\" { extern const int layout_##n; const int layout_##n = (int)(v); }\n";

    for (const std::string &n : names) {
        StructDef sd;
        bool found = false;
        for (auto &s : srcs) if (findStruct(s, n, sd)) { found = true; break; }
        if (!found) { fprintf(stderr, "layoutgen: struct %s not found\n", n.c_str()); return 2; }
        std::string mn = mangle(n);
        probe << "K(" << mn << "____size, sizeof(" << n << "))\n";
        tsv << "S\t" << n << "\n";
        for (const Member &m : sd.members) {
            std::string k = mn + "__" + m.name;
            probe << "K(" << k << "____off, offsetof(" << n << ", " << m.name << "))\n";
            probe << "K(" << k << "____size, sizeof(((" << n << " *)0)->" << m.name << "))\n";
            const char *kind = m.ptr ? "P" : (opaque.count(n + "." + m.name) ? "O" : "M");
            tsv << kind << "\t" << m.name << "\t" << (m.type.empty() ? "-" : m.type) << "\n";
        }
    }
    return 0;
}

// ---- emit ----
struct Val { long lp = -1, il = -1; };

static std::map<std::string, Val> readVals(const char *lp, const char *il) {
    std::map<std::string, Val> v;
    for (int pass = 0; pass < 2; ++pass) {
        std::ifstream f(pass ? il : lp);
        std::string k;
        long x;
        while (f >> k >> x) (pass ? v[k].il : v[k].lp) = x;
    }
    return v;
}

struct SMembers { std::string name; std::vector<std::pair<std::string, Member>> m; };  // kind, member

struct Span { long x, l, n; };
struct Ptr { std::string path; long x, l; };

static std::map<std::string, SMembers> g_structs;
static std::map<std::string, Val> g_v;

static Val get(const std::string &k) {
    auto it = g_v.find(k);
    if (it == g_v.end() || it->second.lp < 0 || it->second.il < 0) {
        fprintf(stderr, "layoutgen: no value for %s\n", k.c_str());
        exit(2);
    }
    return it->second;
}

struct Opaque { std::string path; long x, l, xn, ln; };

static void flatten(const std::string &sname, long bx, long bl, const std::string &path,
                    bool first, std::vector<Span> &spans, std::vector<Ptr> &ptrs,
                    std::vector<Opaque> &opaques) {
    const SMembers &s = g_structs[sname];
    std::string mn = mangle(sname);
    for (auto &km : s.m) {
        const std::string &kind = km.first;
        const Member &m = km.second;
        Val off = get(mn + "__" + m.name + "____off");
        Val sz = get(mn + "__" + m.name + "____size");
        std::string p = path.empty() ? m.name : path + "__" + m.name;
        if (kind == "P") {
            if (sz.il % 4 || sz.lp % 8 || sz.il / 4 != sz.lp / 8) {
                fprintf(stderr, "layoutgen: %s.%s pointer sizes %ld/%ld\n", sname.c_str(),
                        m.name.c_str(), sz.il, sz.lp);
                exit(2);
            }
            if (first) ptrs.push_back({p, bx + off.il, bl + off.lp});
            continue;
        }
        if (kind == "O") {
            if (first) opaques.push_back({p, bx + off.il, bl + off.lp, sz.il, sz.lp});
            continue;
        }
        auto sub = g_structs.find(m.type);
        if (sub != g_structs.end() && m.type != sname) {
            Val tsz = get(mangle(m.type) + "____size");
            long n = sz.il / tsz.il;
            if (n * tsz.il != sz.il || n * tsz.lp != sz.lp) {
                fprintf(stderr, "layoutgen: %s.%s is not a whole number of %s\n",
                        sname.c_str(), m.name.c_str(), m.type.c_str());
                exit(2);
            }
            for (long i = 0; i < n; ++i)
                flatten(m.type, bx + off.il + i * tsz.il, bl + off.lp + i * tsz.lp, p,
                        first && i == 0, spans, ptrs, opaques);
            continue;
        }
        if (sz.il != sz.lp) {
            // LAYOUTGEN_LOOSE: an offset dump (layout.sh with STRUCTS/OUT), not
            // a converter map -- name the member and move on.
            if (getenv("LAYOUTGEN_LOOSE")) {
                if (first) ptrs.push_back({p, bx + off.il, bl + off.lp});
                continue;
            }
            fprintf(stderr, "layoutgen: %s.%s (%s) is %ld bytes on x86 but %ld on LP64 --"
                    " it hides a pointer; add its type to structs.txt or mark it opaque\n",
                    sname.c_str(), m.name.c_str(), m.type.c_str(), sz.il, sz.lp);
            exit(2);
        }
        if (first) ptrs.push_back({p, bx + off.il, bl + off.lp});   // named, not a pointer
        spans.push_back({bx + off.il, bl + off.lp, sz.il});
    }
}

static int cmdEmit(char **argv) {
    // members.tsv lp64.txt ilp32.txt out.h
    {
        std::ifstream f(argv[0]);
        std::string line, cur;
        while (std::getline(f, line)) {
            std::istringstream is(line);
            std::string kind, name, type;
            std::getline(is, kind, '\t');
            std::getline(is, name, '\t');
            std::getline(is, type, '\t');
            if (kind == "S") { cur = name; g_structs[cur].name = cur; continue; }
            Member m;
            m.name = name;
            m.type = type == "-" ? "" : type;
            m.ptr = kind == "P";
            g_structs[cur].m.push_back({kind, m});
        }
    }
    g_v = readVals(argv[1], argv[2]);

    // Keep the structs.txt order: members.tsv lists them in it.
    std::vector<std::string> order;
    {
        std::ifstream f(argv[0]);
        std::string line;
        while (std::getline(f, line))
            if (line.rfind("S\t", 0) == 0) order.push_back(line.substr(2));
    }

    FILE *o = fopen(argv[3], "wb");
    fprintf(o, "// layout_gen.h -- generated by tools/ffconv/layout/layout.sh. Do not edit.\n"
               "//\n"
               "// Per struct: X_ (x86, as the fastfiles were written) and L_ (LP64, as the\n"
               "// Switch build lays it out) sizes and member offsets, and the spans of\n"
               "// pointer-free bytes that copy across unchanged. Members nested in member\n"
               "// structs are named by path; in arrays of structs, the first element's.\n"
               "#pragma once\n#include <cstdint>\n\n"
               "struct LayoutSpan { uint32_t x, l, n; };\n\n");
    for (const std::string &sname : order) {
        std::vector<Span> spans;
        std::vector<Ptr> ptrs;
        std::vector<Opaque> opaques;
        flatten(sname, 0, 0, "", true, spans, ptrs, opaques);
        // merge contiguous spans
        std::vector<Span> merged;
        for (const Span &s : spans) {
            if (!merged.empty() && merged.back().x + merged.back().n == s.x &&
                merged.back().l + merged.back().n == s.l)
                merged.back().n += s.n;
            else
                merged.push_back(s);
        }
        Val sz = get(mangle(sname) + "____size");
        std::string mn = mangle(sname);
        fprintf(o, "// %s\nenum : uint32_t { X_sizeof_%s = %ld, L_sizeof_%s = %ld };\n",
                sname.c_str(), mn.c_str(), sz.il, mn.c_str(), sz.lp);
        for (const Ptr &p : ptrs)
            fprintf(o, "enum : uint32_t { X_%s__%s = %ld, L_%s__%s = %ld };\n", mn.c_str(),
                    p.path.c_str(), p.x, mn.c_str(), p.path.c_str(), p.l);
        // Opaque members: not copied by the spans; the transcoder handles them.
        for (const Opaque &p : opaques)
            fprintf(o, "enum : uint32_t { X_%s__%s = %ld, L_%s__%s = %ld, XN_%s__%s = %ld, "
                       "LN_%s__%s = %ld };\n",
                    mn.c_str(), p.path.c_str(), p.x, mn.c_str(), p.path.c_str(), p.l,
                    mn.c_str(), p.path.c_str(), p.xn, mn.c_str(), p.path.c_str(), p.ln);
        fprintf(o, "static const LayoutSpan SPANS_%s[] = {", mn.c_str());
        for (size_t i = 0; i < merged.size(); ++i)
            fprintf(o, "%s{%ld,%ld,%ld}", i ? "," : "", merged[i].x, merged[i].l, merged[i].n);
        if (merged.empty()) fprintf(o, "{0,0,0}");
        fprintf(o, "};\n\n");
    }
    fclose(o);
    return 0;
}

int main(int argc, char **argv) {
    if (argc >= 5 && !strcmp(argv[1], "probe")) return cmdProbe(argc - 2, argv + 2);
    if (argc == 6 && !strcmp(argv[1], "emit")) return cmdEmit(argv + 2);
    fprintf(stderr, "usage: layoutgen probe <structs.txt> <probe.cpp> <members.tsv> <header>...\n"
                    "       layoutgen emit <members.tsv> <lp64.txt> <ilp32.txt> <layout_gen.h>\n");
    return 1;
}

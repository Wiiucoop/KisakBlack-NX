// nx_d3d9_shader.cpp -- D3D9 shader-model 3 bytecode to GLSL ES 3.00.
//
// Adapted from riicchhaarrd/KisakBlack, branch web-port, src/gfx_gl/gl_shader.cpp
// (TranslateD3D9Shader and the attribute mapping), GPL-3.0 like this tree. That
// translator has been run against this engine's real shaders through WebGL2 --
// its comments record the bugs the maps exposed (relative addressing for bone
// matrices, texld flavours, dcl dimensions, ddx/ddy, alpha test) -- so the
// instruction semantics are kept as they are there. What changed for this
// backend:
//
//   * GLSL ES 3.00 only. The desktop #version 120 path is gone: this is a core
//     4.3 context, which accepts ES 3.00 and not 1.20.
//   * The web build's batching variants (lightmap and material texture arrays,
//     instanced matrices, the ?dumpenv disassembler, env-var switches) are gone.
//   * The vertex epilogue adds, after the existing D3D9 -> GL depth fix, D3D9's
//     half-pixel offset (nxHalfPixel) and the y flip nx_d3d9_null.cpp stores
//     every render target with -- see "Render targets" there.
//   * vPos is gl_FragCoord less half a pixel: D3D9 puts pixel centres on whole
//     coordinates, GL on halves.
//   * Vertex-shader samplers are named svN, not sN, so that a vertex and a
//     pixel shader both declaring s0 do not collide in one program -- they are
//     different D3D9 slots (D3DVERTEXTEXTURESAMPLER0 + N versus N).
//   * A destination the translator cannot name (oC1+, oDepth, an undeclared
//     output) writes to a local instead of to the rvalue vec4(0.0), which was a
//     compile error; oDepth reaches gl_FragDepth.
//   * Skipped opcodes are counted and reported instead of vanishing.
#include "nx_d3d9_shader.h"

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

// --- DX9 bytecode constants (documented format) ---
enum {  // opcodes (token & 0xFFFF) -- D3DSIO_*
    OP_MOV = 1, OP_ADD = 2, OP_SUB = 3, OP_MAD = 4, OP_MUL = 5, OP_RCP = 6, OP_RSQ = 7,
    OP_DP3 = 8, OP_DP4 = 9, OP_MIN = 10, OP_MAX = 11, OP_SLT = 12, OP_SGE = 13,
    OP_EXP = 14, OP_LOG = 15, OP_LRP = 18, OP_FRC = 19,
    OP_CALL = 25, OP_CALLNZ = 26, OP_LOOP = 27, OP_RET = 28, OP_ENDLOOP = 29, OP_LABEL = 30,
    OP_POW = 32, OP_ABS = 35, OP_NRM = 36, OP_SINCOS = 37,
    OP_REP = 38, OP_ENDREP = 39, OP_IF = 40, OP_IFC = 41, OP_ELSE = 42, OP_ENDIF = 43,
    OP_BREAK = 44, OP_BREAKC = 45, OP_MOVA = 46, OP_DEFB = 47, OP_DEFI = 48,
    OP_TEXKILL = 65, OP_TEXLD = 66, OP_CMP = 88, OP_DP2ADD = 90, OP_SETP = 94,
    OP_DSX = 91, OP_DSY = 92, OP_TEXLDD = 93, OP_TEXLDL = 95,
    OP_DCL = 31, OP_DEF = 81, OP_COMMENT = 0xFFFE, OP_END = 0xFFFF,
};
enum {  // register types
    RT_TEMP = 0, RT_INPUT = 1, RT_CONST = 2, RT_TEXTURE = 3, RT_RASTOUT = 4,
    RT_ATTROUT = 5, RT_OUTPUT = 6, RT_COLOROUT = 8, RT_DEPTHOUT = 9, RT_SAMPLER = 10,
    RT_MISCTYPE = 17,   // ps_3.0 vPos (reg 0 = pixel position) / vFace (reg 1)
};

struct Operand { int type, reg, swizzle, writemask, mod, dmod;
                 bool rel = false; int relComp = 0; };  // c[a0.<relComp> + reg]

Operand decodeParam(DWORD t) {
    Operand o;
    o.reg       = (int)(t & 0x7FF);
    o.type      = (int)(((t >> 28) & 0x7) | ((t & 0x1800) >> 8));
    o.writemask = (int)((t >> 16) & 0xF);
    o.swizzle   = (int)((t >> 16) & 0xFF);
    o.mod       = (int)((t >> 24) & 0xF);   // source modifier (D3DSPSM_*)
    o.dmod      = (int)((t >> 20) & 0xF);   // dest modifier (_sat = bit 0)
    return o;
}

// Decode a parameter that may use RELATIVE ADDRESSING (bit 13): `c[a0.x + N]`, the
// skinned-mesh bone-matrix indexing pattern. In sm2.0+ the relative flag adds an
// EXTRA address token after the parameter; treating it as the next source operand
// reads bones from garbage registers. Returns tokens used.
int decodeParamRel(const DWORD *tok, Operand &o) {
    o = decodeParam(tok[0]);
    if (tok[0] & 0x2000) {                    // D3DSHADER_ADDRMODE_RELATIVE
        o.rel = true;
        Operand a = decodeParam(tok[1]);      // address token: a0 (or aL) + swizzle
        o.relComp = a.swizzle & 0x3;          // .x/.y/.z/.w of the address register
        return 2;
    }
    return 1;
}

const char *kComp = "xyzw";

std::string maskStr(int m) {  // dest write-mask, e.g. 0b0111 -> ".xyz"
    if (m == 0xF || m == 0) return "";
    std::string s = ".";
    for (int i = 0; i < 4; ++i) if (m & (1 << i)) s += kComp[i];
    return s;
}

std::string swizStr(int sw) {  // src swizzle as a 4-component selector, "" if identity
    if (sw == 0xE4) return "";  // .xyzw
    std::string s = ".";
    for (int i = 0; i < 4; ++i) s += kComp[(sw >> (i * 2)) & 0x3];
    return s;
}

struct Ctx {
    bool isPixel = false;
    const char *cArr() const { return isPixel ? "psc" : "vsc"; }
    std::map<int, std::pair<int,int>> inputs;   // reg -> (usage, usageIndex)
    std::map<int, std::pair<int,int>> outputs;  // reg -> (usage, usageIndex)  (vertex)
    std::set<int> samplers;
    std::map<int, int> samplerDim;   // sampler reg -> D3DSTT: 2=2D, 3=CUBE, 4=VOLUME
    std::map<int, float[4]> defs;
    std::map<int, int> idefs;                   // DEFI integer constant reg -> count (.x)
    std::set<int> usedTemps;
    bool usedConst = false;
    bool usedA0    = false;   // MOVA / relative addressing: declare ivec4 a0
    bool usedSink  = false;   // a destination with no GLSL name: write nxSink
    bool usedDepth = false;   // oDepth: nxDepth.x goes to gl_FragDepth
    int  maxConst  = -1;      // highest runtime-constant register referenced (sizes vsc[]/psc[])
    unsigned unknownOps = 0, firstUnknownOp = 0;
};

// D3DDECLUSAGE -> a stable varying name shared between the vs and ps stages.
std::string varyingName(int usage, int index) {
    static const char *u[] = {"POSITION","BLENDW","BLENDI","NORMAL","PSIZE","TEXCOORD",
                              "TANGENT","BINORMAL","TESSFACTOR","POSITIONT","COLOR","FOG",
                              "DEPTH","SAMPLE"};
    std::ostringstream s;
    s << "v_" << (usage >= 0 && usage < 14 ? u[usage] : "X") << index;
    return s.str();
}

std::string attribName(int usage, int index) {
    char buf[32];
    NX_ShaderAttribName(usage, index, buf, sizeof(buf));
    return buf;
}

std::string regName(Ctx &c, const Operand &o, bool isDest) {
    switch (o.type) {
        case RT_TEMP:    c.usedTemps.insert(o.reg); { std::ostringstream s; s << "r" << o.reg; return s.str(); }
        case RT_CONST:
            if (o.rel) {
                // c[a0.<comp> + N]: the runtime index can reach any register, so the
                // array must span all 256 (clamped -- OOB indexing is UB in GLSL).
                c.usedConst = true; c.usedA0 = true; c.maxConst = 255;
                std::ostringstream s;
                s << c.cArr() << "[clamp(a0." << kComp[o.relComp] << " + " << o.reg << ", 0, 255)]";
                return s.str();
            }
            if (c.defs.count(o.reg)) { std::ostringstream s; s << "c" << o.reg << "_def"; return s.str(); }
            c.usedConst = true; if (o.reg > c.maxConst) c.maxConst = o.reg;
            { std::ostringstream s; s << c.cArr() << "[" << o.reg << "]"; return s.str(); }
        case RT_INPUT:
            if (c.isPixel) { auto it = c.inputs.find(o.reg);
                             return it != c.inputs.end() ? varyingName(it->second.first, it->second.second) : "vec4(0.0)"; }
            else { auto it = c.inputs.find(o.reg);
                   return it != c.inputs.end() ? attribName(it->second.first, it->second.second) : "aPos"; }
        case RT_OUTPUT: {
            auto it = c.outputs.find(o.reg);
            if (it != c.outputs.end()) {
                if (it->second.first == D3DDECLUSAGE_POSITION || it->second.first == D3DDECLUSAGE_POSITIONT)
                    return "gl_Position";
                return varyingName(it->second.first, it->second.second);
            }
            if (o.reg == 0) return "gl_Position";
            break;
        }
        case RT_RASTOUT:  return "gl_Position";
        case RT_COLOROUT:
            // Only render target 0 is bound here; a second colour written to the same
            // output would overwrite the first.
            if (o.reg == 0) return "fragColor";
            break;
        case RT_DEPTHOUT: c.usedDepth = true; return "nxDepth";
        case RT_SAMPLER:  { std::ostringstream s; s << (c.isPixel ? "s" : "sv") << o.reg; return s.str(); }
        case RT_MISCTYPE: // vPos (pixel position) / vFace (front-facing)
            return o.reg == 1 ? "vec4(gl_FrontFacing ? 1.0 : -1.0)"
                              : "(gl_FragCoord - vec4(0.5, 0.5, 0.0, 0.0))";
        default: break;
    }
    if (isDest) { c.usedSink = true; return "nxSink"; }
    return "vec4(0.0)";
}

// Apply a D3DSPSM_* source modifier (bias/sign/x2 etc. expand packed normals).
std::string applyMod(const std::string &e, int mod) {
    switch (mod) {
        case 1:  return "(-" + e + ")";                 // negate
        case 2:  return "(" + e + " - 0.5)";            // bias
        case 3:  return "(-(" + e + " - 0.5))";         // bias-negate
        case 4:  return "(2.0*(" + e + ") - 1.0)";      // sign / _bx2
        case 5:  return "(-(2.0*(" + e + ") - 1.0))";   // _bx2 negate
        case 6:  return "(1.0 - " + e + ")";            // complement
        case 7:  return "(2.0*(" + e + "))";            // _x2
        case 8:  return "(-2.0*(" + e + "))";           // _x2 negate
        case 11: return "abs(" + e + ")";               // abs
        case 12: return "(-abs(" + e + "))";            // abs-negate
        default: return e;                              // none / unsupported (dz,dw,not)
    }
}

std::string srcExpr(Ctx &c, const Operand &o) {
    return applyMod(regName(c, o, false) + swizStr(o.swizzle), o.mod);
}

// Emit one instruction as `dst<mask> = (<expr>)<mask>;`.
void emitInstr(Ctx &c, int op, const Operand *src, int nsrc, const Operand &dst,
               std::ostringstream &body, const std::string &ind, int ctrl = 0) {
    auto s = [&](int i) { return srcExpr(c, src[i]); };
    // TEXKILL discards the fragment when any of the tested register's xyz < 0; it has
    // no destination, so handle it before the dst<mask>= path below.
    if (op == OP_TEXKILL) {
        body << ind << "if (any(lessThan((" << regName(c, dst, false) << ").xyz, vec3(0.0)))) discard;\n";
        return;
    }
    // MOVA loads the address register a0 (round-to-nearest per D3D9 spec); a0 then
    // drives relative constant addressing (bone-matrix indexing in skinned meshes).
    if (op == OP_MOVA) {
        c.usedA0 = true;
        std::string m = maskStr(dst.writemask);
        body << ind << "a0" << m << " = ivec4(floor(" << s(0) << " + 0.5))" << m << ";\n";
        return;
    }
    std::string expr;
    switch (op) {
        case OP_MOV:   expr = s(0); break;
        case OP_ADD:   expr = s(0) + " + " + s(1); break;
        case OP_SUB:   expr = s(0) + " - " + s(1); break;
        case OP_MUL:   expr = s(0) + " * " + s(1); break;
        case OP_MAD:   expr = s(0) + " * " + s(1) + " + " + s(2); break;
        case OP_LRP:   expr = "mix(" + s(2) + ", " + s(1) + ", " + s(0) + ")"; break;  // dst = src2 + src0*(src1-src2)
        case OP_MIN:   expr = "min(" + s(0) + ", " + s(1) + ")"; break;
        case OP_MAX:   expr = "max(" + s(0) + ", " + s(1) + ")"; break;
        case OP_FRC:   expr = "fract(" + s(0) + ")"; break;
        case OP_ABS:   expr = "abs(" + s(0) + ")"; break;
        case OP_SLT:   expr = "vec4(lessThan(" + s(0) + ", " + s(1) + "))"; break;
        case OP_SGE:   expr = "vec4(greaterThanEqual(" + s(0) + ", " + s(1) + "))"; break;
        case OP_CMP:   expr = "mix(" + s(2) + ", " + s(1) + ", vec4(greaterThanEqual(" + s(0) + ", vec4(0.0))))"; break;  // src0>=0 ? src1 : src2
        case OP_RCP:   expr = "vec4(1.0 / (" + s(0) + ").x)"; break;
        case OP_RSQ:   expr = "vec4(inversesqrt((" + s(0) + ").x))"; break;
        case OP_EXP:   expr = "vec4(exp2((" + s(0) + ").x))"; break;
        case OP_LOG:   expr = "vec4(log2(abs((" + s(0) + ").x)))"; break;
        case OP_POW:   expr = "vec4(pow(abs((" + s(0) + ").x), (" + s(1) + ").x))"; break;
        case OP_NRM:   expr = "vec4(normalize((" + s(0) + ").xyz), 0.0)"; break;
        case OP_SINCOS:expr = "vec4(cos((" + s(0) + ").x), sin((" + s(0) + ").x), 0.0, 0.0)"; break;
        case OP_DP3:   expr = "vec4(dot((" + s(0) + ").xyz, (" + s(1) + ").xyz))"; break;
        case OP_DP4:   expr = "vec4(dot(" + s(0) + ", " + s(1) + "))"; break;
        case OP_DP2ADD:expr = "vec4(dot((" + s(0) + ").xy, (" + s(1) + ").xy) + (" + s(2) + ").x)"; break;
        // Screen-space partial derivatives (mip selection, distance detail).
        case OP_DSX:   expr = "dFdx(" + s(0) + ")"; break;
        case OP_DSY:   expr = "dFdy(" + s(0) + ")"; break;
        case OP_TEXLD: {
            // The instruction-token control bits select the texld flavour:
            // 1 = texldp (projective divide), 2 = texldb (bias in .w).
            std::string samp = regName(c, src[1], false);
            int sreg = src[1].reg;
            int dim = c.samplerDim.count(sreg) ? c.samplerDim.at(sreg) : 2;
            if (ctrl == 1 && dim == 2) {
                expr = "textureProj(" + samp + ", " + s(0) + ")";
            } else {
                // Coordinate arity follows the sampler's declared dimension: cube and
                // volume lookups take .xyz.
                const char *coords = (dim == 3 || dim == 4) ? ".xyz" : ".xy";
                if (ctrl == 2 && c.isPixel)
                    expr = "texture(" + samp + ", (" + s(0) + ")" + coords + ", (" + s(0) + ").w)";
                else
                    expr = "texture(" + samp + ", (" + s(0) + ")" + coords + ")";
            }
            break;
        }
        case OP_TEXLDL: {
            // Explicit-LOD sample: the LOD rides in coord .w (CoD blurs reflections by
            // gloss through it).
            int sreg2 = src[1].reg;
            int dim = c.samplerDim.count(sreg2) ? c.samplerDim.at(sreg2) : 2;
            const char *coords = (dim == 3 || dim == 4) ? ".xyz" : ".xy";
            expr = "textureLod(" + regName(c, src[1], false) + ", (" + s(0) + ")" + coords + ", (" + s(0) + ").w)";
            break;
        }
        case OP_TEXLDD: {
            // Explicit-gradient sample (ddx/ddy in src[2]/src[3]).
            int sreg2 = src[1].reg;
            int dim = c.samplerDim.count(sreg2) ? c.samplerDim.at(sreg2) : 2;
            const char *coords = (dim == 3 || dim == 4) ? ".xyz" : ".xy";
            if (nsrc >= 4)
                expr = "textureGrad(" + regName(c, src[1], false) + ", (" + s(0) + ")" + coords +
                       ", (" + srcExpr(c, src[2]) + ")" + coords + ", (" + srcExpr(c, src[3]) + ")" + coords + ")";
            else
                expr = "texture(" + regName(c, src[1], false) + ", (" + s(0) + ")" + coords + ")";
            break;
        }
        default:
            if (!c.unknownOps++) c.firstUnknownOp = (unsigned)op;
            return;
    }
    std::string dn = regName(c, dst, true);
    std::string m  = maskStr(dst.writemask);
    if (dst.dmod & 1) expr = "clamp(" + expr + ", 0.0, 1.0)";   // _sat: clamp result to [0,1]
    body << ind << dn << m << " = (" << expr << ")" << m << ";\n";
}

} // namespace

// ---- Canonical vertex-attribute mapping -----------------------------------
//
// One generic attribute location per (usage, usageIndex), laid out to fit the 16
// locations GL guarantees.
int NX_ShaderAttribLocation(int usage, int usageIndex) {
    switch (usage) {
        case D3DDECLUSAGE_POSITION:
        case D3DDECLUSAGE_POSITIONT:   return usageIndex == 0 ? 0 : -1;
        case D3DDECLUSAGE_BLENDWEIGHT: return usageIndex == 0 ? 1 : -1;
        case D3DDECLUSAGE_BLENDINDICES:return usageIndex == 0 ? 2 : -1;
        case D3DDECLUSAGE_NORMAL:      return usageIndex == 0 ? 3 : -1;
        case D3DDECLUSAGE_TANGENT:     return usageIndex == 0 ? 4 : -1;
        case D3DDECLUSAGE_COLOR:       return usageIndex <= 1 ? 5 + usageIndex : -1;   // COLOR0..1  -> 5,6
        case D3DDECLUSAGE_TEXCOORD:    return usageIndex <= 8 ? 7 + usageIndex : -1;   // TEXCOORD0..8 -> 7..15
        default:                       return -1;
    }
}

void NX_ShaderAttribName(int usage, int usageIndex, char *buf, unsigned size) {
    switch (usage) {
        case D3DDECLUSAGE_POSITION:
        case D3DDECLUSAGE_POSITIONT:    snprintf(buf, size, "aPos"); return;
        case D3DDECLUSAGE_BLENDWEIGHT:  snprintf(buf, size, "aBlendWeight"); return;
        case D3DDECLUSAGE_BLENDINDICES: snprintf(buf, size, "aBlendIndices"); return;
        case D3DDECLUSAGE_NORMAL:       snprintf(buf, size, "aNormal"); return;
        case D3DDECLUSAGE_TANGENT:      snprintf(buf, size, "aTangent"); return;
        case D3DDECLUSAGE_COLOR:        snprintf(buf, size, "aColor%d", usageIndex); return;
        case D3DDECLUSAGE_TEXCOORD:     snprintf(buf, size, "aTexCoord%d", usageIndex); return;
        default:                        snprintf(buf, size, "aUsage%d_%d", usage, usageIndex); return;
    }
}

int NX_ShaderAttribAll(int (*pairs)[2], int max) {
    static const int kUsages[] = { D3DDECLUSAGE_POSITION, D3DDECLUSAGE_BLENDWEIGHT,
                                   D3DDECLUSAGE_BLENDINDICES, D3DDECLUSAGE_NORMAL,
                                   D3DDECLUSAGE_TANGENT, D3DDECLUSAGE_COLOR,
                                   D3DDECLUSAGE_TEXCOORD };
    int n = 0;
    for (int u : kUsages) {
        int maxIdx = (u == D3DDECLUSAGE_TEXCOORD) ? 8 : (u == D3DDECLUSAGE_COLOR ? 1 : 0);
        for (int i = 0; i <= maxIdx && n < max; ++i) {
            pairs[n][0] = u;
            pairs[n][1] = i;
            ++n;
        }
    }
    return n;
}

char *NX_TranslateD3D9Shader(const DWORD *tok, NxShaderInfo *info)
{
    if (!tok)
        return nullptr;
    Ctx c;
    DWORD ver = *tok++;
    if ((ver >> 16) != 0xFFFF && (ver >> 16) != 0xFFFE)
        return nullptr;   // not a shader token stream
    c.isPixel = (ver >> 16) == 0xFFFF;

    std::ostringstream body;
    int indent = 1, loopId = 0;
    auto ind = [](int n) { return std::string(2 * (n < 1 ? 1 : n), ' '); };
    // D3DSHADER_COMPARISON (ctrl): 1 GT, 2 EQ, 3 GE, 4 LT, 5 NE, 6 LE.
    auto cmpStr = [&](int cc, const Operand &a, const Operand &b) {
        static const char *ops[] = { ">", ">", "==", ">=", "<", "!=", "<=" };
        const char *o = (cc >= 1 && cc <= 6) ? ops[cc] : ">";
        return "(" + srcExpr(c, a) + ").x " + o + " (" + srcExpr(c, b) + ").x";
    };
    const DWORD *start = tok;
    for (;;) {
        if (tok - start > (1 << 16))
            return nullptr;   // runaway: no END token
        DWORD t = *tok++;
        int op = (int)(t & 0xFFFF);
        if (op == OP_END) break;
        if (op == OP_COMMENT) { tok += (t >> 16) & 0x7FFF; continue; }
        int len  = (int)((t >> 24) & 0xF);
        int ctrl = (int)((t >> 16) & 0xFF);   // comparison field for IFC/BREAKC

        if (op == OP_DCL) {
            DWORD usageTok = *tok;
            Operand reg = decodeParam(tok[1]);
            tok += len;
            if (reg.type == RT_SAMPLER) {
                c.samplers.insert(reg.reg);
                // dcl_2d/dcl_cube/dcl_volume: texture type in usage token bits 27..30.
                // Typing every sampler sampler2D breaks the volume and cube lookups.
                c.samplerDim[reg.reg] = (int)((usageTok >> 27) & 0xF);
            }
            else {
                int usage = (int)(usageTok & 0x1F);
                int index = (int)((usageTok >> 16) & 0xF);
                if (reg.type == RT_INPUT)       c.inputs[reg.reg]  = {usage, index};
                else if (reg.type == RT_OUTPUT) c.outputs[reg.reg] = {usage, index};
            }
            continue;
        }
        if (op == OP_DEF) {
            Operand reg = decodeParam(tok[0]);
            float f[4]; std::memcpy(f, &tok[1], sizeof(f));
            std::memcpy(c.defs[reg.reg], f, sizeof(f));
            tok += len; continue;
        }
        if (op == OP_DEFI) { Operand reg = decodeParam(tok[0]); c.idefs[reg.reg] = (int)tok[1]; tok += len; continue; }
        if (op == OP_DEFB || op == OP_LABEL || op == OP_CALL || op == OP_CALLNZ ||
            op == OP_RET  || op == OP_SETP) {
            if (op != OP_DEFB && !c.unknownOps++) c.firstUnknownOp = (unsigned)op;
            tok += len; continue;
        }

        // Structured control flow. (Dropping IF/ELSE/ENDIF would run both branches.)
        if (op == OP_ELSE)    { if (indent > 1) --indent; body << ind(indent) << "} else {\n"; ++indent; tok += len; continue; }
        if (op == OP_ENDIF || op == OP_ENDREP || op == OP_ENDLOOP) { if (indent > 1) --indent; body << ind(indent) << "}\n"; tok += len; continue; }
        if (op == OP_BREAK)   { body << ind(indent) << "break;\n"; tok += len; continue; }
        if (op == OP_IF)      { Operand a = decodeParam(tok[0]); body << ind(indent) << "if (bool((" << srcExpr(c, a) << ").x)) {\n"; ++indent; tok += len; continue; }
        if (op == OP_IFC)     { Operand a = decodeParam(tok[0]), b = decodeParam(tok[1]); body << ind(indent) << "if (" << cmpStr(ctrl, a, b) << ") {\n"; ++indent; tok += len; continue; }
        if (op == OP_BREAKC)  { Operand a = decodeParam(tok[0]), b = decodeParam(tok[1]); body << ind(indent) << "if (" << cmpStr(ctrl, a, b) << ") break;\n"; tok += len; continue; }
        if (op == OP_REP || op == OP_LOOP) {
            Operand cnt = decodeParam(tok[0]);
            int n = 4; auto di = c.idefs.find(cnt.reg); if (di != c.idefs.end() && di->second > 0) n = di->second;
            int id = loopId++;
            body << ind(indent) << "for (int aL" << id << " = 0; aL" << id << " < " << n << "; ++aL" << id << ") {\n";
            ++indent; tok += len; continue;
        }

        // Arithmetic / sample: dst then sources, decoded sequentially -- a parameter
        // with the relative-addressing bit consumes an extra address token.
        Operand dst;
        int k = decodeParamRel(tok, dst);
        Operand src[4];                      // texldd carries 4 sources (coords, sampler, ddx, ddy)
        int nsrc = 0;
        while (k < len && nsrc < 4) k += decodeParamRel(tok + k, src[nsrc++]);
        tok += len;
        emitInstr(c, op, src, nsrc, dst, body, ind(indent), ctrl);
    }

    std::ostringstream o;
    o << "#version 300 es\n";
    o << "precision highp float;\n";
    o << "precision highp int;\n";
    // ES 3.00 has no default precision for sampler3D.
    o << "precision highp sampler3D;\n";
    auto samplerType = [&](int sN) {
        int dim = c.samplerDim.count(sN) ? c.samplerDim.at(sN) : 2;
        return dim == 4 ? "sampler3D" : dim == 3 ? "samplerCube" : "sampler2D";
    };
    if (c.isPixel) {
        for (auto &in : c.inputs)  o << "in vec4 " << varyingName(in.second.first, in.second.second) << ";\n";
        for (int sN : c.samplers)  o << "uniform " << samplerType(sN) << " s" << sN << ";\n";
        o << "out vec4 fragColor;\n";
        // Alpha test through discard (the core profile has no GL_ALPHA_TEST).
        o << "uniform int uAlphaTestFunc;\n";
        o << "uniform float uAlphaRef;\n";
    } else {
        for (auto &in : c.inputs)  o << "in vec4 " << attribName(in.second.first, in.second.second) << ";\n";
        for (auto &ou : c.outputs)
            if (ou.second.first != D3DDECLUSAGE_POSITION && ou.second.first != D3DDECLUSAGE_POSITIONT)
                o << "out vec4 " << varyingName(ou.second.first, ou.second.second) << ";\n";
        for (int sN : c.samplers)  o << "uniform " << samplerType(sN) << " sv" << sN << ";\n";
        o << "uniform vec2 nxHalfPixel;\n";
    }
    // Size the constant array to the highest register the shader references (not a blanket
    // 256). Relative addressing (c[a0.x+N]) forces maxConst to 255.
    if (c.usedConst) o << "uniform vec4 " << c.cArr() << "[" << (c.maxConst + 1) << "];\n";
    for (auto &d : c.defs) {
        // %.9g round-trips a float exactly; the ostream default of 6 digits does not.
        char v[4][32];
        for (int i = 0; i < 4; ++i) {
            snprintf(v[i], sizeof(v[i]), "%.9g", d.second[i]);
            // A GLSL float literal needs a '.' or exponent; "1" alone is an int.
            if (!strpbrk(v[i], ".eEn")) strcat(v[i], ".0");
        }
        o << "const vec4 c" << d.first << "_def = vec4(" << v[0] << ", " << v[1]
          << ", " << v[2] << ", " << v[3] << ");\n";
    }
    // Depth-prepass invariance: the prepass and the colour pass must compute the same
    // gl_Position.z bit for bit, which D3D9 guarantees and GLSL only does if told.
    if (!c.isPixel) o << "invariant gl_Position;\n";
    o << "void main() {\n";
    for (int r : c.usedTemps) o << "  vec4 r" << r << " = vec4(0.0);\n";
    if (c.usedA0)    o << "  ivec4 a0 = ivec4(0);\n";
    if (c.usedSink)  o << "  vec4 nxSink = vec4(0.0);\n";
    if (c.usedDepth) o << "  vec4 nxDepth = vec4(gl_FragCoord.z);\n";
    o << body.str();
    if (!c.isPixel) {
        // D3D9 clip-space z is [0,w]; GL's is [-w,w].
        o << "  gl_Position.z = 2.0 * gl_Position.z - gl_Position.w;\n";
        // D3D9 puts pixel centres on whole coordinates, GL on halves: shift by half a
        // pixel, the way D3D9 would have rasterised it. nxHalfPixel is (-1/W, 1/H) of
        // the viewport, set by the device per draw.
        o << "  gl_Position.xy += nxHalfPixel * gl_Position.w;\n";
        // Every target is stored top row first (nx_d3d9_null.cpp, "Render targets").
        o << "  gl_Position.y = -gl_Position.y;\n";
    } else {
        if (c.usedDepth) o << "  gl_FragDepth = nxDepth.x;\n";
        // D3DCMP_*: 1=NEVER 2=LESS 3=EQUAL 4=LEQUAL 5=GREATER 6=NOTEQUAL 7=GEQUAL 8=ALWAYS.
        // Discard when the test fails; uAlphaTestFunc == 0 disables it.
        o << "  if (uAlphaTestFunc != 0) {\n"
          << "    float aT = fragColor.a; bool passA = true;\n"
          << "    if      (uAlphaTestFunc == 1) passA = false;\n"
          << "    else if (uAlphaTestFunc == 2) passA = (aT <  uAlphaRef);\n"
          << "    else if (uAlphaTestFunc == 3) passA = (aT == uAlphaRef);\n"
          << "    else if (uAlphaTestFunc == 4) passA = (aT <= uAlphaRef);\n"
          << "    else if (uAlphaTestFunc == 5) passA = (aT >  uAlphaRef);\n"
          << "    else if (uAlphaTestFunc == 6) passA = (aT != uAlphaRef);\n"
          << "    else if (uAlphaTestFunc == 7) passA = (aT >= uAlphaRef);\n"
          << "    if (!passA) discard;\n"
          << "  }\n";
    }
    o << "}\n";

    if (info) {
        memset(info, 0, sizeof(*info));
        info->isPixel = c.isPixel;
        info->version = ver & 0xFFFF;
        for (int sN : c.samplers) {
            if (sN < 0 || sN >= 16) continue;
            info->samplerMask |= 1u << sN;
            info->samplerDim[sN] = (unsigned char)(c.samplerDim.count(sN) ? c.samplerDim.at(sN) : 2);
        }
        info->unknownOps = c.unknownOps;
        info->firstUnknownOp = c.firstUnknownOp;
    }
    std::string out = o.str();
    char *ret = (char *)malloc(out.size() + 1);
    if (ret)
        memcpy(ret, out.c_str(), out.size() + 1);
    return ret;
}

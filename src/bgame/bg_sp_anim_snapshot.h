#pragma once

// OpenBLOPS SP protocol extension, not the retail wire layout. No pointers,
// script-string handles, or native structure padding cross the connection.
#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <utility>

namespace SpAnimSnapshot
{
constexpr unsigned MaxBytes = 49152;
constexpr unsigned MaxEntities = 1024;
constexpr unsigned MaxNodes = 2048;
constexpr unsigned Version = 2;
struct Node
{
    // One-based parent instance in this entity's preorder list; zero is root.
    // Animation indices may repeat while an animation restart blends out.
    unsigned index = 0, notifyChild = 0, parent = 0;
    float time = 0, goalTime = 0, goalWeight = 0, weight = 0, rate = 0;
    std::int16_t cycle = 0;
};
struct Attachment { unsigned model = 0; std::string tag; };
struct Entity
{
    unsigned number = 0, treeSize = 0, ignoreCollision = 0;
    std::string tree;
    Attachment attachments[6];
    std::vector<Node> nodes;
};
struct Frame { int serverTime = 0; std::vector<Entity> entities; };

// The temporary script VM may disappear while its map-hunk trees remain live.
// Keep a bounded map-lifetime catalog, separate from packet/clock resets.
template <typename Tree> struct TreeCatalog
{
    struct Entry { std::string name; unsigned size; Tree *tree; };
    std::vector<Entry> entries;
    void Clear() { entries.clear(); }
    void Add(const char *name, unsigned size, Tree *tree)
    {
        if (name && tree && entries.size() < 256) entries.push_back({name, size, tree});
    }
    Tree *Find(const std::string &name, unsigned size) const
    {
        for (const Entry &entry : entries)
            if (entry.name == name && entry.size == size) return entry.tree;
        return nullptr;
    }
};

inline bool Valid(const Frame &frame)
{
    if (frame.serverTime < 0 || frame.entities.size() > MaxEntities) return false;
    unsigned nodes = 0;
    int previousEntity = -1;
    for (const Entity &entity : frame.entities)
    {
        if (entity.number >= MaxEntities-1 || static_cast<int>(entity.number) <= previousEntity
            || entity.tree.empty() || entity.tree.size() > 127 || entity.tree.find('\0') != std::string::npos || !entity.treeSize
            || entity.treeSize > 65535 || entity.ignoreCollision > 63 || entity.nodes.size() > MaxNodes) return false;
        previousEntity = static_cast<int>(entity.number);
        // An empty tag means shared-bone model melding, even with a model.
        for (const Attachment &attachment : entity.attachments)
            if (attachment.model >= 512 || attachment.tag.size() > 127 || attachment.tag.find('\0') != std::string::npos) return false;
        unsigned instance = 0;
        std::vector<unsigned> ancestors;
        std::vector<int> lastChild(entity.nodes.size()+1, -1);
        for (const Node &node : entity.nodes)
        {
            if (++nodes > MaxNodes || node.index >= entity.treeSize
                || node.parent > instance || node.notifyChild >= entity.treeSize
                || !std::isfinite(node.time) || node.time < 0 || node.time > 1
                || !std::isfinite(node.goalTime) || node.goalTime < 0
                || !std::isfinite(node.goalWeight) || node.goalWeight < 0
                || !std::isfinite(node.weight) || node.weight < 0
                || !std::isfinite(node.rate)) return false;
            // Parents precede children, subtrees are contiguous, siblings retain
            // the engine's ascending (but not unique) animation-index order.
            while (!ancestors.empty() && ancestors.back() != node.parent) ancestors.pop_back();
            if (node.parent && ancestors.empty()) return false;
            if (static_cast<int>(node.index) < lastChild[node.parent]) return false;
            lastChild[node.parent] = static_cast<int>(node.index);
            ancestors.push_back(++instance);
        }
    }
    return true;
}

struct Writer
{
    std::vector<unsigned char> bytes;
    void U32(unsigned value) { for (unsigned i = 0; i < 4; ++i) bytes.push_back(static_cast<unsigned char>(value >> (8*i))); }
    void Float(float value) { unsigned bits; std::memcpy(&bits, &value, 4); U32(bits); }
    void String(const std::string &value) { U32(static_cast<unsigned>(value.size())); bytes.insert(bytes.end(), value.begin(), value.end()); }
};
struct Reader
{
    const unsigned char *bytes; unsigned size, offset = 0; bool ok = true;
    unsigned U32() { if (offset > size || size-offset < 4) { ok = false; return 0; } unsigned v = 0; for (unsigned i=0;i<4;++i) v |= unsigned(bytes[offset++]) << (8*i); return v; }
    float Float() { unsigned bits = U32(); float v; std::memcpy(&v, &bits, 4); return v; }
    std::string String() { unsigned n = U32(); if (!ok || n > 127 || offset > size || n > size-offset) { ok=false; return {}; } std::string v(reinterpret_cast<const char *>(bytes+offset), n); offset+=n; if (v.find('\0') != std::string::npos) ok=false; return v; }
};
inline bool Encode(const Frame &frame, std::vector<unsigned char> &bytes)
{
    if (!Valid(frame)) return false;
    Writer out;
    out.U32(Version); out.U32(static_cast<unsigned>(frame.serverTime)); out.U32(static_cast<unsigned>(frame.entities.size()));
    for (const Entity &entity : frame.entities)
    {
        out.U32(entity.number); out.U32(entity.treeSize); out.String(entity.tree); out.U32(entity.ignoreCollision);
        for (const Attachment &attachment : entity.attachments) { out.U32(attachment.model); out.String(attachment.tag); }
        out.U32(static_cast<unsigned>(entity.nodes.size()));
        for (const Node &node : entity.nodes)
        {
            out.U32(node.index); out.U32(node.notifyChild); out.U32(node.parent); out.U32(static_cast<std::uint16_t>(node.cycle));
            out.Float(node.time); out.Float(node.goalTime); out.Float(node.goalWeight); out.Float(node.weight); out.Float(node.rate);
        }
        if (out.bytes.size() > MaxBytes) return false;
    }
    bytes = std::move(out.bytes); return true;
}
inline bool Decode(const unsigned char *bytes, unsigned size, Frame &frame)
{
    if (!bytes || size > MaxBytes) return false;
    Reader in{bytes,size}; Frame candidate;
    if (in.U32() != Version) return false;
    candidate.serverTime = static_cast<int>(in.U32());
    unsigned count = in.U32(), totalNodes = 0;
    if (!in.ok || count > MaxEntities) return false;
    for (unsigned i = 0; i < count; ++i)
    {
        Entity entity;
        entity.number = in.U32(); entity.treeSize = in.U32(); entity.tree = in.String(); entity.ignoreCollision = in.U32();
        for (Attachment &attachment : entity.attachments) { attachment.model = in.U32(); attachment.tag = in.String(); }
        unsigned nodeCount = in.U32();
        if (!in.ok || nodeCount > MaxNodes-totalNodes) return false;
        totalNodes += nodeCount;
        for (unsigned j = 0; j < nodeCount; ++j)
        {
            Node node;
            node.index = in.U32(); node.notifyChild = in.U32(); node.parent = in.U32(); unsigned cycle = in.U32();
            if (cycle > 65535) return false;
            node.cycle = static_cast<std::int16_t>(cycle);
            node.time = in.Float(); node.goalTime = in.Float(); node.goalWeight = in.Float(); node.weight = in.Float(); node.rate = in.Float();
            entity.nodes.push_back(node);
        }
        candidate.entities.push_back(std::move(entity));
    }
    if (!in.ok || in.offset != size || !Valid(candidate)) return false;
    frame = std::move(candidate); return true;
}
}

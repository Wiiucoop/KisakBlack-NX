#pragma once

enum ConstraintType : __int32
{                                                                             // XREF: PhysConstraint/r
                                                                                // ?XDoll_IsXDollConstraint@@YA_NW4ConstraintType@@@Z/r
    CONSTRAINT_NONE = 0x0,
    CONSTRAINT_POINT = 0x1,
    CONSTRAINT_DISTANCE = 0x2,
    CONSTRAINT_HINGE = 0x3,
    CONSTRAINT_JOINT = 0x4,
    CONSTRAINT_ACTUATOR = 0x5,
    CONSTRAINT_FAKE_SHAKE = 0x6,
    CONSTRAINT_LAUNCH = 0x7,
    CONSTRAINT_ROPE = 0x8,
    CONSTRAINT_LIGHT = 0x9,
    NUM_CONSTRAINT_TYPES = 0xA,
};

enum AttachPointType : __int32
{                                                                             // XREF: PhysConstraint/r
                                                                                // PhysConstraint/r ...
    ATTACH_POINT_WORLD = 0x0,
    ATTACH_POINT_DYNENT = 0x1,
    ATTACH_POINT_ENT = 0x2,
    ATTACH_POINT_BONE = 0x3,
};

struct PhysConstraint // sizeof=0xA8
{                                                                             // XREF: PhysConstraints/r
    unsigned __int16 targetname;
    // padding byte
    // padding byte
    ConstraintType type;
    AttachPointType attach_point_type1;
    int target_index1;
    unsigned __int16 target_ent1;
    // padding byte
    // padding byte
    const char *target_bone1;
    AttachPointType attach_point_type2;
    int target_index2;
    unsigned __int16 target_ent2;
    // padding byte
    // padding byte
    const char *target_bone2;
    float offset[3];
    float pos[3];
    float pos2[3];
    float dir[3];
    int flags;
    int timeout;
    int min_health;
    int max_health;
    float distance;
    float damp;
    float power;
    float scale[3];
    float spin_scale;
    float minAngle;
    float maxAngle;
    struct Material *material;
    intptr_t constraintHandle;   // nx-port: holds a rigid_body_constraint pointer
    int rope_index;
    int centity_num[4];
};

struct PhysConstraints // sizeof=0xA88
{                                                                             // XREF: XAssetPoolEntry<PhysConstraints>/r
    const char *name;
    unsigned int count;
    PhysConstraint data[16];
};

PhysConstraints *__cdecl PhysConstraintsLoadFile(const char *name, void *(__cdecl *Alloc)(int));
void __cdecl PhysConstraints_Strcpy(unsigned __int8 *member, const char *keyValue);
int __cdecl CON_ParseSpecificField(unsigned __int8 *pStruct, const char *pValue, int fieldType);
void __cdecl PhysConstraintsPostLoadFixup(PhysConstraints *physConstraints);
PhysConstraints *__cdecl PhysConstraintsPrecache(const char *name, void *(__cdecl *Alloc)(int));
void __cdecl PhysConstraintsUpdateField(char *name, char *keyvalue);
PhysConstraints *__cdecl GetPhysConstraints(char *name);

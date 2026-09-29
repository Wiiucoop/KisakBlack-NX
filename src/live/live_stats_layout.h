#pragma once

// Retail BlackOpsMP.exe uses a 0xa000-byte DDL blob (including checksum/header).
// Keep the historical SP contract separate; MP sizes must agree at every owner.
#ifdef KISAK_MP
constexpr unsigned int STATS_BUFFER_SIZE = 0xa000;
constexpr unsigned int STATS_PERSISTENT_PAYLOAD_SIZE = STATS_BUFFER_SIZE;
constexpr unsigned long long STATS_PACKET_MASK = 0xfffffffffULL;
#else
constexpr unsigned int STATS_BUFFER_SIZE = 40168;
constexpr unsigned int STATS_PERSISTENT_PAYLOAD_SIZE = 39272;
constexpr unsigned long long STATS_PACKET_MASK = 0x7ffffffffULL;
#endif
constexpr unsigned int STATS_RECORD_SIZE = STATS_BUFFER_SIZE + 4;
constexpr unsigned int MODIFIED_STATS_BYTE_SIZE = (STATS_BUFFER_SIZE + 7) / 8;
constexpr unsigned int STATS_PACKET_MASK_HIGH = static_cast<unsigned int>(STATS_PACKET_MASK >> 32);

#ifdef KISAK_MP
static_assert(STATS_BUFFER_SIZE >= 40548, "Retail version-100 DDL must fit");
static_assert(MODIFIED_STATS_BYTE_SIZE == 0x1400, "Retail MP dirty bitmap size");
static_assert(STATS_PACKET_MASK_HIGH == 0xf, "Retail MP stats packet mask");
#endif

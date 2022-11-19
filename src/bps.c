/**
 * Copyright 2022 TotalJustice.
 * SPDX-License-Identifier: Zlib
 */

/* SOURCE: https://www.romhacking.net/documents/764/ */
#include "bps.h"
#include <string.h>


#define PATCH_HEADER_SIZE 0x4
/* header + src / dst / meta sizes + command + crc32 */
#define PATCH_MIN_SIZE (PATCH_HEADER_SIZE + 3 + 1 + 12)


/* SOURCE: https://web.archive.org/web/20190108202303/http://www.hackersdelight.org/hdcodetxt/crc.c.txt */
static uint32_t crc32(const uint8_t* data, const size_t size)
{
    int crc;
    unsigned int byte, c;
    const unsigned int g0 = 0xEDB88320,    g1 = g0>>1,
        g2 = g0>>2, g3 = g0>>3, g4 = g0>>4, g5 = g0>>5,
        g6 = (g0>>6)^g0, g7 = ((g0>>6)^g0)>>1;

    crc = 0xFFFFFFFF;
    for (size_t i = 0; i < size; i++) {
        byte = data[i];
        crc = crc ^ byte;
        c = ((crc<<31>>31) & g7) ^ ((crc<<30>>31) & g6) ^
            ((crc<<29>>31) & g5) ^ ((crc<<28>>31) & g4) ^
            ((crc<<27>>31) & g3) ^ ((crc<<26>>31) & g2) ^
            ((crc<<25>>31) & g1) ^ ((crc<<24>>31) & g0);
        crc = ((unsigned)crc >> 8) ^ c;
    }
    return ~crc;
}

/* this can fail if the int is bigger than 8 bytes */
static size_t vln_read(const uint8_t* data, size_t* offset)
{
    size_t result = 0;
    size_t shift = 0;

    /* just in case its a bad patch, only run until max size */
    for (uint8_t i = 0; i < sizeof(size_t); ++i)
    {
        const uint8_t value = data[*offset];
        ++*offset;

        if (value & 0x80)
        {
            result += (value & 0x7F) << shift;
            break;
        }

        result += (value | 0x80) << shift;
        shift += 7;
    }

    return result;
}

bool bps_verify_header(const uint8_t* patch, size_t patch_size)
{
    if (!patch || patch_size < PATCH_HEADER_SIZE)
    {
        return false;
    }

    if (patch[0] != 'B' || patch[1] != 'P' || patch[2] != 'S' || patch[3] != '1')
    {
        return false;
    }

    return true;
}

bool bps_get_sizes(
    const uint8_t* patch, size_t patch_size,
    size_t* dst_size, size_t* src_size, size_t* meta_size, size_t* offset
) {
    (void)patch_size; // unused

    /* the offset is after the header */
    size_t offset_local = PATCH_HEADER_SIZE;

    const size_t source_size = vln_read(patch, &offset_local);
    const size_t target_size = vln_read(patch, &offset_local);
    const size_t metadata_size = vln_read(patch, &offset_local);

    if (dst_size)
    {
        *dst_size = target_size;
    }
    if (src_size)
    {
        *src_size = source_size;
    }
    if (meta_size)
    {
        *meta_size = metadata_size;
    }
    if (offset)
    {
        *offset = offset_local;
    }

    return true;
}

/* dst_size: large enough to fit entire output */
bool bps_patch(
    uint8_t* dst, size_t dst_size,
    const uint8_t* src, size_t src_size,
    const uint8_t* patch, size_t patch_size
) {
    if (!bps_verify_header(patch, patch_size))
    {
        return false;
    }

    size_t patch_offset = PATCH_HEADER_SIZE;
    size_t src_relative_offset = 0;
    size_t dst_offset = 0;
    size_t dst_relative_offset = 0;

    size_t source_size = 0;
    size_t target_size = 0;
    size_t metadata_size = 0;

    if (!bps_get_sizes(patch, patch_size, &target_size, &source_size, &metadata_size, &patch_offset))
    {
        return false;
    }

    if (src_size != source_size)
    {
        return false;
    }

    if (dst_size != target_size)
    {
        return false;
    }

    /* skip over metadata */
    patch_offset += metadata_size;

    /* crc's are at the last 12 bytes, each 4 bytes each. */
    uint32_t src_crc = 0;
    uint32_t dst_crc = 0;
    uint32_t patch_crc = 0;

    memcpy(&src_crc, patch + (patch_size - 12), sizeof(src_crc));
    memcpy(&dst_crc, patch + (patch_size - 8), sizeof(dst_crc));
    memcpy(&patch_crc, patch + (patch_size - 4), sizeof(patch_crc));

    /* check that the src and patch is valid. */
    /* dst is checked at the end. */
    if (src_crc != crc32(src, src_size))
    {
        return false;
    }

    /* we don't check it's own crc32 (obviously) */
    if (patch_crc != crc32(patch, patch_size - 4))
    {
        return false;
    }

    /* we've read the crc's now, reduce the size. */
    patch_size -= 12;

    enum Action
    {
        SourceRead = 0,
        TargetRead = 1,
        SourceCopy = 2,
        TargetCopy = 3,
    };

    while (patch_offset < patch_size)
    {
        const size_t data = vln_read(patch, &patch_offset);
        const enum Action action = data & 3;
        size_t len = (data >> 2) + 1;

        switch (action)
        {
            case SourceRead: {
                while (len--)
                {
                    dst[dst_offset] = src[dst_offset];
                    dst_offset++;
                }
            } break;

            case TargetRead: {
                while (len--)
                {
                    dst[dst_offset++] = patch[patch_offset++];
                }
            } break;

            case SourceCopy: {
                const size_t data = vln_read(patch, &patch_offset);
                src_relative_offset += (data & 1 ? -1 : +1) * (data >> 1);
                while (len--)
                {
                    dst[dst_offset++] = src[src_relative_offset++];
                }
            } break;

            case TargetCopy: {
                const size_t data = vln_read(patch, &patch_offset);
                dst_relative_offset += (data & 1 ? -1 : +1) * (data >> 1);
                while (len--)
                {
                    dst[dst_offset++] = dst[dst_relative_offset++];
                }
            } break;

        }
    }

    if (dst_crc != crc32(dst, dst_size))
    {
        return false;
    }

    return true;
}

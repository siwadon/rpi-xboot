#include "lzma.h"
#include "lib/lzma/Precomp.h"
#include "lib/lzma/CpuArch.h"
#include "lib/lzma/Alloc.h"
#include "lib/lzma/7zFile.h"
#include "lib/lzma/7zVersion.h"
#include "lib/lzma/LzmaDec.h"

SRes decode(unsigned int input_base, unsigned int output_base, unsigned int file_size) {
    UInt64 unpackSize = 0;
    int i;
    SRes res = 0;
    CLzmaDec state;
    int header_size = LZMA_PROPS_SIZE + 8;

    // header: 5 bytes of LZMA properties and 8 bytes of uncompressed size
    Byte (*header)[header_size] = (Byte (*)[header_size]) input_base;

    // Read and parse header
    for (i = 0; i < 8; i++)
        unpackSize += (UInt64) (*header)[LZMA_PROPS_SIZE + i] << (i * 8);

    int sizeUnknown = (unpackSize == (UInt64)(Int64)-1);

    LzmaDec_Construct(&state);

    res = LzmaDec_Allocate(&state, (*header), LZMA_PROPS_SIZE, &g_Alloc);

    Byte *dest = (Byte *) output_base;
    Byte *src = (*header + 13);

    size_t destLen = sizeUnknown ? OUT_BUF_SIZE : unpackSize;
    size_t srcLen = file_size;

    ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
    ELzmaStatus status;

    res = LzmaDecode(dest, &destLen, src, &srcLen,
        (*header), LZMA_PROPS_SIZE, finishMode,
        &status, &g_Alloc);

    LzmaDec_Free(&state, &g_Alloc);

    return res;
}
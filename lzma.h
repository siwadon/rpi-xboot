#include "lib/lzma/7zFile.h"

#define IN_BUF_SIZE (1 << 20)
#define OUT_BUF_SIZE (1 << 20)

static const char * const kCantReadMessage = "Can not read input file";
static const char * const kCantWriteMessage = "Can not write output file";
static const char * const kCantAllocateMessage = "Can not allocate memory";
static const char * const kDataErrorMessage = "Data error";

SRes decode(unsigned int input_base, unsigned int output_base, unsigned int file_size);
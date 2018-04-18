#include "lib/lzma/7zFile.h"

#define IN_BUF_SIZE (1 << 20)
#define OUT_BUF_SIZE (1 << 20)

SRes decode(unsigned int input_base, unsigned int output_base, unsigned int file_size);
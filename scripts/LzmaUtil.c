/* LzmaUtil.c -- Test application for LZMA compression
2017-04-27 : Igor Pavlov : Public domain */

#include "../lib/lzma/Precomp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/lzma/CpuArch.h"

#include "../lib/lzma/Alloc.h"
#include "../lib/lzma/7zFile.h"
#include "../lib/lzma/7zVersion.h"
#include "../lib/lzma/LzmaDec.h"

#define IN_BUF_SIZE (1 << 20)
#define OUT_BUF_SIZE (1 << 20)

static const char * const kCantReadMessage = "Can not read input file";
static const char * const kCantWriteMessage = "Can not write output file";
static const char * const kCantAllocateMessage = "Can not allocate memory";
static const char * const kDataErrorMessage = "Data error";

static void PrintHelp(char *buffer)
{
  strcat(buffer,
    "\nLZMA-C " MY_VERSION_CPU " : " MY_COPYRIGHT_DATE "\n\n"
    "Usage:  lzma inputFile outputFile\n");
}

static int PrintError(char *buffer, const char *message)
{
  strcat(buffer, "\nError: ");
  strcat(buffer, message);
  strcat(buffer, "\n");
  return 1;
}

static int PrintErrorNumber(char *buffer, SRes val)
{
  sprintf(buffer + strlen(buffer), "\nError code: %x\n", (unsigned)val);
  return 1;
}

static int PrintUserError(char *buffer)
{
  return PrintError(buffer, "Incorrect command");
}

static void HexDump(const Byte *bytes) {
  int i = 0;
  int cols = 16;
  int limit = cols * 2;

  for (i=0; i < limit; i++) {
    if (i % cols == 0) {
      printf("\n");
      printf("0000000: ");
    }
    printf("%02x ", bytes[i]);
  }

  printf("\n");
}

static SRes Decode2(CLzmaDec *state, ISeqOutStream *outStream, ISeqInStream *inStream,
    UInt64 unpackSize)
{
  int thereIsSize = (unpackSize != (UInt64)(Int64)-1);
  Byte inBuf[IN_BUF_SIZE];
  Byte outBuf[OUT_BUF_SIZE];
  size_t inPos = 0, inSize = 0, outPos = 0;
  LzmaDec_Init(state);
  for (;;)
  {
    if (inPos == inSize)
    {
      inSize = IN_BUF_SIZE;
      printf("inSize = %lu (before read)\n", inSize);
      RINOK(inStream->Read(inStream, inBuf, &inSize));
      printf("inSize = %lu (after read)\n\n", inSize);
      inPos = 0;
    }
    {
      SRes res;
      SizeT inProcessed = inSize - inPos;
      SizeT outProcessed = OUT_BUF_SIZE - outPos;
      ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
      ELzmaStatus status;
      if (thereIsSize && outProcessed > unpackSize)
      {
        outProcessed = (SizeT)unpackSize;
        finishMode = LZMA_FINISH_END;
      }
      
      printf("srcLen  = %lu\n", inProcessed);
      printf("destLen = %lu\n", outProcessed);
      res = LzmaDec_DecodeToBuf(state, outBuf + outPos, &outProcessed,
        inBuf + inPos, &inProcessed, finishMode, &status);

      printf("status  = %d\n", res);
      printf("srcLen  = %lu\n", inProcessed);
      printf("destLen = %lu\n\n", outProcessed);

      inPos += inProcessed;
      outPos += outProcessed;
      unpackSize -= outProcessed;
      
      if (outStream)
        if (outStream->Write(outStream, outBuf, outPos) != outPos)
          return SZ_ERROR_WRITE;
        
      outPos = 0;
      
      if (res != SZ_OK || (thereIsSize && unpackSize == 0)) {
        HexDump(outBuf);
        return res;
      }
      
      if (inProcessed == 0 && outProcessed == 0)
      {
        HexDump(outBuf);
        if (thereIsSize || status != LZMA_STATUS_FINISHED_WITH_MARK)
          return SZ_ERROR_DATA;
        return res;
      }
    }
  }
}

static SRes DecodeSingleCall(ISeqInStream *inStream, ISeqOutStream *outStream, unsigned char *header, UInt64 unpackSize) {
  int sizeUnknown = (unpackSize == (UInt64)(Int64)-1);
  printf("sizeUnknown = %d\n\n", sizeUnknown);

  SRes res;
  Byte inBuf[IN_BUF_SIZE];
  Byte outBuf[OUT_BUF_SIZE];
  ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
  ELzmaStatus status;
  size_t inSize = IN_BUF_SIZE;
  size_t outSize = sizeUnknown ? OUT_BUF_SIZE : unpackSize;

  printf("inSize = %lu (before read)\n", inSize);
  RINOK(inStream->Read(inStream, inBuf, &inSize));
  printf("inSize = %lu (after read)\n\n", inSize);

  printf("srcLen  = %lu\n", inSize);
  printf("destLen = %lu\n", outSize);

  res = LzmaDecode(outBuf, &outSize, inBuf, &inSize,
    header, LZMA_PROPS_SIZE, finishMode,
    &status, &g_Alloc);

  outStream->Write(outStream, outBuf, outSize);
  printf("status  = %d\n", res);
  printf("srcLen  = %lu\n", inSize);
  printf("destLen = %lu\n", outSize);

  HexDump(outBuf);

  return res;
}

static SRes Decode(ISeqOutStream *outStream, ISeqInStream *inStream)
{
  UInt64 unpackSize;
  int i;
  SRes res = 0;

  CLzmaDec state;

  /* header: 5 bytes of LZMA properties and 8 bytes of uncompressed size */
  unsigned char header[LZMA_PROPS_SIZE + 8];

  /* Read and parse header */
  RINOK(SeqInStream_Read(inStream, header, sizeof(header)));

  unpackSize = 0;
  for (i = 0; i < 8; i++)
    unpackSize += (UInt64)header[LZMA_PROPS_SIZE + i] << (i * 8);
  
  printf("unpackSize = %llu\n", unpackSize);

  LzmaDec_Construct(&state);
  RINOK(LzmaDec_Allocate(&state, header, LZMA_PROPS_SIZE, &g_Alloc));

  // printf("\n====== Multi-call ====== \n");
  // res = Decode2(&state, outStream, inStream, unpackSize);

  printf("\n====== Single-call ====== \n");
  res = DecodeSingleCall(inStream, outStream, header, unpackSize);
  LzmaDec_Free(&state, &g_Alloc);
  return res;
}

static int main2(int numArgs, const char *args[], char *rs)
{
  CFileSeqInStream inStream;
  CFileOutStream outStream;
  int res;
  Bool useOutFile = False;

  FileSeqInStream_CreateVTable(&inStream);
  File_Construct(&inStream.file);

  FileOutStream_CreateVTable(&outStream);
  File_Construct(&outStream.file);

  if (numArgs == 1)
  {
    PrintHelp(rs);
    return 0;
  }

  if (numArgs < 2 || numArgs > 3)
    return PrintUserError(rs);

  {
    size_t t4 = sizeof(UInt32);
    size_t t8 = sizeof(UInt64);
    if (t4 != 4 || t8 != 8)
      return PrintError(rs, "Incorrect UInt32 or UInt64");
  }

  if (InFile_Open(&inStream.file, args[1]) != 0)
    return PrintError(rs, "Can not open input file");

  if (numArgs > 2)
  {
    useOutFile = True;
    if (OutFile_Open(&outStream.file, args[2]) != 0)
      return PrintError(rs, "Can not open output file");
  }

    res = Decode(&outStream.vt, useOutFile ? &inStream.vt : NULL);

  if (useOutFile)
    File_Close(&outStream.file);
  File_Close(&inStream.file);

  if (res != SZ_OK)
  {
    if (res == SZ_ERROR_MEM)
      return PrintError(rs, kCantAllocateMessage);
    else if (res == SZ_ERROR_DATA)
      return PrintError(rs, kDataErrorMessage);
    else if (res == SZ_ERROR_WRITE)
      return PrintError(rs, kCantWriteMessage);
    else if (res == SZ_ERROR_READ)
      return PrintError(rs, kCantReadMessage);
    return PrintErrorNumber(rs, res);
  }
  return 0;
}


int MY_CDECL main(int numArgs, const char *args[])
{
  char rs[800] = { 0 };
  int res = main2(numArgs, args, rs);
  fputs(rs, stdout);
  return res;
}

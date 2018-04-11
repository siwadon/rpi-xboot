PROG = lzma
CXX = gcc
LIB =
RM = rm -f
CFLAGS = -c -O2 -Wall -D_7ZIP_ST

OBJS = \
  LzmaUtil.o \
  Alloc.o \
  LzmaDec.o \
  7zFile.o \
  7zStream.o \


all: $(PROG)

$(PROG): $(OBJS)
	$(CXX) -o $(PROG) $(LDFLAGS) $(OBJS) $(LIB) $(LIB2)

LzmaUtil.o: LzmaUtil.c
	$(CXX) $(CFLAGS) LzmaUtil.c

Alloc.o: ../lib/lzma/Alloc.c
	$(CXX) $(CFLAGS) ../lib/lzma/Alloc.c

LzmaDec.o: ../lib/lzma/LzmaDec.c
	$(CXX) $(CFLAGS) ../lib/lzma/LzmaDec.c

7zFile.o: ../lib/lzma/7zFile.c
	$(CXX) $(CFLAGS) ../lib/lzma/7zFile.c

7zStream.o: ../lib/lzma/7zStream.c
	$(CXX) $(CFLAGS) ../lib/lzma/7zStream.c

clean:
	-$(RM) $(PROG) $(OBJS)

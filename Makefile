### make
### make TGT64=64
CC                 = gcc
CCFLAGS32          = -g
CCFLAGS64          = $(CCFLAGS32) -maix64
AR                 = ar
AROPT              = -X32_64
INCLUDEPATH        = -I. -I/usr/include -I..
LIBDEPALL          = -L. -lpthreads -liconv -ldl -lpthread -ldb400

###  shared lib

### DB2UTIL
DB2UTILPGM32      = db2util
DB2UTILLIBOBJS32  = db2util.o
DB2UTILLIBDEPS32  = $(LIBDEPALL)
DB2UTILPGM64      = db2util_64
DB2UTILLIBOBJS64  = $(DB2UTILLIBOBJS32)
DB2UTILLIBDEPS64  = $(LIBDEPALL)
DB2UTILLIBEXPS    = -Wl,-bE:db2util.exp

### make TGT64=64
ifdef TGT64
CCFLAGS            = $(CCFLAGS64)
DB2UTILPGM         = $(DB2UTILPGM64)
DB2UTILLIBOBJS     = $(DB2UTILLIBOBJS64)
DB2UTILLIBDEPS     = $(DB2UTILLIBDEPS64)
### make (32)
else
CCFLAGS            = $(CCFLAGS32)
DB2UTILPGM         = $(DB2UTILPGM32)
DB2UTILLIBOBJS     = $(DB2UTILLIBOBJS32)
DB2UTILLIBDEPS     = $(DB2UTILLIBDEPS32)
endif

### tells make all things to do (ordered)
all: $(DB2UTILPGM) install

### generic rules
### (note: .c.o compiles all c parts in OBJS list)
.SUFFIXES: .o .c
.c.o:
	$(CC) $(CCFLAGS) $(INCLUDEPATH) -c $<

### -- db2util_32/64
$(DB2UTILPGM32): $(DB2UTILLIBOBJS)
	$(CC) $(CCFLAGS) $(DB2UTILLIBOBJS) $(DB2UTILLIBDEPS) $(DB2UTILLIBEXPS) -o $(DB2UTILPGM32)
$(DB2UTILPGM64): $(DB2UTILLIBOBJS)
	$(CC) $(CCFLAGS) $(DB2UTILLIBOBJS) $(DB2UTILLIBDEPS) $(DB2UTILLIBEXPS) -o $(DB2UTILPGM64)

install:
	cp $(DB2UTILPGM32)  /QOpenSys/usr/bin/.


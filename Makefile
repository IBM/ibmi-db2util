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
LIBOBJS            = thing.o
LIBEXPS            = -bE:libthing400.exp
LIBSHR32           = libthing400.a
LIBOBJ32           = shr.o
LIBDEPS32          = $(LIBDEPALL) -shared -Wl,-brtl
LIBSHR64           = libthing400_64.a
LIBOBJ64           = shr_64.o
LIBDEPS64          = $(LIBDEPS32)

### DB2UTIL
DB2UTILPGM32      = db2util
DB2UTILLIBOBJS32  = db2util.o
DB2UTILLIBDEPS32  = $(LIBDEPALL)
DB2UTILPGM64      = db2util_64
DB2UTILLIBOBJS64  = $(DB2UTILLIBOBJS32)
DB2UTILLIBDEPS64  = $(LIBDEPALL)

### make TGT64=64
ifdef TGT64
CCFLAGS            = $(CCFLAGS64)
SHRLIB             = $(LIBSHR32)
SHROBJ             = $(LIBOBJ64)
LIBDEPS            = $(LIBDEPS64)
DB2UTILPGM         = $(DB2UTILPGM64)
DB2UTILLIBOBJS     = $(DB2UTILLIBOBJS64)
DB2UTILLIBDEPS     = $(DB2UTILLIBDEPS64)
### make (32)
else
CCFLAGS            = $(CCFLAGS32)
SHRLIB             = $(LIBSHR32)
SHROBJ             = $(LIBOBJ32)
LIBDEPS            = $(LIBDEPS32)
DB2UTILPGM         = $(DB2UTILPGM32)
DB2UTILLIBOBJS     = $(DB2UTILLIBOBJS32)
DB2UTILLIBDEPS     = $(DB2UTILLIBDEPS32)
endif

### tells make all things to do (ordered)
### all: cpy $(SHRLIB) $(DB2UTILPGM) 
all: $(DB2UTILPGM) 

### generic rules
### (note: .c.o compiles all c parts in OBJS list)
.SUFFIXES: .o .c
.c.o:
	$(CC) $(CCFLAGS) $(INCLUDEPATH) -c $<

### -- Build the shared lib(s).
$(SHROBJ): $(LIBOBJS)
	$(CC) $(CCFLAGS) $(LIBOBJS) $(LIBEXPS) $(LIBDEPS) -o$(SHROBJ)
$(SHRLIB): $(SHROBJ)
	$(AR) $(AROPT) ruv $(SHRLIB) $(SHROBJ)

### -- db2util_32/64
$(DB2UTILPGM32): $(DB2UTILLIBOBJS)
	$(CC) $(CCFLAGS) $(DB2UTILLIBOBJS) $(DB2UTILLIBDEPS) -o $(DB2UTILPGM32)
$(DB2UTILPGM64): $(DB2UTILLIBOBJS)
	$(CC) $(CCFLAGS) $(DB2UTILLIBOBJS) $(DB2UTILLIBDEPS) -o $(DB2UTILPGM64)


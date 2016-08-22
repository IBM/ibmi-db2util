### make
### make TGT64=64
FCGI               = ./fcgi-2.4.1-SNAP-0311112127
CC                 = gcc
CCFLAGS32          = -g
CCFLAGS64          = $(CCFLAGS32) -maix64
AR                 = ar
AROPT              = -X32_64
INCLUDEPATH        = -I. -I/usr/include -I.. -I$(FCGI)/include
LIBDEPALL          = -L. -lpthreads -liconv -ldl -lpthread

###  shared lib

### DB2FCGI
DB2FCGIPGM32      = db2fcgi
DB2FCGILIBOBJS32  = db2fcgi.o fcgiapp.o os_unix.o fcgi_stdio.o
DB2FCGILIBDEPS32  = $(LIBDEPALL)
DB2FCGIPGM64      = db2fcgi_64
DB2FCGILIBOBJS64  = $(DB2FCGILIBOBJS32)
DB2FCGILIBDEPS64  = $(LIBDEPALL)
DB2FCGILIBEXPS    = 

### make TGT64=64
ifdef TGT64
CCFLAGS            = $(CCFLAGS64)
DB2FCGIPGM         = $(DB2FCGIPGM64)
DB2FCGILIBOBJS     = $(DB2FCGILIBOBJS64)
DB2FCGILIBDEPS     = $(DB2FCGILIBDEPS64)
### make (32)
else
CCFLAGS            = $(CCFLAGS32)
DB2FCGIPGM         = $(DB2FCGIPGM32)
DB2FCGILIBOBJS     = $(DB2FCGILIBOBJS32)
DB2FCGILIBDEPS     = $(DB2FCGILIBDEPS32)
endif

### tells make all things to do (ordered)
all: cpy $(DB2FCGIPGM) install

### generic rules
### (note: .c.o compiles all c parts in OBJS list)
.SUFFIXES: .o .c
.c.o:
	$(CC) $(CCFLAGS) $(INCLUDEPATH) -c $<

### -- db2fcgi_32/64
$(DB2FCGIPGM32): $(DB2FCGILIBOBJS)
	$(CC) $(CCFLAGS) $(DB2FCGILIBOBJS) $(DB2FCGILIBDEPS) $(DB2FCGILIBEXPS) -o $(DB2FCGIPGM32)
$(DB2FCGIPGM64): $(DB2FCGILIBOBJS)
	$(CC) $(CCFLAGS) $(DB2FCGILIBOBJS) $(DB2FCGILIBDEPS) $(DB2FCGILIBEXPS) -o $(DB2FCGIPGM64)

install:
	cp $(DB2FCGIPGM32)  /QOpenSys/usr/bin/.

cpy:
	cp $(FCGI)/libfcgi/fcgiapp.c .
	cp $(FCGI)/libfcgi/fcgiapp.c .
	cp $(FCGI)/libfcgi/os_unix.c .
	cp $(FCGI)/libfcgi/fcgi_stdio.c .

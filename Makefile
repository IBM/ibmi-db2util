### make
CC                 = gcc
INCLUDEPATH        = -I. -I/QOpenSys/pkgs/include/cli
LIBDEPALL          = -L.  -liconv -ldl -lpthread -ldb400

###  shared lib

### DB2UTIL
DB2UTILPGM      = db2util
DB2UTILLIBOBJS  = db2util.o
DB2UTILLIBDEPS  = $(LIBDEPALL)
DB2UTILLIBEXPS    = -Wl,-bE:db2util.exp
CCFLAGS            = -g -maix64 -pthread

### tells make all things to do (ordered)
all: $(DB2UTILPGM)

### generic rules
### (note: .c.o compiles all c parts in OBJS list)
.SUFFIXES: .o .c
.c.o:
	$(CC) $(CCFLAGS) $(INCLUDEPATH) -c $<

$(DB2UTILPGM): $(DB2UTILLIBOBJS)
	$(CC) $(CCFLAGS) $(DB2UTILLIBOBJS) $(DB2UTILLIBDEPS) $(DB2UTILLIBEXPS) -o $(DB2UTILPGM)

install: $(DB2UTILPGM)
	mkdir -p $(DESTDIR)/QOpenSys/pkgs/bin/
	cp $(DB2UTILPGM)  $(DESTDIR)/QOpenSys/pkgs/bin

clean:
	rm -f $(DB2UTILPGM) $(DB2UTILLIBOBJS)


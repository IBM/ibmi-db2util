### make
PREFIX=/QOpenSys/pkgs
BINDIR=$(PREFIX)/bin
LIBDIR=$(PREFIX)/lib

CC=gcc
CPPFLAGS=-I/QOpenSys/pkgs/include/cli
CFLAGS=-g -maix64
LDFLAGS=-maix64 -ldb400 -Wl,-blibpath:/QOpenSys/usr/lib

all: db2util

### generic rules
### (note: .c.o compiles all c parts in OBJS list)
.SUFFIXES: .o .c

db2util: db2util.o format_json.o format_csv.o format_space.o
	$(CC) -o $@ $(LDFLAGS) -o $@ $^

install: db2util
	mkdir -p $(DESTDIR)$(BINDIR)
	cp db2util $(DESTDIR)$(BINDIR)

clean:
	rm *.o db2util


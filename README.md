# IBM i db2util

## :rotating_light: ------ IMPORTANT WARNING ------- :rotating_light:

This project is (and possibly always will be) under construction. It is released as a stable version 1.0.0. Future major releases may break compatibility.

## About

Welcome to the db2util project. This project includes a `db2util` command which is similar to the `db2` command provided by `QSH` on IBM i, but runs from PASE and interfaces through libdb400.a.

## Building

This project uses a very simple Makefile with no configuration step:

```sh
make
make install
```

The project can be configured using the following Makefile variables:

- `PREFIX` - the install prefix, default: `/QOpenSys/pkgs`
- `BINDIR` - the install path for binaries, default `/QOpenSys/pkgs/bin`
- `CC` - the compiler, default: `gcc`
- `CPPFLAGS` - the C pre-processor flags, eg. `-I`, `-D`, ...
- `CFLAGS` - the C compiler flags
- `LDFLAGS` - the linker flags
- `DESTDIR` - set the install destination prefix, useful for installing in to a temporary directory for packaging

## Build Dependencies

In order to build you will need:

- GNU make
- GCC
- PASE CLI headers

You can use the following command to install them:

```sh
yum install gcc make-gnu sqlcli-devel
```

## Compiled version

A pre-compiled version is available from the IBM i yum repository:

```sh
yum install db2util
```

**NOTE: See [here](http://ibm.biz/ibmi-rpms) for more information on getting `yum` installed on your IBM i system.**

## Usage

### Comma delimter output (default or with `-o csv`)

```sh
db2util "select * from QIWS/QCUSTCDT where LSTNAM='Jones' or LSTNAM='Vine'"
"839283","Jones   ","B D","21B NW 135 St","Clay  ","NY","13041","400","1","100.00","0.00"
"392859","Vine    ","S S","PO Box 79    ","Broton","VT","5046","700","1","439.00","0.00"
```

### JSON output `(-o json`)

```sh
db2util -o json "select * from QIWS/QCUSTCDT where LSTNAM='Jones' or LSTNAM='Vine'"
[
{"CUSNUM":839283,"LSTNAM":"Jones   ","INIT":"B D","STREET":"21B NW 135 St","CITY":"Clay  ","STATE":"NY","ZIPCOD":13041,"CDTLMT":400,"CHGCOD":1,"BALDUE":100.00,"CDTDUE":0.00},
{"CUSNUM":392859,"LSTNAM":"Vine    ","INIT":"S S","STREET":"PO Box 79    ","CITY":"Broton","STATE":"VT","ZIPCOD":5046,"CDTLMT":700,"CHGCOD":1,"BALDUE":439.00,"CDTDUE":0.00}
]
```

### Space delimter output (`-o space`)

```sh
db2util -o space "select * from QIWS/QCUSTCDT where LSTNAM='Jones' or LSTNAM='Vine'"
"839283" "Jones   " "B D" "21B NW 135 St" "Clay  " "NY" "13041" "400" "1" "100.00" "0.00"
"392859" "Vine    " "S S" "PO Box 79    " "Broton" "VT" "5046" "700" "1" "439.00" "0.00"
```

### Help

```sh
db2util -h
db2util [options] <sql statement>
Options:
  -o <fmt>                   Output format. Value values for fmt:
                               json:  [{"name"}:{"value"},{"name"}:{"value"},...]
                               csv:   "value","value",...
                               space: "value" "value" ...
  -p <parm>                  Input parameter(s) (max 32)
  -h                         This help text
  -v                         Show version number and quit

Examples:
db2util "select * from QIWS.QCUSTCDT where LSTNAM='Jones' or LSTNAM='Vine'"
db2util -p Jones -p Vine -o json "select * from QIWS.QCUSTCDT where LSTNAM=? or LSTNAM=?"
db2util -p Jones -p Vine -o space "select * from QIWS.QCUSTCDT where LSTNAM=? or LSTNAM=?"
```

## Contributing

Please read the [contribution guidelines](CONTRIBUTING.md).

## Contributors

See [contributors.txt](contributors.txt).

## License

[MIT](LICENSE)

(This project has been migrated from [http://bitbucket.org/litmis/db2util](http://bitbucket.org/litmis/db2util))

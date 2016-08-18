#------IMPORTANT WARNING -------
This project is under construction.


#db2util

Welcome to the db2util project. Goal is PASE DB2 CLI command line interface driver (libdb400.a).

Aaron asked about PASE db2 utility similar to the one in ILE (qsh -> db2).

PHP team asked about use db2util as RPGCGI json (see Sources db2util/rpgcgi/README.md).

#Compile

Assume Option 3 GCC pkg_perzl_gcc-4.8.3.lst.

```
$ export PATH=/opt/freeware/bin:$PATH
$ export LIBPATH=/opt/freeware/lib:$LIBPATH
$ cd db2util
$ make
gcc -g -I. -I/usr/include -I.. -c db2util.c
gcc -g db2util.o -L. -lpthreads -liconv -ldl -lpthread -ldb400 -o db2util
```

#Compiled version

You may try pre-compiled test version db2util at Yips link (V7r1+).

* http://yips.idevcloud.com/wiki/index.php/Databases/DB2Util

#Run

Help:

```
Syntax: db2util 'sql statement' [-h -xc -o [json|comma|space] -p parm1 parm2 ...]
-h      help
-xc      sql statement is xmlservice command
-o [json|comma|space]
 json  - {"records":[{"name"}:{"value"},{"name"}:{"value"},...]}
 comma - "value","value",...
 space - "value" "value" ...
-p parm1 parm2 ...
Version: 1.0.4 beta

Example (DB2)
db2util "select * from QIWS/QCUSTCDT where LSTNAM='Jones' or LSTNAM='Vine'"
db2util "select * from QIWS/QCUSTCDT where LSTNAM=? or LSTNAM=?" -p Jones Vine -o json
db2util "select * from QIWS/QCUSTCDT where LSTNAM=? or LSTNAM=?" -p Jones Vine -o space

Example (XMLSERVICE):
db2util "DSPLIBL" -xc
```

Comma delimter output (default or -o comma):

```
bash-4.3$ db2util "select * from QIWS/QCUSTCDT where LSTNAM='Jones' or LSTNAM='Vine'"
"839283","Jones   ","B D","21B NW 135 St","Clay  ","NY","13041","400","1","100.00",".00"
"392859","Vine    ","S S","PO Box 79    ","Broton","VT","5046","700","1","439.00",".00"
"392859","Vine    ","S S","PO Box 79    ","Broton","VT","5046","700","1","439.00",".00"
```

JSON output (-o json):

```
bash-4.3$ db2util "select * from QIWS/QCUSTCDT where LSTNAM=? or LSTNAM=?" -p Jones Vine -o json 
{"records":[
{"CUSNUM":"839283","LSTNAM":"Jones   ","INIT":"B D","STREET":"21B NW 135 St","CITY":"Clay  ","STATE":"NY","ZIPCOD":"13041","CDTLMT":"400","CHGCOD":"1","BALDUE":"100.00","CDTDUE":".00"},
{"CUSNUM":"392859","LSTNAM":"Vine    ","INIT":"S S","STREET":"PO Box 79    ","CITY":"Broton","STATE":"VT","ZIPCOD":"5046","CDTLMT":"700","CHGCOD":"1","BALDUE":"439.00","CDTDUE":".00"},
{"CUSNUM":"392859","LSTNAM":"Vine    ","INIT":"S S","STREET":"PO Box 79    ","CITY":"Broton","STATE":"VT","ZIPCOD":"5046","CDTLMT":"700","CHGCOD":"1","BALDUE":"439.00","CDTDUE":".00"}
]}
```

Space delimter output (-o space)

```
bash-4.3$ db2util "select * from QIWS/QCUSTCDT where LSTNAM=? or LSTNAM=?" -p Jones Vine -o space
"839283" "Jones   " "B D" "21B NW 135 St" "Clay  " "NY" "13041" "400" "1" "100.00" ".00"
"392859" "Vine    " "S S" "PO Box 79    " "Broton" "VT" "5046" "700" "1" "439.00" ".00"
"392859" "Vine    " "S S" "PO Box 79    " "Broton" "VT" "5046" "700" "1" "439.00" ".00"
```

call XMLSERVICE (cmd):

```
bash-4.3$ db2util "DSPLIBL" -xc
"<?xml version='1.0'?>
<xmlservice>
<sh>
 5770SS1 V7R1M0  100423                    Library List                                          8/18/16 15:48:38        Page    1
                          ASP
   Library     Type       Device      Text Description
   QSYS        SYS                    System Library
   QSYS2       SYS                    System Library for CPI's
   QHLPSYS     SYS
   QUSRSYS     SYS                    System Library for Users
   QGPL        USR                    General Purpose Library
   QTEMP       USR
   QDEVELOP    USR
   QBLDSYS     USR
   QBLDSYSR    USR
                          * * * * *  E N D  O F  L I S T I N G  * * * * *
</sh>
</xmlservice>"
```

#Contributors
- Tony Cairns, IBM
- Aaron Bartell, Krengel Technology, Inc.

#License
MIT


#------IMPORTANT WARNING -------
This project is under construction.


#db2util

Welcome to the db2util project. Goal is PASE DB2 CLI command line interface driver (libdb400.a).

Aaron asked about PASE db2 utility similar to the one in ILE (qsh -> db2).

#Compile

```
$ cd db2util
$ make
gcc -g -I. -I/usr/include -I.. -c db2util.c
gcc -g db2util.o -L. -lpthreads -liconv -ldl -lpthread -ldb400 -o db2util
```

#Compiled version

You may try pre-compiled test version at Yips link (V7r1+).

* http://yips.idevcloud.com/wiki/index.php/Databases/DB2Util

#Run

```
bash-4.3$ db2util "select * from QIWS/QCUSTCDT where LSTNAM='Jones' or LSTNAM='Vine'"
"839283","Jones   ","B D","21B NW 135 St","Clay  ","NY","13041","400","1","100.00",".00"
"392859","Vine    ","S S","PO Box 79    ","Broton","VT","5046","700","1","439.00",".00"
"392859","Vine    ","S S","PO Box 79    ","Broton","VT","5046","700","1","439.00",".00"

bash-4.3$ db2util "select * from QIWS/QCUSTCDT where LSTNAM='Jones' or LSTNAM='Vine'" space
"839283" "Jones   " "B D" "21B NW 135 St" "Clay  " "NY" "13041" "400" "1" "100.00" ".00"
"392859" "Vine    " "S S" "PO Box 79    " "Broton" "VT" "5046" "700" "1" "439.00" ".00"
"392859" "Vine    " "S S" "PO Box 79    " "Broton" "VT" "5046" "700" "1" "439.00" ".00"

bash-4.3$ db2util "select * from QIWS/QCUSTCDT where LSTNAM='Jones' or LSTNAM='Vine'" json 
{"records":[
{"CUSNUM":"839283","LSTNAM":"Jones   ","INIT":"B D","STREET":"21B NW 135 St","CITY":"Clay  ","STATE":"NY","ZIPCOD":"13041","CDTLMT":"400","CHGCOD":"1","BALDUE":"100.00","CDTDUE":".00"},
{"CUSNUM":"392859","LSTNAM":"Vine    ","INIT":"S S","STREET":"PO Box 79    ","CITY":"Broton","STATE":"VT","ZIPCOD":"5046","CDTLMT":"700","CHGCOD":"1","BALDUE":"439.00","CDTDUE":".00"},
{"CUSNUM":"392859","LSTNAM":"Vine    ","INIT":"S S","STREET":"PO Box 79    ","CITY":"Broton","STATE":"VT","ZIPCOD":"5046","CDTLMT":"700","CHGCOD":"1","BALDUE":"439.00","CDTDUE":".00"}
]}
```

##Future:

```
DB2:
  /* TODO:
   * if we have args, switch to use prepare/excute
   * ("select * from QIWS/QCUSTCDT where LSTNAM=? or LSTNAM=?")
   */


XMLSERVICE:
  /* TODO:
   * Add an xmlservice interface using Db2 connection (iplugr)
   */
```


#Contributors
- Tony Cairns, IBM
- Aaron Bartell, Krengel Technology, Inc.

#License
MIT


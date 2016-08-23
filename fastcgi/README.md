#db2util/fastcgi

==Experimental==

REST interface to db2util. Results return JSON format.

# json syntax
```
 *************************************************
 example: 
 http://myibmi/db2/db2util.pgm?json="
 {
 "query":"select * from QIWS/QCUSTCDT where LSTNAM=?",
 "parm":["Jones"]
 }"
 -- or (qxmlserv) --
 http://myibmi/db2/db2util.pgm?json="
 {
 "cmd":"CRTLIB LIB(db2util) TYPE(*PROD) TEXT('frog')"
 }"
 -- or --
 http://myibmi/db2/db2util.pgm?json="
 {
 "cmd":"dsplibl",
 "xlib":"xmlservice"
 }"
*************************************************
```

# compile

```
$ cd db2util/fastcgi
$ gunzip fcgi.tar.gz
$ tar -xf fcgi.tar
$ make
```
(*) Above compile assumes copy of fcgi project (see Makefile)


#Compiled version

You may try pre-compiled test version db2util at Yips link (V7r1+).

* http://yips.idevcloud.com/wiki/index.php/Databases/DB2Util


# configure db2fcgi (rpg cgi for db2util)

1) You should have latest version of db2fcgi in main root machine.
```
bash-4.3$ ls -l /QOpenSys/usr/bin/db2fcgi 
-rwxr-xr-x    1 adc      0            306554 Aug 22 17:04 /QOpenSys/usr/bin/db2fcgi
```

2) You should have latest version of db2util in main root machine.
```
bash-4.3$ ls -l /QOpenSys/usr/bin/db2util 
-rwxr-xr-x    1 adc      0            123359 Aug 18 15:44 /QOpenSys/usr/bin/db2util
```

# db2util/rpgcgi/test
```
htmltest.html -- json query using html
curltest.sh -- json query using curl
```
Note: Some tests assume Basic Authorization (see httpd.conf).





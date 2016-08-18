#db2util/rpgcgi

==Experimental==

Apache REST json interface to db2util. 

# compile

```
$ cd db2util/rpgcgi
$ ./makedb2util.sh
```
Note: makedb2util.sh script uses db2util. 
Allows script to work from a chroot (Litmis Space).
You should have latest version of db2util in main root machine.
```
bash-4.3$ ls -l /QOpenSys/usr/bin/db2util 
-rwxr-xr-x    1 adc      0            123359 Aug 18 15:44 /QOpenSys/usr/bin/db2util
```

#Compiled version

You may try pre-compiled test version db2util at Yips link (V7r1+).

* http://yips.idevcloud.com/wiki/index.php/Databases/DB2Util


# configure db2util.pgm (rpg cgi for db2util)

```
ScriptAlias /db2/ /QSYS.LIB/DB2UTIL.LIB/
<Directory /QSYS.LIB/DB2UTIL.LIB/>
  AllowOverride None
  order allow,deny
  allow from all
  SetHandler cgi-script
  Options +ExecCGI
  CGIConvMode BINARY
</Directory>
```
Note: Must use CGIConvMode BINARY for directory.
You should have latest version of db2util in main root machine.
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


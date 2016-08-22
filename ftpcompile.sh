# bash ftpcompile.sh ut28p63 adc
MYPWD=$(<$HOME/.ftprc)
ftp -i -n -v $1 << ftp_end
user $2 $MYPWD

quote namefmt 1

mkdir /QOpenSys/zend7/home/zend7/db2util
mkdir /QOpenSys/zend7/home/zend7/db2util/rpgcgi


bin
cd /QOpenSys/zend7/home/zend7/db2util
mput *
lcd rpgcgi
cd /QOpenSys/zend7/home/zend7/db2util/rpgcgi
mput *
lcd ../fastcgi
cd /QOpenSys/zend7/home/zend7/db2util/fastcgi
mput *

quit

ftp_end


#!/bin/sh
MYCHRROT="/QOpenSys/zend7/home/zend7/db2util/rpgcgi/"
RPGLIB="db2util"
RPGFILES='iconv ipase db2util'
CMD="CRTLIB LIB($RPGLIB) TYPE(*PROD) TEXT('frog')"
db2util "$CMD" -xc
for i in $RPGFILES ; do
  echo '===================================='
  echo "==> $RPGLIB/$i ..."
  RES=$(db2util "CRTRPGMOD MODULE($RPGLIB/$i) SRCSTMF('$MYCHRROT$i.rpgle') DBGVIEW(*SOURCE) OUTPUT(*PRINT) REPLACE(*YES)" -xc)
  OK=$(echo "$RES" | grep -c "00 highest severity")
  if (($OK==0))
  then
    echo "$RES"
    echo "==> $RPGLIB/$i -- $OK of 00 highest severity"
    BAD=$(echo "$RES" | grep "*RNF")
    echo '===================================='
    echo '===================================='
    echo "$BAD"
    echo '===================================='
    echo '===================================='
    exit
  else
    echo "==> $RPGLIB/$i -- 00 highest severity"
  fi
  echo '===================================='
done
RPGPGM="CRTPGM PGM($RPGLIB/$RPGLIB) MODULE("
for i in $RPGFILES ; do
  RPGPGM="$RPGPGM $RPGLIB/$i"
done
RPGPGM="$RPGPGM) BNDSRVPGM(QHTTPSVR/QZSRCORE)"
echo "$RPGPGM\n";
db2util "$RPGPGM" -xc


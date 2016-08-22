#!/bin/sh
SQL="{\"query\":\"select * from QIWS/QCUSTCDT where LSTNAM=? or LSTNAM=?\",\"parm\":[\"Jones\",\"Vine\"]}"
echo "================="
echo "$SQL"
echo "================="

curl -u "$SQL_UID400:$SQL_PWD400" \
-G -v "http://ut28p63/ibm.db2" \
--data-urlencode json="$SQL"

echo "\n"


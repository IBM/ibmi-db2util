#!/bin/sh
SQL="{\"cmd\":\"dsplibl\",\"xlib\":\"qxmlserv\"}"
echo "================="
echo "$SQL"
echo "================="

curl -u "$SQL_UID400:$SQL_PWD400" \
-G -v "http://ut28p63/db2/db2util.pgm" \
--data-urlencode json="$SQL"

echo "\n"


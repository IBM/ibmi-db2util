#ifndef DB2UTIL_H
#define DB2UTIL_H

#include <as400_protos.h>
#include <sqlcli1.h>
#include <stddef.h>

typedef struct {
  SQLSMALLINT type;
  SQLSMALLINT bind_type;
  SQLINTEGER ind;
  char* buffer;
  char* data;
  size_t buffer_length;
  char name[129];
} col_info_t;

#endif

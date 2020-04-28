#ifndef DB2UTIL_H
#define DB2UTIL_H

#include <as400_protos.h>
#include <sqlcli1.h>
#include <stddef.h>

#define DB2UTIL_VERSION "1.0.10"
#define DB2UTIL_MAX_ARGS 32

typedef struct {
  SQLSMALLINT type;
  SQLSMALLINT bind_type;
  SQLINTEGER ind;
  char* buffer;
  char* data;
  size_t buffer_length;
  char name[129];
} col_info_t;

typedef void* (*format_new_t)(void);
typedef void (*format_free_t)(void*);
typedef void (*format_row_t)(FILE*, void*, col_info_t*, int);
typedef void (*format_start_rows_t)(FILE*, void*);
typedef void (*format_end_rows_t)(FILE*, void*);


typedef struct {
    format_new_t new;
    format_free_t free;
    format_row_t row;
    format_start_rows_t start_rows;
    format_end_rows_t end_rows;
} format_t;

#endif

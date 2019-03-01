#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "db2util.h"

#define DB2UTIL_VERSION "1.0.7 beta"

#define DB2UTIL_MAX_ARGS 32

enum {
  FORMAT_JSON,
  FORMAT_CSV,
  FORMAT_SPACE
};

void db2util_output_record_array_beg(int fmt) {
  switch (fmt) {
  case FORMAT_JSON:
    printf("{\"records\":[");
    break;
  case FORMAT_SPACE:
    break;
  case FORMAT_CSV:
  default:
    break;
  }
}
void db2util_output_record_row_beg(int fmt, int flag) {
  switch (fmt) {
  case FORMAT_JSON:
    if (flag) {
      printf(",\n{");
    } else {
      printf("\n{");
    }
    break;
  case FORMAT_SPACE:
    break;
  case FORMAT_CSV:
  default:
    break;
  }
}
void db2util_output_record_name_value(int fmt, int flag, char *name, char *value) {
  switch (fmt) {
  case FORMAT_JSON:
    if (flag) {
      printf(",");
    }
    printf("\"%s\":\"%s\"",name,value);
    break;
  case FORMAT_SPACE:
    if (flag) {
      printf(" ");
    }
    printf("\"%s\"",value);
    break;
  case FORMAT_CSV:
  default:
    if (flag) {
      printf(",");
    }
    printf("\"%s\"",value);
    break;
  }
}
void db2util_output_record_row_end(int fmt) {
  switch (fmt) {
  case FORMAT_JSON:
    printf("}");
    break;
  case FORMAT_SPACE:
    printf("\n");
    break;
  case FORMAT_CSV:
  default:
    printf("\n");
    break;
  }
}
void db2util_output_record_array_end(int fmt) {
  switch (fmt) {
  case FORMAT_JSON:
    printf("\n]}\n");
    break;
  case FORMAT_SPACE:
    printf("\n");
    break;
  case FORMAT_CSV:
  default:
    printf("\n");
    break;
  }
}

/*
  check_error(henv, SQL_HANDLE_ENV,   rc);
  check_error(hdbc, SQL_HANDLE_DBC,   rc);
  check_error(hstmt, SQL_HANDLE_STMT, rc);
*/
static void check_error(SQLHANDLE handle, SQLSMALLINT hType, SQLRETURN rc)
{
  SQLCHAR msg[SQL_MAX_MESSAGE_LENGTH + 1];
  SQLCHAR sqlstate[SQL_SQLSTATE_SIZE+1];
  SQLINTEGER sqlcode = 0;
  SQLSMALLINT length = 0;

  if (rc != SQL_ERROR) return;

  memset(msg, '\0', sizeof(msg));
  memset(sqlstate, '\0', sizeof(sqlstate));

  if ( SQLGetDiagRec(hType, handle, 1, sqlstate, &sqlcode, msg, sizeof(msg), &length) != SQL_SUCCESS ) {
    fprintf(stderr, "Critical error: couldn't retrieve error info\n");
    exit(1);
  }

  char* p;
  while ((p = strchr(msg, '\n'))) {
    *p = '\0';
  }
  
  fprintf(stderr, "%s SQLSTATE=%s SQLCODE=%d\n", msg, sqlstate, (int)sqlcode);
  exit(1);
}

static inline void set_env_attrs(SQLHENV henv) {
  SQLRETURN rc;
  SQLINTEGER attr = SQL_TRUE;

  rc = SQLSetEnvAttr(henv, SQL_ATTR_SERVER_MODE, &attr, 0);
  check_error(henv, SQL_HANDLE_ENV, rc);

  attr = SQL_FALSE;
  rc = SQLSetEnvAttr(henv, SQL_ATTR_INCLUDE_NULL_IN_LEN, &attr, 0);
  check_error(henv, SQL_HANDLE_ENV, rc);
}

static inline void set_conn_attrs(SQLHDBC hdbc) {
  SQLRETURN rc;
  SQLINTEGER attr = SQL_TRUE;

  rc = SQLSetConnectAttr(hdbc, SQL_ATTR_DBC_SYS_NAMING, &attr, 0);
  check_error(hdbc, SQL_HANDLE_DBC, rc);

  rc = SQLSetConnectAttr(hdbc, SQL_ATTR_NULLT_ARRAY_RESULTS, &attr, 0);
  check_error(hdbc, SQL_HANDLE_DBC, rc);

  rc = SQLSetConnectAttr(hdbc, SQL_ATTR_NULLT_OUTPUT_PARMS, &attr, 0);
  check_error(hdbc, SQL_HANDLE_DBC, rc);

  attr = SQL_TXN_NO_COMMIT;
  rc = SQLSetConnectAttr(hdbc, SQL_ATTR_TXN_ISOLATION, &attr, 0);
  check_error(hdbc, SQL_HANDLE_DBC, rc);
}

int db2util_query(char * stmt_str, int fmt, int argc, char *argv[]) {
  SQLRETURN rc = 0;
  SQLHENV henv = 0;
  SQLHDBC hdbc = 0;
  SQLHSTMT hstmt = 0;
  SQLSMALLINT param_count = 0;
  SQLSMALLINT column_count = 0;


  SQLOverrideCCSID400(1208);

  /* env */
  rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
  check_error(henv, SQL_HANDLE_ENV, rc);
  set_env_attrs(henv);

  /* connect */
  rc = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
  check_error(hdbc, SQL_HANDLE_DBC, rc);
  
  rc = SQLConnect(hdbc, NULL, 0, NULL, 0, NULL, 0);
  check_error(hdbc, SQL_HANDLE_DBC, rc);
  set_conn_attrs(hdbc);


  /* statement */
  rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
  check_error(hstmt, SQL_HANDLE_STMT, rc);

  /* prepare */
  rc = SQLPrepare(hstmt, (SQLCHAR*)stmt_str, SQL_NTS);
  check_error(hstmt, SQL_HANDLE_STMT, rc);

  /* number of input parms */
  rc = SQLNumParams(hstmt, &param_count);
  check_error(hstmt, SQL_HANDLE_STMT, rc);

  if (param_count != argc) {
    fprintf(stderr, "Invalid parameter count: expected %d got %d\n", param_count, argc);
    exit(1);
  }
  
  SQLINTEGER input_indicator = SQL_NTS;
  for (int i = 0; i < param_count; i++) {
    SQLSMALLINT type;
    SQLUINTEGER precision;
    SQLSMALLINT scale;
    SQLSMALLINT nullable;

    rc = SQLDescribeParam(hstmt, i+1, &type, &precision, &scale, &nullable);
    check_error(hstmt, SQL_HANDLE_STMT, rc);

    rc = SQLBindParameter(hstmt, i+1, SQL_PARAM_INPUT, SQL_C_CHAR, type,
                          precision, scale, argv[i], 0, &input_indicator);
    check_error(hstmt, SQL_HANDLE_STMT, rc);
  }

  /* execute */
  rc = SQLExecute(hstmt);
  check_error(hstmt, SQL_HANDLE_STMT, rc);

  /* result set */
  rc = SQLNumResultCols(hstmt, &column_count);
  check_error(hstmt, SQL_HANDLE_STMT, rc);

  if (column_count < 1) exit(0);

  col_info_t* cols = malloc(sizeof(col_info_t) * column_count);
 
  for (int i = 0 ; i < column_count; i++) {
    col_info_t* col = &cols[i];

    SQLUINTEGER size;
    SQLSMALLINT ignore;

    rc = SQLDescribeCol(hstmt, i+1, col->name, sizeof(col->name), &ignore, &col->type, &size, &ignore, &ignore);
    check_error(hstmt, SQL_HANDLE_STMT, rc);

    switch (col->type) {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_CLOB:
    case SQL_DBCLOB:
    case SQL_UTF8_CHAR:
    case SQL_WCHAR:
    case SQL_WVARCHAR:
    case SQL_GRAPHIC:
    case SQL_VARGRAPHIC:
    case SQL_XML:
      col->bind_type = SQL_C_CHAR;
      col->buffer_length = size * 3;
      col->buffer = col->data = malloc(col->buffer_length);
      break;

    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_BLOB:
      col->bind_type = SQL_C_CHAR;
      col->buffer_length = size * 3;
      col->buffer = col->data = malloc(col->buffer_length);
      break;

    case SQL_TYPE_DATE:
    case SQL_TYPE_TIME:
    case SQL_TYPE_TIMESTAMP:
    case SQL_DATETIME:
    case SQL_BIGINT:
    case SQL_DECFLOAT:
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    default:
      col->bind_type = SQL_C_CHAR;
      col->buffer_length = 100;
      col->buffer = col->data = malloc(col->buffer_length);
      break;
    }

    rc = SQLBindCol(hstmt, i+1, col->bind_type, col->buffer, col->buffer_length, &col->ind);
    check_error(hstmt, SQL_HANDLE_STMT, rc);
  }

  db2util_output_record_array_beg(fmt);

  int recs = 0;
  while ((rc = SQLFetch(hstmt)) == SQL_SUCCESS) {

    db2util_output_record_row_beg(fmt, recs);

    for (int i = 0 ; i < column_count; i++) {
      col_info_t* col = &cols[i];

      db2util_output_record_name_value(fmt,i,col->name,col->buffer);
    }
    db2util_output_record_row_end(fmt);

    recs += 1;
  }
  db2util_output_record_array_end(fmt);

  for (int i = 0 ; i < column_count; i++) {
    free(cols[i].buffer);
  }
  free(cols);

  rc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  rc = SQLDisconnect(hdbc);
  rc = SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
  /* SQLFreeHandle(SQL_HANDLE_ENV, henv); */
  return rc;
}

void db2util_help() {
  printf("Syntax: db2util [-h -o [json|comma|space] -p parm1 -p parm2 ...] 'sql statement'\n");
  printf("-h      help\n");
  printf("-o [json|comma|space]\n");
  printf(" json  - {\"records\":[{\"name\"}:{\"value\"},{\"name\"}:{\"value\"},...]}\n");
  printf(" comma - \"value\",\"value\",...\n");
  printf(" space - \"value\" \"value\" ...\n");
  printf("-p parm1 \n");
  printf("Version: %s\n", DB2UTIL_VERSION);
  printf("\nExample (DB2)\n");
  printf("db2util \"select * from QIWS/QCUSTCDT where LSTNAM='Jones' or LSTNAM='Vine'\"\n");
  printf("db2util \"select * from QIWS/QCUSTCDT where LSTNAM=? or LSTNAM=?\" -p Jones Vine -o json\n");
  printf("db2util \"select * from QIWS/QCUSTCDT where LSTNAM=? or LSTNAM=?\" -p Jones Vine -o space\n");
}

int main(int argc, char* const* argv) {
  int parm_count = 0;
  static const char* parms[DB2UTIL_MAX_ARGS];
  int format = FORMAT_CSV;

  int opt;
  while ((opt = getopt(argc, argv, "ho:p:")) != -1) {
    switch(opt) {
    case 'h':
      db2util_help();
      return 0;

    case 'o':
      if (strcmp(optarg, "json") == 0) {
        format = FORMAT_JSON;
      }
      else if (strcmp(optarg, "comma") == 0 ||
               strcmp(optarg, "csv") == 0) {
        format = FORMAT_CSV;
      }
      else if (strcmp(optarg, "space") == 0) {
        format = FORMAT_SPACE;
      }
      else {
        db2util_help();
        return 1;
      }
      break;

    case 'p':
      if (parm_count == DB2UTIL_MAX_ARGS) {
        fprintf(stderr, "Exceeded the max # of input arguments (%d)\n", DB2UTIL_MAX_ARGS);
        return 1;
      }

      parms[parm_count++] = optarg;
      break;

    case '?':
    case ':':
      db2util_help();
      return 1;
    }
  }

  if (argc - optind != 1) {
    db2util_help();
    return 1;
  }

  db2util_query(argv[optind], format, parm_count, parms);

  return 0;
}

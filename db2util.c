#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "db2util.h"

#include "format_json.h"
#include "format_csv.h"
#include "format_space.h"

enum {
  FORMAT_JSON,
  FORMAT_CSV,
  FORMAT_SPACE
};

format_t* formats[] = {
  &json_format,
  &csv_format,
  &space_format
};

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

static int db2util_query(char* stmt, int fmt, int argc, const char* argv[]) {
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
  rc = SQLPrepare(hstmt, (SQLCHAR*)stmt, SQL_NTS);
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
    SQLINTEGER precision;
    SQLSMALLINT scale;
    SQLSMALLINT nullable;

    rc = SQLDescribeParam(hstmt, i+1, &type, &precision, &scale, &nullable);
    check_error(hstmt, SQL_HANDLE_STMT, rc);

    rc = SQLBindParameter(hstmt, i+1, SQL_PARAM_INPUT, SQL_C_CHAR, type,
                          precision, scale, (SQLCHAR*) argv[i], 0, &input_indicator);
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

    SQLINTEGER size;
    SQLSMALLINT ignore;

    rc = SQLDescribeCol(hstmt, i+1, col->name, sizeof(col->name), &ignore, &col->type, &size, &ignore, &ignore);
    check_error(hstmt, SQL_HANDLE_STMT, rc);

    size_t bind_length;
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
    default:
      col->bind_type = SQL_C_CHAR;
      col->buffer_length = bind_length = size * 3;
      col->buffer = col->data = malloc(col->buffer_length);
      break;

    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_BLOB:
      col->bind_type = SQL_C_CHAR;
      col->buffer_length = bind_length = size * 3;
      col->buffer = col->data = malloc(col->buffer_length);
      break;

    case SQL_DECFLOAT:
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
      col->bind_type = SQL_C_CHAR;
      col->buffer_length = 100;
      col->buffer = malloc(col->buffer_length);
      col->data = col->buffer + 1;
      bind_length = col->buffer_length - 1;
      break;

    case SQL_TYPE_DATE:
    case SQL_TYPE_TIME:
    case SQL_TYPE_TIMESTAMP:
    case SQL_DATETIME:
    case SQL_BIGINT:
    case SQL_SMALLINT:
    case SQL_INTEGER:
      col->bind_type = SQL_C_CHAR;
      col->buffer_length = bind_length = 100;
      col->buffer = col->data = malloc(col->buffer_length);
      break;
    }

    rc = SQLBindCol(hstmt, i+1, col->bind_type, col->data, bind_length, &col->ind);
    check_error(hstmt, SQL_HANDLE_STMT, rc);
  }

  format_t* format = formats[fmt];

  void* state = format->new();
  format->start_rows(stdout, state);

  while ((rc = SQLFetch(hstmt)) == SQL_SUCCESS) {
    for (int i = 0; i < column_count; ++i) {
      col_info_t* col = cols + i;

      switch(col->type) {
      case SQL_DECIMAL:
      case SQL_NUMERIC:
      case SQL_DECFLOAT:
      case SQL_REAL:
      case SQL_FLOAT:
      case SQL_DOUBLE:
        if (col->ind == SQL_NULL_DATA) break;
        if (col->buffer[1] == '.') {
          col->buffer[0] = '0';
          col->buffer[1] = '.';
          col->data = col->buffer;
          col->ind++;
        }
        else if (col->buffer[1] == '-' && col->buffer[2] == '.') {
          col->buffer[0] = '-';
          col->buffer[1] = '0';
          col->data = col->buffer;
          col->ind++;
        }
        else {
          col->data = col->buffer + 1;
        }
        break;

      default:
        break;
      }
    }

    format->row(stdout, state, cols, column_count);

    for (int i = 0; i < column_count; ++i) {
      cols[i].ind = cols[i].buffer_length;
    }
  }

  format->end_rows(stdout, state);
  format->free(state);

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

#define STR(s) #s
#define XSTR(s) STR(s)

static void exit_with_usage(const char* program, int rc) {
  printf("%s [options] <sql statement>\n", program);
  printf("Options:\n");
  printf("  -o <fmt>                   Output format. Value values for fmt:\n");
  printf("                               json:  [{\"name\"}:{\"value\"},{\"name\"}:{\"value\"},...]\n");
  printf("                               csv:   \"value\",\"value\",...\n");
  printf("                               space: \"value\" \"value\" ...\n");
  printf("  -p <parm>                  Input parameter(s) (max " XSTR(DB2UTIL_MAX_ARGS) ")\n");
  printf("  -h                         This help text\n");
  printf("  -v                         Show version number and quit\n");
  printf("\nExamples:\n");
  printf("%s \"select * from QIWS.QCUSTCDT where LSTNAM='Jones' or LSTNAM='Vine'\"\n", program);
  printf("%s -p Jones -p Vine -o json \"select * from QIWS.QCUSTCDT where LSTNAM=? or LSTNAM=?\"\n", program);
  printf("%s -p Jones -p Vine -o space \"select * from QIWS.QCUSTCDT where LSTNAM=? or LSTNAM=?\"\n", program);

  exit(rc);
}

int main(int argc, char* const* argv) {
  int parm_count = 0;
  static const char* parms[DB2UTIL_MAX_ARGS];
  int format = FORMAT_CSV;

  int opt;
  while ((opt = getopt(argc, argv, "hvo:p:")) != -1) {
    switch(opt) {
    case 'h':
      exit_with_usage(argv[0], 0);

    case 'v':
      printf("db2util " DB2UTIL_VERSION "\n");
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
        exit_with_usage(argv[0], 1);
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
      exit_with_usage(argv[0], 1);
    }
  }

  if (argc - optind != 1) {
    exit_with_usage(argv[0], 1);
  }

  db2util_query(argv[optind], format, parm_count, parms);

  return 0;
}

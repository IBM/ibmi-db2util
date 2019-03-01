#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <as400_protos.h>
#include <sqlcli1.h>

#define DB2UTIL_VERSION "1.0.7 beta"

#define DB2UTIL_MAX_ARGS 32
#define DB2UTIL_MAX_COLS 1024
#define DB2UTIL_MAX_ERR_MSG_LEN (SQL_MAX_MESSAGE_LENGTH + SQL_SQLSTATE_SIZE + 10)

#define DB2UTIL_EXPAND_CHAR 3
#define DB2UTIL_EXPAND_BINARY 2
#define DB2UTIL_EXPAND_OTHER 64
#define DB2UTIL_EXPAND_COL_NAME 128

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

int db2util_query(char * stmt_str, int fmt, int argc, char *argv[]) {
  int recs = 0;
  int rc = 0;
  SQLHENV henv = 0;
  SQLHDBC hdbc = 0;
  SQLHSTMT hstmt = 0;
  SQLINTEGER attr = SQL_TRUE;
  SQLINTEGER attr_isolation = SQL_TXN_NO_COMMIT;
  SQLSMALLINT nParms = 0;
  SQLSMALLINT nResultCols = 0;
  SQLSMALLINT name_length = 0;
  SQLCHAR *buff_name[DB2UTIL_MAX_COLS];
  SQLCHAR *buff_value[DB2UTIL_MAX_COLS];
  SQLINTEGER buff_len[DB2UTIL_MAX_COLS];
  SQLSMALLINT type = 0;
  SQLUINTEGER size = 0;
  SQLSMALLINT scale = 0;
  SQLSMALLINT nullable = 0;
  SQLINTEGER lob_loc = 0;
  SQLINTEGER loc_ind = 0;
  SQLSMALLINT loc_type = 0;
  SQLINTEGER fStrLen = SQL_NTS;
  SQLSMALLINT sql_data_type = 0;
  SQLUINTEGER sql_precision = 0;
  SQLSMALLINT sql_scale = 0;
  SQLSMALLINT sql_nullable = SQL_NO_NULLS;

  /* init */
  for (int i=0;i < DB2UTIL_MAX_COLS;i++) {
    buff_name[i] = NULL;
    buff_value[i] = NULL;
    buff_len[i] = 0;
  }

  SQLOverrideCCSID400(1208);

  /* env */
  rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
  rc = SQLSetEnvAttr(henv, SQL_ATTR_SERVER_MODE, &attr, 0);
  rc = SQLSetEnvAttr(henv, SQL_ATTR_INCLUDE_NULL_IN_LEN, &attr, 0);

  /* connect */
  rc = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
  check_error(hdbc, SQL_HANDLE_DBC, rc);
  
  rc = SQLConnect(hdbc, NULL, 0, NULL, 0, NULL, 0);
  check_error(hdbc, SQL_HANDLE_DBC, rc);

  rc = SQLSetConnectAttr(hdbc, SQL_ATTR_DBC_SYS_NAMING, &attr, 0);
  check_error(hdbc, SQL_HANDLE_DBC, rc);

  rc = SQLSetConnectAttr(hdbc, SQL_ATTR_TXN_ISOLATION, &attr_isolation, 0);
  check_error(hdbc, SQL_HANDLE_DBC, rc);

  /* statement */
  rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
  check_error(hstmt, SQL_HANDLE_STMT, rc);

  /* prepare */
  rc = SQLPrepare(hstmt, (SQLCHAR*)stmt_str, SQL_NTS);
  check_error(hstmt, SQL_HANDLE_STMT, rc);

  /* number of input parms */
  rc = SQLNumParams(hstmt, &nParms);
  check_error(hstmt, SQL_HANDLE_STMT, rc);

  if (nParms > 0) {
    for (int i = 0; i < nParms; i++) {
      rc = SQLDescribeParam(hstmt, i+1, 
             &sql_data_type, &sql_precision, &sql_scale, &sql_nullable);
      check_error(hstmt, SQL_HANDLE_STMT, rc);

      buff_len[i] = SQL_NTS;
      rc = SQLBindParameter(hstmt, i+1,
             SQL_PARAM_INPUT, SQL_CHAR, sql_data_type,
             sql_precision, sql_scale, argv[i], 0, &buff_len[i]);
      check_error(hstmt, SQL_HANDLE_STMT, rc);

    }
  }
  /* execute */
  rc = SQLExecute(hstmt);
  check_error(hstmt, SQL_HANDLE_STMT, rc);

  /* result set */
  rc = SQLNumResultCols(hstmt, &nResultCols);
  check_error(hstmt, SQL_HANDLE_STMT, rc);

  if (nResultCols > 0) {
    for (int i = 0 ; i < nResultCols; i++) {
      size = DB2UTIL_EXPAND_COL_NAME;
      buff_name[i] = malloc(size);
      buff_value[i] = NULL;
      rc = SQLDescribeCol(hstmt, i+1, buff_name[i], size, &name_length, &type, &size, &scale, &nullable);
      check_error(hstmt, SQL_HANDLE_STMT, rc);

      /* dbcs expansion */
      switch (type) {
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
        size = size * DB2UTIL_EXPAND_CHAR;
        buff_value[i] = malloc(size);
        rc = SQLBindCol(hstmt, i+1, SQL_CHAR, buff_value[i], size, &fStrLen);
        break;
      case SQL_BINARY:
      case SQL_VARBINARY:
      case SQL_BLOB:
        size = size * DB2UTIL_EXPAND_BINARY;
        buff_value[i] = malloc(size);
        rc = SQLBindCol(hstmt, i+1, SQL_CHAR, buff_value[i], size, &fStrLen);
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
        size = DB2UTIL_EXPAND_OTHER;
        buff_value[i] = malloc(size);
        rc = SQLBindCol(hstmt, i+1, SQL_CHAR, buff_value[i], size, &fStrLen);
        break;
      }
      check_error(hstmt, SQL_HANDLE_STMT, rc);
    }
    rc = SQL_SUCCESS;
    db2util_output_record_array_beg(fmt);
    while (rc == SQL_SUCCESS) {
      rc = SQLFetch(hstmt);
      if (rc == SQL_NO_DATA_FOUND) {
        break;
      }
      db2util_output_record_row_beg(fmt, recs);
      recs += 1;
      for (int i = 0 ; i < nResultCols; i++) {
        if (buff_value[i]) {
          db2util_output_record_name_value(fmt,i,buff_name[i],buff_value[i]);
        }
      }
      db2util_output_record_row_end(fmt);
    }
    db2util_output_record_array_end(fmt);
    for (int i = 0 ; i < nResultCols; i++) {
      if (buff_value[i]) {
        free(buff_name[i]);
        buff_name[i] = NULL;
      }
      if (buff_name[i]) {
        free(buff_name[i]);
        buff_name[i] = NULL;
      }
    }
  }
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

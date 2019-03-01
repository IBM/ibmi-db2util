#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <as400_protos.h>
#include <sqlcli1.h>

#define DB2UTIL_VERSION "1.0.7 beta"

#define SQL_IS_INTEGER 0

#define DB2UTIL_MAX_ARGS 32
#define DB2UTIL_MAX_COLS 1024
#define DB2UTIL_MAX_ERR_MSG_LEN (SQL_MAX_MESSAGE_LENGTH + SQL_SQLSTATE_SIZE + 10)

#define DB2UTIL_EXPAND_CHAR 3
#define DB2UTIL_EXPAND_BINARY 2
#define DB2UTIL_EXPAND_OTHER 64
#define DB2UTIL_EXPAND_COL_NAME 128

#define DB2UTIL_UNKNOWN -1

#define DB2UTIL_CMD_HELP 1
#define DB2UTIL_CMD_QUERY 2

#define DB2UTIL_OUT_FORMAT 10
#define DB2UTIL_OUT_COMMA 11
#define DB2UTIL_OUT_JSON 12
#define DB2UTIL_OUT_SPACE 13

#define DB2UTIL_ARG_INPUT 20

static char * db2util_name = "db2util";
static char * db2util_out_set_p = "-p";
static char * db2util_out_set_o = "-o";


int db2util_ccsid() {
  char * env_ccsid = getenv("CCSID");
  int ccsid = Qp2paseCCSID();
  if (env_ccsid) {
     ccsid = atoi(env_ccsid);
  }
  return ccsid;
}

void * db2util_new(int size) {
  void * buffer = malloc(size + 1);
  memset(buffer,0,size + 1);
  return buffer;
}

void db2util_free(char *buffer) {
  if (buffer) {
    free(buffer);
  }
}

void db2util_output_record_array_beg(int fmt) {
  switch (fmt) {
  case DB2UTIL_OUT_JSON:
    printf("{\"records\":[");
    break;
  case DB2UTIL_OUT_SPACE:
    break;
  case DB2UTIL_OUT_COMMA:
  default:
    break;
  }
}
void db2util_output_record_row_beg(int fmt, int flag) {
  switch (fmt) {
  case DB2UTIL_OUT_JSON:
    if (flag) {
      printf(",\n{");
    } else {
      printf("\n{");
    }
    break;
  case DB2UTIL_OUT_SPACE:
    break;
  case DB2UTIL_OUT_COMMA:
  default:
    break;
  }
}
void db2util_output_record_name_value(int fmt, int flag, char *name, char *value) {
  switch (fmt) {
  case DB2UTIL_OUT_JSON:
    if (flag) {
      printf(",");
    }
    printf("\"%s\":\"%s\"",name,value);
    break;
  case DB2UTIL_OUT_SPACE:
    if (flag) {
      printf(" ");
    }
    printf("\"%s\"",value);
    break;
  case DB2UTIL_OUT_COMMA:
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
  case DB2UTIL_OUT_JSON:
    printf("}");
    break;
  case DB2UTIL_OUT_SPACE:
    printf("\n");
    break;
  case DB2UTIL_OUT_COMMA:
  default:
    printf("\n");
    break;
  }
}
void db2util_output_record_array_end(int fmt) {
  switch (fmt) {
  case DB2UTIL_OUT_JSON:
    printf("\n]}\n");
    break;
  case DB2UTIL_OUT_SPACE:
    printf("\n");
    break;
  case DB2UTIL_OUT_COMMA:
  default:
    printf("\n");
    break;
  }
}

/*
  db2util_check_sql_errors(fmt, henv, SQL_HANDLE_ENV,   rc);
  db2util_check_sql_errors(fmt, hdbc, SQL_HANDLE_DBC,   rc);
  db2util_check_sql_errors(fmt, hstmt, SQL_HANDLE_STMT, rc);
*/
int db2util_check_sql_errors(int fmt, SQLHANDLE handle, SQLSMALLINT hType, int rc)
{
  SQLCHAR msg[SQL_MAX_MESSAGE_LENGTH + 1];
  SQLCHAR sqlstate[SQL_SQLSTATE_SIZE + 1];
  SQLCHAR errMsg[DB2UTIL_MAX_ERR_MSG_LEN];
  SQLINTEGER sqlcode = 0;
  SQLSMALLINT length = 0;
  SQLCHAR *p = NULL;
  SQLSMALLINT recno = 1;
  if (rc == SQL_ERROR) {
    memset(msg, '\0', SQL_MAX_MESSAGE_LENGTH + 1);
    memset(sqlstate, '\0', SQL_SQLSTATE_SIZE + 1);
    memset(errMsg, '\0', DB2UTIL_MAX_ERR_MSG_LEN);
    if ( SQLGetDiagRec(hType, handle, recno, sqlstate, &sqlcode, msg, SQL_MAX_MESSAGE_LENGTH + 1, &length)  == SQL_SUCCESS ) {
      if (msg[length-1] == '\n') {
        p = &msg[length-1];
        *p = '\0';
      }
    }
  }
  return SQL_SUCCESS;
}

int db2util_query(char * stmt_str, int fmt, int argc, char *argv[]) {
  int i = 0;
  int recs = 0;
  int rc = 0;
  int ccsid = db2util_ccsid();
  SQLHENV henv = 0;
  SQLHANDLE hdbc = 0;
  SQLHANDLE hstmt = 0;
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
  for (i=0;i < DB2UTIL_MAX_COLS;i++) {
    buff_name[i] = NULL;
    buff_value[i] = NULL;
    buff_len[i] = 0;
  }

  /* ccsid */
  rc = SQLOverrideCCSID400(ccsid);

  /* env */
  rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
  rc = SQLSetEnvAttr((SQLHENV)henv, SQL_ATTR_SERVER_MODE, (SQLPOINTER)&attr, SQL_IS_INTEGER);
  rc = SQLSetEnvAttr((SQLHENV)henv, SQL_ATTR_INCLUDE_NULL_IN_LEN, (SQLPOINTER)&attr, SQL_IS_INTEGER);
  if (ccsid == 1208) {
    rc = SQLSetEnvAttr((SQLHENV)henv, SQL_ATTR_UTF8, (SQLPOINTER)&attr, SQL_IS_INTEGER);
  }
  /* connect */
  rc = SQLAllocHandle( SQL_HANDLE_DBC, henv, &hdbc);
  if (db2util_check_sql_errors(fmt, hdbc, SQL_HANDLE_DBC,   rc) == SQL_ERROR) {
    return SQL_ERROR;
  }
  rc = SQLConnect( (SQLHDBC)hdbc, 
         (SQLCHAR *)NULL, (SQLSMALLINT)0,
         (SQLCHAR *)NULL, (SQLSMALLINT)0,
         (SQLCHAR *)NULL, (SQLSMALLINT)0);
  if (db2util_check_sql_errors(fmt, hdbc, SQL_HANDLE_DBC, rc) == SQL_ERROR) {
    return SQL_ERROR;
  }
  rc = SQLSetConnectAttr((SQLHDBC)hdbc, SQL_ATTR_DBC_SYS_NAMING, (SQLPOINTER)&attr, SQL_IS_INTEGER);
  if (db2util_check_sql_errors(fmt, hdbc, SQL_HANDLE_DBC, rc) == SQL_ERROR) {
    return SQL_ERROR;
  }
  rc = SQLSetConnectAttr((SQLHDBC)hdbc, SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)&attr_isolation, SQL_IS_INTEGER);
  if (db2util_check_sql_errors(fmt, hdbc, SQL_HANDLE_DBC, rc) == SQL_ERROR) {
    return SQL_ERROR;
  }
  /* statement */
  rc = SQLAllocHandle(SQL_HANDLE_STMT, (SQLHDBC) hdbc, &hstmt);
  if (db2util_check_sql_errors(fmt, hstmt, SQL_HANDLE_STMT, rc) == SQL_ERROR) {
    return SQL_ERROR;
  }
  /* prepare */
  rc = SQLPrepare((SQLHSTMT)hstmt, (SQLCHAR*)stmt_str, (SQLINTEGER)SQL_NTS);
  if (db2util_check_sql_errors(fmt, hstmt, SQL_HANDLE_STMT, rc) == SQL_ERROR) {
    return SQL_ERROR;
  }
  /* number of input parms */
  rc = SQLNumParams((SQLHSTMT)hstmt, (SQLSMALLINT*)&nParms);
  if (db2util_check_sql_errors(fmt, hstmt, SQL_HANDLE_STMT, rc) == SQL_ERROR) {
    return SQL_ERROR;
  }
  if (nParms > 0) {
    for (i = 0; i < nParms; i++) {
      rc = SQLDescribeParam((SQLHSTMT)hstmt, (SQLUSMALLINT)(i + 1), 
             &sql_data_type, &sql_precision, &sql_scale, &sql_nullable);
      if (db2util_check_sql_errors(fmt, hstmt, SQL_HANDLE_STMT, rc) == SQL_ERROR) {
        return SQL_ERROR;
      }
      buff_len[i] = SQL_NTS;
      rc = SQLBindParameter((SQLHSTMT)hstmt, (SQLUSMALLINT)(i + 1),
             SQL_PARAM_INPUT, SQL_CHAR, sql_data_type,
             sql_precision, sql_scale, argv[i], 0, &buff_len[i]);
      if (db2util_check_sql_errors(fmt, hstmt, SQL_HANDLE_STMT, rc) == SQL_ERROR) {
        return SQL_ERROR;
      }
    }
  }
  /* execute */
  rc = SQLExecute((SQLHSTMT)hstmt);
  if (db2util_check_sql_errors(fmt, hstmt, SQL_HANDLE_STMT, rc) == SQL_ERROR) {
    return SQL_ERROR;
  }
  /* result set */
  rc = SQLNumResultCols((SQLHSTMT)hstmt, &nResultCols);
  if (db2util_check_sql_errors(fmt, hstmt, SQL_HANDLE_STMT, rc) == SQL_ERROR) {
    return SQL_ERROR;
  }
  if (nResultCols > 0) {
    for (i = 0 ; i < nResultCols; i++) {
      size = DB2UTIL_EXPAND_COL_NAME;
      buff_name[i] = db2util_new(size);
      buff_value[i] = NULL;
      rc = SQLDescribeCol((SQLHSTMT)hstmt, (SQLSMALLINT)(i + 1), (SQLCHAR *)buff_name[i], size, &name_length, &type, &size, &scale, &nullable);
      if (db2util_check_sql_errors(fmt, hstmt, SQL_HANDLE_STMT, rc) == SQL_ERROR) {
        return SQL_ERROR;
      }
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
        buff_value[i] = db2util_new(size);
        rc = SQLBindCol((SQLHSTMT)hstmt, (i + 1), SQL_CHAR, buff_value[i], size, &fStrLen);
        break;
      case SQL_BINARY:
      case SQL_VARBINARY:
      case SQL_BLOB:
        size = size * DB2UTIL_EXPAND_BINARY;
        buff_value[i] = db2util_new(size);
        rc = SQLBindCol((SQLHSTMT)hstmt, (i + 1), SQL_CHAR, buff_value[i], size, &fStrLen);
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
        buff_value[i] = db2util_new(size);
        rc = SQLBindCol((SQLHSTMT)hstmt, (i + 1), SQL_CHAR, buff_value[i], size, &fStrLen);
        break;
      }
      if (db2util_check_sql_errors(fmt, hstmt, SQL_HANDLE_STMT, rc) == SQL_ERROR) {
        return SQL_ERROR;
      }
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
      for (i = 0 ; i < nResultCols; i++) {
        if (buff_value[i]) {
          db2util_output_record_name_value(fmt,i,buff_name[i],buff_value[i]);
        }
      }
      db2util_output_record_row_end(fmt);
    }
    db2util_output_record_array_end(fmt);
    for (i = 0 ; i < nResultCols; i++) {
      if (buff_value[i]) {
        db2util_free(buff_name[i]);
        buff_name[i] = NULL;
      }
      if (buff_name[i]) {
        db2util_free(buff_name[i]);
        buff_name[i] = NULL;
      }
    }
  }
  rc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  rc = SQLDisconnect((SQLHDBC)hdbc);
  rc = SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
  /* SQLFreeHandle(SQL_HANDLE_ENV, henv); */
  return rc;
}

void db2util_help(int fmt) {
  printf("Syntax: db2util 'sql statement' [-h -o [json|comma|space] -p parm1 parm2 ...]\n");
  printf("-h      help\n");
  printf("-o [json|comma|space]\n");
  printf(" json  - {\"records\":[{\"name\"}:{\"value\"},{\"name\"}:{\"value\"},...]}\n");
  printf(" comma - \"value\",\"value\",...\n");
  printf(" space - \"value\" \"value\" ...\n");
  printf("-p parm1 parm2 ...\n");
  printf("Version: %s\n", DB2UTIL_VERSION);
  printf("\nExample (DB2)\n");
  printf("db2util \"select * from QIWS/QCUSTCDT where LSTNAM='Jones' or LSTNAM='Vine'\"\n");
  printf("db2util \"select * from QIWS/QCUSTCDT where LSTNAM=? or LSTNAM=?\" -p Jones Vine -o json\n");
  printf("db2util \"select * from QIWS/QCUSTCDT where LSTNAM=? or LSTNAM=?\" -p Jones Vine -o space\n");
}

int db2util_hash_key(char * str) {
  int key = DB2UTIL_UNKNOWN;
  if (strcmp(str,"-h") == 0) {
    key = DB2UTIL_CMD_HELP;
  } else if (strcmp(str,"-o") == 0) {
    key = DB2UTIL_OUT_FORMAT;
  } else if (strcmp(str,"json") == 0) {
    key = DB2UTIL_OUT_JSON;
  } else if (strcmp(str,"comma") == 0) {
    key = DB2UTIL_OUT_COMMA;
  } else if (strcmp(str,"space") == 0) {
    key = DB2UTIL_OUT_SPACE;
  } else if (strcmp(str,"-p") == 0) {
    key = DB2UTIL_ARG_INPUT;
  }
  return key;
}

int main(int argc, char *argv[]) {
  SQLRETURN rc = 0;
  int i = 0;
  int iargc = 0;
  char *iargv[DB2UTIL_MAX_ARGS];
  int command = DB2UTIL_CMD_HELP;
  int fmt = DB2UTIL_OUT_COMMA;
  int have = DB2UTIL_UNKNOWN;
  char * query = NULL;
  int test = DB2UTIL_UNKNOWN;
  int test2 = DB2UTIL_UNKNOWN;
  char buffer[4096];
  /* clear parm markers */
  for (i=0; i < DB2UTIL_MAX_ARGS; i++) {
    iargv[i] = NULL;
  }
  /* input args */
  for (i=1; i < argc; i++) {
    test = db2util_hash_key(argv[i]);
    switch (test) {
    /* -p parm1 parm2 */
    case DB2UTIL_ARG_INPUT:
      for (i++; i < argc; i++) {
        test2 = db2util_hash_key(argv[i]);
        if (test2 != DB2UTIL_UNKNOWN) {
          i--;
          break;
        }
        iargv[iargc] = argv[i];
        iargc += 1;
      }
      break;
    /* -o [json|comma|space] */
    case DB2UTIL_OUT_FORMAT:
      if (i + 1 < argc) {
        test2 = db2util_hash_key(argv[i+1]);
        switch (test2) {
        case DB2UTIL_OUT_COMMA:
        case DB2UTIL_OUT_JSON:
        case DB2UTIL_OUT_SPACE:
          fmt = test2;
          i += 1;
          break;
        default:
          break;
        }
      }
      break;
    /* -h */
    case DB2UTIL_CMD_HELP:
      command = DB2UTIL_CMD_HELP;
      i = argc + 1;
      break;
    case DB2UTIL_UNKNOWN:
    default:
      if (i == 1) {
        query = argv[i];
        command = DB2UTIL_CMD_QUERY;
      }
      break;
    }
  }
  /* -h */
  if (!query) {
    command = DB2UTIL_CMD_HELP;
  }
  /* run */
  switch(command) {
  case DB2UTIL_CMD_QUERY:
    return db2util_query(query,fmt,iargc,iargv);
    break;
  case DB2UTIL_CMD_HELP:
  default:
    db2util_help(fmt);
    break;
  }
  return -1;
}

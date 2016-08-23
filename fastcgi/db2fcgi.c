#include "fcgi_stdio.h" /* fcgi library; put it first*/


#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>

static char *db2fcgi_service = "/QOpenSys/usr/bin/db2util";
static void *db2fcgi_dlhandle;
static int (*db2fcgi_functionstub)(char *, int, char *, int);

static void db2fcgi_initialize(void)
{
  if (db2fcgi_dlhandle == NULL)  {
    db2fcgi_dlhandle = dlopen(db2fcgi_service, RTLD_NOW);
    if (db2fcgi_dlhandle == NULL)  {
      fprintf(stderr, "Service %s Not Found:  %s\n", db2fcgi_service, dlerror());
    }
    db2fcgi_functionstub = dlsym(db2fcgi_dlhandle, "db2util_query_json");
    if (!db2fcgi_functionstub)  {
      fprintf(stderr, "Problem loading db2util_query_json(): %s\n", dlerror());
      dlclose(db2fcgi_dlhandle);
      db2fcgi_dlhandle = NULL;
    }
  }
}

static void db2fcgi_http_200(char *json)
{
  printf("HTTP/1.1 200 Ok\r\n"
         "Content-type: application/json; charset=utf-8\r\n"
         "\r\n"
         "%s",json);
}

static void db2fcgi_http_403(char *reason)
{
  printf("HTTP/1.1 403 Forbidden\r\n"
         "Content-type: application/json; charset=utf-8\r\n"
         "\r\n"
         "{\"ok\":false,\"reason\":\"%s\"}", reason);
}

void main(void)
{
  int retcode = 0, szContent=0, rTot = 0, rSz = 0;
  char * req = (char *) NULL;
  char * get = (char *) NULL;
  char * pContent = NULL;
  char getbuf[512000];
  char buff[512000];

  db2fcgi_initialize();

  while (FCGI_Accept() >= 0)   {
    /* http://www.restapitutorial.com/lessons/httpmethods.html
     * POST   - create
     * GET    - read
     * PUT    - update/replace
     * PATCH  - update/modify
     * DELETE - delete
     */
    req = getenv("REQUEST_METHOD");
    switch (req[0]) {
    case 'P':
      switch (req[1]) {
      case 'O':
        get = getenv("CONTENT_LENGTH");
        szContent = atoi(get);
        // -----
        // read from stdin (Apache)
        pContent = (char *) &getbuf;
        memset(getbuf,0,sizeof(getbuf));
        fread(getbuf, szContent, 1, stdin);
        // sprintf(buff,"\"raw\":\"%s\"","post unsuported (tbd)");
        retcode = db2fcgi_functionstub(getbuf,strlen(getbuf),buff,sizeof(buff));
        db2fcgi_http_200(buff);
        continue;
        break;
      default:
        break;
      }
      break;
    case 'G':
      get = getenv("QUERY_STRING");
      // Make service call.  Passing string argument.
      // sprintf(buff,"\"raw\":\"%s\"",get);
      retcode = db2fcgi_functionstub(get,strlen(get),buff,sizeof(buff));
      db2fcgi_http_200(buff);
      continue;
      break;
    default:
      break;
    }
    /* error */
    sprintf(buff,"REQUEST_METHOD %s is unsupported.",req);
    db2fcgi_http_403(buff);
  } /* while */
}

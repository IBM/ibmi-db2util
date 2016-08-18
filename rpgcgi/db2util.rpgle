     H AlwNull(*UsrCtl)
     H BNDDIR('QC2LE')

       // *************************************************
       // example: 
       // http://myibmi/db2/db2util.pgm?json="
       // {
       // "query":"select * from QIWS/QCUSTCDT where LSTNAM=?",
       // "parm":["Jones"]
       // }"
       // -- or --
       // http://myibmi/db2/db2util.pgm?json="
       // {
       // "cmd":"CRTLIB LIB($RPGLIB) TYPE(*PROD) TEXT('frog')"
       // }"
       // *************************************************

      /copy ios_h
      /copy iconv_h
      /copy ipase_h

       // *************************************************
       // templates
       // *************************************************
       dcl-ds PaseAlloc_t qualified template; 
         ilePtr POINTER; 
         pasePtr UNS(20);
         sz INT(10);
       end-ds;
       dcl-ds db2util_query_json_t qualified template;
         json_in_str UNS(10); 
         json_in_len INT(10); 
         json_out_str UNS(10); 
         json_out_len INT(10); 
         sig1 INT(5);
         sig2 INT(5);
         sig3 INT(5);
         sig4 INT(5);
         ret INT(10);
       end-ds;

       // *************************************************
       // global
       // *************************************************
       DCL-C DB2_ARG0_PGM CONST('/usr/lib/start32');
       DCL-C DB2_ENV1_PATH CONST('PATH=/usr/bin');
       DCL-C DB2_ENV2_LIBPATH CONST('LIBPATH=/usr/lib');
       DCL-C DB2_ENV3_ATTACH CONST('PASE_THREAD_ATTACH=Y');
       DCL-C DB2_PATH_DB2UTIL CONST('/QOpenSys/usr/bin/');
       DCL-C DB2_FILE_DB2UTIL CONST('db2util');
       DCL-C DB2_SYM_DB2UTIL CONST('db2util_query_json');
       dcl-c NULLTERM const(x'00');
       dcl-c CRLF const(x'15');
       dcl-c LF const(x'25');
       DCL-C DB2_MAX_OUTPUT CONST(512000);

       dcl-s sLibDb2Util UNS(20);
       dcl-s db2util_query_json POINTER inz(*NULL);

       dcl-s webPaseAllocFlag int(10);
       dcl-s db2utilAllocFlag int(10);
       dcl-ds webPaseAlloc likeds(PaseAlloc_t);
       dcl-ds db2utilAlloc likeds(PaseAlloc_t);
       dcl-ds db2utilCall likeds(db2util_query_json_t);

       dcl-pr error37;
         reason char(4096) value;
       end-pr;

       dcl-pr header1208;
       end-pr;

       dcl-pr error1208;
         reason char(4096) value;
       end-pr;

       dcl-pr allocPaseBlock POINTER;
         sz int(10) value;
       end-pr;

       dcl-pr loadDb2Util32;
       end-pr;

       dcl-pr ap_unescape_url INT(10) EXTPROC('ap_unescape_url');
         url POINTER VALUE options(*string);
       end-pr;

       // *************************************************
       // main
       // *************************************************
       dcl-s isBinary char(20) inz('BINARY');
       dcl-s pos int(10) inz(0);
       dcl-s rCopy POINTER inz(*NULL);
       dcl-s cMode char(20) inz('BINARY');
       dcl-s cMethod char(20) inz(*BLANKS);
       dcl-s cContent char(20) inz(*BLANKS);
       dcl-s pContent POINTER inz(*NULL);
       dcl-s szContent int(10) inz(0);
       dcl-s rTot int(10) inz(0);
       dcl-s rSz int(10) inz(0);
       dcl-s rc int(10) inz(0);
       dcl-s newIlePtr POINTER inz(*NULL);
       dcl-s newPasePtr UNS(20) inz(0);
       DCL-S outPtr POINTER INZ(*NULL);
       DCL-S outLen INT(10) INZ(0);
       DCL-S db2utilRc INT(10) inz(0);

       dcl-s i int(10) inz(0);
       dcl-s pover pointer inz(*NULL);
       dcl-s oc char(1) based(pover);

       // must CGIConvMode BINARY
       rCopy = getenv('CGI_MODE');
       if rCopy <> *NULL;
         cMode = %str(rCopy:strlen(rCopy));
         pos = %scan(%trim(isBinary):cMode);
         if pos = 0;
           rCopy = *NULL;
         endif;
       endif;
       if rCopy = *NULL;
         error37('Require CGIConvMode BINARY');
         return;
       endif;

       // set-up pase (if needed)
       loadDb2Util32();

       // web request (json input)
       rCopy = getenv('REQUEST_METHOD');
       if rCopy <> *NULL;
         cMethod = %str(rCopy:strlen(rCopy));
       endif;
       if cMethod='POST';
         rCopy = getenv('CONTENT_LENGTH');
         cContent = %str(rCopy:strlen(rCopy));
         szContent= %int(cContent);
         pContent = allocPaseBlock(szContent);
         if szContent > 0;
           // -----
           // read from stdin (Apache)
           rTot = 0;
           rSz = 1;
           dou rTot >= szContent or rSz <= 0;
             rSz = readIFS(0:pContent+rTot:szContent-rTot);
             rTot += rSz;
           enddo;
         endif;
       // json={...}
       elseif cMethod='GET';
         rCopy = getenv('QUERY_STRING');
         szContent= strlen(rCopy);
         // json=({)...}
         if szContent > 5;
           rCopy += 5;
           szContent= strlen(rCopy);
         endif;
         pContent = allocPaseBlock(szContent);
         if szContent > 0;
           cpybytes(pContent:rCopy:szContent);
         endif;
       endif;
       if szContent > 0;
         pover = pContent;
         for i = 1 to szContent;
           if oc = x'2B'; // ascii '+'
              oc = x'20'; // ascii space
           endif;
           pover += 1;
         endfor;
         rc = ap_unescape_url(pContent);
       endif;

       header1208();

       if szContent < 2;
         error1208('JSON input missing');
         return;
       endif;

       if db2utilAllocFlag = 0;
         newIlePtr = Qp2malloc(DB2_MAX_OUTPUT:%addr(newPasePtr));
         db2utilAlloc.ilePtr = newIlePtr;
         db2utilAlloc.pasePtr = newPasePtr;
         db2utilAlloc.sz = DB2_MAX_OUTPUT;
         db2utilAllocFlag = 1;
       endif;
       db2utilCall.json_in_str = webPaseAlloc.pasePtr;
       db2utilCall.json_in_len = webPaseAlloc.sz;
       db2utilCall.json_out_str = db2utilAlloc.pasePtr;
       db2utilCall.json_out_len = db2utilAlloc.sz;
       db2utilCall.sig1 = QP2_ARG_PTR32;
       db2utilCall.sig2 = QP2_ARG_WORD;
       db2utilCall.sig3 = QP2_ARG_PTR32;
       db2utilCall.sig4 = QP2_ARG_WORD;
       db2utilCall.ret = QP2_RESULT_WORD;
       if db2util_query_json <> *NULL;
         rc = Qp2CallPase(db2util_query_json:
                          %addr(db2utilCall.json_in_str):
                          %ADDR(db2utilCall.sig1):
                          db2utilCall.ret:
                          %addr(db2utilRc));
         if rc = QP2CALLPASE_NORMAL;
           //header1208();
           outPtr = db2utilAlloc.ilePtr;
           outLen = strlen(outPtr);
           rc = writeIFS(1:outPtr:outLen);
         elseif rc = QP2CALLPASE_TERMINATING;
           error1208('db2util_query_json call error, PASE terminating');
           sLibDb2Util = 0;
           db2utilAllocFlag = 0;
           webPaseAllocFlag = 0;
         else;
           error1208('db2util_query_json call error');
         endif;
       else;
         error1208('db2util_query_json was not found');
       endif;
       return;

       // *************************************************
       // output error
       // return (na)
       // *************************************************
       dcl-proc error37;
         dcl-pi *N;
          reason char(4096) value;
         end-pi;
         dcl-s talk char(4096) inz(*BLANKS);
         dcl-s rc int(20) inz(0);
         DCL-S outPtr POINTER INZ(*NULL);
         DCL-S outLen INT(10) INZ(0);

         talk = *BLANKS;
         talk =
           'Content-type: text/plain' + CRLF
         + CRLF
         + NULLTERM;
         outPtr = %addr(talk);
         outLen = strlen(outPtr);
         rc = writeIFS(1:outPtr:outLen);

         talk = *BLANKS;
         talk =
           '{"ok":false,"reason":"'+%trim(reason)+'"}'
         + CRLF
         + NULLTERM;
         outPtr = %addr(talk);
         outLen = strlen(outPtr);
         rc = writeIFS(1:outPtr:outLen);

       end-proc; 


       // *************************************************
       // output error
       // return (na)
       // *************************************************
       dcl-proc header1208;
         dcl-pi *N;
         end-pi;
         dcl-s talk char(4096) inz(*BLANKS);
         dcl-s rc int(20) inz(0);
         DCL-S outPtr POINTER INZ(*NULL);
         DCL-S outLen INT(10) INZ(0);
         dcl-s talk1208 char(4096) inz(*BLANKS);
         DCL-S buffPtr POINTER INZ(*NULL);
         DCL-S buffLen INT(10) INZ(0);
         DCL-S fromCCSID INT(10) INZ(37);
         DCL-S toCCSID INT(10) INZ(1208);

         talk = *BLANKS;
         talk =
           'Content-type: application/json; charset=utf-8' + LF
         + LF
         + NULLTERM;
         buffPtr = %addr(talk);
         buffLen = strlen(buffPtr);
         outPtr = %addr(talk1208);
         outLen = %size(talk1208);
         memset(outPtr:0:outLen);
         rc = convCCSID(fromCCSID:toCCSID:buffPtr:buffLen:outPtr:outLen);
         outLen = strlen(outPtr);
         rc = writeIFS(1:outPtr:outLen);

       end-proc; 


       // *************************************************
       // output error
       // return (na)
       // *************************************************
       dcl-proc error1208;
         dcl-pi *N;
          reason char(4096) value;
         end-pi;
         dcl-s talk char(4096) inz(*BLANKS);
         dcl-s rc int(20) inz(0);
         DCL-S outPtr POINTER INZ(*NULL);
         DCL-S outLen INT(10) INZ(0);
         dcl-s talk1208 char(4096) inz(*BLANKS);
         DCL-S buffPtr POINTER INZ(*NULL);
         DCL-S buffLen INT(10) INZ(0);
         DCL-S fromCCSID INT(10) INZ(37);
         DCL-S toCCSID INT(10) INZ(1208);

         // header1208();

         talk = *BLANKS;
         talk =
           '{"ok":false,"reason":"'+%trim(reason)+'"}'
         + LF
         + NULLTERM;
         buffPtr = %addr(talk);
         buffLen = strlen(buffPtr);
         outPtr = %addr(talk1208);
         outLen = %size(talk1208);
         memset(outPtr:0:outLen);
         rc = convCCSID(fromCCSID:toCCSID:buffPtr:buffLen:outPtr:outLen);
         outLen = strlen(outPtr);
         rc = writeIFS(1:outPtr:outLen);

       end-proc; 


       // *************************************************
       // alloc pase block
       // return (ilePTR to PASE memory)
       // *************************************************
       dcl-proc allocPaseBlock;
         dcl-pi *N POINTER;
           sz int(10) value;
         end-pi;
         dcl-s newIlePtr POINTER inz(*NULL);
         dcl-s newPasePtr UNS(20) inz(0);

         newIlePtr = webPaseAlloc.ilePtr;
         if webPaseAllocFlag = 0
         or (sz > 0 and webPaseAlloc.sz < sz);
           if webPaseAllocFlag > 0;
             dealloc(en) newIlePtr;
             newIlePtr = *NULL;
           endif;
           newIlePtr = Qp2malloc(sz:%addr(newPasePtr));
           webPaseAlloc.ilePtr = newIlePtr;
           webPaseAlloc.pasePtr = newPasePtr;
           webPaseAlloc.sz = sz;
           webPaseAllocFlag = 1;
         endif;
         if webPaseAlloc.sz > 0;
           memset(newIlePtr:0:webPaseAlloc.sz);
         endif;

         return newIlePtr;
       end-proc;


       // *************************************************
       // PHP API -- start 32bit
       // *************************************************
       dcl-proc loadDb2Util32;
         dcl-pi *N;
         end-pi;
         DCL-S rcb IND INZ(*OFF);
         dcl-s pgm CHAR(65500) inz(*BLANKS);
         dcl-s arg CHAR(65500) dim(8) inz(*BLANKS);
         dcl-s env CHAR(65500) dim(8) inz(*BLANKS);
         dcl-s myPaseCCSID INT(10) inz(819);
         dcl-s apilib CHAR(1024) inz(*BLANKS);
         dcl-s apiname CHAR(1024) inz(*BLANKS);

         pgm = DB2_ARG0_PGM;

         env(1) = DB2_ENV1_PATH;
         env(2) = DB2_ENV2_LIBPATH;
         env(3) = DB2_ENV3_ATTACH;

         rcb = PaseStart32(pgm:arg:env:myPaseCCSID);
         if rcb = *ON;
           if sLibDb2Util = 0 or db2util_query_json = *NULL;
             if sLibDb2Util < 1;
               apilib = 
                 DB2_PATH_DB2UTIL 
               + DB2_FILE_DB2UTIL 
               + NULLTERM;
               sLibDb2Util = Qp2dlopen(%addr(apilib)
                           :X'00040002':0);
             endif;
             if sLibDb2Util > 0;
               apiname = 
                 DB2_SYM_DB2UTIL
               + NULLTERM;
               db2util_query_json = Qp2dlsym(sLibDb2Util
                            :%ADDR(apiname):0:*NULL);
             endif;
           endif;
         endif;

       end-proc; 




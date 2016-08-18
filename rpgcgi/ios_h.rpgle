      /if defined(IOS_H)
      /eof
      /endif
      /define IOS_H

       dcl-pr cpybytes EXTPROC('_CPYBYTES');
         pTarget POINTER VALUE;
         pSource POINTER VALUE;
         nLength UNS(10) VALUE;
       end-pr;

       dcl-pr cpybwp EXTPROC('_CPYBWP');
         pTarget POINTER VALUE;
         pSource POINTER VALUE;
         nLength UNS(10) VALUE;
       end-pr;

       dcl-pr memset POINTER EXTPROC('__memset');
         pTarget POINTER VALUE;
         nChar INT(10) VALUE;
         nBufLen UNS(10) VALUE;
       end-pr;

       dcl-pr memcmp INT(10) EXTPROC('__memcmp');
         pS1 POINTER VALUE;
         pS2 POINTER VALUE;
         nBufLen UNS(10) VALUE;
       end-pr;

       dcl-pr readIFS INT(20) EXTPROC('read');
         fd INT(10) VALUE;
         buf POINTER VALUE;
         size INT(10) VALUE;
       end-pr;

       dcl-pr writeIFS INT(20) EXTPROC('write');
         fd INT(10) VALUE;
         buf POINTER VALUE;
         size INT(10) VALUE;
       end-pr;

       dcl-pr getenv POINTER EXTPROC('getenv');
         buf POINTER VALUE options(*string);
       end-pr;

       dcl-pr strlen INT(10) EXTPROC('strlen');
         buf POINTER VALUE options(*string);
       end-pr;


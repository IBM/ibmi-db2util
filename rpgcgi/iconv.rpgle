     H NOMAIN
     H AlwNull(*UsrCtl)
     H BNDDIR('QC2LE')

      /copy ios_h
      /copy iconv_h

       // *************************************************
       // template
       // *************************************************
       dcl-ds iconv_t qualified template; 
         rtn INT(10); 
         cd INT(10) DIM(12); 
       end-ds;

       dcl-ds qtqCode_t qualified template; 
         qtqCCSID INT(10); 
         qtqAltCnv INT(10); 
         qtqAltSub INT(10); 
         qtqAltSft INT(10); 
         qtqOptLen INT(10); 
         qtqMixErr INT(10); 
         qtqRsv INT(20); 
       end-ds;

       dcl-ds ciconv_t qualified template; 
         conviok CHAR(4); 
         conv likeds(iconv_t); 
         tocode likeds(qtqCode_t); 
         fromcode likeds(qtqCode_t); 
       end-ds;

       dcl-ds over_t qualified template; 
         ubufx CHAR(32);
         bytex char(1) overlay(ubufx);
         boolx ind overlay(ubufx);
         twox char(2) overlay(ubufx);
         uchrx uns(3) overlay(ubufx);
         shortx int(5) overlay(ubufx);
         ushortx uns(5) overlay(ubufx);
         intx int(10) overlay(ubufx);
         uintx uns(10) overlay(ubufx);
         longlongx int(20) overlay(ubufx);
         ulonglong uns(20) overlay(ubufx);
         floatx float(4) overlay(ubufx);
         double float(8) overlay(ubufx);
         ptrx pointer overlay(ubufx);
         char1 char(1) overlay(ubufx);
         char2 char(2) overlay(ubufx);
         char3 char(3) overlay(ubufx);
         char4 char(4) overlay(ubufx);
         char5 char(5) overlay(ubufx);
         char6 char(6) overlay(ubufx);
         char7 char(7) overlay(ubufx);
         char8 char(8) overlay(ubufx);
         char9 char(9) overlay(ubufx);
         char10 char(10) overlay(ubufx);
         char11 char(11) overlay(ubufx);
         chardim char(1) dim(32) overlay(ubufx);
       end-ds;


       // *************************************************
       // global
       // *************************************************
       DCL-C CNVOPNMAX CONST(128);
       DCL-C CNVOPNOK CONST('okok');

       dcl-ds myiConv likeds(ciconv_t) dim(CNVOPNMAX);


       // *************************************************
       // convert APIs -- operating system
       // *************************************************
       DCL-C QP2_2_ASCII CONST('QTCPASC');
       DCL-C QP2_2_EBCDIC CONST('QTCPEBC');

       dcl-pr Translate EXTPROC('QDCXLATE');
         Length PACKED(15:5) CONST;
         Data VARCHAR(32766);
         Table CHAR(10);
       end-pr;

       dcl-pr iconvOpen likeds(iconv_t) EXTPROC('QtqIconvOpen');
         tocode likeds(qtqCode_t);
         fromcode likeds(qtqCode_t);
       end-pr;

       dcl-pr iconv INT(10) EXTPROC('iconv');
         hConv likeds(iconv_t) value;
         pInBuff POINTER VALUE;
         nInLen POINTER VALUE;
         pOutBuff POINTER VALUE;
         nOutLen POINTER VALUE;
       end-pr;

       dcl-pr iconvClose EXTPROC('iconv_close');
         cd likeds(iconv_t);
       end-pr;

       dcl-pr QXXDTOP ExtProc('QXXDTOP');
         pTarget POINTER VALUE;
         digit INT(10) VALUE;
         frac INT(10) VALUE;
         value FLOAT(8) VALUE;
       end-pr;

       dcl-pr QXXDTOZ ExtProc('QXXDTOZ');
         pTarget POINTER VALUE;
         digit INT(10) VALUE;
         frac INT(10) VALUE;
         value FLOAT(8) VALUE;
       end-pr;

       dcl-pr QXXPTOD FLOAT(8) ExtProc('QXXPTOD');
         pTarget POINTER VALUE;
         digit INT(10) VALUE;
         frac INT(10) VALUE;
       end-pr;

       dcl-pr QXXZTOD FLOAT(8) ExtProc('QXXZTOD');
         pTarget POINTER VALUE;
         digit INT(10) VALUE;
         frac INT(10) VALUE;
       end-pr;


       // *************************************************
       // convert APIs -- custom
       // *************************************************

       dcl-pr convOpen INT(10);
         fromCCSID INT(10);
         toCCSID INT(10);
         conv likeds(ciconv_t);
       end-pr;

       dcl-pr convClose INT(10);
         conv likeds(ciconv_t);
       end-pr;

       dcl-pr convCall INT(10);
         conv likeds(ciconv_t);
         buffPtr POINTER;
         buffLen INT(10);
         outPtr POINTER;
         outLen INT(10);
       end-pr;

       // *************************************************
       // convert open
       // return (>-1 - good, <0 - error)
       // *************************************************
       dcl-proc convOpen;
         dcl-pi *N INT(10);
          fromCCSID INT(10);
          toCCSID INT(10);
          conv likeds(ciconv_t);
         end-pi;
         DCL-S rc INT(10) INZ(0);
 
         Monitor;
           // qtqCode_t:
           //             to         from
           // qtqCCSID    0 - job    PaseCCSID
           // qtqAltCnv   0 - na     0 - IBM default
           // qtqAltSub   0 - na     0 - not returned to initial shift state
           // qtqAltSft   0 - na     0 - substitution characters not returned
           // qtqOptLen   0 - na     0 - inbytesleft parameter must be specified
           // qtqMixErr   0 - na     0 - no error dbcs
           // qtqRsv      0 - na     0 - na
           memset(%addr(conv.tocode):0:%size(conv.tocode));
           memset(%addr(conv.fromcode):0:%size(conv.fromcode));
           memset(%addr(conv.conv):0:%size(conv.conv));
   
           // If unsuccessful, QtqIconvOpen() returns -1 
           // and in the return value of the conversion 
           // descriptor and sets errno to indicate the error.
           conv.fromcode.qtqCCSID = fromCCSID;
           conv.tocode.qtqCCSID = toCCSID;
           conv.conv = iconvOpen(conv.tocode:conv.fromcode);
           if conv.conv.rtn < 0;
             rc = -1;
           endif;
  
         On-error;
           rc = -1;
         Endmon;
 
         if rc < 0;
           memset(%addr(conv.tocode):0:%size(conv.tocode));
           memset(%addr(conv.fromcode):0:%size(conv.fromcode));
           memset(%addr(conv.conv):0:%size(conv.conv));
         endif;
 
         return rc;
       end-proc; 

       // *************************************************
       // convert close
       // return (>-1 - good, <0 - error)
       // *************************************************
       dcl-proc convClose;
         dcl-pi *N INT(10);
          conv likeds(ciconv_t);
         end-pi;
         DCL-S rc INT(10) INZ(0);
         Monitor;

           iconvClose(conv.conv);

         On-error;
           rc = -1;
         Endmon;

         memset(%addr(conv.tocode):0:%size(conv.tocode));
         memset(%addr(conv.fromcode):0:%size(conv.fromcode));
         memset(%addr(conv.conv):0:%size(conv.conv));
 
         return rc;
       end-proc; 


       // *************************************************
       // convert string
       // return (>-1 - good, <0 - error)
       // *************************************************
       dcl-proc convCall;
         dcl-pi *N INT(10);
          conv likeds(ciconv_t);
          buffPtr POINTER;
          buffLen INT(10);
          outPtr POINTER;
          outLen INT(10);
         end-pi;
         DCL-S rc INT(10) INZ(0);
         DCL-S rcb IND INZ(*OFF);
         DCL-S isOpen IND INZ(*OFF);
         DCL-S aConvPtrP POINTER INZ(*NULL);
         DCL-S fromPtr POINTER INZ(*NULL);
         DCL-S toPtr POINTER INZ(*NULL);
         DCL-S aConvSz INT(10) INZ(0);
         DCL-S fromBytes INT(10) INZ(0);
         DCL-S toBytes INT(10) INZ(0);
         DCL-S toMax INT(10) INZ(0);
         DCL-S maxBytes INT(10) INZ(0);
         DCL-S maxOut INT(10) INZ(0);
         DCL-S maxZero INT(10) INZ(0);

         Monitor;

           // size_t iconv (cd, inbuf, inbytesleft, outbuf, outbytesleft)
           // inbytesleft  - number of bytes not converted input buffer
           // outbytesleft - available bytes to end output buffer
           // If an error occurs, iconv() returns -1 
           // in the return value, and sets errno to indicate the error.
           maxBytes   = buffLen;
           fromPtr    = buffPtr;
           fromBytes  = maxBytes;
           toPtr      = outPtr;
           toBytes    = outLen;
           // convert
           toMax = toBytes;
           rc = iconv(conv.conv
                 :%addr(fromPtr):%addr(fromBytes)
                 :%addr(toPtr):%addr(toBytes));
           maxBytes = toMax - toBytes;
           // did it work?
           if rc > -1;
             // out buffer provided (no copy)
             toPtr = outPtr;
             maxOut = outLen;
             // how many bytes unused output buffer?
             outLen = toBytes;
             // possible null terminate
             maxZero = maxOut - maxBytes;
             if maxZero > 4096;
               maxZero = 4096;
             endif;
             if maxZero > 0;
                memset(toPtr + maxBytes:0:maxZero);
             endif;
             // how many input bytes did not convert?
             buffLen = fromBytes;
           endif;

         On-error;
           rc = -1;
         Endmon;
 
         return rc;
       end-proc; 


       // *************************************************
       // convert API -- custom iconv convert
       // return (>-1 - good, <0 - error)
       // *************************************************
       dcl-proc convCCSID export;
         dcl-pi *N INT(10);
          fromCCSID INT(10);
          toCCSID INT(10);
          buffPtr POINTER;
          buffLen INT(10);
          outPtr POINTER;
          outLen INT(10);
         end-pi;
         DCL-S i INT(10) INZ(0);
         DCL-S rc INT(10) INZ(0);
         DCL-S rcb IND INZ(*OFF);
         DCL-DS conv likeds(ciconv_t);

         // scan the cache ...
         for i = 1 to CNVOPNMAX;
           // found in cache?
           select;
           when myiConv(i).conviok = *BLANKS;
             rc = convOpen(fromCCSID:toCCSID:conv);
             memset(%addr(myiConv(i)):0:%size(conv));
             myiConv(i).conviok = CNVOPNOK;
             cpybytes(%addr(myiConv(i)):%addr(conv):%size(conv));
             leave;
           when myiConv(i).fromcode.qtqCCSID = fromCCSID
           and myiConv(i).tocode.qtqCCSID = toCCSID
           and myiConv(i).conviok = CNVOPNOK;
             cpybytes(%addr(conv):%addr(myiConv(i)):%size(conv));
             leave;
           // other?
           other;
           endsl;
         endfor;
         rc = convCall(conv:buffPtr:buffLen:outPtr:outLen);
 
         return rc;
       end-proc; 

       // *************************************************
       // convert API -- custom iconv convert
       // return (>-1 - good, <0 - error)
       // *************************************************
       dcl-proc bigTrim export;
         dcl-pi *N UNS(20);
          start POINTER VALUE;
          len UNS(20) VALUE;
         end-pi;
         DCL-S pCopy POINTER INZ(*NULL);
         DCL-DS myCopy likeds(over_t) based(pCopy);

         dow len > 0;
           pCopy = start + (len-1);
           if myCopy.bytex <> *BLANKS 
           and myCopy.bytex <> x'00';
             leave;
           endif;
           len -= 1;
         enddo;

         return len;
       end-proc; 



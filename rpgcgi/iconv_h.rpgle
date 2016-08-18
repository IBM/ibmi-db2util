      /if defined(ICONV_H)
      /eof
      /endif
      /define ICONV_H


       dcl-pr convCCSID INT(10);
         fromCCSID INT(10);
         toCCSID INT(10);
         buffPtr POINTER;
         buffLen INT(10);
         outPtr POINTER;
         outLen INT(10);
       end-pr;


       dcl-pr bigTrim UNS(20);
         start POINTER VALUE;
         len UNS(20) VALUE;
       end-pr;


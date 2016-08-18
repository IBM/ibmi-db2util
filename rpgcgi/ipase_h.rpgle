      /if defined(IPASE_H)
      /eof
      /endif
      /define IPASE_H

       // *************************************************
       // PASE APIs -- custom
       // *************************************************
       dcl-pr PaseStart32 IND;
          pgm CHAR(65500) VALUE;
          arg CHAR(65500) dim(8) VALUE;
          env CHAR(65500) dim(8) VALUE;
          paseCCSID INT(10) VALUE;
       end-pr;

       dcl-pr Qp2CallPase INT(10);
         ptarget POINTER VALUE;
         parglist POINTER VALUE;
         psignature POINTER VALUE;
         presult INT(5) VALUE;
         pbuf POINTER VALUE;
       end-pr;

       dcl-pr PaseStop;
       end-pr;

       dcl-pr PaseJobCCSID INT(10);
       end-pr;

       dcl-pr PaseLstCCSID INT(10);
       end-pr;

       dcl-pr PaseErrno INT(10);
       end-pr;

       dcl-pr PaseEZero IND;
       end-pr;

       // *************************************************
       // PASE APIs -- operating system
       // *************************************************
       DCL-C QP2_ARG_END CONST(0);
       DCL-C QP2_ARG_WORD CONST(-1);
       DCL-C QP2_ARG_DWORD CONST(-2);
       DCL-C QP2_ARG_FLOAT32 CONST(-3);
       DCL-C QP2_ARG_FLOAT64 CONST(-4);
       DCL-C QP2_ARG_PTR32 CONST(-5);
       DCL-C QP2_ARG_PTR64 CONST(-6);

       DCL-C QP2_RESULT_VOID CONST(0);
       DCL-C QP2_RESULT_WORD CONST(-1);
       DCL-C QP2_RESULT_DWORD CONST(-2);
       DCL-C QP2_RESULT_FLOAT64 CONST(-4);
       DCL-C QP2_RESULT_PTR32 CONST(-5);
       DCL-C QP2_RESULT_PTR64 CONST(-6);

       DCL-C QP2RUNPASE_ERROR CONST(-1);
       DCL-C QP2RUNPASE_RETURN_NOEXIT CONST(-2);

       dcl-pr Qp2RunPase INT(10) EXTPROC('Qp2RunPase');
         ppathName POINTER VALUE options(*string);
         psymName POINTER VALUE options(*string);
         psymlDat POINTER VALUE options(*string);
         psymDataL UNS(10) VALUE;
         pccsid INT(10) VALUE;
         pargv POINTER VALUE;
         penvp POINTER VALUE;
       end-pr;

       dcl-pr Qp2malloc POINTER EXTPROC('Qp2malloc');
         pmem_size UNS(20) VALUE;
         pmem_pase POINTER VALUE;
       end-pr;

       dcl-pr Qp2free EXTPROC('Qp2free');
         pmem_pase POINTER VALUE;
       end-pr;

       dcl-pr Qp2dlopen UNS(20) EXTPROC('Qp2dlopen');
         ppath POINTER VALUE options(*string);
         pflags INT(10) VALUE;
         pccsid INT(10) VALUE;
       end-pr;

       dcl-pr Qp2dlsym POINTER EXTPROC('Qp2dlsym');
         pid UNS(20) VALUE;
         pname POINTER VALUE options(*string);
         pccsid INT(10) VALUE;
         psym_Pase POINTER VALUE;
       end-pr;

      * PASE procedure ran to completion 
      * and its function result stored
       DCL-C QP2CALLPASE_NORMAL CONST(0);
      * PASE procedure ran to completion, 
      * but its function result could not be stored
       DCL-C QP2CALLPASE_RESULT_ERROR CONST(1);
      * no i5/OS PASE program is running in the job
       DCL-C QP2CALLPASE_ENVIRON_ERROR CONST(2);
      * One or more values in the signature array are not valid.
       DCL-C QP2CALLPASE_ARG_ERROR CONST(4);
      * The i5/OS PASE program is terminating. 
      * No function result was returned.
       DCL-C QP2CALLPASE_TERMINATING CONST(6);
      * The i5/OS PASE program returned without exiting
       DCL-C QP2CALLPASE_RETURN_NOEXIT CONST(7);

       dcl-pr Qp2CallNow INT(10) EXTPROC('Qp2CallPase');
         ptarget POINTER VALUE;
         parglist POINTER VALUE;
         psignature POINTER VALUE;
         presult INT(5) VALUE;
         pbuf POINTER VALUE;
       end-pr;

       dcl-pr Qp2EndPase INT(10) EXTPROC('Qp2EndPase');
       end-pr;

       dcl-pr Qp2SignalPase INT(10) EXTPROC('Qp2SignalPase');
         sig INT(10) VALUE;
       end-pr;

       dcl-pr Qp2errnop POINTER EXTPROC('Qp2errnop');
       end-pr;

       dcl-pr Qp2paseCCSID INT(10) EXTPROC('Qp2paseCCSID');
       end-pr;

       dcl-pr Qp2jobCCSID INT(10) EXTPROC('Qp2jobCCSID');
       end-pr;



     H NOMAIN
     H AlwNull(*UsrCtl)
     H BNDDIR('QC2LE')

      /copy ipase_h

       // *************************************************
       // template
       // *************************************************
       dcl-ds iArg_t qualified template; 
         pgm POINTER;
         arg POINTER dim(9);
       end-ds;

       dcl-ds iEnv_t qualified template; 
         env POINTER dim(9);
       end-ds;

       // *************************************************
       // global
       // *************************************************
       dcl-s sLib UNS(20);
       dcl-s sPaseAdopt IND;

       dcl-s paErrnop POINTER;
       dcl-s paErrno INT(10) based(paErrnop);

       // *************************************************
       // PASE errno
       // *************************************************
       dcl-proc PaseEZero export;
         dcl-pi *N IND;
         end-pi;

         Monitor;
           paErrnop = Qp2errnop();
           if paErrnop <> *NULL;
             paErrno = 0;
             return *ON;
           endif;
         On-error;
         Endmon;

         return *OFF;
       end-proc; 

       dcl-proc PaseErrno export;
         dcl-pi *N INT(10);
         end-pi;

         Monitor;
           // errno address
           paErrnop = Qp2errnop();
           if paErrnop <> *NULL;
             return paErrno;
           endif;
         On-error;
         Endmon;

         return 0;
       end-proc; 

       // *************************************************
       // PASE ccsid
       // *************************************************
       dcl-proc PaseLstCCSID export;
         dcl-pi *N INT(10);
         end-pi;
         DCL-S rcb IND INZ(*OFF);

         // pase really alive ...
         rcb = PaseEZero();
         if rcb = *OFF;
           return 0;
         endif;

         return Qp2paseCCSID();
       end-proc; 

       dcl-proc PaseJobCCSID export;
         dcl-pi *N INT(10);
         end-pi;
         DCL-S rcb IND INZ(*OFF);

         // pase really alive ...
         rcb = PaseEZero();
         if rcb = *OFF;
           return 0;
         endif;

         return Qp2jobCCSID();
       end-proc; 

       // *************************************************
       // PASE start 32bit
       // *************************************************
       dcl-proc PaseStart32 export;
         dcl-pi *N IND;
          pgm CHAR(65500) VALUE;
          arg CHAR(65500) dim(8) VALUE;
          env CHAR(65500) dim(8) VALUE;
          paseCCSID INT(10) VALUE;
         end-pi;
         DCL-S rcb IND INZ(*OFF);
         DCL-S i INT(10) INZ(0);
         DCL-S rc INT(10) INZ(0);
         DCL-S p POINTER INZ(*NULL);
         DCL-S a POINTER INZ(*NULL);
         DCL-S e POINTER INZ(*NULL);
         DCL-S pl INT(10) INZ(0);
         DCL-S al INT(10) INZ(0);
         DCL-S el INT(10) INZ(0);
         dcl-ds iarg likeds(iArg_t);
         dcl-ds ienv likeds(iEnv_t);
         dcl-s paNullp POINTER;
         dcl-s paNull INT(5) based(paNullp);

         // PASE available
         if sLib > 0;
           // pase really alive ...
           rcb = PaseEZero();
           if rcb = *ON;
             return *ON;
           endif;
         endif;

         // PASE already usable in this process ???
         // load PASE common libraries libc.a (0=bad, !0=good)
         Monitor;
           sLib = Qp2dlopen(*NULL: X'00000002': 0);
         On-error;
         Endmon;
         // adopting already running PASE
         if sLib > 0;
           sPaseAdopt = *ON;
         else;
           sPaseAdopt = *OFF;
           PaseStop();
         endif;
         if sPaseAdopt = *ON;
           return *ON;
         endif;

 
         // PASE still not available,
         // try to start PASE
         p = %addr(pgm);
         pl = %len(%trim(pgm));
         paNullp = p + pl;
         paNull = 0;
         iarg.pgm = p;
         for i = 1 to 8;
           if arg(i) = *BLANKS;
             iarg.arg(i) = *NULL;
           else;
             a = %addr(arg(i));
             al = %len(%trim(arg(i)));
             paNullp = a + al;
             paNull = 0;
             iarg.arg(i) = a;
           endif;
           if env(i) = *BLANKS;
             ienv.env(i) = *NULL;
           else;
             e = %addr(env(i));
             el = %len(%trim(env(i)));
             paNullp = e + el;
             paNull = 0;
             ienv.env(i) = e;
           endif;
         endfor;

         // start PASE now
         // return (0==good, -1==bad, -2=exit PASE alive)
         Monitor;
           if paseCCSID <= 0;
             paseCCSID = 819;
           endif;
           rc = Qp2RunPase(iarg.pgm: *NULL: *NULL: 0: paseCCSID: 
                     %ADDR(iarg): %ADDR(ienv));
         On-error;
           rc = -1;
           PaseStop();
         Endmon;
         if rc = -1;
           return *OFF;
         endif;
         // load PASE common libraries libc.a (0=bad, !0=good)
         Monitor;
           sLib = Qp2dlopen(*NULL: X'00000002': 0);
         On-error;
           PaseStop();
         Endmon;
         if sLib = 0;
           return *OFF;
         endif;

         return *ON;
       end-proc; 

       // *************************************************
       // PASE call function
       // *************************************************
       dcl-proc Qp2CallPase export;
         dcl-pi *N INT(10);
          ptarget POINTER VALUE;
          parglist POINTER VALUE;
          psignature POINTER VALUE;
          presult INT(5) VALUE;
          pbuf POINTER VALUE;
         end-pi;
         DCL-S rc INT(10) INZ(0);
         DCL-S i INT(10) INZ(0);

         Monitor;
         // call pase now
         rc = Qp2CallNow(ptarget:parglist:psignature:presult:pbuf);
         On-error;
         Endmon;
         // PASE death
         if rc = QP2CALLPASE_TERMINATING;
           PaseStop();
         endif;

         return rc;
       end-proc; 


       // *************************************************
       // PASE stop 
       // *************************************************
       dcl-proc PaseStop export;

         DCL-S rcb IND INZ(*OFF);
         DCL-S rc INT(10) INZ(0);
         DCL-S i INT(10) INZ(0);

         // clear all
         // free memory 1.6.8
         rcb = PaseEZero();
         // PASE not available
         if sLib = 0;
           // PASE already usable in this process ???
           // load PASE common libraries libc.a (0=bad, !0=good)
           Monitor;
             sLib = Qp2dlopen(*NULL: X'00000002': 0);
           On-error;
           Endmon;
           // adopting already running PASE
           if sLib > 0;
             sPaseAdopt = *ON;
           else;
             sPaseAdopt = *OFF;
           endif;
         endif;

         sLib = 0;

         // you cannot end PASE in adopt
         if sPaseAdopt = *OFF;
           Monitor;
             // SIGKILL PASE (just in case)
             rc=Qp2SignalPase(-9);
             // end PASE (!0=bad, 0=good)
             rc=Qp2EndPase();
           On-error;
           Endmon;
         endif;

       end-proc; 


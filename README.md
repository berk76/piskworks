Piskworks is a simple desk game known as gomoku
===============================================
      
How to build
------------

* Windows (WIN32):  
  make -f Makefile.gcc pisk_w32

* Console (ANSI C):  
  make -f Makefile.gcc pisk_con

* Turbo C 2.01  
  cd src  
  make -fmakefile.tc   

* ZX Spectrum - Z88DK (http://www.z88dk.org)  
  make -f Makefile.z88dk pisk_zx  
  
* ZX81 - Z88DK (http://www.z88dk.org)  
  make -f Makefile.z88dk pisk_81  

* APPLE 1 (replica 1)  
  cd src  
  make -f Makefile.app  
  bintomon -v -f pisk_app > pisk_app.mon  
   
  for more info about replica 1 see:   
  http://jefftranter.blogspot.cz/2012/04/c-programming-tutorial-with-cc65-on.html  

* AS400  
  Create your personal library if you dont have one  
  crtlib mylib  
  
  Setup your private library as default library  
  chgprf curlib(mylib)  
  
  Create new file QCSRC  
  crtsrcpf mylib/qcsrc  
  
  Copy new member GOM (from src/qcsrc.gom) into QCSRC fiel using FTP client.  
  
  You can see and modify source code in SEU editor  
  strseu srcfile(QCSRC) srcmbr(GOM) 
  
  Compile it using crtbndc command  
  crtbndc pgm(GOM)  
  
  Finally you can run it  
  call GOM  

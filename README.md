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


# Piskworks
Piskworks is a simple desk game known as gomoku.

## Supported platforms

### MS Windows
  1. download and install MinGw from http://www.mingw.org/
  1. download project `git clone https://github.com/berk76/piskworks Piskworks`
  1. go into project directory `cd Piskworks`
  1. run `make -f Makefile.gcc pisk_w32`
  1. find and run pisk_w32.exe


### UNIX, Linux, MS Windows console:
  1. download project `git clone https://github.com/berk76/piskworks Piskworks`
  1. go into project directory `cd Piskworks`
  1. run `make -f Makefile.gcc pisk_con`
  1. find and run pisk_con.exe


### DOS
  1. download and install Turbo C from http://edn.embarcadero.com/article/20841
  1. download project `git clone https://github.com/berk76/piskworks Piskworks`
  1. go into Piskworks/src directory 
  1. run `make -fMakefile.tc`
  1. find and run main_con.exe


### ZX Spectrum
  1. download and install Z88DK c compiler from http://www.z88dk.org
  1. download project `git clone https://github.com/berk76/piskworks Piskworks`
  1. go into project directory `cd Piskworks`
  1. run `make -f Makefile.z88dk pisk_zx`
  1. find and load pisk_zx.tap


### ZX81
  1. download and install Z88DK c compiler from http://www.z88dk.org
  1. download project `git clone https://github.com/berk76/piskworks Piskworks`
  1. go into project directory `cd Piskworks`
  1. run `make -f Makefile.z88dk pisk_81`
  1. find and load pisk_81.P


### APPLE 1 (replica 1)
  1. install c compiler according to this article: http://jefftranter.blogspot.cz/2012/04/c-programming-tutorial-with-cc65-on.html
  1. download project `git clone https://github.com/berk76/piskworks Piskworks`
  1. go into Piskworks/src directory
  1. run `make -f Makefile.app`
  1. run `bintomon -v -f pisk_app > pisk_app.mon`


### AS400
  1. download project `git clone https://github.com/berk76/piskworks Piskworks`
  1. create your personal library if you dont have one `crtlib mylib`
  1. setup your private library as default library `chgprf curlib(mylib)`
  1. create new file QCSRC `crtsrcpf mylib/qcsrc`
  1. copy new member GOM (from src/qcsrc.gom) into QCSRC file using FTP client.
  1. You can see and modify source code in SEU editor `strseu srcfile(QCSRC) srcmbr(GOM)`
  1. Compile program using crtbndc command `crtbndc pgm(GOM)`
  1. Finally you can run program `call GOM`

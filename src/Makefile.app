#       Makefile.app
#       
#       This file is part of Piskworks game.
#       https://bitbucket.org/berk76/piskworks
#       
#       Piskworks is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
#       (at your option) any later version. <http://www.gnu.org/licenses/>
#       
#       Written by Jaroslav Beran <jaroslav.beran@gmail.com>, on 17.10.2016


CC = cl65
CFLAGS = -O -t replica1 -DNO_TIME -c -o $@

        
pisk_app: main_app.o pisk_lib.o  
	$(CC) -t replica1 -o $@ main_app.o pisk_lib.o

main_app.o: main_app.c pisk_lib.h

pisk_lib.o: pisk_lib.c pisk_lib.h

clean:
	rm -f *.o pisk_app



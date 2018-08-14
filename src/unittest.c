/*
*       unittest.c
*       
*       This file is part of Piskworks game.
*       https://github.com/berk76/piskworks
*       
*       Piskworks is free software; you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation; either version 3 of the License, or
*       (at your option) any later version. <http://www.gnu.org/licenses/>
*       
*       Written by Jaroslav Beran <jaroslav.beran@gmail.com>, on 14.8.2018        
*/


#include <stdio.h>
#include <string.h>
#include "pisk_lib.h"


static PISKWORKS_T p;
#define BUFLEN 250
char buffer[BUFLEN];

/* returns 0 if both match or 1 if differ or -1 in case of error */
static int compareFiles(char *f1, char *f2);

/* returns 0 in case of success */
static int findSection(FILE *f, char *section);

int main(int argc, char **argv) {
        int result;

        /* validate arguments */
        if (argc != 3) {
                printf("Usage:\n");
                printf("unittest testing_scene expected_result\n");
                return -1;
        }
        
        printf("* Running %s ... ", argv[1]);
        
        /* init game */
        p_create_new_game(&p);
        
        /* load scenario */
        result = load_game(&p, argv[1]);
        if (result != 0) {
                printf("ERROR: Cannot load file %s!\n", argv[1]);
                return -1;
        }
        
        /* play */
        check_and_play(&p, 1);
        
        /* save result */
        strncpy(buffer, argv[1], BUFLEN - 1);
        strncat(buffer, ".result", BUFLEN - strlen(buffer) - 1);
        result = save_game(&p, buffer);
        if (result != 0) {
                printf("ERROR: Cannot save file %s!\n", buffer);
                return -1;
        }
        
        /* compare files */
        result = compareFiles(argv[2], buffer);
        
        switch (result) {
                case -1:        printf("error!\n");
                                break;
                case  0:        printf("test passed.\n");
                                remove(buffer);
                                break;
                case  1:        printf("test failed.\n");
                                break;
        }
         
        return result;
}

int compareFiles(char *f1, char *f2) {
        int ret;
        FILE *fin1, *fin2;
        char l1[BUFLEN], l2[BUFLEN];
        char *p;
        
        fin1 = fopen(f1, "r");
        if (fin1 == NULL) {
                printf("Cannot open file %s ", f1);
                return -1;
        }
        
        fin2 = fopen(f2, "r");
        if (fin2 == NULL) {
                printf("Cannot open file %s ", f2);
                return -1;
        }
        
        ret = findSection(fin1, "board:");
        if (ret != 0) {
                return -1;
        }
        
        ret = findSection(fin2, "board:");
        if (ret != 0) {
                return -1;
        }
        
        ret = 0;
        
        while (fgets(l1, BUFLEN, fin1) != NULL) {
        
                p = fgets(l2, BUFLEN, fin2);                
                if (p == NULL) {
                        ret = 1;
                        break;
                }
                
                ret = strcmp(l1, l2);
                if (ret != 0) {
                        ret = 1;
                        break;
                } 
        }
        
        fclose(fin1);
        fclose(fin2);
        
        return ret;
}


int findSection(FILE *f, char *section) {
        char line[BUFLEN];
        char *p;
        
        while (fgets(line, BUFLEN, f) != NULL) {
                p = strstr(line, section);
                if (p != NULL) {
                        return 0;
                }
        }
        
        return -1;
}

/*
*       main_app.c
*       
*       This file is part of Piskworks game.
*       https://github.com/berk76/piskworks
*       
*       Piskworks is free software; you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation; either version 3 of the License, or
*       (at your option) any later version. <http://www.gnu.org/licenses/>
*       
*       Written by Jaroslav Beran <jaroslav.beran@gmail.com>, on 17.10.2016  
*/


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pisk_lib.h"


static PISKWORKS_T pisk;
static char cross_char = 'X';
static char circle_char = 'O';


static void setup_preferences();
static int get_option(char *message, char *values);
static void get_input_con();
static void print_grid();


int main(void) {
        int result, c;
        
        /* For Z88DK compiler */
        #ifdef SCCZ80
        printf("%c%c", 1, 32);   /* 32 characters */
        printf("%c",12);         /* cls */
        #endif
        
        printf("\n*******************\n");
        printf("* PISKWORKS %s *\n", VERSION);
        printf("*******************\n\n");
        do {
                pisk.difficulty = 3;
                pisk.computer_starts_game = 1;
                setup_preferences();
                p_create_new_game(&pisk);

                print_grid();
                result = 0;
                
                do {             
                        get_input_con();
                        print_grid();
                        result = check_and_play(&pisk, 0);
                        
                        if (result == 0) {
                                result = check_and_play(&pisk, 1);
                                print_grid();
                        }
                        
                        if (result == 0) {
                                result = check_and_play(&pisk, 0);
                        }
        
                } while (result == 0);
                
                printf("GAME OVER!\n");
                if (result == 1) {
                        pisk.score_computer++;
                        printf("COMPUTER IS WINNER\n");
                } else {
                        pisk.score_player++;
                        printf("YOU ARE WINNER\n");
                }
                
                printf("COMPUTER:YOU  %d:%d\n", pisk.score_computer, pisk.score_player);
                c = get_option("\nANOTHER GAME? (Y/N)", "YyNn");        
        } while (tolower(c) == 'y');
        
        return 0;
}

void setup_preferences() {
        int c;
                
        printf("PREFERENCES:\n\n");
        printf("* YOUR STONES ... %c\n", toupper(cross_char));
        printf("* DIFFICULTY  ... %d\n", pisk.difficulty);
        printf("* %s WILL PUT FIRST MOVE\n", (pisk.computer_starts_game) ? "COMPUTER" : "YOU");
        
        c = get_option("\n(C)HANGE PREFERENCES,(S)TART", "CcSs");
        
        if (tolower(c) == 'c') {
                
                c = get_option("DO YOU WANT TO PLAY WITH X OR O?", "XxOo");
                if (tolower(c) == 'x') {
                        cross_char = 'X';
                        circle_char = 'O';
                } else {
                        cross_char = 'O';
                        circle_char = 'X';
                }
                
                c = get_option("WHICH DIFFICULTY (1,2,3)?", "123");
                pisk.difficulty = c - '1' + 1;
                
                c = get_option("DO YOU WANT TO PUT FIRST MOVE (Y/N)?", "YyNn");
                if (tolower(c) == 'y') {
                        pisk.computer_starts_game = 0;
                } else {
                        pisk.computer_starts_game = 1;
                }
                
                setup_preferences();
        }
}

int get_option(char *message, char *values) {
        int c;
        
        do {
                puts(message);
                c = getchar();
                while (getchar() != '\n');      /* clear stdin */        
        } while (strchr(values, c) == NULL);
        
        return c;
}

void get_input_con() {
        #define LINELEN 10
        char line[LINELEN];
        int y, is_input_correct;
        char x;
        STONE s;

        do {
                is_input_correct = 0;                
                printf("PUT YOUR MOVE. (FOR EX. B3)\n");
                fgets(line, LINELEN - 1, stdin);
               
                if (strlen(line) < 2)
                        continue;
                
                x = toupper(*line);
                if ((x < 'A') || (x > 'Z'))
                        continue;
                
                y = atoi(line + 1);                
                if (y == 0)
                        continue;
                        
                s = get_stone(&pisk, pisk.gs.minx + (x - 'A'), pisk.gs.miny + y - 1);
                if (s != EMPTY) {
                        printf("THIS FIELD IS ALREADY OCCUPIED\n");
                        continue;
                }  
                
                is_input_correct = 1;
        } while (is_input_correct == 0);
        
        get_input(&pisk, (x - 'A'), y - 1);
}

void print_grid() {
        int x, y;
        STONE stone;
        
        printf("\nMOVE %d\n\n", pisk.move_cnt);
        printf("  ");
        #ifndef SCCZ80 
        putchar(' '); 
        #endif
        for (x = 0; x <= (pisk.gs.maxx - pisk.gs.minx); x++) {
                putchar('A' + x);
                #ifndef SCCZ80 
                putchar(' '); 
                #endif
        }
        putchar('\n');
        
        for (y = 0; y <= (pisk.gs.maxy - pisk.gs.miny); y++) {
                /* For Z88DK compiler */
                #ifdef SCCZ80
                printf("%d", (y + 1) / 10);
                printf("%d", (y + 1) % 10);
                #else
                printf("%2d", y + 1);
                putchar(' ');
                #endif
                for (x = 0; x <= (pisk.gs.maxx - pisk.gs.minx); x++) {
                        stone = get_stone(&pisk, x + pisk.gs.minx, y + pisk.gs.miny);
                        if (stone == EMPTY) {
                                putchar('.');
                        } else
                        if (stone == CIRCLE) {
                                if ((pisk.last_move_x == (x + pisk.gs.minx)) && (pisk.last_move_y == (y + pisk.gs.miny))) {
                                        putchar(toupper(circle_char));
                                } else {
                                        putchar(circle_char);
                                }
                        } else
                        if (stone == CROSS) {
                                if ((pisk.last_move_x == (x + pisk.gs.minx)) && (pisk.last_move_y == (y + pisk.gs.miny))) {
                                        putchar(toupper(cross_char));
                                } else {
                                        putchar(cross_char);
                                }
                        } else 
                        if (stone == NA) {
                                putchar(' ');
                        }
                        
                        #ifndef SCCZ80 
                        putchar(' '); 
                        #endif                        
                }
                putchar('\n');
        }
        putchar('\n');
}


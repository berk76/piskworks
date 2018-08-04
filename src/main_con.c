/*
*       main_con.c
*       
*       This file is part of Piskworks game.
*       https://github.com/berk76/piskworks
*       
*       Piskworks is free software; you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation; either version 3 of the License, or
*       (at your option) any later version. <http://www.gnu.org/licenses/>
*       
*       Written by Jaroslav Beran <jaroslav.beran@gmail.com>, on 24.2.2016        
*/


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pisk_lib.h"

#ifdef _EINSTEIN_
#include "res_con.h"
#endif

#define FILELEN 100

static PISKWORKS_T pisk;
static char cross_char = 'x';
static char circle_char = 'o';


static void print_char(char c, int n);
static void setup_preferences(void);
static int get_option(char *message, char *values);
static int get_input_con(void); /* return value 0 = input done, 1 = quit game */
static void print_grid(void);


int main(void) {
        int result, c;
        
        /* For Z88DK compiler */
        #ifdef SCCZ80
        printf("%c%c", 1, 32);   /* 32 characters */
        printf("%c",12);         /* cls */
        #endif
      
	printf("\n");
	print_char('*', 14 + strlen(VERSION));
	printf("\n");
        printf("* Piskworks %s *\n", VERSION);
	print_char('*', 14 + strlen(VERSION));
        printf("\n\n");
        
        #ifdef _EINSTEIN_
        printf("%s\n\n", gfx_einstein);
        #endif
        
        do {
                pisk.difficulty = 3;
                pisk.computer_starts_game = 1;
                setup_preferences();

                print_grid();
                
                do {             
                        result = get_input_con();
                        if (result == 1) {
                                /* game is aborted */
                                result = 3;
                                break;
                        }
                                
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
                
                switch (result) {
                        case 1: printf("GAME OVER!\n");
                                pisk.score_computer++;
                                printf("Computer is winner\n");
                                printf("(d=%d, e=%d, %s started)\n", pisk.difficulty, pisk.eagerness, (pisk.computer_starts_game) ? "computer" : "you");
                                break;
                        case 2: printf("GAME OVER!\n");
                                pisk.score_player++;
                                printf("You are winner\n");
                                printf("(d=%d, e=%d, %s started)\n", pisk.difficulty, pisk.eagerness, (pisk.computer_starts_game) ? "computer" : "you");
                                break;
                        case 3: printf("GAME ABORTED!\n");
                }

                printf("Computer:You  %d:%d\n", pisk.score_computer, pisk.score_player);
                c = get_option("\nAnother game? (y/n)", "YyNn");        
        } while (tolower(c) == 'y');
        
        return 0;
}


void print_char(char c, int n) {
	int i;
	for (i = 0; i < n; i++)
		putchar(c);
}


void setup_preferences(void) {
        int c;
        #ifdef _SAVE_GAME_
        int ret;
        char filename[FILELEN];
        #endif
                
        while (1) {
                printf("Preferences:\n\n");
                printf("* your stones ... %c\n", toupper(cross_char));
                printf("* difficulty  ... %d\n", pisk.difficulty);
                printf("* %s will put first move\n", (pisk.computer_starts_game) ? "computer" : "you");
                
                #ifdef _SAVE_GAME_
                c = get_option("\n(C)hange preferences, (S)tart, (L)oad", "CcSsLl");
                #else
                c = get_option("\n(C)hange preferences, (S)tart", "CcSs");
                #endif
                
                switch (tolower(c)) {
                        case 's':
                                p_create_new_game(&pisk);
                                return;
                        case 'c':
                                c = get_option("Do you want to play with X or O?", "XxOo");
                                if (tolower(c) == 'x') {
                                        cross_char = 'x';
                                        circle_char = 'o';
                                } else {
                                        cross_char = 'o';
                                        circle_char = 'x';
                                }
                                
                                c = get_option("Which difficulty (1,2,3)?", "123");
                                pisk.difficulty = c - '1' + 1;
                                
                                c = get_option("Do you want to put first move (y/n)?", "YyNn");
                                if (tolower(c) == 'y') {
                                        pisk.computer_starts_game = 0;
                                } else {
                                        pisk.computer_starts_game = 1;
                                }
                                break;
                        #ifdef _SAVE_GAME_
                        case 'l':
                                p_create_new_game(&pisk);
                                printf("Type filename:\n");
                                fgets(filename, FILELEN - 1, stdin);
                        
                                if (filename[strlen(filename) - 2] == '\n')
                                        filename[strlen(filename) - 2] = '\0';
                                if (filename[strlen(filename) - 1] == '\n')
                                        filename[strlen(filename) - 1] = '\0';
                        
                                ret = load_game(&pisk, filename);
                                if (ret == 0) {
                                        printf("Loaded\n");
                                        return;
                                } else {
                                        printf("Load failed\n");
                                }
                                break;
                        #endif
                }
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


int get_input_con(void) {
        #define LINELEN 10
        char line[LINELEN];
        int y, is_input_correct;
        char x;
        STONE s;

        do {
                is_input_correct = 0;
                                
                printf("Your move (for ex. B3) or (Q)uit");
                #ifdef _SAVE_GAME_
                printf(", (S)ave");
                #endif
                printf("\n");
                
                fgets(line, LINELEN - 1, stdin);
                
                if (((line[0] == 'q') || (line[0] == 'Q')) && (line[1] == '\n')) {
                        return 1;
                }
                
                #ifdef _SAVE_GAME_
                if (((line[0] == 's') || (line[0] == 'S')) && (line[1] == '\n')) {
                        int ret;
                        char filename[FILELEN];
                        
                        printf("Type filename:\n");
                        fgets(filename, FILELEN - 1, stdin);
                        
                        if (filename[strlen(filename) - 2] == '\n')
                                filename[strlen(filename) - 2] = '\0';
                        if (filename[strlen(filename) - 1] == '\n')
                                filename[strlen(filename) - 1] = '\0';
                        
                        ret = save_game(&pisk, filename);
                        if (ret == 0) {
                                printf("Saved\n");
                        } else {
                                printf("Save failed\n");
                        }
                        
                        print_grid();        
                        continue;
                }
                #endif
               
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
                        printf("This field is already occupied\n");
                        continue;
                }  
                
                is_input_correct = 1;
        } while (is_input_correct == 0);
        
        get_input(&pisk, (x - 'A'), y - 1);
        
        return 0;
}


void print_grid(void) {
        int x, y;
        STONE stone;
        
        printf("\nMove %d\n\n", pisk.move_cnt);
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

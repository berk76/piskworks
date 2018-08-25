/*
*       pisk_lib.h
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


#ifndef _PISK_LIB_
#define _PISK_LIB_


#define VERSION "0.6.0 SN"
#define GRID_OFFSET 2
#define grid_size_x 40
#define grid_size_y 40
#define free_double_size 50

typedef enum {UNKNOWN, NA, EMPTY, CROSS, CIRCLE} STONE;

typedef struct {
        int minx;
        int miny;
        int maxx;
        int maxy;
} GRID_SIZE;

typedef struct {
        int x;
        int y;
        int count;
        STONE stone;
} FREE_DOUBLE;

typedef struct {
        char grid[grid_size_x * grid_size_y];
        GRID_SIZE gs;
        int move_cnt;
        int last_move_x;
        int last_move_y;
        FREE_DOUBLE free_double[free_double_size];
        int free_double_last_used;
        int computer_starts_game;
        int score_computer;
        int score_player;
        int difficulty;
} PISKWORKS_T;


/*
* Create new game
* Returns PISKWORKS_T initialized
*/
extern void p_create_new_game(PISKWORKS_T *p);

/*
* Get input from human
* Returns:
* 0: input was received successfully
* 1: input failed
*/
extern int get_input(PISKWORKS_T *p, int x, int y);

/* 
* Check situation at board and computer will put his move if play == 1
* Returns:
* 0: game continue
* 1: computer is winner
* 2: human is winner
*/
extern int check_and_play(PISKWORKS_T *p, int play);

/*
* Get stone
* Returns stone at given position
*/
extern STONE get_stone(PISKWORKS_T *p, int x, int y);

#ifdef _SAVE_GAME_

/*
* Save game
* Returns:
* 0: saved successfully
* 1: save failed
*/
extern int save_game(PISKWORKS_T *p, char *filename);

/*
* Load game
* Returns:
* 0: loaded successfully
* 1: load failed
*/
extern int load_game(PISKWORKS_T *p, char *filename);

#endif

#endif

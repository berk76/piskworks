/*
*       pisk_lib.c
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


#include <stdio.h>
#include <stdlib.h>
#ifndef NO_TIME
#include <time.h>
#endif
#ifdef _SAVE_GAME_
#include <string.h>
#endif
#include "pisk_lib.h"

typedef struct {
        STONE stone;
        STONE first;
        STONE last;
        int priority;
        int random_priority;
        int empty_cnt;
        int stone_cnt;
        int stone_cnt_together;
        int move_x;
        int move_y;
        int move_is_first;
} NEXT_MOVE;

#define get_random_priority() (rand() % 10)

static int computer_count(PISKWORKS_T *p, int x, int y, int *num_x, int *num_o);
static int computer_play(PISKWORKS_T *p, int x, int y, NEXT_MOVE *nm, NEXT_MOVE *tmp_nm);
static void move_copy_higher_priority(PISKWORKS_T *p, NEXT_MOVE *dest, NEXT_MOVE *src);
static void move_copy(NEXT_MOVE *dest, NEXT_MOVE *src);
static void move_empty(NEXT_MOVE *m);
static void add_free_double(PISKWORKS_T *p, int x, int y, STONE stone, NEXT_MOVE *nm);
static void put_stone(PISKWORKS_T *p, int x, int y, STONE s);
static void clear_grid(PISKWORKS_T *p);
#ifdef _SAVE_GAME_
static char *getValueFromSavedFile(FILE *f, char *name, char *buff, int buflen);
#endif


void p_create_new_game(PISKWORKS_T *p) {
        if (p == NULL) 
                return;
	#ifndef NO_TIME
        srand(time(NULL) % 37);
	#else
	srand(1000);
	#endif
        p->eagerness = ((rand() % 3) - 1) * 2;
        clear_grid(p);
        if (p->computer_starts_game) {
                put_stone(p, 0, 0, CIRCLE);
        }
}

int get_input(PISKWORKS_T *p, int x, int y) {
        STONE s;
                
        s = get_stone(p, p->gs.minx + x, p->gs.miny + y);
        if (s != EMPTY) {
                return 1;
        }  
        
        put_stone(p, p->gs.minx + x, p->gs.miny + y, CROSS);
        return 0;
}

int check_and_play(PISKWORKS_T *p, int play) {
        int x, y, xx, yy;
        int num_x, num_o;
        int result;
        NEXT_MOVE nm, tmp_nm;
        
        move_empty(&nm);
        p->free_double_last_used = -1;
        

        /* Lines */        
        for (y = 0; y <= (p->gs.maxy - p->gs.miny); y++) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (x = 0; x <= (p->gs.maxx - p->gs.minx); x++) {
                        if (play) {
                                result = computer_play(p, x + p->gs.minx, y + p->gs.miny, &nm, &tmp_nm);
                        } else {
                                result = computer_count(p, x + p->gs.minx, y + p->gs.miny, &num_x, &num_o);
                        }
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(p, &nm, &tmp_nm);
        }
        /* Columns */
        for (x = 0; x <= (p->gs.maxx - p->gs.minx); x++) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (y = 0; y <= (p->gs.maxy - p->gs.miny); y++) {
                        if (play) {
                                result = computer_play(p, x + p->gs.minx, y + p->gs.miny, &nm, &tmp_nm);
                        } else {
                                result = computer_count(p, x + p->gs.minx, y + p->gs.miny, &num_x, &num_o);
                        }
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(p, &nm, &tmp_nm); 
        }
        /* Askew left-right */
        for (x = (p->gs.maxx - p->gs.minx); x > 0 ; x--) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (xx = x, yy = 0; (xx <= (p->gs.maxx - p->gs.minx)) && (yy <= (p->gs.maxy - p->gs.miny)); xx++, yy++) {
                        if (play) {
                                result = computer_play(p, xx + p->gs.minx, yy + p->gs.miny, &nm, &tmp_nm);
                        } else {
                                result = computer_count(p, xx + p->gs.minx, yy + p->gs.miny, &num_x, &num_o);
                        }
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(p, &nm, &tmp_nm);
        }
        for (y = 0; y <= (p->gs.maxy - p->gs.miny); y++) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (xx = 0, yy = y; (xx <= (p->gs.maxx - p->gs.minx)) && (yy <= (p->gs.maxy - p->gs.miny)); xx++, yy++) {
                        if (play) {
                                result = computer_play(p, xx + p->gs.minx, yy + p->gs.miny, &nm, &tmp_nm);
                        } else {
                                result = computer_count(p, xx + p->gs.minx, yy + p->gs.miny, &num_x, &num_o);
                        }
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(p, &nm, &tmp_nm);
        }
        /* Askew right-left */
        for (x = 0; x < (p->gs.maxx - p->gs.minx) ; x++) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (xx = x, yy = 0; (xx >= 0) && (yy <= (p->gs.maxy - p->gs.miny)); xx--, yy++) {
                        if (play) {
                                result = computer_play(p, xx + p->gs.minx, yy + p->gs.miny, &nm, &tmp_nm);
                        } else {
                                result = computer_count(p, xx + p->gs.minx, yy + p->gs.miny, &num_x, &num_o);
                        }
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(p, &nm, &tmp_nm);
        }
        for (y = 0; y <= (p->gs.maxy - p->gs.miny); y++) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (xx = (p->gs.maxx - p->gs.minx), yy = y; (xx >= 0) && (yy <= (p->gs.maxy - p->gs.miny)); xx--, yy++) {
                        if (play) {
                                result = computer_play(p, xx + p->gs.minx, yy + p->gs.miny, &nm, &tmp_nm);
                        } else {
                                result = computer_count(p, xx + p->gs.minx, yy + p->gs.miny, &num_x, &num_o);
                        }
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(p, &nm, &tmp_nm);
        }

        /* do computer move */
        if (play) {
                put_stone(p, nm.move_x, nm.move_y, CIRCLE);
        }
        
        return 0;
}

int computer_count(PISKWORKS_T *p, int x, int y, int *num_x, int *num_o) {
        STONE stone;
        
        stone = get_stone(p, x, y);
        if (stone == EMPTY) {
                *num_x = 0;
                *num_o = 0;
        }
        if (stone == CIRCLE) {
                *num_x = 0;
                *num_o += 1;
        }
        if (stone == CROSS) {
                *num_o = 0;
                *num_x += 1;                                
        }
                
        if (*num_o == 5) {
                return 1;
        }       
        if (*num_x == 5) {
                return 2;
        }
        return 0;
}

int computer_play(PISKWORKS_T *p, int x, int y, NEXT_MOVE *nm, NEXT_MOVE *tmp_nm) {
        STONE stone;
        NEXT_MOVE tmp_local;
        
        stone = get_stone(p, x, y);
        
        switch (stone) {
                /* 
                 *  You got out of board. 
                 */
                case NA:
                        move_copy_higher_priority(p, nm, tmp_nm);
                        break;
                        
                /* 
                 *  Empty field 
                 */        
                case EMPTY:
                        
                        /* Check for doubles */
                        if ((p->difficulty > 2) && (tmp_nm->move_is_first == 1) && 
                                (((tmp_nm->first == EMPTY) && (tmp_nm->stone_cnt_together == 2)) || (tmp_nm->stone_cnt_together > 2))) {
                                
                                add_free_double(p, tmp_nm->move_x, tmp_nm->move_y, tmp_nm->stone, nm);
                                add_free_double(p, x, y, tmp_nm->stone, nm);
                        }
                        
                        /* 
                         *  Finish pattern if previous was also empty
                         *  and set first as empty                          
                         */
                        if (tmp_nm->last == EMPTY) {
                                move_copy_higher_priority(p, nm, tmp_nm);
                                tmp_nm->first = EMPTY;
                        }
                        
                        /* Increase priority about one in case of empty */
                        tmp_nm->priority++;
                        tmp_nm->empty_cnt++;                 
                        
                        /* 
                         *  Set move coorditates in case of:
                         *  - new pattern (stone == unknown)
                         *  - coorditates had not been set yet
                         *  - move is first in pattern
                         *  (in case of tmp_nm->stone == unknown
                         *   leave it unknown until you will discover 
                         *   the first stone in pattern)                                                                                                   
                         */
                        if ((tmp_nm->stone == UNKNOWN) || 
                            ((tmp_nm->stone != UNKNOWN) && (tmp_nm->move_x == 0) && (tmp_nm->move_y == 0)) ||
                            ((tmp_nm->stone != UNKNOWN) && (tmp_nm->move_is_first == 1))
                           ) {
                                        tmp_nm->move_x = x;
                                        tmp_nm->move_y = y;
                                        tmp_nm->move_is_first = (tmp_nm->stone == UNKNOWN);
                        }
                        
                        tmp_nm->last = EMPTY;
                        break;
                        
                /*
                 *  Circle is computer
                 */                 
                case CIRCLE:
                        if (tmp_nm->stone == CROSS) {
                                tmp_local.last = tmp_nm->last;
                                tmp_local.move_x = tmp_nm->move_x;
                                tmp_local.move_y = tmp_nm->move_y; 
                                move_copy_higher_priority(p, nm, tmp_nm);
                                if (tmp_local.last == EMPTY) {
                                        tmp_nm->first = EMPTY;
                                        tmp_nm->last = EMPTY;
                                        tmp_nm->empty_cnt = 1;
                                        tmp_nm->move_x = tmp_local.move_x;
                                        tmp_nm->move_y = tmp_local.move_y;
                                        tmp_nm->move_is_first = 1;
                                        tmp_nm->priority++;        
                                }
                        }
                        tmp_nm->stone = CIRCLE;
                        if (tmp_nm->first == UNKNOWN)                        
                                tmp_nm->first = CIRCLE;
                        /* Increase priority about two in case of stone */
                        tmp_nm->priority += 2;
                        tmp_nm->stone_cnt++;
                        if (tmp_nm->last == CIRCLE) {                        
                                tmp_nm->stone_cnt_together++;
                        } else {
                                tmp_nm->stone_cnt_together = 1;
                        }
                        tmp_nm->last = CIRCLE;
                        
                        /* handle urgent situation */
                        if (p->difficulty > 1) {
                                if ((tmp_nm->stone_cnt_together == 2) && (tmp_nm->stone_cnt >= 3) && (tmp_nm->empty_cnt >= 3))
                                        tmp_nm->priority += 92;
                                        
                                if ((tmp_nm->stone_cnt_together == 3) && (tmp_nm->empty_cnt >= 2))
                                        tmp_nm->priority += 94;
                                        
                                if ((tmp_nm->stone_cnt_together == 4) && (tmp_nm->empty_cnt >= 1))
                                        tmp_nm->priority += 96;
                        }
                        break;
                
                /*
                 *  Cross is human
                 */        
                case CROSS:
                        if (tmp_nm->stone == CIRCLE) {
                                tmp_local.last = tmp_nm->last;
                                tmp_local.move_x = tmp_nm->move_x;
                                tmp_local.move_y = tmp_nm->move_y;
                                move_copy_higher_priority(p, nm, tmp_nm);
                                if (tmp_local.last == EMPTY) {
                                        tmp_nm->first = EMPTY;
                                        tmp_nm->last = EMPTY;
                                        tmp_nm->empty_cnt = 1;
                                        tmp_nm->move_x = tmp_local.move_x;
                                        tmp_nm->move_y = tmp_local.move_y;
                                        tmp_nm->move_is_first = 1;
                                        tmp_nm->priority++;        
                                }
                        }
                        tmp_nm->stone = CROSS;
                        if (tmp_nm->first == UNKNOWN)                        
                                tmp_nm->first = CROSS;
                        /* Increase priority about two in case of stone */
                        tmp_nm->priority += 2;
                        tmp_nm->stone_cnt++;
                        if (tmp_nm->last == CROSS) {                        
                                tmp_nm->stone_cnt_together++;
                        } else {
                                tmp_nm->stone_cnt_together = 1;
                        }
                        tmp_nm->last = CROSS;
                        
                        /* handle urgent situation */
                        if (p->difficulty > 1) {
                                if ((tmp_nm->stone_cnt_together == 2) && (tmp_nm->stone_cnt >= 3) && (tmp_nm->empty_cnt >= 3))
                                        tmp_nm->priority += 91;
                                        
                                if ((tmp_nm->stone_cnt_together == 3) && (tmp_nm->empty_cnt >= 2))
                                        tmp_nm->priority += 93;
                                        
                                if ((tmp_nm->stone_cnt_together == 4) && (tmp_nm->empty_cnt >= 1))
                                        tmp_nm->priority += 95;
                        }
                        break;
                /* 
                 *  Will never happen because 
                 *  get_stone doesn't return this value. 
                 */        
                case UNKNOWN:
                        break;
        }

        return 0;
}

void move_copy_higher_priority(PISKWORKS_T *p, NEXT_MOVE *dest, NEXT_MOVE *src) {
        /*
        if ((src->stone == CIRCLE) && (src->priority < 99))
                src->priority += p->eagerness;
        */
                
        src->random_priority = get_random_priority();
                                        
        if ((src->stone != UNKNOWN) &&
                ((src->priority > dest->priority) || ((src->priority == dest->priority) && (src->random_priority > dest->random_priority))) && 
                !((src->move_x == 0) && (src->move_y == 0)))
                        move_copy(dest, src);
                                
        move_empty(src);
}

void move_copy(NEXT_MOVE *dest, NEXT_MOVE *src) {
        dest->stone = src->stone;
        dest->first = src->first;
        dest->last = src->last;
        dest->priority = src->priority;
        dest->random_priority = src->random_priority;
        dest->empty_cnt = src->empty_cnt;
        dest->stone_cnt = src->stone_cnt;
        dest->stone_cnt_together = src->stone_cnt_together;
        dest->move_x = src->move_x;
        dest->move_y = src->move_y;
        dest->move_is_first = src->move_is_first;        
}

void move_empty(NEXT_MOVE *m) {
        m->stone = UNKNOWN;
        m->first = UNKNOWN;
        m->last = UNKNOWN;
        m->priority = 0;
        m->random_priority = 0;
        m->empty_cnt = 0;
        m->stone_cnt = 0;
        m->stone_cnt_together = 0;
        m->move_x = 0;
        m->move_y = 0;
        m->move_is_first = 1;        
}

void add_free_double(PISKWORKS_T *p, int x, int y, STONE stone, NEXT_MOVE *nm) {
        FREE_DOUBLE *pt = p->free_double;
        int i;
        
        for(i = 0; i <= p->free_double_last_used; i++) {
                if ((pt[i].x == x) && (pt[i].y == y) && (pt[i].stone == stone)) {
                        pt[i].count += 1;
                        if ((pt[i].stone == CIRCLE) && (nm->priority < 7)) {
                                move_empty(nm);
                                nm->move_x = pt[i].x;
                                nm->move_y = pt[i].y;
                                nm->priority = 7;
                        } else if ((pt[i].stone == CROSS) && (nm->priority < 5)) {
                                move_empty(nm);
                                nm->move_x = pt[i].x;
                                nm->move_y = pt[i].y;
                                nm->priority = 5;
                        }
                        return;
                }
        }
        
        p->free_double_last_used++;
        if (p->free_double_last_used < free_double_size) {
                pt[p->free_double_last_used].x = x;
                pt[p->free_double_last_used].y = y;
                pt[p->free_double_last_used].count = 1;
                pt[p->free_double_last_used].stone = stone;
        }
        
        return;
}

STONE get_stone(PISKWORKS_T *p, int x, int y) {
        x += grid_size_x / 2;
        y += grid_size_y / 2;
        
        if ((x < 0) || (x >= grid_size_x) || (y < 0) || (y >= grid_size_y))
                return NA;
        
        return p->grid[(y * grid_size_x) + x];
}

void put_stone(PISKWORKS_T *p, int x, int y, STONE s) {
        p->last_move_x = x;
        p->last_move_y = y;
        
        p->move_cnt++;
        
        if ((x - GRID_OFFSET) < p->gs.minx)
                p->gs.minx = x - GRID_OFFSET;
                 
        if ((x + GRID_OFFSET) > p->gs.maxx)
                p->gs.maxx = x + GRID_OFFSET;
                 
        if ((y - GRID_OFFSET) < p->gs.miny)
                p->gs.miny = y - GRID_OFFSET;
                 
        if ((y + GRID_OFFSET) > p->gs.maxy)
                p->gs.maxy = y + GRID_OFFSET;
                
        x += grid_size_x / 2;
        y += grid_size_y / 2;
        p->grid[(y * grid_size_x) + x] = s;
}

void clear_grid(PISKWORKS_T *p) {
        int i;
        
        p->last_move_x = -1;
        p->last_move_y = -1;
        
        for (i = 0; i < grid_size_x * grid_size_y; i++) {
                p->grid[i] = EMPTY;
        }

        p->move_cnt = 0;
                
        p->gs.minx = -GRID_OFFSET;
        p->gs.maxx = GRID_OFFSET;
        p->gs.miny = -GRID_OFFSET;
        p->gs.maxy = GRID_OFFSET; 
}

#ifdef _SAVE_GAME_

int save_game(PISKWORKS_T *p, char *filename) {
        FILE *f;
        int x, y;
        
        if (filename == NULL)
                return 1;
                
        f = fopen(filename, "wb");
        if (f == NULL)
                return 1;
                
        fprintf(f, "gs.minx:\t%d\n", p->gs.minx);
        fprintf(f, "gs.miny:\t%d\n", p->gs.miny);
        fprintf(f, "gs.maxx:\t%d\n", p->gs.maxx);
        fprintf(f, "gs.maxy:\t%d\n", p->gs.maxy);
        
        fprintf(f, "move_cnt:\t%d\n", p->move_cnt);
        fprintf(f, "last_move_x:\t%d\n", p->last_move_x);
        fprintf(f, "last_move_y:\t%d\n", p->last_move_y);
        fprintf(f, "computer_starts_game:\t%d\n", p->computer_starts_game);
        fprintf(f, "score_computer:\t%d\n", p->score_computer);
        fprintf(f, "score_player:\t%d\n", p->score_player);
        fprintf(f, "difficulty:\t%d\n", p->difficulty);
        fprintf(f, "eagerness:\t%d\n", p->eagerness);

        fprintf(f, "board:\t\n");
        for (y = 0; y < grid_size_y; y ++) {
                for (x = 0; x < grid_size_x; x ++) {
                        switch (p->grid[y * grid_size_y + x]) {
                                case CROSS:     fprintf(f, "x");
                                                break;
                                case CIRCLE:    fprintf(f, "o");
                                                break;
                                default:        fprintf(f, ".");
                        }
                }
                fprintf(f, "\n");
        }

        fclose(f);
        
        return 0;
}

int load_game(PISKWORKS_T *p, char *filename) {
        FILE *f;
        char *v;
        #define BUFLEN 100
        char buff[BUFLEN];
        
        f = fopen(filename, "rb");
        if (f == NULL)
                return 1;
        
        v = getValueFromSavedFile(f, "gs.minx:\t", buff, BUFLEN);
        if (v != NULL) p->gs.minx = atoi(v); else goto error;
        
        v = getValueFromSavedFile(f, "gs.miny:\t", buff, BUFLEN);
        if (v != NULL) p->gs.miny = atoi(v); else goto error;
        
        v = getValueFromSavedFile(f, "gs.maxx:\t", buff, BUFLEN);
        if (v != NULL) p->gs.maxx = atoi(v); else goto error;
        
        v = getValueFromSavedFile(f, "gs.maxy:\t", buff, BUFLEN);
        if (v != NULL) p->gs.maxy = atoi(v); else goto error;
        
        v = getValueFromSavedFile(f, "move_cnt:\t", buff, BUFLEN);
        if (v != NULL) p->move_cnt = atoi(v); else goto error;
        
        v = getValueFromSavedFile(f, "last_move_x:\t", buff, BUFLEN);
        if (v != NULL) p->last_move_x = atoi(v); else goto error;
        
        v = getValueFromSavedFile(f, "last_move_y:\t", buff, BUFLEN);
        if (v != NULL) p->last_move_y = atoi(v); else goto error;
        
        v = getValueFromSavedFile(f, "computer_starts_game:\t", buff, BUFLEN);
        if (v != NULL) p->computer_starts_game = atoi(v); else goto error;
        
        v = getValueFromSavedFile(f, "score_computer:\t", buff, BUFLEN);
        if (v != NULL) p->score_computer = atoi(v); else goto error;
        
        v = getValueFromSavedFile(f, "score_player:\t", buff, BUFLEN);
        if (v != NULL) p->score_player = atoi(v); else goto error;
        
        v = getValueFromSavedFile(f, "difficulty:\t", buff, BUFLEN);
        if (v != NULL) p->difficulty = atoi(v); else goto error;
        
        v = getValueFromSavedFile(f, "eagerness:\t", buff, BUFLEN);
        if (v != NULL) p->eagerness = atoi(v); else goto error;
        
        v = getValueFromSavedFile(f, "board:", buff, BUFLEN);
        if (v != NULL) {
                int i;
                int c;

                for (i = 0; i < grid_size_y * grid_size_x; i++) {
                        c = fgetc(f);
                        
                        if (strchr(".xo",c) == NULL) {
                                i--;
                                continue;
                        }
                        
                        switch (c) {
                                case '.': p->grid[i] = EMPTY;
                                          break;
                                case 'x': p->grid[i] = CROSS;
                                          break;
                                case 'o': p->grid[i] = CIRCLE;
                                          break;
                        }
                }
        }  else {
                goto error;
        }
        
        fclose(f);
        return 0;        
error:
        fclose(f);
        return 1;
}


char *getValueFromSavedFile(FILE *f, char *name, char *buff, int buflen) {
        char *ret;
        
        if ((f == NULL) || (name == NULL))
                return NULL;
                
        fseek(f, 0, SEEK_SET);
        
        while (fgets(buff, buflen, f) != NULL) {
                ret = strstr(buff, name);
                if (ret != NULL) {
                        ret += strlen(name);
                        return ret;
                }
        }
        
        return NULL;
}

#endif

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
#include "pisk_lib.h"

#define get_random_priority() (rand() % 10)

static int computer_count(PISKWORKS_T *p, int x, int y, int *num_x, int *num_o);
static int computer_play(PISKWORKS_T *p, int x, int y, NEXT_MOVE *nm, NEXT_MOVE *tmp_nm);
static void move_copy_higher_priority(PISKWORKS_T *p, NEXT_MOVE *dest, NEXT_MOVE *src);
static void move_copy(NEXT_MOVE *dest, NEXT_MOVE *src);
static void move_empty(NEXT_MOVE *m);
static void add_free_double(PISKWORKS_T *p, int x, int y, STONE stone, NEXT_MOVE *nm);
static void put_stone(PISKWORKS_T *p, int x, int y, STONE s);
static void clear_grid(PISKWORKS_T *p);


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
        NEXT_MOVE tmp;
        
        stone = get_stone(p, x, y);
        if (stone == NA) {
                move_copy_higher_priority(p, nm, tmp_nm);
        } else
        if (stone == EMPTY) {                             
                if (tmp_nm->last == EMPTY) {
                        move_copy_higher_priority(p, nm, tmp_nm);
                }
                
                /* add doubles*/
                if ((p->difficulty > 2) && (tmp_nm->first == EMPTY) && 
                        (tmp_nm->move_is_first == 1) && (tmp_nm->stone_cnt_together == 2)) {
                        
                        add_free_double(p, tmp_nm->move_x, tmp_nm->move_y, tmp_nm->stone, nm);
                        add_free_double(p, x, y, tmp_nm->stone, nm);
                }
                
                if (tmp_nm->first == UNKNOWN) 
                        tmp_nm->first = EMPTY;
                
                tmp_nm->priority++;
                tmp_nm->empty_cnt++;                 
                
                if ((tmp_nm->stone == UNKNOWN) || 
                        ((tmp_nm->stone != UNKNOWN) && (tmp_nm->move_x == 0) && (tmp_nm->move_y == 0)) ||
                        ((tmp_nm->stone != UNKNOWN) && (tmp_nm->move_is_first == 1))
                        ) {
                                tmp_nm->move_x = x;
                                tmp_nm->move_y = y;
                                tmp_nm->move_is_first = (tmp_nm->stone == UNKNOWN);
                }
                
                tmp_nm->last = EMPTY;
        } else
        if (stone == CIRCLE) {                                                
                if (tmp_nm->stone == CROSS) {
                        tmp.last = tmp_nm->last;
                        tmp.move_x = tmp_nm->move_x;
                        tmp.move_y = tmp_nm->move_y; 
                        move_copy_higher_priority(p, nm, tmp_nm);
                        if (tmp.last == EMPTY) {
                                tmp_nm->first = EMPTY;
                                tmp_nm->last = EMPTY;
                                tmp_nm->empty_cnt = 1;
                                tmp_nm->move_x = tmp.move_x;
                                tmp_nm->move_y = tmp.move_y;
                                tmp_nm->move_is_first = 1;
                                tmp_nm->priority++;        
                        }
                }
                tmp_nm->stone = CIRCLE;
                if (tmp_nm->first == UNKNOWN)                        
                        tmp_nm->first = CIRCLE;
                tmp_nm->priority += 2;
                tmp_nm->stone_cnt++;
                if (tmp_nm->last == CIRCLE) {                        
                        tmp_nm->stone_cnt_together++;
                } else {
                        tmp_nm->stone_cnt_together = 1;
                }
                tmp_nm->last = CIRCLE;                
        } else
        if (stone == CROSS) {
                if (tmp_nm->stone == CIRCLE) {
                        tmp.last = tmp_nm->last;
                        tmp.move_x = tmp_nm->move_x;
                        tmp.move_y = tmp_nm->move_y;
                        move_copy_higher_priority(p, nm, tmp_nm);
                        if (tmp.last == EMPTY) {
                                tmp_nm->first = EMPTY;
                                tmp_nm->last = EMPTY;
                                tmp_nm->empty_cnt = 1;
                                tmp_nm->move_x = tmp.move_x;
                                tmp_nm->move_y = tmp.move_y;
                                tmp_nm->move_is_first = 1;
                                tmp_nm->priority++;        
                        }
                }
                tmp_nm->stone = CROSS;
                if (tmp_nm->first == UNKNOWN)                        
                        tmp_nm->first = CROSS;
                tmp_nm->priority += 2;
                tmp_nm->stone_cnt++;
                if (tmp_nm->last == CROSS) {                        
                        tmp_nm->stone_cnt_together++;
                } else {
                        tmp_nm->stone_cnt_together = 1;
                }
                tmp_nm->last = CROSS;                                
        }
        
        /* handle urgent situation */
        if (p->difficulty > 1) {
                
                if ((tmp_nm->stone == CROSS) && (tmp_nm->stone_cnt_together >= 2) && (tmp_nm->stone_cnt >= 3) && (tmp_nm->empty_cnt >= 3))
                                tmp_nm->priority = 100;
                if ((tmp_nm->stone == CIRCLE) && (tmp_nm->stone_cnt_together >= 2) && (tmp_nm->stone_cnt >= 3) && (tmp_nm->empty_cnt >= 3))
                                tmp_nm->priority = 101;
                                
                if ((tmp_nm->stone == CROSS) && (tmp_nm->stone_cnt_together == 3) && (tmp_nm->empty_cnt >= 2))
                                tmp_nm->priority = 102;
                if ((tmp_nm->stone == CIRCLE) && (tmp_nm->stone_cnt_together == 3) && (tmp_nm->empty_cnt >= 2))
                                tmp_nm->priority = 103;
                
                if ((tmp_nm->stone == CROSS) && (tmp_nm->stone_cnt_together == 4) && (tmp_nm->empty_cnt >= 1))
                                tmp_nm->priority = 104;
                if ((tmp_nm->stone == CIRCLE) && (tmp_nm->stone_cnt_together == 4) && (tmp_nm->empty_cnt >= 1))
                                tmp_nm->priority = 105;
                                
                if (tmp_nm->priority > 99)
                        move_copy_higher_priority(p, nm, tmp_nm);
        }

        return 0;
}

void move_copy_higher_priority(PISKWORKS_T *p, NEXT_MOVE *dest, NEXT_MOVE *src) {
        if ((src->stone == CIRCLE) && (src->priority < 99))
                src->priority += p->eagerness;
                
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
                        if ((pt[i].stone == CIRCLE) && (nm->priority < 50)) {
                                move_empty(nm);
                                nm->move_x = pt[i].x;
                                nm->move_y = pt[i].y;
                                nm->priority = 50;
                        } else if ((pt[i].stone == CROSS) && (nm->priority < 49)) {
                                move_empty(nm);
                                nm->move_x = pt[i].x;
                                nm->move_y = pt[i].y;
                                nm->priority = 49;
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
                
        f = fopen(filename, "w");
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

#endif

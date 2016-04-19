/*
*       piskworks.c
*       
*       This file is part of Piskworks game.
*       https://bitbucket.org/berk76/piskworks
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

#define VERSION "0.4.1"
#define GRID_OFFSET 2
#define get_random_priority() (rand() % 10)
 

typedef enum {UNKNOWN, NA, EMPTY, CROSS, CIRCLE} STONE; 

typedef struct {
        int minx;
        int miny;
        int maxx;
        int maxy;
} GRID_SIZE;

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

typedef struct {
        int x;
        int y;
        int count;
        STONE stone;
} FREE_DOUBLE;


/* grid */
#define grid_size_x 40
#define grid_size_y 40
static STONE grid[grid_size_x * grid_size_y];
static GRID_SIZE gs;
static int move_cnt;
static int last_move_x;
static int last_move_y;

/* free double vector */
#define free_double_size 50
static FREE_DOUBLE free_double[free_double_size];
static int free_double_last_used;

static char cross_char = 'x';
static char circle_char = 'o';
static int computer_starts_game = 1;
static int score_computer, score_player;
static int difficulty = 3;
static int eagerness = 2;


static void setup_preferences();
static int get_option(char *message, char *values);
static void get_input();
static int check_and_play(int play);
static int computer_count(int x, int y, int *num_x, int *num_o);
static int computer_play(int x, int y, NEXT_MOVE *nm, NEXT_MOVE *tmp_nm);
static void move_copy_higher_priority(NEXT_MOVE *dest, NEXT_MOVE *src);
static void move_copy(NEXT_MOVE *dest, NEXT_MOVE *src);
static void move_empty(NEXT_MOVE *m);
static void add_free_double(int x, int y, STONE stone, NEXT_MOVE *nm);
static void print_grid();
static STONE get_stone(int x, int y);
static void put_stone(int x, int y, STONE s);
static void clear_grid();


int main(int argc, char **argv) {
        int result, c;
        
        /* For Z88DK compiler */
        #ifdef SCCZ80
        printf("%c%c", 1, 32);   /* 32 characters */
        printf("%c",12);         /* cls */
        #endif
        
        srand(time(NULL) % 37);
        
        score_computer = 0;
        score_player = 0;
        
        printf("*******************\n");
        printf("* Piskworks %s *\n", VERSION);
        printf("*******************\n\n");
        do {
                setup_preferences();
                eagerness = (rand() % 5) - 2;
                printf("eagerness=%d", eagerness);
                    
                clear_grid();
                if (computer_starts_game) {
                        put_stone(0, 0, CIRCLE);
                }
                
                print_grid();
                result = 0;
                
                do {             
                        get_input();
                        print_grid();
                        result = check_and_play(0);
                        
                        if (result == 0) {
                                result = check_and_play(1);
                                print_grid();
                        }
                        
                        if (result == 0) {
                                result = check_and_play(0);
                        }
        
                } while (result == 0);
                
                printf("GAME OVER!\n");
                if (result == 1) {
                        score_computer++;
                        printf("Computer is winner\n");
                } else {
                        score_player++;
                        printf("You are winner\n");
                }
                printf("(difficulty=%d, %s started)\n", difficulty, (computer_starts_game) ? "computer" : "you");        
                
                printf("Computer:You  %d:%d\n", score_computer, score_player);
                c = get_option("\nAnother game? (y/n)", "YyNn");        
        } while (tolower(c) == 'y');
        
        return 0;
}

void setup_preferences() {
        int c;
                
        printf("Preferences:\n\n");
        printf("* your stones ... %c\n", toupper(cross_char));
        printf("* difficulty  ... %d\n", difficulty);
        printf("* %s will put first move\n", (computer_starts_game) ? "computer" : "you");
        
        c = get_option("\n(C)hange preferences,(S)tart", "CcSs");
        
        if (tolower(c) == 'c') {
                
                c = get_option("Do you want to play with X or O?", "XxOo");
                if (tolower(c) == 'x') {
                        cross_char = 'x';
                        circle_char = 'o';
                } else {
                        cross_char = 'o';
                        circle_char = 'x';
                }
                
                c = get_option("Which difficulty (1,2,3)?", "123");
                difficulty = c - '1' + 1;
                
                c = get_option("Do you want to put first move (y/n)?", "YyNn");
                if (tolower(c) == 'y') {
                        computer_starts_game = 0;
                } else {
                        computer_starts_game = 1;
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

void get_input() {
        #define LINELEN 256
        char line[LINELEN];
        int y, is_input_correct;
        char x;
        STONE s;

        do {
                is_input_correct = 0;                
                printf("Put your move. (for ex. B3)\n");
                fgets(line, LINELEN - 1, stdin);
               
                if (strlen(line) < 2)
                        continue;
                
                x = toupper(*line);
                if ((x < 'A') || (x > 'Z'))
                        continue;
                
                y = atoi(line + 1);                
                if (y == 0)
                        continue;
                        
                s = get_stone(gs.minx + (x - 'A'), gs.miny + y - 1);
                if (s != EMPTY) {
                        printf("This field is already occupied\n");
                        continue;
                }  
                
                is_input_correct = 1;
        } while (is_input_correct == 0);
        
        put_stone(gs.minx + (x - 'A'), gs.miny + y - 1, CROSS);
}

int check_and_play(int play) {
        int i, x, y, xx, yy;
        int num_x, num_o;
        int result;
        NEXT_MOVE nm, tmp_nm;
        
        move_empty(&nm);
        free_double_last_used = -1;
        

        /* Lines */        
        for (y = 0; y <= (gs.maxy - gs.miny); y++) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (x = 0; x <= (gs.maxx - gs.minx); x++) {
                        if (play) {
                                result = computer_play(x + gs.minx, y + gs.miny, &nm, &tmp_nm);
                        } else {
                                result = computer_count(x + gs.minx, y + gs.miny, &num_x, &num_o);
                        }
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(&nm, &tmp_nm);
        }
        /* Columns */
        for (x = 0; x <= (gs.maxx - gs.minx); x++) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (y = 0; y <= (gs.maxy - gs.miny); y++) {
                        if (play) {
                                result = computer_play(x + gs.minx, y + gs.miny, &nm, &tmp_nm);
                        } else {
                                result = computer_count(x + gs.minx, y + gs.miny, &num_x, &num_o);
                        }
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(&nm, &tmp_nm); 
        }
        /* Askew left-right */
        for (x = (gs.maxx - gs.minx); x > 0 ; x--) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (xx = x, yy = 0; (xx <= (gs.maxx - gs.minx)) && (yy <= (gs.maxy - gs.miny)); xx++, yy++) {
                        if (play) {
                                result = computer_play(xx + gs.minx, yy + gs.miny, &nm, &tmp_nm);
                        } else {
                                result = computer_count(xx + gs.minx, yy + gs.miny, &num_x, &num_o);
                        }
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(&nm, &tmp_nm);
        }
        for (y = 0; y <= (gs.maxy - gs.miny); y++) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (xx = 0, yy = y; (xx <= (gs.maxx - gs.minx)) && (yy <= (gs.maxy - gs.miny)); xx++, yy++) {
                        if (play) {
                                result = computer_play(xx + gs.minx, yy + gs.miny, &nm, &tmp_nm);
                        } else {
                                result = computer_count(xx + gs.minx, yy + gs.miny, &num_x, &num_o);
                        }
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(&nm, &tmp_nm);
        }
        /* Askew right-left */
        for (x = 0; x < (gs.maxx - gs.minx) ; x++) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (xx = x, yy = 0; (xx >= 0) && (yy <= (gs.maxy - gs.miny)); xx--, yy++) {
                        if (play) {
                                result = computer_play(xx + gs.minx, yy + gs.miny, &nm, &tmp_nm);
                        } else {
                                result = computer_count(xx + gs.minx, yy + gs.miny, &num_x, &num_o);
                        }
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(&nm, &tmp_nm);
        }
        for (y = 0; y <= (gs.maxy - gs.miny); y++) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (xx = (gs.maxx - gs.minx), yy = y; (xx >= 0) && (yy <= (gs.maxy - gs.miny)); xx--, yy++) {
                        if (play) {
                                result = computer_play(xx + gs.minx, yy + gs.miny, &nm, &tmp_nm);
                        } else {
                                result = computer_count(xx + gs.minx, yy + gs.miny, &num_x, &num_o);
                        }
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(&nm, &tmp_nm);
        }

        /* do computer move */
        if (play) {
                put_stone(nm.move_x, nm.move_y, CIRCLE);
                #ifdef DEBUG
                printf("Priority: %d Count together: %d\n", nm.priority, nm.stone_cnt_together);
                #endif
        }
        
        return 0;
}

int computer_count(int x, int y, int *num_x, int *num_o) {
        STONE stone;
        
        stone = get_stone(x, y);
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

int computer_play(int x, int y, NEXT_MOVE *nm, NEXT_MOVE *tmp_nm) {
        STONE stone;
        NEXT_MOVE tmp;
        
        stone = get_stone(x, y);
        if (stone == NA) {
                move_copy_higher_priority(nm, tmp_nm);
        } else
        if (stone == EMPTY) {                             
                if (tmp_nm->last == EMPTY) {
                        move_copy_higher_priority(nm, tmp_nm);
                }
                
                /* add doubles*/
                if ((difficulty > 2) && (tmp_nm->first == EMPTY) && 
                        (tmp_nm->move_is_first == 1) && (tmp_nm->stone_cnt_together == 2)) {
                        #ifdef DEBUG
                        printf("Adding doubles %s\n", (tmp_nm->stone == CROSS) ? "cross" : "circle");
                        #endif
                        
                        add_free_double(tmp_nm->move_x, tmp_nm->move_y, tmp_nm->stone, nm);
                        add_free_double(x, y, tmp_nm->stone, nm);
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
                        move_copy_higher_priority(nm, tmp_nm);
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
                        move_copy_higher_priority(nm, tmp_nm);
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
        if (difficulty > 1) {
                if ((tmp_nm->stone == CROSS) && (tmp_nm->stone_cnt_together == 3) && (tmp_nm->empty_cnt >= 2))
                                tmp_nm->priority = 100;
                if ((tmp_nm->stone == CROSS) && (tmp_nm->stone_cnt >= 3) && (tmp_nm->empty_cnt >= 3))
                                tmp_nm->priority = 100;
                                
                if ((tmp_nm->stone == CIRCLE) && (tmp_nm->stone_cnt_together == 3) && (tmp_nm->empty_cnt >= 2))
                                tmp_nm->priority = 101;
                if ((tmp_nm->stone == CIRCLE) && (tmp_nm->stone_cnt >= 3) && (tmp_nm->empty_cnt >= 3))
                                tmp_nm->priority = 101;
                
                if ((tmp_nm->stone == CROSS) && (tmp_nm->stone_cnt_together == 4) && (tmp_nm->empty_cnt >= 1))
                                tmp_nm->priority = 102;
                if ((tmp_nm->stone == CIRCLE) && (tmp_nm->stone_cnt_together == 4) && (tmp_nm->empty_cnt >= 1))
                                tmp_nm->priority = 103;
                                
                if (tmp_nm->priority > 99)
                        move_copy_higher_priority(nm, tmp_nm);
        }

        return 0;
}

void move_copy_higher_priority(NEXT_MOVE *dest, NEXT_MOVE *src) {
        if ((src->stone == CIRCLE) && (src->priority < 99))
                src->priority += eagerness;
                
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

void add_free_double(int x, int y, STONE stone, NEXT_MOVE *nm) {
        FREE_DOUBLE *p = free_double;
        int i;
        
        for(i = 0; i <= free_double_last_used; i++) {
                if ((p[i].x == x) && (p[i].y == y) && (p[i].stone == stone)) {
                        p[i].count += 1;
                        if ((p[i].stone == CIRCLE) && (nm->priority < 50)) {
                                move_empty(nm);
                                nm->move_x = p[i].x;
                                nm->move_y = p[i].y;
                                nm->priority = 50;
                        } else if ((p[i].stone == CROSS) && (nm->priority < 49)) {
                                move_empty(nm);
                                nm->move_x = p[i].x;
                                nm->move_y = p[i].y;
                                nm->priority = 49;
                        }
                        return;
                }
        }
        
        free_double_last_used++;
        if (free_double_last_used < free_double_size) {
                p[free_double_last_used].x = x;
                p[free_double_last_used].y = y;
                p[free_double_last_used].count = 1;
                p[free_double_last_used].stone = stone;
        }
        
        return;
}

void print_grid() {
        int x, y;
        STONE stone;
        
        printf("\nMove %d\n\n", move_cnt);
        printf("  ");
        #ifndef SCCZ80 
        putchar(' '); 
        #endif
        for (x = 0; x <= (gs.maxx - gs.minx); x++) {
                putchar('A' + x);
                #ifndef SCCZ80 
                putchar(' '); 
                #endif
        }
        putchar('\n');
        
        for (y = 0; y <= (gs.maxy - gs.miny); y++) {
                /* For Z88DK compiler */
                #ifdef SCCZ80
                printf("%d", (y + 1) / 10);
                printf("%d", (y + 1) % 10);
                #else
                printf("%2d", y + 1);
                putchar(' ');
                #endif
                for (x = 0; x <= (gs.maxx - gs.minx); x++) {
                        stone = get_stone(x + gs.minx, y + gs.miny);
                        if (stone == EMPTY) {
                                putchar('.');
                        } else
                        if (stone == CIRCLE) {
                                if ((last_move_x == (x + gs.minx)) && (last_move_y == (y + gs.miny))) {
                                        putchar(toupper(circle_char));
                                } else {
                                        putchar(circle_char);
                                }
                        } else
                        if (stone == CROSS) {
                                if ((last_move_x == (x + gs.minx)) && (last_move_y == (y + gs.miny))) {
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

STONE get_stone(int x, int y) {
        x += grid_size_x / 2;
        y += grid_size_y / 2;
        
        if ((x < 0) || (x >= grid_size_x) || (y < 0) || (y >= grid_size_y))
                return NA;
        
        return grid[(y * grid_size_x) + x];
}

void put_stone(int x, int y, STONE s) {
        last_move_x = x;
        last_move_y = y;
        
        move_cnt++;
        
        if ((x - GRID_OFFSET) < gs.minx)
                gs.minx = x - GRID_OFFSET;
                 
        if ((x + GRID_OFFSET) > gs.maxx)
                gs.maxx = x + GRID_OFFSET;
                 
        if ((y - GRID_OFFSET) < gs.miny)
                gs.miny = y - GRID_OFFSET;
                 
        if ((y + GRID_OFFSET) > gs.maxy)
                gs.maxy = y + GRID_OFFSET;
                
        x += grid_size_x / 2;
        y += grid_size_y / 2;
        grid[(y * grid_size_x) + x] = s;
}

void clear_grid() {
        int i;
        
        last_move_x = -1;
        last_move_y = -1;
        
        for (i = 0; i < grid_size_x * grid_size_y; i++) {
                grid[i] = EMPTY;
        }

        move_cnt = 0;
                
        gs.minx = -GRID_OFFSET;
        gs.maxx = GRID_OFFSET;
        gs.miny = -GRID_OFFSET;
        gs.maxy = GRID_OFFSET; 
}

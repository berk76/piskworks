/*
*       Piskworks
*       Jaroslav Beran
*       jaroslav.beran@gmail.com
*       24.2.2016
*/

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define GRID_OFFSET 2

typedef enum {EMPTY, CROSS, CIRCLE} STONE; 

typedef struct {
        int x;
        int y;
        STONE s;
} FIELD;

typedef struct {
        int minx;
        int miny;
        int maxx;
        int maxy;
} GRID_SIZE;

typedef struct {
        STONE first;
        STONE last;
        int priority;
        int empty_cnt;
        int stone_cnt;
        int move_x;
        int move_y;
} NEXT_MOVE;

static FIELD *grid;
static size_t grid_size;
static size_t grid_last_used;
#define GRID_ALLOC_BLOCK 1024;


static void get_input();
static int check_and_play(int play);
static int computer_play_count(int x, int y, int *num_x, int *num_o, NEXT_MOVE *nm, NEXT_MOVE *tmp_nm);
static void print_grid(int move_no);
static void get_grid_size(GRID_SIZE *gs);
static void move_copy_higher_priority(NEXT_MOVE *dest, NEXT_MOVE *src);
static void move_copy(NEXT_MOVE *dest, NEXT_MOVE *src);
static void move_empty(NEXT_MOVE *m);
static STONE get_stone(int x, int y);
static void allocate_grid();
static void deallocate_grid();


int main(int argc, char **argv) {
        int result, move_no;
        
        allocate_grid();
        grid_last_used = 0;
        move_no = 0;
        grid[grid_last_used].x = 0;
        grid[grid_last_used].y = 0;
        grid[grid_last_used].s = CIRCLE;
        print_grid(++move_no);
        result = 0;
        
        do {
                if (grid_size <= grid_last_used + 2)
                        allocate_grid();
                         
                get_input();
                print_grid(++move_no);
                result = check_and_play(0);
                
                if (result == 0) {
                        result = check_and_play(1);
                        print_grid(++move_no);
                }
                
                if (result == 0) {
                        result = check_and_play(0);
                }

        } while (result == 0);
        
        printf("GAME OVER!\n");
        if (result == 1) {
                printf("Computer is winner\n");
        } else {
                printf("You are winner\n");
        }        
        
        printf("Press a key...\n");
        getchar();        
        deallocate_grid();
        return 0;
}

void get_input() {
        #define LINELEN 256
        char *pc, line[LINELEN];
        int y, is_input_correct;
        char x;
        GRID_SIZE gs;
        STONE s;

        get_grid_size(&gs);

        do {
                is_input_correct = 0;                
                printf("Put your move. (for ex. B-3)\n");
                fgets(line, LINELEN - 1, stdin);
               
                pc = strchr(line, '-');
                if (pc == NULL)
                        continue;
                
                if ((pc - line) != 1)
                        continue;
                
                x = toupper(*line);
                if ((x < 'A') || (x > 'Z'))
                        continue;
                
                y = atoi(pc + 1);                
                if (y == 0)
                        continue;
                        
                s = get_stone(gs.minx + (x - 'A'), gs.miny + y - 1);
                if (s != EMPTY) {
                        printf("This field is already occupied\n");
                        continue;
                }  
                
                is_input_correct = 1;
        } while (is_input_correct == 0);
        
        printf("\nYour choice was %c-%d\n", x, y);
        
        grid_last_used++;
        grid[grid_last_used].x = gs.minx + (x - 'A');
        grid[grid_last_used].y = gs.miny + y - 1;
        grid[grid_last_used].s = CROSS;
}

int check_and_play(int play) {
        int i, x, y, xx, yy;
        int num_x, num_o;
        GRID_SIZE gs;
        int result;
        NEXT_MOVE nm, tmp_nm;
        
        get_grid_size(&gs);
        move_empty(&nm);
        

        /* Lines */        
        for (y = 0; y <= (gs.maxy - gs.miny); y++) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (x = 0; x <= (gs.maxx - gs.minx); x++) {
                        result = computer_play_count(x + gs.minx, y + gs.miny, &num_x, &num_o, &nm, &tmp_nm);
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(&nm, &tmp_nm);
        }
        /* Rows */
        for (x = 0; x <= (gs.maxx - gs.minx); x++) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (y = 0; y <= (gs.maxy - gs.miny); y++) {
                        result = computer_play_count(x + gs.minx, y + gs.miny, &num_x, &num_o, &nm, &tmp_nm);
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(&nm, &tmp_nm); 
        }
        /* Askew left-right */
        for (x = (gs.maxx - gs.minx); x >= 0 ; x--) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (xx = x, yy = 0; (xx <= (gs.maxx - gs.minx)) && (yy <= (gs.maxy - gs.miny)); xx++, yy++) {
                        result = computer_play_count(xx + gs.minx, yy + gs.miny, &num_x, &num_o, &nm, &tmp_nm);
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
                        result = computer_play_count(xx + gs.minx, yy + gs.miny, &num_x, &num_o, &nm, &tmp_nm);
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(&nm, &tmp_nm);
        }
        /* Askew right-left */
        for (x = 0; x <= (gs.maxx - gs.minx) ; x++) {
                num_x = 0;
                num_o = 0;
                move_empty(&tmp_nm);
                for (xx = x, yy = 0; (xx >= 0) && (yy <= (gs.maxy - gs.miny)); xx--, yy++) {
                        result = computer_play_count(xx + gs.minx, yy + gs.miny, &num_x, &num_o, &nm, &tmp_nm);
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
                        result = computer_play_count(xx + gs.minx, yy + gs.miny, &num_x, &num_o, &nm, &tmp_nm);
                        if (result != 0)
                                return result;
                }
                move_copy_higher_priority(&nm, &tmp_nm);
        }

        /* do computer move */
        if (play) {
                grid_last_used++;
                grid[grid_last_used].x = nm.move_x;
                grid[grid_last_used].y = nm.move_y;
                grid[grid_last_used].s = CIRCLE;
        }
        
        return 0;
}

int computer_play_count(int x, int y, int *num_x, int *num_o, NEXT_MOVE *nm, NEXT_MOVE *tmp_nm) {
        STONE stone;
        
        stone = get_stone(x, y);
        if (stone == EMPTY) {
                *num_x = 0;
                *num_o = 0;
                                
                if (tmp_nm->last != EMPTY) {
                        tmp_nm->priority++;                 
                } else 
                if (tmp_nm->first == EMPTY) {
                        tmp_nm->priority = 1;
                }
                
                if (tmp_nm->stone_cnt >= 3) {
                        tmp_nm->priority += 2;
                }
                
                if ((tmp_nm->first == EMPTY) || 
                        ((tmp_nm->last != EMPTY) && !((tmp_nm->move_x == 0) && (tmp_nm->move_x == 0)) && (tmp_nm->stone_cnt < 5))
                        ) {
                                tmp_nm->move_x = x;
                                tmp_nm->move_y = y;
                }
                
                tmp_nm->last = EMPTY;
                tmp_nm->empty_cnt++;
        }
        if (stone == CIRCLE) {
                *num_x = 0;
                *num_o += 1;
                                                
                if (tmp_nm->first == CROSS) 
                        move_copy_higher_priority(nm, tmp_nm);
                tmp_nm->first = CIRCLE;
                tmp_nm->priority += 2;
                tmp_nm->stone_cnt++;
                tmp_nm->last = CIRCLE;                
        }
        if (stone == CROSS) {
                *num_o = 0;
                *num_x += 1;
                
                if (tmp_nm->first == CIRCLE) {
                        move_copy_higher_priority(nm, tmp_nm);
                        /* defensive priority */
                        tmp_nm->priority ++;
                }
                tmp_nm->first = CROSS;
                tmp_nm->priority += 2;
                tmp_nm->stone_cnt++;
                tmp_nm->last = CROSS;                                
        }

        
        if (*num_o == 5) {
                return 1;
        }       
        if (*num_x == 5) {
                return 2;
        }
        return 0;
}

void move_copy_higher_priority(NEXT_MOVE *dest, NEXT_MOVE *src) {
        if ((src->first != EMPTY) &&
                ((src->empty_cnt + src->stone_cnt) > 4) &&
                (src->priority > dest->priority) && 
                !((src->move_x == 0) && (src->move_y == 0)))
                        move_copy(dest, src);
                                
        move_empty(src);
}

void move_copy(NEXT_MOVE *dest, NEXT_MOVE *src) {
        dest->first = src->first;
        dest->last = src->last;
        dest->priority = src->priority;
        dest->empty_cnt = src->empty_cnt;
        dest->stone_cnt = src->stone_cnt;
        dest->move_x = src->move_x;
        dest->move_y = src->move_y;        
}

void move_empty(NEXT_MOVE *m) {
        m->first = EMPTY;
        m->last = EMPTY;
        m->priority = 0;
        m->empty_cnt = 0;
        m->stone_cnt = 0;
        m->move_x = 0;
        m->move_y = 0;        
}

void print_grid(int move_no) {
        size_t x, y;
        STONE stone;
        GRID_SIZE gs;
        
        get_grid_size(&gs);
        printf("\n  Move No. #%d\n", move_no);
        printf("  ");
        for (x = 0; x <= (gs.maxx - gs.minx); x++) {
                putchar('A' + x);
        }
        putchar('\n');
        
        for (y = 0; y <= (gs.maxy - gs.miny); y++) {
                printf("%2d", y + 1);
                for (x = 0; x <= (gs.maxx - gs.minx); x++) {
                        stone = get_stone(x + gs.minx, y + gs.miny);
                        if (stone == EMPTY) {
                                putchar('.');
                                continue;
                        }
                        if (stone == CIRCLE) {
                                putchar('o');
                                continue;
                        }
                        if (stone == CROSS) {
                                putchar('x');
                                continue;
                        }                        
                }
                putchar('\n');
        }
}

void get_grid_size(GRID_SIZE *gs) {
        int i;
        
        gs->minx = 0;
        gs->maxx = 0;
        gs->miny = 0;
        gs->maxy = 0;
                
        for (i = 0; i <= grid_last_used; i++) {
                if (gs->minx > grid[i].x)
                        gs->minx = grid[i].x; 
                if (gs->miny > grid[i].y)
                        gs->miny = grid[i].y;
                if (gs->maxx < grid[i].x)
                        gs->maxx = grid[i].x;
                if (gs->maxy < grid[i].y)
                        gs->maxy = grid[i].y;
        }
        
        gs->minx -= GRID_OFFSET;
        gs->maxx += GRID_OFFSET;
        gs->miny -= GRID_OFFSET;
        gs->maxy += GRID_OFFSET;
}

STONE get_stone(int x, int y) {
        int i;
        
        for (i = 0; i <= grid_last_used; i++) {
                if ((grid[i].x == x) && (grid[i].y == y)) {
                        return grid[i].s;
                }
        }       
        return EMPTY;
}

void allocate_grid() {
        if (grid_size == 0) {
                grid_size = GRID_ALLOC_BLOCK;
                grid = (FIELD *) malloc (sizeof(FIELD) * grid_size);
                assert(grid != NULL);  
        } else {
                grid_size += GRID_ALLOC_BLOCK;
                grid = (FIELD *) realloc (grid, sizeof(FIELD) * grid_size);
                assert(grid != NULL);
        }
}

void deallocate_grid() {
        free((void *) grid);
        grid_size = 0;
        grid_last_used = 0;        
}

/*
*       Piskworks
*       Jaroslav Beran
*       24.2.2016
*/

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define GRID_OFFSET 2

typedef enum {CROSS, CIRCLE} STONE; 

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


static FIELD *grid;
static size_t grid_size;
static size_t grid_last_used;
#define GRID_ALLOC_BLOCK 1024;


static void get_input();
static int computer_play();
static int computer_play_count(GRID_SIZE *gs, int x, int y, int *num_x, int *num_o);
static void print_grid();
static void get_grid_size(GRID_SIZE *gs);
static STONE *get_stone(int x, int y);
static void allocate_grid();
static void deallocate_grid();


int main(int argc, char **argv) {
        allocate_grid();
        
        grid_last_used = 0;
        grid[grid_last_used].x = 0;
        grid[grid_last_used].y = 0;
        grid[grid_last_used].s = CIRCLE;
        int result;
        
        do {
                if (grid_size <= grid_last_used + 2)
                        allocate_grid();
                         
                print_grid();
                get_input();
                print_grid();
                result = computer_play();                 
        } while (result == 0);
        
        print_grid();
        printf("GAME OVER!\n");
        if (result == 1) {
                printf("Computer is winner\n");
        } else {
                printf("You are winner\n");
        }        
        
        deallocate_grid();
        return 0;
}

void get_input() {
        #define LINELEN 256
        char *pc, line[LINELEN];
        int y, is_input_correct;
        char x;
        GRID_SIZE gs;
        STONE *s;

        get_grid_size(&gs);

        do {
                is_input_correct = 0;                
                printf("Put your move. (for ex. B-9)\n");
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
                if (s != NULL) {
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

int computer_play() {
        int i, x, y, xx, yy;
        int num_x, num_o;
        GRID_SIZE gs;
        int result;
        
        get_grid_size(&gs);

        /* Lines */        
        for (y = 0; y <= (gs.maxy - gs.miny); y++) {
                num_x = 0;
                num_o = 0;
                for (x = 0; x <= (gs.maxx - gs.minx); x++) {
                        result = computer_play_count(&gs, x, y, &num_x, &num_o);
                        if (result != 0)
                                return result;
                }
        }
        /* Rows */
        for (x = 0; x <= (gs.maxx - gs.minx); x++) {
                num_x = 0;
                num_o = 0;
                for (y = 0; y <= (gs.maxy - gs.miny); y++) {
                        result = computer_play_count(&gs, x, y, &num_x, &num_o);
                        if (result != 0)
                                return result;
                } 
        }
        /* Askew left-right */
        for (x = (gs.maxx - gs.minx); x >= 0 ; x--) {
                num_x = 0;
                num_o = 0;
                for (xx = x, yy = 0; (xx <= (gs.maxx - gs.minx)) && (yy <= (gs.maxy - gs.miny)); xx++, yy++) {
                        result = computer_play_count(&gs, xx, yy, &num_x, &num_o);
                        if (result != 0)
                                return result;
                }
        }
        for (y = 0; y <= (gs.maxy - gs.miny); y++) {
                num_x = 0;
                num_o = 0;
                for (xx = 0, yy = y; (xx <= (gs.maxx - gs.minx)) && (yy <= (gs.maxy - gs.miny)); xx++, yy++) {
                        result = computer_play_count(&gs, xx, yy, &num_x, &num_o);
                        if (result != 0)
                                return result;
                }
        }
        /* Askew right-left */
        for (x = 0; x <= (gs.maxx - gs.minx) ; x++) {
                num_x = 0;
                num_o = 0;
                for (xx = x, yy = 0; (xx >= 0) && (yy <= (gs.maxy - gs.miny)); xx--, yy++) {
                        result = computer_play_count(&gs, xx, yy, &num_x, &num_o);
                        if (result != 0)
                                return result;
                }
        }
        for (y = 0; y <= (gs.maxy - gs.miny); y++) {
                num_x = 0;
                num_o = 0;
                for (xx = (gs.maxx - gs.minx), yy = y; (xx >= 0) && (yy <= (gs.maxy - gs.miny)); xx--, yy++) {
                        result = computer_play_count(&gs, xx, yy, &num_x, &num_o);
                        if (result != 0)
                                return result;
                }
        }
        
        return 0;
}

int computer_play_count(GRID_SIZE *gs, int x, int y, int *num_x, int *num_o) {
        STONE *stone;
        
        stone = get_stone(x + gs->minx, y + gs->miny);
        if (stone == NULL) {
                *num_x = 0;
                *num_o = 0;
        } else {
                if (*stone == CIRCLE) {
                        *num_x = 0;
                        *num_o += 1;
                }
                if (*stone == CROSS) {
                        *num_o = 0;
                        *num_x += 1;
                }
        }
        
        if (*num_o == 5) {
                return 1;
        }       
        if (*num_x == 5) {
                return 2;
        }
        return 0;
}

void print_grid() {
        size_t x, y;
        STONE *stone;
        GRID_SIZE gs;
        
        get_grid_size(&gs);
        printf("  ");
        for (x = 0; x <= (gs.maxx - gs.minx); x++) {
                putchar('A' + x);
        }
        putchar('\n');
        
        for (y = 0; y <= (gs.maxy - gs.miny); y++) {
                printf("%2d", y + 1);
                for (x = 0; x <= (gs.maxx - gs.minx); x++) {
                        stone = get_stone(x + gs.minx, y + gs.miny);
                        if (stone == NULL) {
                                putchar('.');
                                continue;
                        }
                        if (*stone == CIRCLE) {
                                putchar('o');
                                continue;
                        }
                        if (*stone == CROSS) {
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
                /*
                printf("%d, %d, %c\n", grid[i].x, grid[i].y, (grid[i].s == CIRCLE) ? 'o' : 'x');
                */
        }
        
        gs->minx -= GRID_OFFSET;
        gs->maxx += GRID_OFFSET;
        gs->miny -= GRID_OFFSET;
        gs->maxy += GRID_OFFSET;
}

STONE *get_stone(int x, int y) {
        int i;
        
        for (i = 0; i <= grid_last_used; i++) {
                if ((grid[i].x == x) && (grid[i].y == y)) {
                        return &grid[i].s;
                }
        }       
        return NULL;
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

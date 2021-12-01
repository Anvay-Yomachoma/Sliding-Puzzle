/**
 * Implements Game of Fifteen (generalized to dim x dim).
 *
 * Usage: fifteen d
 *
 * whereby the board's dimensions are to be d x d,
 * where d must be in [DIM_MIN,DIM_MAX]
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>



typedef char* string;
// hopefully shut up compiler about usleep()
#define _XOPEN_SOURCE (500)
#define _BSD_SOURCE

#define DIM_MIN (2)
#define DIM_MAX (10)

bool move(int tile);
bool swap(int index1, int index2);
bool valid_index(int index);
bool valid_move(int tile);
bool won();
int get_col(int index);
int get_index(int tile);
int get_row(int index);
void clear();
void draw();
void greet();
void init();
void init_globals();
void save();
void load();

static int last_row;
static int last_col;
static int num_tiles;
static int empty_tile_index;
static int dim;

int board[DIM_MAX][DIM_MAX];

enum {
    EMPTY_SPACE = -1,
    ONE_OFFSET = 2,
    TWO_OFFSET = 3,
};

int main(int argc, string argv[])
{
    if (argc == 2)
    {
        printf("Usage: fifteen\n");
        return 1;
    }
    char op;
    //dim = atoi(argv[1]);
    while (true)
    {
        clear();
        printf("\t\t\t Sliding puzzle Menu");
        printf("\n1.)[n]ew \n2.)[s]ave\n3.)[l]oad\n4.)[q]uit\n\n");
        scanf("%c", &op);
        switch (op)
        {
        case 'n':
            printf("Enter Dimension : ");
            scanf("%d", &dim);
            if (dim < DIM_MIN || dim > DIM_MAX)
            {
                printf("Board must be between %i x %i and %i x %i, inclusive.\n",
                    DIM_MIN, DIM_MIN, DIM_MAX, DIM_MAX);
                return 2;
            }

            init_globals();
            greet();
            init();
            while (true)
            {
                clear();
                draw();

                if (won())
                {
                    puts("Solved! Ya did good, kid.");
                    return 0;
                }

                printf("Tile to move: ");
                int tile;
                scanf("%d", &tile);
                // quit if user inputs 0 (for testing)
                if (tile == 0)
                {
                    break;
                }

                if (!move(tile))
                {
                    printf("\nIllegal move.\n");
                    usleep(500000);
                }

                // sleep thread for animation's sake
                //usleep(500000);
            }
            break;
        case 'q':
            return 0;
        case 's':
            save();
            break;
        case 'l':
            load();
            num_tiles = dim*dim;
            last_row = (num_tiles - 1) / dim;
            last_col = dim - 1;
            greet();
            while (true)
            {
                clear();
                draw();

                if (won())
                {
                    puts("Solved! Ya did good, kid.");
                    return 0;
                }

                printf("Tile to move: ");
                int tile;
                scanf("%d", &tile);
                // quit if user inputs 0 (for testing)
                if (tile == 0)
                {
                    break;
                }

                if (!move(tile))
                {
                    printf("\nIllegal move.\n");
                    usleep(500000);
                }

                // sleep thread for animation's sake
                usleep(500000);
            }
            break;
        default:
            printf("Invalid Option");
            usleep(500000);
            break;
        }
    }

    return 0;
}

/**
 * Clears screen using ANSI escape sequences.
 */
void clear(void)
{
    printf("\033[2J");
    printf("\033[%d;%dH", 0, 0);
}

void greet(void)
{
    clear();
    printf("WELCOME TO GAME OF FIFTEEN\n");
    usleep(2000000);
}

/**
 * Initializes the game's board with tiles numbered 1 through dim*dim - 1
 * (i.e., fills 2D array with values but does not actually print them).
 *
 * Note: tiles in descending order relative to increasing index:
 *
 *     |15|14|13|12|
 *     |11|10| 9| 8|
 *     | 7| 6| 5| 4|
 *     | 3| 1| 2| _| <-- Note: we swap 2 & 1 when dimensions are even
 *
 */
void init(void)
{
    for (int i = 0, num = num_tiles-1; i < num_tiles; i++, num--) {
        board[get_row(i)][get_col(i)] = num;
    }

    /* Per rules, if dimensions are even, tiles 1 & 2 must be swapped */
    if (dim % 2 == 0) {
        swap(num_tiles - ONE_OFFSET, num_tiles - TWO_OFFSET);
    }

    board[last_row][last_col] = EMPTY_SPACE;
}

/**
 * Swap the board elements at index1 & index2
 * Returns false on error, else true.
 */
bool swap(int index1, int index2)
{
    if (!valid_index(index1)) {
        return false;
    } else if(!valid_index(index2)) {
        return false;
    }

    int row1 = get_row(index1);
    int col1 = get_col(index1);

    int row2 = get_row(index2);
    int col2 = get_col(index2);

    int tmp = board[row1][col1];
    board[row1][col1] = board[row2][col2];
    board[row2][col2] = tmp;

    return true;
}

/**
 * Prints the board in its current state.
 */
void draw(void)
{
    int tile;
    bool is_end_of_row;

    for (int i = 0; i < num_tiles; i++) {
        is_end_of_row = (i % dim == dim - 1);
        tile = board[get_row(i)][get_col(i)];

        if (tile == EMPTY_SPACE) {
            printf("%2c ", '_');
        } else {
            printf("%2d ", tile);
        }

        if (is_end_of_row) {
            puts("\n");
        }
    }
}

/**
 * If tile borders empty space, moves tile and returns true, else
 * returns false.
 */
bool move(int tile)
{
    int swapped_tile_index = get_index(tile);

    if (!valid_move(tile)) {
        return false;
    }

    bool swapped = swap(get_index(tile), empty_tile_index);
    empty_tile_index = swapped_tile_index;

    return swapped;
}

/**
 * Validate a move using the given tile's index by comparing
 * to potential moves indexes depicted in the figure below:
 *
 * Valid moves exist in a + shape around the empty tile:
 *     | |o| |
 *     |o|_|o|
 *     | |o| |
 *
 * Returns true if valid, otherwise false.
 */
bool valid_move(int tile)
{
    int tile_index = get_index(tile);

    if (!valid_index(tile_index)) {
        return false;
    }

    /* NOTE: at times these will be invalid board coordinates but it's fine
     * because they will never match the current tile_index
     */
    int row_above_index = empty_tile_index - dim;
    int row_below_index = empty_tile_index + dim;
    int col_left_index  = empty_tile_index - 1;
    int col_right_index = empty_tile_index + 1;

    if (tile_index == row_above_index) {
        return true;
    } else if (tile_index == row_below_index) {
        return true;
    } else if (tile_index == col_left_index) {
        return true;
    } else if (tile_index == col_right_index) {
        return true;
    } else {
        return false;
    }
}

/**
 * Returns true if game is won (i.e., board is in winning configuration),
 * else false.
 */
bool won()
{
    if (board[last_row][last_col] != EMPTY_SPACE) {
        return false;
    }

    for (int i = 0; i < num_tiles-1; i++) {
        if (board[get_row(i)][get_col(i)] != i+1) {
            return false;
        }
    }

    return true;
}


/**
 * Linear search to retrieve index of the tile from the board array.
 * Returns index of value if it exists, else -1 if not in board.
 */
int get_index(int tile)
{
    // valid tiles {1, num_tiles-1}
    if (tile <= 0 || tile >= num_tiles) {
        return -1;
    }

    for (int i = 0; i < num_tiles; ++i) {
        if (board[get_row(i)][get_col(i)] == tile) {
            return i;
        }
    }

    return -1;
}

/**
 * Returns false if the given index is outside the bounds of the board,
 * else true.
 */
bool valid_index(int index)
{
    if (index < 0 || index >= num_tiles) {
        return false;
    }

    return true;
}

int get_row(int index)
{
    return index / dim;
}

int get_col(int index)
{
    return index % dim;
}

/* Initializes global variables dependent on dimension argument */
void init_globals()
{
    num_tiles = dim*dim;
    last_row = (num_tiles - 1) / dim;
    last_col = dim - 1;
    empty_tile_index = num_tiles - 1;
}

/*Saves The board array, dim and the index of empty tile in the file with desired filename*/

void save()
{
    char fn[30];
    printf("Enter File Name : ");
    scanf("%s", fn);

    FILE *fp;
    int i,j;

    if((fp=fopen(fn, "wb"))==NULL) {
        printf("Cannot open file.\n");
    }
    printf("%d %d\n", dim, empty_tile_index);
    for(i = 0; i<dim; i++)
    {
        for(j = 0; j<dim; j++)
        {
            printf("%d\t", board[i][j]);
        }
        printf("\n");
    }
    putw(dim, fp);
    putw(empty_tile_index, fp);
    if(fwrite(board, sizeof(float), dim*dim, fp) != dim*dim)
        printf("File write error.");
    fclose(fp);
}
/*Loads the game by retreiving The board array, dim and the index of empty tile
 from the file in which it  was saved earlier*/
void load()
{
    char fn[30];
    FILE *fp;
    printf("Enter File Name : ");
    scanf("%s", fn);
    
    if((fp=fopen(fn, "rb"))==NULL) {
        printf("Cannot open file.\n");
    }
    dim = getw(fp);
    empty_tile_index = getw(fp);
    if(fread(board, sizeof(float), dim*dim, fp) != dim*dim) {
        if(feof(fp))
            printf("Premature end of file.");
        else
            printf("File read error.");
    }
    fclose(fp);
}
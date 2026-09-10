/**
 * Filename: amazeraptor.c
 * 
 * Raptor maze assignment
 * 
 * Proc Functions:
 *  maze_innit() - Creates the proc entry "amazeingraptor" and displays a welcome message
 *  maze_exit() - Removes the custom amazeingraptor entry and displays a goodbye message
 *  maze_read() - Prints a randomly generated maze when the proc is read
 * 
 * amaze.py can be run after the proc has been initialized to print out the maze
 * 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Works Cited:
 * 
 *  Built off of the hello.c file from:
 *   Operating System Concepts - 10th edition
 *   Copyright John Wiley & Sons - 2018 
 * 
 *  The same file was also referenced through the following repo:
 *   https://github.com/greggagne/osc10e/tree/master/ch2
 * 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 
 */

//maybe change asm to linux
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include <linux/random.h>
#include <linux/string.h>

// Setting module metadata
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Raptor Maze Module: Displays a randomly generated maze.");
MODULE_AUTHOR("Kaitlyn Holbert");

// Custom proc name
#define PROC_NAME "amazeraptor"

// Global variable definitions for the maze's dimensions
#define MAZE_COLUMNS 15
#define MAZE_ROWS 5
#define MAZE_WIDTH (2*MAZE_COLUMNS + 1)
#define MAZE_HEIGHT (2*MAZE_ROWS + 1)
#define NUM_EDGES (MAZE_ROWS*(MAZE_COLUMNS - 1) + (MAZE_ROWS - 1)*MAZE_COLUMNS) // The size of maze_edges

// Struct for maze edge information (These are what will be traversed with a random Kruskall's algorithm)
struct maze_edge{
    int og_cell;
    int n_cell;
    int row;
    int col;
    int dir;
}

// Global structures for the maze
char maze_grid[MAZE_HEIGHT][MAZE_WIDTH]; // 2D array for calculating the randomly generated maze 
char final_maze[MAZE_HEIGHT*(MAZE_WIDTH + 1) + 1]; // A string for the maze that allows for the maze to be formatted like how the proc displays multi-lined text
size_t final_maze_length;
struct maze_edge maze_edges[NUM_EDGES];
int cell_parent[MAZE_HEIGHT*MAZE_WIDTH];

// Function prototypes for proc specific functions
ssize_t maze_read(struct file *file, char *buf, size_t count, loff_t *pos);

// may need to change this to match what is in the vm
static const struct proc_ops proc_ops = {
    .owner = THIS_MODULE,
    .read = maze_read,
};

// Function prototypes for non-proc specific functions
int build_maze_edges(void);
void shuffle_maze_edges(int edge_number);
void generate_maze(void);
void format_maze(void)


/**
 * Name: Kaitlyn Holbert
 * Date: 9/6/2026
 * Description: This function creates the proc/amazeing raptor entry and displays a custom welcome message
 */
int maze_init(void)
{
    // Create the maze proc with the proc_create method
    proc_create(PROC_NAME, 0, NULL, &proc_ops);

    // Welcome message
    printk(KERN_INFO "Hello! /proc/%s has been created.\n", PROC_NAME);

	return 0;
}


/**
 * Name: Kaitlyn Holbert
 * Date: 9/6/2026
 * Description: This function removes amazeingraptor entry from proc and displays a custom exit message
 */
void maze_exit(void) {

    // Remove /proc/amazeingraptor
    remove_proc_entry(PROC_NAME, NULL);

    // Exit message
    printk( KERN_INFO "Goodbye... /proc/%s has been removed.\n", PROC_NAME);
}


/**
 * Name: Kaitlyn Holbert
 * Date: 9/6/2026, last edited 9/9/2026
 * Description: This function creates the proc/amazeing raptor entry and displays a custom welcome message.
 */
ssize_t maze_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos)
{
    ssize_t copy_bytes;

    // Generating the maze
    generate_maze();
    format_maze();

    if (*pos >= final_maze_length) {
        return 0;
    }

    copy_bytes = min_t(size_t, final_maze_length - *pos, count);

    // final_maze + *pos is the buffer
    if (copy_to_user(usr_buf, final_maze + *pos, copy_bytes)) {
        return -EFAULT;
    }

    *pos += copy_bytes;
    return copy_bytes;
}


/**
 * Name: Kaitlyn Holbert
 * Date: 9/7/2026, last edited 9/9/2026
 * Description: Build the aray of edges "maze_edges"
 * - Each edge holds the values of the cells it connects, its own xy coordinates in the maze,
 * and the direction it is formed
 * - Every cell has two edges that will be considered in the algorithm
 */
int build_maze_edges(void)
{
    int edge_number = 0;
    int r = 0;

    // For each cell, build two edges
    for (r = 0; r < MAZE_ROWS; r++) {
        int c = 0;
        for (c = 0; c < MAZE_COLUMNS; c++) {
            int index = r*MAZE_COLUMNS + c; //translating the index of the cell in the 2D array to 1D
            // N=1, S=2, E=3, W=4
            // Eastern cell 
            if((c < MAZE_COLUMNS - 1))
            {
                maze_edges[edge_number].og_cell = index;
                maze_edges[edge_number].n_cell = index + 1;
                maze_edges[edge_number].row = 2*r + 1; // Translating 3D row to 2D 
                maze_edges[edge_number].col = 2*c + 2; // Edge is one column over
                maze_edges[edge_number].dir = 3; // Dir is currently not used
                edge_number++;
            }

            // Southern cell
              if(r < MAZE_COLUMNS - 1)
            {
                maze_edges[edge_number].og_cell = index;
                maze_edges[edge_number].n_cell = index + MAZE_COLUMNS;
                maze_edges[edge_number].row = 2*r + 2; //Edge is one row down
                maze_edges[edge_number].col = 2*c + 1;
                maze_edges[edge_number].dir = 2;
                edge_number++;
            }
        }
    }

    return edge_number;
}


 /**
  * Name: Kaitlyn Holbert
  * Date: 9/7/2026
  * Description: Randomly shuffle the array of edges. 
  * For each edge, randomly calculate a new valid index and add that to the current. 
  * Then swap the two edges at the original and new index.
  */
 void shuffle_maze_edges(int edge_number)
 {
    struct maze_edge temp;
    int i = 0;
    for(i = 0; i < edge_number; i++) {
        int new_i = prandom_u32() % (i + 1);
        temp = maze_edges[i];
        maze_edges[i] = maze_edges[new_i];
        maze_edges[new_i] = temp;
    }
 }


/**
 * Name: Kaitlyn Holbert
 * Date: 9/7/2026, last edited 9/9/2026
 * Description: Generate the maze with a randomized Kruskal's algorithm
 */
void generate_maze(void)
{
    int h, r, n = 0;
    // Call functions to build and shuffle edges, only depends on the preset maze dimensions
    int edge_number = build_maze_edges();
    shuffle_maze_edges(edge_number);
    final_maze_length = 0;

    // Initialize the grid with #
    for (h = 0; h < MAZE_HEIGHT; h++) {
        int w = 0;
        for (w = 0; w < MAZE_WIDTH; w++) {
            maze_grid[h][w] = '#';
        }
    }

    // Mark each cell center as a space
    for (r = 0; r < MAZE_ROWS; r++) {
        int c = 0;
        for (c = 0; c < MAZE_COLUMNS; c++) {
            int index = r*MAZE_COLUMNS + c;
            maze_grid[2 * r + 1][2 * c + 1] = ' '; //carving out the cell
            cell_parent[index] = index;
        }
    }

    // Go through every edge in the randomly shuffled array  
    for (n = 0; n < edge_number; n++) {
        // Find the parent of the two cells the edge connects
        int og_parent = cell_parent[maze_edges[n].og_cell];
        int n_parent = cell_parent[maze_edges[n].n_cell];

        // Leave a solid row of # at the end
        if(maze_edges[n].row == MAZE_HEIGHT - 1)
        {
            continue;
        }

        // If the two parents aren't the same, break that wall and update the parent
        if(og_parent != n_parent)
        {
            cell_parent[maze_edges[n].n_cell] = og_parent;
            maze_grid[maze_edges[n].row][maze_edges[n].col] = ' ';           
        }
    }   
}


/**
 * Name: Kaitlyn Holbert
 * Date: 9/7/2026
 * Description: Format the maze for proc display.
 * Specifically, format_maze formats the maze grid array as a multi-lined string.
 */
void format_maze(void)
{
    size_t position = 0;
    int h = 0;
    for (h = 0; h < MAZE_HEIGHT; h++) {
        int w = 0;
        // Copy every value from the row
        for (w = 0; w < MAZE_WIDTH; w++) {
            final_maze[position] = maze_grid[h][w];
            position++;
        }
        // Add '\n' to the end of the row
        final_maze[position] = '\n';
        position++;
    }
    // Add a null terminator to the end of the multiline string
    final_maze[position] = '\0';
    final_maze_length = position; //update the final maze length for the maze_read function
}


// Registering the init and exit functions
module_init( maze_init );
module_exit( maze_exit );

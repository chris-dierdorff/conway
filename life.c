#include <stdio.h>
#include "life.h"

// Set this to 1 to enable debug output, 0 to disable debug output
#define DEBUG 1

// The PDEBUG macro prints debug output if (and only if) DEBUG is true;
// use it exactly like printf
#if DEBUG == 1
#  define PDEBUG(args...) printf(args)
#else
#  define PDEBUG(args...)
#endif

Grid *life_load_board(FILE *fp)
{
	int rows, cols;
	fscanf(fp, "%i %i", &rows, &cols);
	//set grid rows and grid cols
	Grid *grid = grid_alloc(rows, cols);
	//set next grid 
	for (int i = 0; i < rows; i++){
	  for (int j = 0; j < cols; j++){
	    int val;
	    fscanf(fp, "%i", &val);
	    grid_set_next(grid, i, j, (uint8_t) val);
	  }
	}
	grid_flip(grid);
	return grid;
}

void life_compute_next_gen(Grid *grid)
{
	// TODO
	
	int lives = 0;
	for(int i = 0; i < grid->rows; i++) {
	  for (int j = 0; j < grid->cols; j++) {
	    //check row containing ij
	    if (grid_get_current(grid, i, j - 1) == 1) lives++;
	    if (grid_get_current(grid, i, j + 1) == 1) lives++;
	   
	    
	    //check row above ij
	    for (int k = j - 1; k <= j + 1; k++) {
	      if (grid_get_current(grid, i - 1, k) == 1) lives++;
	    }
	    
	    
	    //check row below ij
	    for (int k = j - 1; k <= j + 1; k++) {
	      if (grid_get_current(grid, i + 1, k) == 1) lives++;
	    }
	    
	    //If a cell is alive in the current generation, it will be alive in the next generation 
	    //if and only if it has either 2 or 3 neighbors in the current generation.
	    //If a cell is dead in the current generation, it will be alive in the next generation 
	    //if any only if it has exactly 3 neighbors in the current generation.
	    
	    if (lives == 3)
	      grid_set_next(grid, i, j, 1);
	    
	    else if (lives == 2) {
	      if (grid_get_current(grid, i, j) == 1)
		grid_set_next(grid, i, j, 1);
	      else
		grid_set_next(grid, i, j, 0);
	    }
	    else grid_set_next(grid, i, j, 0);
	    lives = 0;
	  }
	  
	}
}

void life_compute_next_gen_inner(Grid *grid)
{
	// TODO
	
	int lives = 0;
	for(int i = 1; i < grid->rows - 1; i++) {
	  for (int j = 1; j < grid->cols - 1; j++) {
	    //check row containing ij
	    if (grid_get_current(grid, i, j - 1) == 1) lives++;
	    if (grid_get_current(grid, i, j + 1) == 1) lives++;
	   
	    
	    //check row above ij
	    for (int k = j - 1; k <= j + 1; k++) {
	      if (grid_get_current(grid, i - 1, k) == 1) lives++;
	    }
	    
	    
	    //check row below ij
	    for (int k = j - 1; k <= j + 1; k++) {
	      if (grid_get_current(grid, i + 1, k) == 1) lives++;
	    }
	    
	    //If a cell is alive in the current generation, it will be alive in the next generation 
	    //if and only if it has either 2 or 3 neighbors in the current generation.
	    
	    //If a cell is dead in the current generation, it will be alive in the next generation 
	    //if any only if it has exactly 3 neighbors in the current generation.
	    
	    if (lives == 3)
	      grid_set_next(grid, i, j, 1);
	    
	    else if (lives == 2) {
	      if (grid_get_current(grid, i, j) == 1)
		grid_set_next(grid, i, j, 1);
	      else
		grid_set_next(grid, i, j, 0);
	    }
	    else grid_set_next(grid, i, j, 0);
	    lives = 0;
	  }
	  
	}
}

void life_save_board(FILE *fp, Grid *grid)
{
	// TODO
 	fprintf(fp, "%i %i\n" , grid->rows, grid->cols);
 	for (int i = 0; i < grid->rows; i++) {
 	  for (int j = 0; j < grid->cols; j++){
	    fprintf(fp, "%i " , (int) grid_get_current(grid, i, j));
	  }
	  fprintf(fp, "\n" );
 	}
}

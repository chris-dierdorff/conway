#include <stdlib.h>
#include <string.h>
#include "grid.h"

Grid *grid_alloc(int rows, int cols)
{
	// TODO
	Grid *thisGrid = malloc(sizeof(Grid));
	thisGrid->rows = rows;
	
	thisGrid->cols = cols;
	
	//everything below this line is questionable
	//buf1 is for current generation buffer
	thisGrid->buf1 = malloc(rows * cols * sizeof(uint8_t));
	for (int i = 0; i < (rows * cols * sizeof(uint8_t)); i++){
	  thisGrid->buf1[i] = 0;
	}
	//buf2 is next gen buffer 
	thisGrid->buf2 = malloc(rows * cols * sizeof(uint8_t));
	for (int i = 0; i < (rows * cols * sizeof(uint8_t)); i++){
	  thisGrid->buf2[i] = 0;
	}
	
	return thisGrid;
}



void grid_destroy(Grid *grid)
{
	// TODO
	free(grid->buf1);
	free(grid->buf2);
	free(grid);
}

void grid_flip(Grid *grid)
{
	uint8_t *tmp;
	tmp = grid->buf1;
	grid->buf1 = grid->buf2;
	grid->buf2 = tmp;
}


uint8_t grid_get_current(Grid *grid, int row, int col)
{
	// TODO
	//bounds check, 
	//if non existing element is requested return value
	//of dead cell
 	if (row == grid->rows || col == grid->cols || row < 0 || col < 0) return 0;
	return *(grid->buf1 + (row * grid->cols) + (col));
}

void grid_set_current(Grid *grid, int row, int col, uint8_t val)
{
	*(grid->buf1 + (row * grid->cols) + col) = val;
}

void grid_set_next(Grid *grid, int row, int col, uint8_t val)
{
	// TODO
	*(grid->buf2 + (row * grid->cols) + (col)) = val;
}

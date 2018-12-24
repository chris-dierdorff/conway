// Conway's Game of Life - parallel version

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "grid.h"
#include "life.h"
#include <assert.h>

// Set this to 1 to enable debug output, 0 to disable debug output
#define DEBUG 1

// The PDEBUG macro prints debug output if (and only if) DEBUG is true;
// use it exactly like printf
#if DEBUG == 1
#  define PDEBUG(args...) printf(args)
#else
#  define PDEBUG(args...)
#endif


static void send_row(Grid* g, int row, int dest) {
	for (int j = 1; j < g->cols - 1; j++) {
		uint8_t cell = grid_get_current(g, row, j);
		MPI_Send(&cell, 1, MPI_CHAR, dest, 0, MPI_COMM_WORLD);	
	}
}

static void recv_row(Grid* g, int row, int src) {
	for (int j = 1; j < g->cols - 1; j++) {
		uint8_t cell;
		MPI_Recv(&cell, 1, MPI_CHAR, src, 0, MPI_COMM_WORLD, NULL);
		grid_set_current(g, row, j, cell);
	}
}
//the global grid sections do not have buffer coloumns...use send_row to to send from the local grid that has buffers...
static void recv_row_global(Grid* g, int row, int src, int startx, int endx) {
	for (int j = startx; j < endx; j++) {
		uint8_t cell;
		MPI_Recv(&cell, 1, MPI_CHAR, src, 0, MPI_COMM_WORLD, NULL);
		grid_set_current(g, row, j, cell);
	}
}
static void send_col(Grid *g, int col, int dest) {
    for (int j = 1; j < g->rows - 1; j++) {
        uint8_t cell = grid_get_current(g, j, col);
        MPI_Send(&cell, 1, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
    }
}

static void recv_col(Grid *g, int col, int src) {
    for (int j = 1; j < g->rows - 1; j++) {
        	uint8_t cell;
        	MPI_Recv(&cell, 1, MPI_CHAR, src, 0, MPI_COMM_WORLD, NULL);
        	grid_set_current(g, j, col, cell);
    }
}
//accepts grid, row and col of cell being sent from calling process, and the 
//rank of the process to comm with
static void comm_corner(Grid *g, int sendRow, int sendCol, int recvRow, int recvCol, int corner) {
	uint8_t send = grid_get_current(g, sendRow, sendCol);
	MPI_Send(&send, 1, MPI_CHAR, corner, 0, MPI_COMM_WORLD);
	uint8_t cell;
	MPI_Status st;
	MPI_Recv(&cell, 1, MPI_CHAR, corner, 0, MPI_COMM_WORLD, &st);
	grid_set_current(g, recvRow, recvCol, cell);
	
}

static void divide_work(int n, int num_chunks, int chunk_index, int *start_index, int *end_index)
{
    assert(n > 0);
    assert(num_chunks < n);
    //printf("assertions passed in divide work...now dividing\n");
    int chunk_size = n / num_chunks;
    int r = n % num_chunks;
    
    *start_index = (chunk_index * chunk_size);

    if (chunk_index < r) {
        *start_index += chunk_index;
    } else {
        *start_index += r;
    }

    *end_index = (*start_index) + chunk_size;
    if (chunk_index < r) {
        (*end_index)++;
    }
}


int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (argc != 5) {
		fprintf(stderr, "Usage: ./runpar <filename> <numgens> <N> <M>\n");
		exit(1);
	}
	
	const char *filename = argv[1];
	int numgens = atoi(argv[2]);
	int N = atoi(argv[3]);  // number of rows of processes
	int M = atoi(argv[4]);  // number of columns of processes

	//PDEBUG("filename=%s, numgens=%i, N=%i, M=%i\n", filename, numgens, N, M);
	
	// TODO: computation
	//get a file...
	FILE *fp = fopen(filename, "r");
	if (fp  == NULL ) printf("file did not open opened\n");
	//printf("File opened\n");
	//fflush(stdout);
	Grid *grid = life_load_board(fp);
	fclose(fp);
	//printf("Done loading board\n");
	//fflush(stdout);
	
	int root = 0;

	
	int proc_row = rank / M;
	int proc_col = rank % M;


	int startx, endx, starty, endy;
	divide_work(grid->cols, M, proc_col, &startx, &endx);
	divide_work(grid->rows, N, proc_row, &starty, &endy);
	
	int c_s_v = endy - starty;
	int c_s_h = endx - startx;
	//printf("Process: %i, starty..endy, %i..%i startx..endx:  %i..%i c_s_v..c_s_h %i..%i\n", rank, starty, endy, startx, endx, c_s_v, c_s_h);
	//fflush(stdout);
	

	//local grid for multiple calcs
	Grid *local = grid_alloc(c_s_v + 2, c_s_h + 2);
	MPI_Status st;
	
	int cols = local->cols;
	int rows = local->rows;
	
	
	for (int i = starty, p = 1; i < endy; i++, p++) {
		for (int j = startx, r = 1; j < endx; j++, r++) {
			uint8_t cell = grid_get_current(grid, i, j);
			grid_set_current(local, p, r, cell);
		}
	}
	//will need to use proc_row and proc_col to find out where you are instead of rank, size...
	static MPI_Datatype s_coltype;	
	MPI_Type_vector(rows - 2, 1, cols, MPI_CHAR, &s_coltype);
	MPI_Type_commit(&s_coltype);
	static MPI_Datatype s_rowtype;
	MPI_Type_vector(cols - 2, 1, rows, MPI_CHAR, &s_rowtype);
	MPI_Type_commit(&s_rowtype);
	int offsetSendrow = rows - 2;
	int offsetRecvrow = rows - 1;
	for (int i = 1; i <= numgens; i++) {
		//share with cols to the right
		if (proc_col != M - 1) {
			//send_col(local, c_s_h, rank + 1);
			//recv_col(local, c_s_h + 1, rank + 1);
			MPI_Send(local->buf1 + (2*cols) - 2, 1, s_coltype, rank + 1, 0, MPI_COMM_WORLD);
			MPI_Recv(local->buf1 + (2*cols) - 1, 1, s_coltype, rank + 1, 0, MPI_COMM_WORLD, &st);
		}
		//share with cols to the left
		if (proc_col != 0) {
			//send_col(local, 1, rank - 1);
			//recv_col(local, 0, rank - 1);
			MPI_Send(local->buf1 + (1*cols) + 1, 1, s_coltype, rank - 1, 0, MPI_COMM_WORLD);
			MPI_Recv(local->buf1 + (1*cols), 1, s_coltype, rank - 1, 0, MPI_COMM_WORLD, &st);
		}
		//share with rows below
		if (proc_row != N - 1) {
			send_row(local, c_s_v, rank + M);
			recv_row(local, c_s_v + 1, rank + M);
			//MPI_Send(local->buf1 + (cols*offsetSendrow) + 1, 1, s_rowtype, rank + M, 0, MPI_COMM_WORLD);
			//MPI_Recv(local->buf1 + (cols*offsetRecvrow) + 1, 1, s_rowtype, rank + M, 0, MPI_COMM_WORLD, &st);
		}
		//share with rows above
		if (proc_row != 0) {
			send_row(local, 1, rank - M);
			recv_row(local, 0, rank - M);
			//MPI_Send(local->buf1 + cols + 1, 1, s_rowtype, rank - M, 0, MPI_COMM_WORLD);
			//MPI_Recv(local->buf1 + 1, 1, s_rowtype, rank - M, 0, MPI_COMM_WORLD, &st);
		}
		
		//corner cell comm***************************
		//*********top row***************************
		//rows = local->rows
		//cols = local->cols
		if (proc_row == 0) {
			//upper left hand process, comm only with bottom right
			
			if (proc_col == 0) {
				comm_corner(local, rows - 2, cols - 2, rows - 1, cols - 1, rank + M + 1);	
			}
			//upper right cell; comm with bottom left only
			else if (proc_col == M - 1) {				
				comm_corner(local, rows - 2, 1, rows - 1, 0, rank + M - 1);
			}
			//comm with bottom right and bottom left
			else {
				//bottomw right
				comm_corner(local, rows - 2, cols - 2, rows - 1, cols - 1, rank + M + 1);
				//bottom left
				comm_corner(local, rows - 2, 1, rows - 1, 0, rank + M - 1);	
			}
		}
		//**********bottom row*************************
		else if (proc_row == N - 1) {
			//bottom left cell, comm only with upper right 
			if (proc_col == 0) {
				comm_corner(local, 1, cols - 2, 0, cols - 1, rank - M + 1);
			}
			//bottom right cell, comm only with upper left
			else if (proc_col == M - 1) {
				comm_corner(local, 1, 1, 0, 0, rank - M - 1);
			}
			//comm with upper left and upper right
			else {
				//upper right
				comm_corner(local, 1, cols - 2, 0, cols - 1, rank - M + 1);
				//upper left
				comm_corner(local, 1, 1, 0, 0, rank - M - 1);
			}
		}
		//*********inner row, border columns************
		//inner row, leftmost col, comm with upper right and bottom right
		else if (proc_col == 0) {
			//upper right
			comm_corner(local, 1, cols - 2, 0, cols - 1, rank - M + 1);
			//bottom right
			comm_corner(local, rows - 2, cols - 2, rows - 1, cols - 1, rank + M + 1);
		}
		//inner row, rightmost col, comm with upper left and bottom left 
		else if (proc_col == M - 1) {
			//upper left			
			comm_corner(local, 1, 1, 0, 0, rank - M - 1);
			//bottom left			
			comm_corner(local, rows - 2, 1, rows - 1, 0, rank + M - 1);
		}
		//inner region, comm with all four corners
		else {
			//bottom left
			comm_corner(local, rows - 2, 1, rows - 1, 0, rank + M - 1);
			//upper left
			comm_corner(local, 1, 1, 0, 0, rank - M - 1);
			//upper right
			comm_corner(local, 1, cols - 2, 0, cols - 1, rank - M + 1);
			//bottom right
			comm_corner(local, rows - 2, cols - 2, rows - 1, cols - 1, rank + M + 1);
		}
		
		
		life_compute_next_gen_inner(local);
		grid_flip(local);
		
	}
	//fill in global grid for output
	int srcStarty, srcEndy, srcStartx, srcEndx;
	if (rank == root) {
		Grid *global = grid_alloc(grid->rows, grid->cols);

		for (int i = starty, p = 1; i < endy; i++, p++) {
			for (int j = startx, r = 1; j < endx; j++, r++) {
				uint8_t cell = grid_get_current(local, p, r);
				grid_set_current(global, i, j, cell);
			}
		}
		
		for (int i = 1; i < size; i++) {
			MPI_Recv(&srcStarty, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &st);
			MPI_Recv(&srcEndy, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &st);
			MPI_Recv(&srcStartx, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &st);
			MPI_Recv(&srcEndx, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &st);
			int srcRows = endy - starty, srcCols = endx - startx;
			for (int j = srcStarty; j < srcEndy; j++) {
				recv_row_global(global, j, i, srcStartx, srcEndx);
			}
			
			
		}
	life_save_board(stdout, global);
	}
	//all other processess send data to root
	else {
		MPI_Send(&starty, 1, MPI_INT, root, 0, MPI_COMM_WORLD);
		MPI_Send(&endy, 1, MPI_INT, root, 0, MPI_COMM_WORLD);
		MPI_Send(&startx, 1, MPI_INT, root, 0, MPI_COMM_WORLD);
		MPI_Send(&endx, 1, MPI_INT, root, 0, MPI_COMM_WORLD);
		for (int i = 1; i < c_s_v + 1; i++) {
			send_row(local, i, root);
		}
	}
	
	MPI_Finalize();

	return 0;
}

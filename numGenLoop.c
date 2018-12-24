//will need to use proc_row and proc_col to find out where you are instead of rank, size...
	for (int i = 1; i <= numgens; i++) {
		//share with cols to the right
		if (proc_col != M - 1) {
			send_col(local, c_s_h, rank + 1);
			recv_col(local, c_s_h + 1, rank + 1);
		}
		//share with cols to the left
		if (proc_col != 0) {
			send_col(local, 1, rank - 1);
			recv_col(local, 0, rank - 1);
		}
		//share with rows below
		if (proc_row != N - 1) {
			send_row(local, c_s_v, rank + N);
			recv_row(local, c_s_v + 1, rank + N);
		}
		//share with rows above
		if (proc_row != 0) {
			send_row(local, 1, rank - N);
			recv_row(local, 0, rank - N);
		}
		
		//corner cell comm**************
		//*********top row***************************
		if (proc_row == 0) {
			//upper left hand process, comm only with bottom right	
			if (proc_col == 0) {
				/*uint8_t send = grid_get_current(local, rows - 2, cols - 2);
				MPI_Send(&send, 1, MPI_CHAR, rank + M + 1, 0, MPI_COMM_WORLD);
				uint8_t cell;
				MPI_Recv(&cell, 1, MPI_CHAR, rank + M + 1, 0, MPI_COMM_WORLD, &st);
				grid_set_current(local, rows - 1, cols - 1, cell);*/
				comm_corner(local, rows - 2, cols - 2, rows - 1, cols - 1, rank + M + 1);	
			}
			//upper right cell; comm with bottom left only
			else if (proc_col == M - 1) {
				/*uint8_t send = grid_get_current(local, rows - 2, 1);
				MPI_Send(&send, 1, MPI_CHAR, rank + M - 1, 0, MPI_COMM_WORLD);
				uint8_t cell;
				MPI_Recv(&cell, 1, MPI_CHAR, rank + M - 1, 0, MPI_COMM_WORLD, &st);
				grid_set_current(local, rows - 1, 0, cell);*/
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
				/*uint8_t send = grid_get_current(local, 1, cols - 2);
				MPI_Send(&send, 1, MPI_CHAR, rank - M + 1, 0, MPI_COMM_WORLD);
				uint8_t cell;
				MPI_Recv(&cell, 1, MPI_CHAR, rank - M + 1, 0, MPI_COMM_WORLD, &st);
				grid_set_current(local, 0, cols - 1, cell);*/
				comm_corner(local, 1, cols - 2, 0, cols - 1, rank - M + 1);
			}
			//bottom right cell, comm only with upper left
			else if (proc_col == M - 1) {
				/*uint8_t send = grid_get_current(local, 1, 1);
				MPI_Send(&send, 1, MPI_CHAR, rank - M - 1, 0, MPI_COMM_WORLD);
				uint8_t cell;
				MPI_Recv(&cell, 1, MPI_CHAR, rank - M - 1, 0, MPI_COMM_WORLD, &st);
				grid_set_current(local, 0, 0, cell);*/
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

#include "defs.h"
/*
 * Load a 1 dimensional array with data from csv.
 */
void load_1d_array( char 	*file, 		// File to parse.
		    		int		num_elems,	// Number of elements to read.
		    		double	*arr		// Pointer to array into which to load data.
		  	  	  );

/*
 * Load a 2 dimensional array with data from csv.
 */
void ld_2d_array( char 		*file, 
				  int		num_rows, 
				  int		num_cols, 
				  double	arr[][NUM_LACT]
				);

/*
 * Special function to read rha_lactation table.
 * Incompatible with ld_2d_array because number of columns = 5.
 * FIXME: Fix this.
 */
void ld_rha_lactation( char 	*file, 
		     	 	   int		num_rows, 
		     	 	   int		num_cols, 
		     	 	   double	arr[][5]
		     	 	 );

/*
 * Write a 2d array to a file.
 */
void
write_csv(	double arr[][NUM_LACT], 
			char *file);


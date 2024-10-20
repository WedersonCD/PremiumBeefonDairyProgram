#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "csv_reader.h"
#include "log.h"

/*
 * Load a 1 dimensional array with data from csv.
 */
void load_1d_array( char 	*file, 		// File to parse.
		    int		num_elems,	// Number of elements to read.
		    double	*arr		// Pointer to array into which to load data.
		    )
{
  FILE *fd;
  char line[1024];
  char delim[] = ",";
  char *p;
  int e;
	
  // Open file.
  fd = fopen(file, "r");
  if (fd == NULL) {
    char* msg = NULL;
    sprintf(msg, "Could not open file %s.\n", file);
    log_err(msg);
  }
	
  // Read contents into array.
  e = 0;
  while (fgets(line, sizeof(line), fd) != NULL) {	// Read a line.
    // Break the record into pieces at the delimiter (here ',').
    p = strtok(line, delim);
    while (p != NULL) {	// Still more pieces.
			// Convert the string into a double.
      arr[e] = strtod(p, NULL);
      //-printf("%d\t%f\n", e, arr[e]);
      e++;
      p = strtok(NULL, delim);
    }
  }
	
  // Close file.
  fclose(fd);
}

/*
 * Load a 2 dimensional array with data from csv.
 */
void ld_2d_array( char 		*file, 
		  int		num_rows, 
		  int		num_cols, 
		  double	arr[][NUM_LACT])
{
  FILE *fd;
  char line[1024];
  char delim[] = ",\n";
  char *p;
  int r, c;
  
  // Open file.
  fd = fopen(file, "r");
  if (fd == NULL) {
    char* msg = NULL;
    sprintf(msg, "Could not open file %s\n", file);
    log_err(msg);
  }
	
  // Read contents into array.
  r = 0;
  while (fgets(line, sizeof(line), fd) != NULL) {	// Read a line.
    // Break the record into pieces at the delimiter (here ',').
    c = 0;
    p = strtok(line, delim);
    while (p != NULL) {	// Still more pieces.
  			// Convert the string into a double.
      arr[r][c] = strtod(p, NULL);
      //-printf("%d\t%d\t%f\n", r, c, arr[r][c]);
      c++;
      p = strtok(NULL, delim);
    }
    r++;
  }
	
  // Close file.
  fclose(fd);
}

/*
 * Special function to read rha_lactation table.
 * Incompatible with ld_2d_array because number of columns = 5.
 * FIXME: Fix this.
 */
void ld_rha_lactation( char 	*file, 
		       int		num_rows, 
		       int		num_cols, 
		       double	arr[][5]
		       )
{
  FILE *fd;
  char line[1024];
  char delim[] = ",";
  char *p;
  int r, c;

  // Open file.
  fd = fopen(file, "r");
  if (fd == NULL) {
    char* msg = NULL;
    sprintf(msg, "Could not open file %s\n", file);
    log_err(msg);
  }
	
  // Read contents into array.
  r = 0;	
  while (fgets(line, sizeof(line), fd) != NULL) {	// Read a line.
    // Break the record into pieces at the delimiter (here ',').
    c = 0;
    p = strtok(line, delim);
    while (p != NULL) {	// Still more pieces.
			// Convert the string into a double.
      arr[r][c] = strtod(p, NULL);
      //-printf("%d\t%d\t%f\n", r, c, arr[r][c]);
      c++;
      p = strtok(NULL, delim);
    }
    r++;
  }
  //-printf("c: %d", r);
	
  // Close file.
  fclose(fd);
}

/*
 * Write a 2d array to a file.
 */
void
write_csv(	double arr[][NUM_LACT], 
		char *file) 
{
  FILE* fd;
  int i, j;
	
  // Open file.
  fd = fopen(file, "w");
  if (fd == NULL) {
    char* msg = NULL;
    sprintf(msg, "Could not open file %s\n", file);
    log_err(msg);
  }
	
  // Write contents of array to file.
  for (i = 0; i < NUM_MIM; i++) {
    for (j =0; j < NUM_LACT; j++) {
      fprintf(fd, "%f,", arr[i][j]);
    }
    fprintf(fd, "\n");
  }
	
  fclose(fd);
}


/* System includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* User defined includes */
#include "defs.h"
#include "log.h"

/* Global variables */
struct user_inputs user;	// Holds user data.
int calc_for_cow;
double cow_total, rep_total;
double cow_value;
int calc_for_cow_or_herd;

/* These variables should be reset (= 0) for every new cow in the herd. */
int mim_track_incrementer,
  counter_16,
  counter_17;

// File pointer to output file.
char file_out[20] = "./output/output.csv";	
FILE *fp_out;
char delim[] = ",";
FILE* fp_herd;
FILE* fp_cmpltd;
FILE* fp_;

char* units;
double dollar_to_pound;

/*****************************************/
/* EXIT CODES:				 */
/* 1: Some file could not be opened.	 */
/* 2: Iteration sum does not equal COWS. */
/*****************************************/


int main(int argc, char *argv[])
{
  units = argv[12];
  dollar_to_pound = atof(argv[14]);

  log_msg("Starting analysis...\n");
  
  int iteration_invalid = 0;
  char* msg = NULL;
  char e_msg[200];

  // For cow or herd?
  calc_for_cow_or_herd = atoi(argv[2]);

  // Open file for output. Output should be in csv format.
  fp_out = fopen(file_out, "w");
  if (fp_out == NULL) {
    sprintf(e_msg, "Could not open file %s.\n", file_out);
    log_err(e_msg);
  }
	
  // If only for one cow, read user data and go ahead.
  if (calc_for_cow_or_herd == 1) {
    log_msg("SINGLE COW analysis started...\n");
    read_user_data(argv[4]);

    /* REPLACEMENT */
    int lact_cow = user.lactation;
    int mim_cow  = user.month_after_calving;
    int prg_cow  = user.month_in_pregnancy;
    user.lactation = 1;
    user.month_after_calving = 1;
    user.month_in_pregnancy = 0;
    calc_for_cow = 0;
    // Calculate monthly data (yellow columns in the spreadsheet) and iteration values.
    calculate_monthly_data();
    iteration_invalid = calculate_iterations();
    if (iteration_invalid == 2) {
      // Iteration sum does not equal COWS; notify the user.
      sprintf(e_msg, "[ERROR]: An iteration for the cow did not sum to %d.\n", COWS);    
      log_msg(e_msg);
      exit_msg(e_msg);
    }

    /* COW */
    user.lactation = lact_cow;
    user.month_after_calving = mim_cow;
    user.month_in_pregnancy = prg_cow;
    calc_for_cow = 1;
    // Calculate monthly data (yellow columns in the spreadsheet) and iteration values.
    calculate_monthly_data();
    iteration_invalid = calculate_iterations();
    if (iteration_invalid == 2) {
      // Iteration sum does not equal COWS; notify the user.
      sprintf(e_msg, "[ERROR]: An iteration for the cow did not sum to %d.\n", COWS);
      exit_msg(e_msg);
    }

  } else {    // Calculate for entire herd.
    log_msg("HERD analysis started...\n");		
    // Read common data.
    read_common_data(argv[6]);
		
    /** REPLACEMENT **/
    user.lactation = 1;
    user.month_after_calving = 1;
    user.month_in_pregnancy = 0;
    calc_for_cow = 0;
		
    // Calculate monthly data (yellow columns in the spreadsheet) and iteration values.
    calculate_monthly_data();
    iteration_invalid = calculate_iterations();
    if (iteration_invalid == 2) {
      // Iteration sum does not equal COWS; notify the user.
      sprintf(e_msg, "[ERROR]: An iteration for the replacement cow did not sum to %d.\n", COWS);
      exit_msg(e_msg);
    }

    // Open file for herd output. Output should be in csv format.
    fp_herd = fopen(argv[8], "w");
    if (fp_herd == NULL) {
      printf("Could not open file %s\n", argv[8]);
    }

    /** COWS **/
    FILE *fc;
    char line[1024];
    char delim[] = ",\n";
    char *p;
    int k;
		
    calc_for_cow = 1;
		
    // Open file containing herd data.
    fc = fopen(argv[4], "r");
    if (fc == NULL) {
      printf("Could not open file %s\n", argv[4]);
    }
		
    // Read each line into array.
    int arr[6] = {0, 0, 0, 0, 0};
    if (fgets(line, sizeof(line), fc) == NULL)	{	// Waste the first line (headers).
      printf("Nothing in the file %s\n", argv[4]);
    }
    int cows_cmpltd = 0;
    while (fgets(line, sizeof(line), fc) != NULL) {	// Read a line.
    
      // Have to set these to zero for every new cow.
      mim_track_incrementer = 0;
      counter_16 = 0;
      counter_17 = 0;
			
      // Break the record into pieces at the delimiter (here ',').
      p = strtok(line, delim);
      if (p == NULL || !strcmp(p, " ")) {
  	break;	// Oops, bad line!
      }
      // First field is CowID.
      user.cowID = (char*)malloc(strlen(p));
      if (user.cowID == NULL) {
  	printf("Ran out of memory. Could not assign enough space to user.cowID.\n");
      }
      user.cowID = strcpy(user.cowID, p);
      //printf("%s\t", p);
      // Get rest of the fields.
      for (k = 1; k < 6; k++) {
  	p = strtok(NULL, delim);
  	if (p == NULL || !strcmp(p, " ")) {
  	  arr[k] = 100;
  	  //-printf("%d\t", arr[k]);
  	} else {
  	  arr[k] = atoi(p);
  	  //-printf("%d\t", arr[k]);
  	}
      }
			
      // Set lactation.
      user.lactation = arr[1];
      //printf("%d\t", user.lactation);
			
      int mim, mip;
      int dim, dip;
      // Convert Days in Milk to Month in Milk.
      dim = arr[2];
      mim = ceil((double)dim / 30.00);
			
      // Convert Days in Pregnancy to Month in Pregnancy.
      dip = arr[3];
      mip = ceil((double)dip / 30.00);
      
      // Try to make the difference between mim and mip atleast two, if dip is not zero.
      if (dip != 0) {
	if (mim - mip == 1) {
	  mim++;
	}
      }

      // But mip can have a maximum value of 9 only.
      mip = mip > 9? 9 : mip;
      user.month_after_calving = mim;
      user.dim = dim;
      user.month_in_pregnancy = mip;
      user.dip = dip;
					
      // Set extra_rest_of_lactation
      user.extra_rest_of_lactation = ((double)arr[4] / 100.00) - 1.00;
      //printf("%f\t", user.extra_rest_of_lactation);
			
      // Set extra_next_lactations
      user.extra_next_lactations = (arr[5] / 100.00) -1.00;
      //printf("%f\t", user.extra_next_lactations);
      //printf("\n");
			
      // Check validity of data (mim and prg)
      int valid = chk_lact_mim_prg(user.lactation, user.month_after_calving, user.month_in_pregnancy, &msg);
      if (valid) {
  	calculate_iterations();
      } else {
  	// Write error information to file.
  	fprintf(fp_herd, "%s,%d,%d,%d,%f,%f,%s\n", user.cowID, user.lactation, user.dim, user.dip,
  		user.extra_rest_of_lactation + 1.00, user.extra_next_lactations + 1.00, msg);
      }

      /* One more cow done. */
      cows_cmpltd ++;

      /* Write number of cows completed. */
      fp_cmpltd = fopen(argv[10], "w");
      if(fp_cmpltd == NULL) {
  	printf("Could not open file %s\n", argv[10]);
      }
      if(!fprintf(fp_cmpltd, "%d\n", cows_cmpltd)) {
  	printf("Could not update %s\n", argv[10]);
      }
      fclose(fp_cmpltd);

    }

    /* Done with herd data file. */
    fclose(fc);
    fclose(fp_herd);

  }

  // Close output files.
  fclose(fp_out);

  log_msg("ANALYSIS COMPLETE!\n");
  return 0;
}



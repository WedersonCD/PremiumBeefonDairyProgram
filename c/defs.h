
#ifndef __INC_DEF_H
#define __INC_DEF_H

/* Macros */
// Various constants used to generate monthly data.
#define CULL_C5 	0.35
#define MORTALITY 	0.10
#define CULL_PREG	0.25
#define STD_AVG_BODY_WEIGHT 1306.11566204261
#define	MILK_E		2.71828
#define REPRO_C85	0.0845

#define COWS 		100
#define SIM_ITERS	600

// Set number of user parameters.
// Update this number if a new parameter is added. 
// FIXME: Make this more flexible.
#define NUM_PARAMS 23
#define NUM_PARAMS_COMMON 17

#define NUM_LACT 10
#define NUM_MIM 33
#define NUM_PRG 10

// Number of rows.
#define NUM_ROWS 2320 


//Jacob Code

struct user_inputs {	
	// Evaluated Cow Variables
	int 	lactation;
	int 	month_after_calving;
	int 	month_in_pregnancy;
	double 	milk_production;
	double 	extra_rest_of_lactation;
	double 	extra_next_lactations;
	
	// Replacement Cow Variable
	double 	additional_milk_replacement;
	
	// Herd Production and Reproduction Variables
	double 	herd_turnover_ratio;
	int 	rha;
	double	pregnancy_rate_21_d;
	double	repro_cost;
	int		last_dim_to_breed;
	int		milk_threshold_to_cull;
	double	between_35_to_250_days_in_pregnancy;
	double	avg_cow_body_weight;
	
	// Herd Economic Variables
	double	replacement_value;
	double	salvage_value;
	double	calf_value;
	double	milk_price;
	double	milk_butterfat;
	double	feed_price_production;
	double	feed_price_dry;
	double	interest_rate;

  int dim; // Days in milk.
  int dip; // Days in pregnancy.
	
	// CowID
	char	*cowID;
};

struct total_cow_costs {
	double repro_cost;
	double calf_value;
	double non_repro_cull_cost;
	double mortality_cost;
	double repro_cull_cost;
	double milk;
	double feed;
	double net_return;
	double present_value;
};

struct struct_steady_state {
	double days_in_milk;
	double days_to_conception;
	double per_pregnant;
	double per_repro_cull;
	double per_mortality;
	double per_1st_lact;
	double per_2nd_lact;
	double per_gt_3rd_lact;
};

struct cow_evaluated {
	
	// Value of the Cow
	double cow_value;
	
	// Compared Against a Replacement
	struct total_cow_costs vs_repl;
	
	// Replacement Transaction
	double replacement_transaction;
	
	// Herd Structure at Steady State
	struct struct_steady_state steady_state;
	
	// Economics of an Average Cow
	struct total_cow_costs avg_cow;
};

//heifer structure for heifer sheet
struct heifer {
	double age;
	double preg_in_month;
};

/*
 * Write a message to the message file and exit.
 */
void exit_msg(char* msg);

/*
 * Read common user data from csv file input_common.csv.
 */
void read_common_data(char *file);

/*
 * Read user data from csv file user_input.csv.
 */
void read_user_data(char *file);

/*
 * Calculate the abortions that return to the flow.
 */
double abortions_to_flow();

/*
 * Calculate monthly data.
 */
int calculate_monthly_data();

/*
 * Calculate first iteration
 */
int calculate_first_iter();

/*
 * Calculate iterations
 */
int calculate_iterations();

/*
 * Calculate various costs.
 * @iter_num: iteration for which the costs have to be calculated.
 */
void 
calculate_costs( double* iter,	/* Iteration values for which the costs are to be calculated. */
				 int iter_num);	/* Which iteration is this? */

/*
 * Calcultate THIS Lacatation Discount Milk and Feed.
 */
void 
calculate_TLDMF(int iter_num);

/*
 * Calcultate NEXT Lacatation Discount Milk and Feed.
 */
void 
calculate_NLDMF(int iter_num);

/*
 * Get indices for THIS lactation calculations.
 */
int 
get_this_lactation_indices( int indices[], 
							int mim_tracker);

/*
 * Evaluate cow
 */
void 
evaluate_cow(int herd_counter);

/*
 * Checks the validity of MIM and PRG values.
 */
int 
chk_lact_mim_prg(int lact, int mim, int prg, char **msg);

/*
 * Print costs
 */
void
print_costs(int iter_num);

/*
 * Print total costs
 */
void
print_total_costs(int herd_counter);

/*
 * Array sum
 */
double
array_sum( double arr[], 
		   int offset,
		   int length);

/*
 * Sumproduct of two arrays.
 */
double sumproduct2(double arr1[], double arr2[], int offset, int length);

/*
 * Sumproduct of two arrays.
 */
double sumproduct3(double arr1[], double arr2[], double arr3[], int offset, int length);

/*
 * Print array
 */
void
print_array(double arr[], int length);



#endif


/* System includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "defs.h"
#include "csv_reader.h"
#include "log.h"

#define MSGFILE "./log/msg.txt"

// Monthly data columns.
double arr_milk_pxion[NUM_ROWS];
double arr_milk_produced[NUM_ROWS];
double arr_feed_intake[NUM_ROWS];
double arr_O[NUM_ROWS];
double arr_P[NUM_ROWS];
double arr_non_pregnant[NUM_ROWS];
double arr_pregnancy_rate[NUM_ROWS];
double arr_pregnancy_cost[NUM_ROWS];
double arr_non_aborted[NUM_ROWS];
double arr_abortion_rate[NUM_ROWS];
double arr_non_leaving[NUM_ROWS];
double arr_no_repro_leaving[NUM_ROWS];
double arr_non_reprocull[NUM_ROWS];
double arr_repro_cull[NUM_ROWS];

// Cost arrays.
double repro_cost[SIM_ITERS];
double calf_value[SIM_ITERS];
double non_repro_cull_cost[SIM_ITERS];
double mortality_cost[SIM_ITERS];
double repro_cull_cost[SIM_ITERS];
double milk[SIM_ITERS];
double feed[SIM_ITERS];
double net_return[SIM_ITERS];
double present_value[SIM_ITERS];
double this_lactation_discount_milk[SIM_ITERS];
double this_lactation_discount_feed[SIM_ITERS];
double next_lactation_discount_milk[SIM_ITERS];
double next_lactation_discount_feed[SIM_ITERS];
double total;
// Array to back up replacement's and cow's net return values for graph.
double net_return_bk[SIM_ITERS][2];

// Iteration value holders
double iter1[NUM_ROWS];
double iter2[NUM_ROWS];
double *curr_iter = iter1;
double *prev_iter = iter2;
double *temp_iter;

// Arrays to store lact, mim and prg values.
int arr_lact[NUM_ROWS], arr_mim[NUM_ROWS], arr_prg[NUM_ROWS];

// Number of cows in the herd.
int herd_size;
int herd_counter = 0;

// An array holding total cost values for all the cows.
// Index 0 corresponds to total cost values for the replacement.
struct total_cow_costs *arr_cow_costs;

// An array holding the evaluated cow variables.
struct cow_evaluated *arr_cow_eval;

double repro_cost_final;
double calf_value_final;
double non_repro_cull_cost_final;
double mortality_cost_final;
double repro_cull_cost_final;
double milk_final;
double feed_final;
double net_return_final;
double glob_com_div;

/* Beef Tool */
double beefOutputs[16];
double need;
int currentCalc = 0;
double conceptionRateSS, conceptionRateCS = 55, femalesCS = 47, femalesSS = 90;
int servicesSS = 0;

// Jacob Code

/// Hold array of heiferSums
double heiferSum_Arr[600];
// Service rate for heifers
double SRH;
double calf_mortality;
// Array to represent most of spreadsheet
// TODO FIX: EMPTY VALUES EXIST ALL OVER THE PLACE TO MAKE ITERATION EASIER!!! Could initalize all to -1??
double heifer_preg_arr[10][50][1024]; //[preg][mim][iter_num]
// Heifer Zero Preg Array
double heifer_zero_preg[50][1024];
double AAF135;

/*
 * Write a message to the message file and exit.
 */
void exit_msg(char *msg)
{
  FILE *file = NULL;
  file = fopen(MSGFILE, "w");

  if (file == NULL)
  {
    char m[200];
    sprintf(m, "Could not open message file %s.\n", MSGFILE);
    log_err(m);
  }
  else
  {
    fprintf(file, "%s", msg);
    fclose(file); // Don't forget to close the file after writing
  }
  exit(2);
}

/*
 * Read common user data from csv file input_common.csv.
 */
void read_common_data(char *file)
{
  // FIXME: This value would be read from user inputs.
  herd_size = 1;

  double arr[NUM_PARAMS_COMMON];

  // Parse csv file into array.
  load_1d_array(file, NUM_PARAMS_COMMON, arr);

  // Update the struct with this data.
  extern struct user_inputs user;
  user = (struct user_inputs){
      /** Parameters NOT common. **/
      /*.lactation = 			arr[0],
        .month_after_calving = 	arr[1],
        .month_in_pregnancy = 	arr[2],
        .milk_production = 		arr[3],
        .extra_rest_of_lactation = 	(arr[4] / 100) - 1,
        .extra_next_lactations = 	(arr[5] / 100) - 1,*/

      // Replacement Cow Variable
      .additional_milk_replacement = arr[6 - 6] / 100,

      // Herd Production and Reproduction Variables
      .herd_turnover_ratio = arr[7 - 6] / 100,
      .rha = arr[8 - 6],
      .pregnancy_rate_21_d = arr[9 - 6] / 100,
      .repro_cost = arr[10 - 6],
      .last_dim_to_breed = arr[11 - 6],
      .milk_threshold_to_cull = arr[12 - 6],
      .between_35_to_250_days_in_pregnancy = arr[13 - 6] / 100,
      .avg_cow_body_weight = arr[14 - 6],

      // Herd Economic Variables
      .replacement_value = arr[15 - 6],
      .salvage_value = arr[16 - 6],
      .calf_value = arr[17 - 6],
      .milk_price = arr[18 - 6] / 100, // FIXME: Is it "arr[17] / NUM_COWS" ?
      .milk_butterfat = arr[19 - 6] / 100,
      .feed_price_production = arr[20 - 6],
      .feed_price_dry = arr[21 - 6],
      .interest_rate = arr[22 - 6] / (12 * 100)};
  user.salvage_value = user.salvage_value * user.avg_cow_body_weight;

  // printf("Herd interest_rate: %f\n", user.interest_rate);

  // Allocate space to hold each cow's cost values.
  // FIXME: Free memory when done.
  arr_cow_costs = calloc((size_t)herd_size + 1, sizeof(struct total_cow_costs));
  assert(arr_cow_costs != NULL);

  // Allocate space to hold each cow's evaluated variables.
  // FIXME: Free memory when done.
  arr_cow_eval = calloc((size_t)herd_size + 1, sizeof(struct cow_evaluated));
  assert(arr_cow_eval != NULL);
}

/*
 * Read user data from csv file user_input.csv.
 */
void read_user_data(char *file)
{
  log_msg("Reading user input data: \n");

  // FIXME: This value would be read from user inputs.
  herd_size = 1;

  double arr[NUM_PARAMS + 6];

  // Parse csv file into array.
  load_1d_array(file, NUM_PARAMS + 6, arr);

  // Update the struct with this data.
  extern struct user_inputs user;
  user = (struct user_inputs){
      .lactation = arr[0],
      .month_after_calving = arr[1],
      .month_in_pregnancy = arr[2],
      .milk_production = arr[3],
      .extra_rest_of_lactation = (arr[4] / 100) - 1,
      .extra_next_lactations = (arr[5] / 100) - 1,

      // Replacement Cow Variable
      .additional_milk_replacement = arr[6] / 100,

      // Herd Production and Reproduction Variables
      .herd_turnover_ratio = arr[7] / 100,
      .rha = arr[8],
      .pregnancy_rate_21_d = arr[9] / 100,
      .repro_cost = arr[10],
      .last_dim_to_breed = arr[11],
      .milk_threshold_to_cull = arr[12],
      .between_35_to_250_days_in_pregnancy = arr[13] / 100,
      .avg_cow_body_weight = arr[14],

      // Herd Economic Variables
      .replacement_value = arr[15],
      .salvage_value = arr[16],
      .calf_value = arr[17],
      .milk_price = arr[18] / 100, // FIXME: Is it "arr[17] / NUM_COWS" ?
      .milk_butterfat = arr[19] / 100,
      .feed_price_production = arr[20],
      .feed_price_dry = arr[21],
      .interest_rate = arr[22] / (12 * 100)};

  user.salvage_value = user.salvage_value * user.avg_cow_body_weight;
  conceptionRateCS = arr[23];
  servicesSS = arr[24];
  femalesCS = arr[25];
  femalesSS = arr[26];

  // Jacob Code Get SRH and Calf mortality
  calf_mortality = (arr[27] / 100);
  SRH = (arr[28] / 100);

  //-printf("Salvage Value: %f\n", user.salvage_value);

  // Allocate space to hold each cow's cost values.
  // FIXME: Free memory when done.
  arr_cow_costs = calloc((size_t)herd_size + 1, sizeof(struct total_cow_costs));
  assert(arr_cow_costs != NULL);

  // Allocate space to hold each cow's evaluated variables.
  // FIXME: Free memory when done.
  arr_cow_eval = calloc((size_t)herd_size + 1, sizeof(struct cow_evaluated));
  assert(arr_cow_eval != NULL);

  log_msg("Finshed read_user_data method from inside method!!!!... \n");
}

/*
 * Calculate monthly data.
 * This function in turn calls a function for each monthly parameter.
 */
int calculate_monthly_data()
{
  log_msg("Calculating monthly data!..\n");
  extern struct user_inputs user;
  char *file;

  // Read in non repro leaving factors.
  file = "./input/non_repro_leaving.csv";
  double tbl_non_repro_leaving[NUM_MIM][NUM_LACT];
  ld_2d_array(file, NUM_MIM, NUM_LACT, tbl_non_repro_leaving);

  // Read in body weights.
  file = "./input/body_weight.csv";
  double tbl_body_weight[NUM_MIM][NUM_LACT];
  double body_weight_ratio = user.avg_cow_body_weight / STD_AVG_BODY_WEIGHT;
  ld_2d_array(file, NUM_MIM, NUM_LACT, tbl_body_weight);

  // Read in lactation curve parameters.
  file = "./input/rha_lactation.csv";
  double tbl_rha_lactation[39][5];
  ld_rha_lactation(file, 39, 5, tbl_rha_lactation);

  // Read in monthly abortion rates.
  file = "./input/monthly_abortion_rate.csv";
  double tbl_monthly_abortion_rate[NUM_PRG][NUM_LACT];
  ld_2d_array(file, 7, NUM_LACT, tbl_monthly_abortion_rate);

  // Generate various monthly tables.
  double tbl_non_repro_leaving_non_preg[NUM_MIM][NUM_LACT];
  double tbl_non_repro_leaving_preg[NUM_MIM][NUM_LACT];
  double tbl_milk_prod_non_preg[NUM_MIM][NUM_LACT];
  double tbl_milk_prod_preg[NUM_PRG] = {1, 1, 1, 1, 1, 0.95, 0.90, 0.85, 0, 0};
  double tbl_preg_rate[NUM_MIM][NUM_LACT];
  double tbl_repro_cost[NUM_MIM][NUM_LACT];
  double tbl_repro_cull[NUM_MIM][NUM_LACT];
  double tbl_abortion[NUM_PRG][NUM_LACT];
  double scale[NUM_LACT], ramp[NUM_LACT], offset[NUM_LACT], decay[NUM_LACT];

  int rha_index_start = (user.rha - 18000) / 1000;
  int parity, rha_index;
  for (parity = 0; parity < NUM_LACT; parity++)
  {
    if (parity >= 2)
    {
      rha_index = rha_index_start + (2 * 13);
    }
    else
    {
      rha_index = rha_index_start + (parity * 13);
    }
    scale[parity] = tbl_rha_lactation[rha_index][0];
    ramp[parity] = tbl_rha_lactation[rha_index][1];
    offset[parity] = tbl_rha_lactation[rha_index][2];
    decay[parity] = tbl_rha_lactation[rha_index][3];
    //-printf("%f\t%f\t%f\t%f\n", scale[parity], ramp[parity], offset[parity], decay[parity]);
  }

  double cull_c6 = user.herd_turnover_ratio / CULL_C5;
  double repro_c5 = user.pregnancy_rate_21_d * 30.4 / 21;
  //-printf("cull_c6: %f\n", cull_c6);
  int m, l;
  int dim_t, m_t;
  for (m = 0; m < NUM_MIM; m++)
  {
    dim_t = (2 * (m + 1) - 1) * 15;
    m_t = m + 1;
    for (l = 0; l < NUM_LACT; l++)
    {
      // Non Repro Leaving for Non Pregnant
      tbl_non_repro_leaving_non_preg[m][l] = tbl_non_repro_leaving[m][l] * cull_c6;

      // Non Repro Leaving for Pregnant
      tbl_non_repro_leaving_preg[m][l] = tbl_non_repro_leaving_non_preg[m][l] * CULL_PREG;
      //-printf("%f\t", tbl_non_repro_leaving_preg[m][l]);

      // Milk Production for Non Pregnant
      tbl_milk_prod_non_preg[m][l] = scale[l] * exp(-decay[l] * dim_t) * (1 - exp((offset[l] - dim_t) / ramp[l]) / 2);
      //-printf("%f\t", tbl_milk_prod_non_preg[m][l]);

      // Monthly Body Weight for Non Pregnant
      tbl_body_weight[m][l] = tbl_body_weight[m][l] * body_weight_ratio;
      // printf("%f\t", tbl_body_weight[m][l]);

      // Pregnancy Rate
      if (m_t == 1 || m_t > 24)
      {
        tbl_preg_rate[m][l] = 0;
      }
      else if (m_t > 2)
      {
        tbl_preg_rate[m][l] = (m_t <= user.last_dim_to_breed) ? (repro_c5) : 0;
      }
      else
      {
        tbl_preg_rate[m][l] = (m_t <= user.last_dim_to_breed) ? (repro_c5 * 10 / 30) : 0;
      }
      //-printf("%f\t", tbl_preg_rate[m][l]);

      // Cost of Reproduction
      if (m_t == 1 || m_t > 24)
      {
        tbl_repro_cost[m][l] = 0;
      }
      else
      {
        tbl_repro_cost[m][l] = (m_t <= user.last_dim_to_breed) ? (user.repro_cost) : 0;
      }
      //-printf("%f\t", tbl_repro_cost[m][l]);

      // Repro Cull
      if (m_t <= 5)
      {
        tbl_repro_cull[m][l] = 1;
      }
      else
      {
        tbl_repro_cull[m][l] = (tbl_milk_prod_non_preg[m][l] > user.milk_threshold_to_cull) ? 1 : 0;
      }
      //-printf("%f\t", tbl_repro_cull[m][l]);
    }
    //-printf("\n");
  }
  // Write tbl_milk_prod_non_preg to file. Needed by a diff calculation later.
  write_csv(tbl_milk_prod_non_preg, "./output/tbl_milk_prod_non_preg.csv");

  // Abortion Rate
  int p;
  double repro_c86 = user.between_35_to_250_days_in_pregnancy / REPRO_C85;
  for (p = 0; p < NUM_PRG; p++)
  {
    for (l = 0; l < NUM_LACT; l++)
    {
      if (p < 2 || p == 9)
      {
        tbl_abortion[p][l] = 0;
      }
      else if (p == 2)
      {
        tbl_abortion[p][l] = 0.035 * repro_c86;
      }
      else if (p == 3)
      {
        tbl_abortion[p][l] = 0.025 * repro_c86;
      }
      else if (p == 4)
      {
        tbl_abortion[p][l] = 0.015 * repro_c86;
      }
      else if (p == 5)
      {
        tbl_abortion[p][l] = 0.005 * repro_c86;
      }
      else if (p == 6)
      {
        tbl_abortion[p][l] = 0.0025 * repro_c86;
      }
      else if (p == 7 || p == 8)
      {
        tbl_abortion[p][l] = 0.001 * repro_c86;
      }
      //-printf("%f\t", tbl_abortion[p][l]);
    }
    //-printf("\n");
  }

  // Calculate monthly data (yellow columns in spreadsheet)

  int lact, mim, prg;
  int mim_start, mim_final;
  int lact_t, mim_t;
  int row_num = 0;
  double temp_dbl;

  for (lact = 0; lact < NUM_LACT; lact++)
  {
    lact_t = lact + 1;

    for (prg = 0; prg < NUM_PRG; prg++)
    {
      // mim_start and mim_final change with prg.
      if (prg != 0)
      {
        mim_start = prg + 2;
        mim_final = prg + 24;
      }
      else
      {
        mim_start = 1;
        mim_final = 25;
      }

      for (mim = (mim_start - 1); mim <= (mim_final - 1); mim++)
      {
        mim_t = mim + 1;

        arr_lact[row_num] = lact_t;
        arr_mim[row_num] = mim_t;
        arr_prg[row_num] = prg;

        // MILK PXION
        arr_milk_pxion[row_num] = tbl_milk_prod_non_preg[mim][lact];
        //-printf("%d\t%f\n", row_num, arr_milk_pxion[row_num]);

        // MILK PRODUCED
        arr_milk_produced[row_num] = tbl_milk_prod_preg[prg];
        //-printf("%d\t%f\n", row_num, arr_milk_produced[row_num]);

        // FEED INTAKE
        temp_dbl = (0.4 + user.milk_butterfat * 15) * arr_milk_pxion[row_num] * arr_milk_produced[row_num];
        arr_feed_intake[row_num] = tbl_body_weight[mim][lact] * 0.02 + 0.3 * temp_dbl;
        //-printf("%d\t%f\n", row_num, arr_feed_intake[row_num]);

        // O
        arr_O[row_num] = (prg < 8) ? 1 : 0;

        // P
        arr_P[row_num] = 1 - arr_O[row_num];

        // PREGNANCY RATE
        arr_pregnancy_rate[row_num] = tbl_preg_rate[mim][lact];
        //-printf("%d\t%f\n", row_num, arr_pregnancy_rate[row_num]);

        // NON PREGNANT
        arr_non_pregnant[row_num] = 1 - arr_pregnancy_rate[row_num];

        // PREGNANCY COST
        if (prg != 0)
        {
          arr_pregnancy_cost[row_num] = 0;
        }
        else
        {
          arr_pregnancy_cost[row_num] = tbl_repro_cost[mim][lact];
        }
        //-printf("%d\t%f\n", row_num, arr_pregnancy_cost[row_num]);

        // ABORTION RATE
        arr_abortion_rate[row_num] = tbl_abortion[prg][lact];
        //-printf("%d\t%f\n", row_num, arr_abortion_rate[row_num]);

        // NON ABORTED
        arr_non_aborted[row_num] = 1 - arr_abortion_rate[row_num];

        // NO-REPRO LEAVING
        if (prg != 0)
        {
          arr_no_repro_leaving[row_num] = tbl_non_repro_leaving_preg[mim][lact];
        }
        else
        {
          arr_no_repro_leaving[row_num] = tbl_non_repro_leaving_non_preg[mim][lact];
        }
        //-printf("%d\t%f\n", row_num, arr_no_repro_leaving[row_num]);

        // NON LEAVING
        arr_non_leaving[row_num] = 1 - arr_no_repro_leaving[row_num];

        // REPRO CULL
        if (prg != 0)
        {
          arr_repro_cull[row_num] = 1;
        }
        else
        {
          arr_repro_cull[row_num] = tbl_repro_cull[mim][lact];
        }
        //-printf("%d\t%f\n", row_num, arr_repro_cull[row_num]);

        // NON REPRO CULL
        arr_non_reprocull[row_num] = 1 - arr_repro_cull[row_num];
        //-printf("%d\t%f\n", row_num, arr_non_reprocull[row_num]);

        row_num++;
      }
    }
  }
  return 0;
}

/*
 * Calculate first iteration
 */
int calculate_first_iter()
{

  extern struct user_inputs user;

  int lact, lact_t, mim, mim_start, mim_final, mim_t, prg;
  int row_num = 0;

  for (lact = 0; lact < NUM_LACT; lact++)
  {
    lact_t = lact + 1;

    for (prg = 0; prg < NUM_PRG; prg++)
    {
      // mim_start and mim_final change with prg.
      if (prg != 0)
      {
        mim_start = prg + 2;
        mim_final = prg + 24;
      }
      else
      {
        mim_start = 1;
        mim_final = 25;
      }

      for (mim = (mim_start - 1); mim <= (mim_final - 1); mim++)
      {
        mim_t = mim + 1;
        if (lact_t == user.lactation && mim_t == user.month_after_calving && prg == user.month_in_pregnancy)
        {
          curr_iter[row_num] = COWS;
        }
        else
        {
          curr_iter[row_num] = 0;
        }
        row_num++;
      }
    }
  }

  // This is a way to check if the iteration calculations are correct.
  // The iteration values must sum up to COWS.
  if (fabs(array_sum(curr_iter, 0, NUM_ROWS) - COWS) > 0.000001)
  {
    char msg[200];
    sprintf(msg, "[NOTICE]: Iteration sum DOES NOT equal %d.\n", COWS);
    log_msg(msg);
    return 2;
  }

  // Calculate cost values for the first iteration.
  calculate_costs(curr_iter, 0);

  return 0;
}

/*
 * Calculate the abortions that return to the flow.
 */
double abortions_to_flow()
{
  int i;
  double sum = 0;
  for (i = 0; i < NUM_ROWS; i++)
  {
    if (arr_prg[i] > 0 && arr_mim[i] > 24)
    {
      sum += (arr_non_leaving[i] * arr_abortion_rate[i] * prev_iter[i]);
    }
  }
  return sum;
}

/*
 * Calculate iterations
 */
int calculate_iterations()
{
  log_msg("Calculating iterations...\n");
  char message[200];
  sprintf(message, "SRH : %f \n calf_mortality: %f  \n", SRH, calf_mortality);
  log_msg(message);

  extern int calc_for_cow;
  extern double cow_total, rep_total;
  extern int calc_for_cow_or_herd;
  extern FILE *fp_herd;
  extern struct user_inputs user;

  int iter_num;
  int lact, mim, prg;
  int mim_start, mim_final;
  int i; // Row counter
  int j;
  int k;
  double sum, temp_dbl;
  // Begin Beef
  double adult_calving = 0, adultHeiferCalf = 0;
  double lactation11, lactation12, lactation13, lactation1g3 = 0;
  double lactation21, lactation22, lactation23, lactation2g3 = 0;
  double lactationG21 = 0, lactationG22 = 0, lactationG23 = 0, lactationG2g3 = 0;
  double heiferPercents1[5][6] = {{0}, {0}}; // Cells AD4:AI8 in CowValue sheet
  double heiferPercents2[5][6] = {{0}, {0}}; // Cells AD11:AI15 in CowValue sheet
  double heiferSumProds[6] = {0};
  double heifer[20], heiferSum = 0, heiferOutput;

  conceptionRateSS = conceptionRateCS * 0.8;

  // Jacob Code!
  double heiferNineMo = 0;

  // First iteration.
  calculate_first_iter();

  // log_msg("Calculating remaining iterations...\n");
  for (iter_num = 1; iter_num < SIM_ITERS; iter_num++)
  {

    //      char msg[200];
    //      sprintf(msg, "SIM_ITERS %d  \n", SIM_ITERS);
    //      log_msg(msg);

    // Exchange pointers.
    temp_iter = prev_iter;
    prev_iter = curr_iter;
    curr_iter = temp_iter;

    i = 0;

    for (lact = 1; lact <= NUM_LACT; lact++)
    {

      for (prg = 0; prg < NUM_PRG; prg++)
      {
        if (prg != 0)
        {
          mim_start = prg + 2;
          mim_final = prg + 24;
        }
        else
        {
          mim_start = 1;
          mim_final = 25;
        }

        for (mim = mim_start; mim <= mim_final; mim++)
        {
          switch (prg)
          {
          case 0:
            if (mim == 1)
            {
              // This term is common the first row of every new $lact.
              sum = sumproduct2(prev_iter, arr_non_leaving, i - 23, 23);
              if (lact == 1)
              { // Extra term for the very first row of the column.
                sum = sum + sumproduct2(prev_iter, arr_no_repro_leaving, 0, NUM_ROWS) + sumproduct3(prev_iter, arr_non_reprocull, arr_non_leaving, 2, 23) + sumproduct3(prev_iter, arr_non_reprocull, arr_non_leaving, 236, 21) + sumproduct3(prev_iter, arr_non_reprocull, arr_non_leaving, 466, 23) + sumproduct3(prev_iter, arr_non_reprocull, arr_non_leaving, 698, 23) + sumproduct3(prev_iter, arr_non_reprocull, arr_non_leaving, 930, 23) + sumproduct3(prev_iter, arr_non_reprocull, arr_non_leaving, 1162, 23) + sumproduct3(prev_iter, arr_non_reprocull, arr_non_leaving, 1394, 23) + sumproduct3(prev_iter, arr_non_reprocull, arr_non_leaving, 1626, 23) + sumproduct3(prev_iter, arr_non_reprocull, arr_non_leaving, 1858, 23) + sumproduct3(prev_iter, arr_non_reprocull, arr_non_leaving, 2090, 23) + sumproduct3(prev_iter, arr_non_reprocull, arr_non_leaving, 2297, 23) + abortions_to_flow();
              }
              curr_iter[i] = sum;
              //-printf("sum: %f\n", sum);

              // Begin Beef
              // Calculate adult calving for the last iteration
              if (iter_num == SIM_ITERS - 1 && lact > 1 && lact < NUM_LACT)
              {
                char msg[200];
                sprintf(msg, "Beef: -------- LMP (%d, %d, %d) - For adding to adult calving %f \n", lact, mim, prg, curr_iter[i]);
                // log_msg(msg);
                adult_calving += curr_iter[i];
              }
              // Calculate 'need' for the last iteration and 1st lactation
              if (iter_num == SIM_ITERS - 1 && lact == 1)
              {
                char msg[200];
                // prev:  need = curr_iter[i] * 12;
                need = curr_iter[i];
                sprintf(msg, "Beef: Need - %f \n", need);
                // log_msg(msg);
              }
              ///
            }
            else if (mim == 2)
            {
              curr_iter[i] = prev_iter[i - 1] * arr_non_leaving[i - 1];
            }
            else if (mim > 2)
            {
              temp_dbl = prev_iter[i - 1] * arr_non_leaving[i - 1] * arr_non_pregnant[i - 1] * arr_repro_cull[i - 1];
              for (j = 1; j <= mim - 3; j++)
              {
                if (j > 11 - 3)
                {
                  break;
                }
                temp_dbl = temp_dbl + (prev_iter[i + (j * 22)] * arr_abortion_rate[i + (j * 22)] * arr_non_leaving[i + (j * 22)]);
              }
              curr_iter[i] = temp_dbl;

              // Begin Beef

              if (iter_num == SIM_ITERS - 1 && lact == 1)
              {
                if (mim == 3)
                {
                  lactation11 = curr_iter[i];
                  char msg[200];
                  sprintf(msg, "Beef: LMP (%d, %d, %d) - Lactation 1. 1st %f \n", lact, mim, prg, curr_iter[i]);
                  // log_msg(msg);
                }
                if (mim == 4)
                {
                  lactation12 = curr_iter[i];
                  char msg[200];
                  sprintf(msg, "Beef: LMP (%d, %d, %d) - Lactation 1. 2nd %f \n", lact, mim, prg, curr_iter[i]);
                  // log_msg(msg);
                }
                if (mim == 5)
                {
                  lactation13 = curr_iter[i];
                  char msg[200];
                  sprintf(msg, "Beef: LMP (%d, %d, %d) - Lactation 1. 3rd %f \n", lact, mim, prg, curr_iter[i]);
                  // log_msg(msg);
                }
                if (mim > 5 && mim <= 10)
                {
                  lactation1g3 += curr_iter[i];
                  char msg[200];
                  sprintf(msg, "Beef: -------- LMP (%d, %d, %d) - For adding to Lactation 1. >3rd %f \n", lact, mim, prg, curr_iter[i]);
                  // log_msg(msg);
                }
              }

              if (iter_num == SIM_ITERS - 1 && lact == 2)
              {
                if (mim == 3)
                {

                  lactation21 = curr_iter[i];
                  char msg[200];
                  sprintf(msg, "Beef: LMP (%d, %d, %d) - Lactation 2. 1st %f \n", lact, mim, prg, curr_iter[i]);
                  // log_msg(msg);
                }
                if (mim == 4)
                {
                  lactation22 = curr_iter[i];
                  char msg[200];
                  sprintf(msg, "Beef: LMP (%d, %d, %d) - Lactation 2. 2nd %f \n", lact, mim, prg, curr_iter[i]);
                  // log_msg(msg);
                }
                if (mim == 5)
                {
                  lactation23 = curr_iter[i];
                  char msg[200];
                  sprintf(msg, "Beef: LMP (%d, %d, %d) - Lactation 2. 3rd %f \n", lact, mim, prg, curr_iter[i]);
                  // log_msg(msg);
                }
                if (mim > 5 && mim <= 10)
                {
                  lactation2g3 += curr_iter[i];
                  char msg[200];
                  sprintf(msg, "Beef: -------- LMP (%d, %d, %d) - For adding to Lactation 2. >3rd %f \n", lact, mim, prg, curr_iter[i]);
                  // log_msg(msg);
                }
              }

              if (iter_num == SIM_ITERS - 1 && lact > 2)
              {
                if (mim == 3)
                {
                  lactationG21 += curr_iter[i];
                  char msg[200];
                  sprintf(msg, "Beef: -------- LMP (%d, %d, %d) - For adding to Lactation >2. 1st %f \n", lact, mim, prg, curr_iter[i]);
                  // log_msg(msg);
                }
                if (mim == 4)
                {
                  lactationG22 += curr_iter[i];
                  char msg[200];
                  sprintf(msg, "Beef: -------- LMP (%d, %d, %d) - For adding to Lactation >2. 2nd %f \n", lact, mim, prg, curr_iter[i]);
                  // log_msg(msg);
                }
                if (mim == 5)
                {
                  lactationG23 += curr_iter[i];
                  char msg[200];
                  sprintf(msg, "Beef: -------- LMP (%d, %d, %d) - For adding to Lactation >2. 3rd %f \n", lact, mim, prg, curr_iter[i]);
                  // log_msg(msg);
                }
                if (mim > 5)
                {
                  lactationG2g3 += curr_iter[i];
                  char msg[200];
                  sprintf(msg, "Beef: -------- LMP (%d, %d, %d) - For adding to Lactation >2. >3rd %f \n", lact, mim, prg, curr_iter[i]);
                  // log_msg(msg);
                }
              }

              ///
            }

            break;

          case 1:
            curr_iter[i] = prev_iter[i - 24] * arr_pregnancy_rate[i - 24] * arr_non_leaving[i - 24] * arr_repro_cull[i - 24];

            break;

          case 9:

            curr_iter[i] = prev_iter[i - 23] * arr_non_leaving[i - 23] * arr_non_aborted[i - 23];
            // Lets get some 9 month pregrancy values
            if (prg == 9)
            {
              //	            char message[200];
              heiferNineMo += curr_iter[i];
              //	            if( curr_iter[i] != 0){
              //                   sprintf(message, " Heifer MPI ---- (%d, %d, %d) HeiferNine %f,  %f \n", mim, prg, curr_iter[i], heiferNineMo);
              //                   log_msg(message);
              //
              //                }
            }

            break;

          default:
            curr_iter[i] = prev_iter[i - 23] * arr_non_leaving[i - 23] * arr_non_aborted[i - 23];

            break;
          }
          i++;
        }
      }
    }

    // This is a way to check if the iteration calculations are correct.
    // The iteration values must sum up to COWS.
    if (fabs(array_sum(curr_iter, 0, NUM_ROWS) - COWS) > 0.000001)
    {
      // If calculating for herd, add a note in the spreadsheet to be generated.
      if (calc_for_cow_or_herd == 2 && herd_counter == 1)
      {
        fprintf(fp_herd, "%s,%d,%d,%d,%f,%f,Iteration %d does not sum to %d\n", user.cowID, user.lactation, user.dim, user.dip,
                user.extra_rest_of_lactation + 1.00, user.extra_next_lactations + 1.00, iter_num, COWS);
      }
      char msg[200];
      sprintf(msg, "[NOTICE]: Iteration sum does not equal %d.\n", COWS);
      // log_msg(msg);
      return 2;
    }

    double sum = heiferNineMo * (femalesCS / 100);
    //    char message[200];
    //    sprintf(message, "HeiferNineMoSum * FemaleCS %:  %f, Iteration Num: (%d) \n", sum , iter_num);
    //    log_msg(message);

    // Add to array and Log HeiferNineMo * Female Conventional Semen
    heiferSum_Arr[iter_num] = sum;

    heiferNineMo = 0;
    // Calculate various cost values for this iteration.
    calculate_costs(curr_iter, iter_num);
  }

  // Begin Beef
  char msg[200];
  adult_calving *= 12;
  sprintf(msg, "Beef: Lactation 1. >3rd %f\nBeef: Lactation 2. >3rd %f\nBeef: Adult Calving - %f \n",
          lactation1g3, lactation2g3, adult_calving);
  // log_msg(msg);

  char msg1[200];
  sprintf(msg1, "Beef: Lactation >2. 1st %f\nBeef: Lactation >2. 2nd %f\nBeef: Lactation >2. 3rd %f \nBeef: Lactation >2. >3rd %f\n",
          lactationG21, lactationG22, lactationG23, lactationG2g3);
  // log_msg(msg1);

  beefOutputs[currentCalc++] = lactation11;
  beefOutputs[currentCalc++] = lactation12;
  beefOutputs[currentCalc++] = lactation13;
  beefOutputs[currentCalc++] = lactation1g3;
  beefOutputs[currentCalc++] = lactation21;
  beefOutputs[currentCalc++] = lactation22;
  beefOutputs[currentCalc++] = lactation23;
  beefOutputs[currentCalc++] = lactation2g3;
  beefOutputs[currentCalc++] = lactationG21;
  beefOutputs[currentCalc++] = lactationG22;
  beefOutputs[currentCalc++] = lactationG23;
  beefOutputs[currentCalc++] = lactationG2g3;

  heiferPercents1[0][0] = conceptionRateCS;
  for (j = 1; j < 6; j++)
  {
    heiferPercents1[0][j] = conceptionRateSS;
  }
  double runningSum, factor, part1;
  char msgSmall[100];
  for (i = 0; i < 6; i++)
  {
    sprintf(msgSmall, "Beef: Heifer Percentages1(%d,%d) : %f \n",
            0, i, heiferPercents1[0][i]);
    // log_msg(msgSmall);
  }
  for (i = 1; i < 5; i++)
  {

    for (j = 0; j < 6; j++)
    {
      runningSum = 0;
      for (k = 0; k < i; k++)
      {
        runningSum += heiferPercents1[k][j];
      }
      part1 = 1 - runningSum / 100;
      if (j <= i)
      {
        factor = conceptionRateCS;
      }
      else
      {
        factor = conceptionRateSS;
      }
      heiferPercents1[i][j] = part1 * factor;
      sprintf(msgSmall, "Beef: Heifer Percentages1(%d,%d) : %f \n",
              i, j, heiferPercents1[i][j]);
      // log _msg(msgSmall);
    }
  }

  for (i = 0; i < 5; i++)
  {
    for (j = 0; j < 6; j++)
    {
      if (j <= i)
      {
        heiferPercents2[i][j] = femalesCS;
      }
      else
      {
        heiferPercents2[i][j] = femalesSS;
      }
    }
  }

  for (j = 0; j < 6; j++)
  {
    heiferSumProds[j] = 0;
    for (i = 0; i < 5; i++)
    {
      heiferSumProds[j] += heiferPercents1[i][j] * heiferPercents2[i][j];
    }
    heiferSumProds[j] /= 100;
  }

  char msg3[200];
  sprintf(msg3, "Beef: Calculated Percentages %f %f %f %f %f %f \n",
          heiferSumProds[0], heiferSumProds[1], heiferSumProds[2], heiferSumProds[3], heiferSumProds[4], heiferSumProds[5]);
  // log_msg(msg3);

  adultHeiferCalf = adult_calving * femalesCS / 100;
  factor = heiferSumProds[servicesSS] / 100;
  heifer[0] = adultHeiferCalf * factor;

  heiferSum += adultHeiferCalf + heifer[0];

  for (i = 1; i < 20; i++)
  {
    heifer[i] = heifer[i - 1] * factor;
    heiferSum += heifer[i];
    sprintf(msgSmall, "Beef: Heifer(%d) : %f \n",
            i, heifer[i]);
    // log_msg(msgSmall);
  }

  sprintf(msgSmall, "Beef: Heifer Sum : %f \n",
          heiferSum);
  // log_msg(msgSmall);

  heiferOutput = heiferSum;
  sprintf(msgSmall, "Beef: Heifer Output : %f \n",
          heiferOutput);
  // log_msg(msgSmall);
  beefOutputs[currentCalc++] = heiferOutput;
  sprintf(msgSmall, "Beef: ServiceSS : %d \n",
          servicesSS);
  // log_msg(msgSmall);
  for (i = 1; i <= 3; i++)
  {
    if (servicesSS < i)
    {
      factor = conceptionRateCS / 100;
    }
    else
    {
      factor = conceptionRateSS / 100;
    }
    sprintf(msgSmall, "Beef: Factor : %f, i : %d \n",
            factor, i);
    // log_msg(msgSmall);
    part1 = heiferOutput * factor;
    heiferOutput -= part1;
    sprintf(msgSmall, "Beef: Part : %f, i : %d \n",
            part1, i);
    // log_msg(msgSmall);
    sprintf(msgSmall, "Beef: Heifer Output : %f \n",
            heiferOutput);
    // log_msg(msgSmall);
    beefOutputs[currentCalc++] = heiferOutput;
  }

  ///

  // Store cow's cost values.
  arr_cow_costs[herd_counter] = (struct total_cow_costs){
      .repro_cost = array_sum(repro_cost, 0, SIM_ITERS),
      .calf_value = array_sum(calf_value, 0, SIM_ITERS),
      .non_repro_cull_cost = array_sum(non_repro_cull_cost, 0, SIM_ITERS),
      .mortality_cost = array_sum(mortality_cost, 0, SIM_ITERS),
      .repro_cull_cost = array_sum(repro_cull_cost, 0, SIM_ITERS),
      .milk = array_sum(milk, 0, SIM_ITERS),
      .feed = array_sum(feed, 0, SIM_ITERS),
      .net_return = array_sum(net_return, 0, SIM_ITERS),
      //.present_value =       array_sum(present_value,       0, SIM_ITERS),
  };

  // Evaluate cow
  if (herd_counter != 0)
  {
    // printf("Col: %d\n", iter_num);
    evaluate_cow(herd_counter);
  }

  // print_total_costs(herd_counter);

  total = array_sum(net_return, 0, SIM_ITERS);

  if (calc_for_cow)
  {
    cow_total = total;
  }
  else
  {
    rep_total = total;
  }

  // Backup replacement's and cow's net return values.
  int n;
  for (n = 0; n < SIM_ITERS; n++)
  {
    net_return_bk[n][herd_counter] = net_return[n];
  }

  // Write backed up net return values to file once cow's calculations have been done.
  if (herd_counter != 0 && calc_for_cow_or_herd == 1)
  { // Cow calculations done.
    char *file = "./output/net_return.csv";
    FILE *fd;
    int i;

    // Open file.
    fd = fopen(file, "w");
    if (fd == NULL)
    {
      char msg[200];
      sprintf(msg, "Could not open file %s.\n", file);
      log_err(msg);
    }

    // Write contents of array to file.
    for (i = 0; i < SIM_ITERS; i++)
    {
      fprintf(fd, "%d,", i + 1);
      fprintf(fd, "%f,", net_return_bk[i][0]);
      fprintf(fd, "%f", net_return_bk[i][1]);
      fprintf(fd, "\n");
    }

    fclose(fd);
  }
  // print_costs(iter_num-1);

  // Write cowID and cow_values to file.
  if (calc_for_cow_or_herd == 2 && herd_counter == 1)
  {
    extern char *units;
    extern double dollar_to_pound;
    double conversion_factor = 1;
    if (strcmp(units, "uk") == 0)
    {
      conversion_factor = dollar_to_pound;
    }
    fprintf(fp_herd, "%s,%d,%d,%d,%f,%f,%.0f\n", user.cowID, user.lactation, user.dim, user.dip,
            user.extra_rest_of_lactation + 1.00, user.extra_next_lactations + 1.00, arr_cow_eval[1].cow_value * conversion_factor);
  }

  // was: herd_counter++;
  herd_counter = 1;
  //-printf("Herd counter: %d\n", herd_counter);

  // Jacob Code
  // Test Vals in heifer sum
  //  for( int i = 0; i < 150; i++){
  //
  //       char message[200];
  //       sprintf(message, "HeiferSum_Arr[%d] : %f \n", i, heiferSum_Arr[i]  );
  //       log_msg(message);
  //
  //  }

  //       char message[200];
  //       sprintf(message, "SRH : %f, calf_mortality: %f, Herd_Turnover_Ratio: %f \n", SRH, calf_mortality, user.herd_turnover_ratio );
  //       log_msg(message);

  // Create the conception rates array
  // B35 Heifer = BEEF!$I$8*SRH ( I8 = conceptionRateCS, SRH = Average Service Rate For heifers I need to add input for this)
  double B25 = (((conceptionRateCS - 40) * SRH) / 100);
  double conceptRates[13] = {
      (((conceptionRateCS - 0) * SRH) / 100),
      (((conceptionRateCS - 10) * SRH) / 100),
      (((conceptionRateCS - 15) * SRH) / 100),
      (((conceptionRateCS - 20) * SRH) / 100),
      (((conceptionRateCS - 25) * SRH) / 100),
      (((conceptionRateCS - 30) * SRH) / 100),
      (((conceptionRateCS - 35) * SRH) / 100),
      B25,
      B25 - .01,
      B25 - .02,
      B25 - .03,
      B25 - .04,
      B25 - .05};

  // Test logger for conception rates
  //    for(int q = 0; q < 13; q++){
  //            char message[200];
  //            sprintf(message, "conceptRates[%d] : %f \n",   q, conceptRates[q]  );
  //            log_msg(message);
  //
  //    }

  // Right now set time zero and time one to zero heifers
  for (int i = 0; i <= 25; i++)
  {
    heifer_zero_preg[i][0] = 0;
    heifer_zero_preg[i][1] = 0;
  }

  // Jacob Code
  // Calculate Heifer Values
  double nine_mo_heifer = 0;
  int breedMo = 15;
  int offset = -1;
  int cull_non_preg = 0;

  // Set iter_num = 2 because T-one of table is user input, lines up with excel better
  for (int iter_num = 2; iter_num < 599; iter_num++)
  {

    for (prg = 0; prg < 10; prg++)
    {
      if (prg != 0)
      {
        // Will always need to subtract 14 from mim counter to index but will help with logging
        mim_start = prg + 14;
        mim_final = prg + 26;
      }
      else
      {
        mim_start = 1;
        mim_final = 26;
      }

      switch (prg)
      {
      case 0:

        for (int mim = mim_start; mim < mim_final; mim++)
        {
          // Top Row Yellow Vals ( 9 mo preg heifers from Cow tool and 9 mo preg from heifer too)
          if (mim == 1)
          {
            // heiferSum_arr[i] is 9 mo preg from Cow tool, Nine mo heifer is from this tool
            heifer_zero_preg[mim][iter_num] = (heiferSum_Arr[iter_num] + (nine_mo_heifer * femalesCS / 100));
            char msg1[200];
            // heiferSum_Arr[i] + (nine_mo_heifer * (femalesCS/100)
            sprintf(msg1, "Heifer MIM 1-(P:%d, M:%d, I:%d) %f + ( %f * %f ) = %0.9f  \n", prg, mim, iter_num, heiferSum_Arr[iter_num], nine_mo_heifer, (femalesCS / 100), heifer_zero_preg[mim][iter_num]);
            log_msg(msg1);
          }
          else if (mim == 2)
          {
            // If Preg == 0, Mim = 2
            heifer_zero_preg[mim][iter_num] = (heifer_zero_preg[mim - 1][iter_num - 1] * (1 - calf_mortality));

            if (heifer_zero_preg[mim][iter_num] != 0)
            {
              char msg1[200];
              sprintf(msg1, "\tHeifer Before Breeding -(P:%d, M:%d, I:%d): %f  * (1 - %f) ) = %0.9f \n", prg, mim, iter_num, heifer_zero_preg[mim - 1][iter_num - 1], calf_mortality, heifer_zero_preg[mim][iter_num]);
              log_msg(msg1);
            }
          }
          else if (mim > 2 && mim < breedMo)
          {
            // If Preg == 0 and mim < 15 (Before Breeding), cull rate is always 0 right now
            heifer_zero_preg[mim][iter_num] = heifer_zero_preg[mim - 1][iter_num - 1];

            if (heifer_zero_preg[mim][iter_num] != 0)
            {
              char msg1[200];
              sprintf(msg1, "\tHeifer BEFORE Breeding-(P:%d, M:%d, I:%d): %f  * .00  = %0.9f \n", prg, mim, iter_num, heifer_zero_preg[mim - 1][iter_num - 1], heifer_zero_preg[mim][iter_num]);
              log_msg(msg1);
            }
          }
          else if (mim >= breedMo)
          {
            // Mim > breed mo and preg == 0
            // Add in pregnancy rate calculation
            offset = (mim - (breedMo + 1));
            heifer_zero_preg[mim][iter_num] = (heifer_zero_preg[mim - 1][iter_num - 1] * (1 - conceptRates[mim - 15]));

            //
            if (heifer_zero_preg[mim][iter_num] != 0)
            {
              char msg1[200];
              sprintf(msg1, "\tHeifer During Breeding Non Pregnant(P:%d, M:%d, I:%d) = %f  * 1 - %f ), %0.15f :\n", prg, mim, iter_num, heifer_zero_preg[mim - 1][iter_num - 1], conceptRates[mim - 15], heifer_zero_preg[mim][iter_num]);
              log_msg(msg1);
            }
          }
        }
              log_msg("EU SAI TÁ?");

        break;

      // Case 1 ref
      //  heifer_preg_arr [10][12][599]
      case 1:

        for (int mim = mim_start; mim < mim_final; mim++)
        {

          offset = prg + mim_start;
          heifer_preg_arr[prg][mim][iter_num] = (heifer_zero_preg[mim - 1][iter_num - 1] * (conceptRates[mim - 15]));

          if (heifer_preg_arr[prg][mim][iter_num] != 0)
          {
            char msg1[200];
            sprintf(msg1, "\t\tHeifer During Breeding & Pregnant-(P:%d, M:%d, I:%d): %f  *  %f =  %f \n", prg, mim, iter_num, heifer_zero_preg[mim - 1][iter_num - 1], (conceptRates[mim - 15]), heifer_preg_arr[prg][mim][iter_num]);
            log_msg(msg1);
          }
        }

        break;

      // After Breeding and getting pregnant
      case 2 ... 8:

        for (int mim = mim_start; mim < mim_final; mim++)
        {

          //(1 - Cull rate) = .99 Right now may change
          heifer_preg_arr[prg][mim][iter_num] = heifer_preg_arr[prg - 1][mim - 1][iter_num - 1] * .99;

          if (heifer_preg_arr[prg][mim][iter_num] != 0)
          {
            char msg1[200];
            sprintf(msg1, "\t\t\tHeifer after Breeding and getting Pregnant-(P:%d, M:%d, I:%d) %f  * .99 = %f)\n", prg, mim, iter_num, heifer_preg_arr[prg - 1][mim - 1][iter_num - 1], heifer_preg_arr[prg][mim][iter_num]);
            log_msg(msg1);
          }
        }

        break;

      case 9:
        nine_mo_heifer = 0;
        for (int mim = mim_start; mim < mim_final; mim++)
        {

          // Weird error for iteration 1
          if (iter_num < 3)
          {
            heifer_preg_arr[prg][mim][iter_num] = 0;
            break;
          }

          //(1 - Cull rate) = .99 Right now may change
          heifer_preg_arr[prg][mim][iter_num] = heifer_preg_arr[prg - 1][mim - 1][iter_num - 1] * .99;
          // Increment nine_mo_heifer for top row of spreadsheet
          nine_mo_heifer = nine_mo_heifer + heifer_preg_arr[prg][mim][iter_num];

          if (heifer_preg_arr[prg][mim][iter_num] != 0)
          {

            char msg1[200];
            sprintf(msg1, "\t\t\t\tHeifer after Breeding and getting Pregnant-(P:%d, M:%d, I:%d) %0.8f  * .99 = %0.8f)\n", prg, mim, iter_num, heifer_preg_arr[prg - 1][mim - 1][iter_num - 1], heifer_preg_arr[prg][mim][iter_num]);
            log_msg(msg1);
          }
        }
        char msg2[200];
        sprintf(msg2, "\t\t\t\t\tnine_mo_heifer: %f \n", nine_mo_heifer);
        log_msg(msg2);

        // Save last iteration value for proportion calculation
        if (iter_num == 598)
        {
          AAF135 = nine_mo_heifer;
        }

        break;
      }
    }
  }

  return 0;
}

/*
 * Evaluate cow
 */
void evaluate_cow(int herd_counter)
{
  log_msg("Evaluating cow...\n");
  log_msg("\t Output.csv written in evaluate_cow...\n");
  extern struct user_inputs user;
  extern FILE *fp_out;

  int i;
  double arr_DO[NUM_ROWS];
  double temp_days_to_conception, temp_per_pregnant, temp_days_in_milk, temp_per_repro_cull, temp_per_mortality;
  double temp_per_1st_lact, temp_per_2nd_lact, temp_per_3rd_lact, temp_per_gt_3rd_lact;
  double temp_mul = 1;

  temp_days_to_conception = temp_per_pregnant = temp_days_in_milk = temp_per_repro_cull = temp_per_mortality = 0;
  temp_per_1st_lact = temp_per_2nd_lact = temp_per_3rd_lact = temp_per_gt_3rd_lact = 0;

  // Heifer 3+ service value
  double AAE21_25 = heifer_zero_preg[17][598] + heifer_zero_preg[18][598] + heifer_zero_preg[19][598] + heifer_zero_preg[20][598] + heifer_zero_preg[21][598];

  // Proportion Value from heiferspreadsheet
  double proportion = AAF135 / heifer_zero_preg[1][598];

  char message[200];
  sprintf(message, "SRH : %f \n calf_mortality: %f \n Herd_Turnover_Ratio: %f \n", SRH, calf_mortality, user.herd_turnover_ratio);
  log_msg(message);

  char msg3[200];
  sprintf(msg3, "FemCalvingsReqd: %f\n Heifer1: %f \n Heifer2: %f \n Heifer3: %f \n Heifer3+: %f \n", ((need * 10) / proportion), heifer_zero_preg[14][598], heifer_zero_preg[15][598], heifer_zero_preg[16][598], AAE21_25);
  log_msg(msg3);

  // Herd Structure at Steady State
  for (i = 0; i < NUM_ROWS; i++)
  {

    // Days to Conception
    if (arr_prg[i] == 0)
    {
      arr_DO[i] = 0;
    }
    else
    {
      arr_DO[i] = (arr_mim[i] - arr_prg[i]) * 25 * curr_iter[i];
    }
    temp_days_to_conception = temp_days_to_conception + arr_DO[i];

    // Percent of Pregnant
    if (arr_prg[i] >= 1)
    {
      temp_per_pregnant = temp_per_pregnant + curr_iter[i];
    }

    // Days In Milk for Herd
    temp_days_in_milk = temp_days_in_milk + arr_mim[i] * curr_iter[i];

    if (arr_lact[i] == 1)
    { // 1st Lactation, %
      temp_per_1st_lact = temp_per_1st_lact + curr_iter[i];
    }
    else if (arr_lact[i] == 2)
    { // 2nd Lactation, %
      temp_per_2nd_lact = temp_per_2nd_lact + curr_iter[i];
    }
    else if (arr_lact[i] == 3)
    { // 3rd Lactation, %
      temp_per_3rd_lact = temp_per_3rd_lact + curr_iter[i];
    }
    else
    { // > 3rd Lactation, %
      temp_per_gt_3rd_lact = temp_per_gt_3rd_lact + curr_iter[i];
    }
  }

  // Reproductive Culling %
  temp_per_repro_cull = repro_cull_cost[SIM_ITERS - 1] * glob_com_div * 12.0 / (user.salvage_value + user.calf_value - user.replacement_value);

  // Mortality %
  temp_per_mortality = mortality_cost[SIM_ITERS - 1] * glob_com_div * 12.0 / (user.calf_value - user.replacement_value);

  temp_mul = glob_com_div * 12.0 / 100.00;

  // Update values
  arr_cow_eval[herd_counter] = (struct cow_evaluated){

      .cow_value = arr_cow_costs[herd_counter].net_return - arr_cow_costs[0].net_return + user.replacement_value - user.calf_value - user.salvage_value,

      .vs_repl = (struct total_cow_costs){
          .repro_cost = arr_cow_costs[herd_counter].repro_cost - arr_cow_costs[0].repro_cost,
          .calf_value = arr_cow_costs[herd_counter].calf_value - arr_cow_costs[0].calf_value,
          .non_repro_cull_cost = arr_cow_costs[herd_counter].non_repro_cull_cost - arr_cow_costs[0].non_repro_cull_cost,
          .mortality_cost = arr_cow_costs[herd_counter].mortality_cost - arr_cow_costs[0].mortality_cost,
          .repro_cull_cost = arr_cow_costs[herd_counter].repro_cull_cost - arr_cow_costs[0].repro_cull_cost,
          .milk = arr_cow_costs[herd_counter].milk - arr_cow_costs[0].milk,
          .feed = arr_cow_costs[herd_counter].feed - arr_cow_costs[0].feed,
      },

      .replacement_transaction = user.replacement_value - user.calf_value - user.salvage_value,

      .steady_state = (struct struct_steady_state){
          .days_in_milk = temp_days_in_milk * 30 / 100,
          .days_to_conception = temp_days_to_conception / temp_per_pregnant,
          .per_pregnant = temp_per_pregnant,
          .per_repro_cull = temp_per_repro_cull,
          .per_mortality = temp_per_mortality,
          .per_1st_lact = temp_per_1st_lact,
          .per_2nd_lact = temp_per_2nd_lact,
          .per_gt_3rd_lact = (temp_per_3rd_lact + temp_per_gt_3rd_lact),
      },

      .avg_cow = (struct total_cow_costs){
          .repro_cost = repro_cost[SIM_ITERS - 1] * temp_mul,
          .calf_value = calf_value[SIM_ITERS - 1] * temp_mul,
          .non_repro_cull_cost = non_repro_cull_cost[SIM_ITERS - 1] * temp_mul,
          .mortality_cost = mortality_cost[SIM_ITERS - 1] * temp_mul,
          .repro_cull_cost = repro_cull_cost[SIM_ITERS - 1] * temp_mul,
          .milk = milk[SIM_ITERS - 1] * temp_mul,
          .feed = feed[SIM_ITERS - 1] * temp_mul,
          .net_return = net_return[SIM_ITERS - 1] * glob_com_div * 12.00 / (double)COWS,
      },
  };

  int wr = fprintf(fp_out, "%.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, "
                           "%.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, "
                           "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f",

                   arr_cow_eval[herd_counter].cow_value,

                   arr_cow_eval[herd_counter].vs_repl.milk,
                   arr_cow_eval[herd_counter].vs_repl.feed,
                   arr_cow_eval[herd_counter].vs_repl.calf_value,
                   arr_cow_eval[herd_counter].vs_repl.non_repro_cull_cost,
                   arr_cow_eval[herd_counter].vs_repl.mortality_cost,
                   arr_cow_eval[herd_counter].vs_repl.repro_cull_cost,
                   arr_cow_eval[herd_counter].vs_repl.repro_cost,

                   arr_cow_eval[herd_counter].replacement_transaction,

                   arr_cow_eval[herd_counter].steady_state.days_in_milk,
                   arr_cow_eval[herd_counter].steady_state.days_to_conception,
                   arr_cow_eval[herd_counter].steady_state.per_pregnant,
                   arr_cow_eval[herd_counter].steady_state.per_repro_cull,
                   arr_cow_eval[herd_counter].steady_state.per_mortality,
                   arr_cow_eval[herd_counter].steady_state.per_1st_lact,
                   arr_cow_eval[herd_counter].steady_state.per_2nd_lact,
                   arr_cow_eval[herd_counter].steady_state.per_gt_3rd_lact,

                   arr_cow_eval[herd_counter].avg_cow.net_return,
                   arr_cow_eval[herd_counter].avg_cow.milk,
                   arr_cow_eval[herd_counter].avg_cow.feed,
                   arr_cow_eval[herd_counter].avg_cow.calf_value,
                   arr_cow_eval[herd_counter].avg_cow.non_repro_cull_cost,
                   arr_cow_eval[herd_counter].avg_cow.mortality_cost,
                   arr_cow_eval[herd_counter].avg_cow.repro_cull_cost,
                   arr_cow_eval[herd_counter].avg_cow.repro_cost,
                   // Output for the beef tool
                   beefOutputs[0], beefOutputs[1], beefOutputs[2], beefOutputs[3],
                   beefOutputs[4], beefOutputs[5], beefOutputs[6], beefOutputs[7],
                   beefOutputs[8], beefOutputs[9], beefOutputs[10], beefOutputs[11],
                   beefOutputs[12], beefOutputs[13], beefOutputs[14], beefOutputs[15],
                   need,
                   heifer_zero_preg[14][598],
                   heifer_zero_preg[15][598],
                   heifer_zero_preg[16][598],
                   AAE21_25,
                   proportion

  );
  if (!wr)
  {
    char msg[200];
    sprintf(msg, "Could not write to output file.\n");
    log_err(msg);
  }

  // printf("\nCOW VALUE: %f\n\n", arr_cow_eval[herd_counter].cow_value);
  // printf("<strong>Compared Against Replacement</strong>\n");
  // printf("Milk Sales: %f\n", arr_cow_eval[herd_counter].vs_repl.milk);
  // printf("Feed Cost: %f\n", arr_cow_eval[herd_counter].vs_repl.feed);
  // printf("Calf Value: %f\n", arr_cow_eval[herd_counter].vs_repl.calf_value);
  // printf("Non-Reproductive Cull: %f\n", arr_cow_eval[herd_counter].vs_repl.non_repro_cull_cost);
  // printf("Mortality Cost: %f\n", arr_cow_eval[herd_counter].vs_repl.mortality_cost);
  // printf("Reproductive Cull: %f\n", arr_cow_eval[herd_counter].vs_repl.repro_cull_cost);
  // printf("Reproduction Costs: %f\n", arr_cow_eval[herd_counter].vs_repl.repro_cost);
  // printf("Reproduction Costs: %f\n", arr_cow_eval[herd_counter].vs_repl.repro_cost);

  // Calculate PR
}

/*
 * Calculate various costs.
 */
void calculate_costs(double *iter, int iter_num)
{
  // FIXME: curr_iter is a global variable! This may be confusing.
  // 	      clean up global variables.
  double *curr_iter = iter;
  extern struct user_inputs user;
  extern int calc_for_cow;

  // FIXME: Use com_mul only in the final cost calculation.
  double com_div = pow(1 + user.interest_rate, iter_num) * COWS;
  assert(com_div != 0);
  double com_mul = 1 / com_div;
  glob_com_div = com_div;

  // REPRO COST
  repro_cost[iter_num] = -sumproduct3(curr_iter, arr_repro_cull, arr_pregnancy_cost, 0, NUM_ROWS) * com_mul;

  // CALF VALUE
  // Sum iteration values for Prg = 9.
  double sum = 0;
  int c, start_row;
  for (c = 1; c <= NUM_LACT; c++)
  {
    start_row = ((c - 1) * 232) + 209;
    sum = sum + array_sum(curr_iter, start_row, 23);
  }
  calf_value[iter_num] = sum * user.calf_value * com_mul;

  // NON-REPRO CULL COST
  non_repro_cull_cost[iter_num] = sumproduct3(curr_iter, arr_no_repro_leaving, arr_repro_cull, 0, NUM_ROWS) * (1 - MORTALITY) * (user.salvage_value + user.calf_value - user.replacement_value) * com_mul;

  // MORTALITY COST
  mortality_cost[iter_num] = sumproduct3(curr_iter, arr_no_repro_leaving, arr_repro_cull, 0, NUM_ROWS) * (MORTALITY) * (user.calf_value - user.replacement_value) * com_mul;

  // REPRO CULL COST
  repro_cull_cost[iter_num] = sumproduct2(curr_iter, arr_non_reprocull, 0, NUM_ROWS) * (user.salvage_value + user.calf_value - user.replacement_value) * com_mul;

  // MILK & FEED
  if (calc_for_cow)
  {
    // THIS LACTATION DISCOUNT MILK & FEED
    calculate_TLDMF(iter_num);

    // NEXT LACTATION DISCOUNT MILK & FEED
    calculate_NLDMF(iter_num);

    milk[iter_num] = (sumproduct3(curr_iter, arr_milk_pxion, arr_milk_produced, 0, NUM_ROWS) * 30.4 * user.milk_price + this_lactation_discount_milk[iter_num] + next_lactation_discount_milk[iter_num]) * com_mul;
    feed[iter_num] = -((sumproduct3(curr_iter, arr_feed_intake, arr_O, 0, NUM_ROWS) * user.feed_price_production + sumproduct3(curr_iter, arr_feed_intake, arr_P, 0, NUM_ROWS) * user.feed_price_dry) * 30.4 + this_lactation_discount_feed[iter_num] + next_lactation_discount_feed[iter_num]) * com_mul;
    //-printf("%d\t\t%f\n", iter_num, milk[iter_num]);
    //-printf("EXTRA REST:\t%f\n", user.extra_rest_of_lactation);
    //-printf("EXTRA NEXT:\t%f\n", user.extra_next_lactations);
  }
  else
  {
    milk[iter_num] = (sumproduct3(curr_iter, arr_milk_pxion, arr_milk_produced, 0, NUM_ROWS) * 30.4 * user.milk_price * (1 + user.additional_milk_replacement)) * com_mul;
    feed[iter_num] = -((sumproduct3(curr_iter, arr_feed_intake, arr_O, 0, NUM_ROWS) * user.feed_price_production * (1 + user.additional_milk_replacement) + sumproduct3(curr_iter, arr_feed_intake, arr_P, 0, NUM_ROWS) * user.feed_price_dry) * 30.4) * com_mul;
  }

  //-print_array(arr_milk_pxion, NUM_ROWS);

  // NET RETURN
  net_return[iter_num] = repro_cost[iter_num] + calf_value[iter_num] + non_repro_cull_cost[iter_num] + mortality_cost[iter_num] + repro_cull_cost[iter_num] + milk[iter_num] + feed[iter_num];
}

/*
 * Check validity of LACT, MIM and PRG values.
 */
int chk_lact_mim_prg(int lact, int mim, int prg, char **msg)
{
  int valid = 1;
  int mim_start, mim_end;
  /* First test absolute bounds */
  if (lact < 1 || lact > 10)
  {
    *msg = "Invalid Lactation value.";
    valid = 0;
  }
  else if (mim < 1 || mim > 33)
  {
    *msg = "Invalid Days in Milk value.";
    valid = 0;
  }
  else if (prg < 0 || prg > 9)
  {
    *msg = "Invalid Days in Pregnancy value.";
    valid = 0;
  }
  /* Then check for compatibility between mim and prg values. */
  else
  {
    if (prg == 0)
    {
      mim_start = 1;
      mim_end = 25;
    }
    else
    {
      mim_start = prg + 2;
      mim_end = mim_start + 22;
    }
    if (!(mim >= mim_start && mim <= mim_end))
    {
      *msg = "Incompatible Days in Milk and Days in Pregnancy values.";
      valid = 0;
    }
  }
  return valid;
}

/*
 * Print costs
 */
void print_costs(int iter_num)
{
  extern double cow_total, rep_total;

  printf("\nRepro Cost:\t\t%f\n", repro_cost[iter_num]);
  printf("Calf Value:\t\t%f\n", calf_value[iter_num]);
  printf("Non-repro Cull Cost:\t%f\n", non_repro_cull_cost[iter_num]);
  printf("Mortality Cost:\t%f\n", mortality_cost[iter_num]);
  printf("Repro Cull Cost:\t%f\n", repro_cull_cost[iter_num]);
  printf("Milk:\t%f\n", milk[iter_num]);
  printf("Feed:\t%f\n", feed[iter_num]);
  printf("This Lactation\n");
  printf("-- Discount Milk:\t%f\n", this_lactation_discount_milk[iter_num]);
  printf("-- Discount Feed:\t%f\n", this_lactation_discount_feed[iter_num]);
  printf("Next Lactation\n");
  printf("-- Discount Milk:\t%f\n", next_lactation_discount_milk[iter_num]);
  printf("-- Discount Feed:\t%f\n", next_lactation_discount_feed[iter_num]);
  printf("Net Return:\t%f\n\n", net_return[iter_num]);
}

/*
 * Print total costs
 */
void print_total_costs(int herd_counter)
{
  printf("\nHerd counter: %d\n", herd_counter);
  printf("Tot Repro Cost:\t\t%f\n", arr_cow_costs[herd_counter].repro_cost);
  printf("Tot Calf Value:\t\t%f\n", arr_cow_costs[herd_counter].calf_value);
  printf("Tot Non-repro Cull Cost:\t%f\n", arr_cow_costs[herd_counter].non_repro_cull_cost);
  printf("Tot Mortality Cost:\t%f\n", arr_cow_costs[herd_counter].mortality_cost);
  printf("Tot Repro Cull Cost:\t%f\n", arr_cow_costs[herd_counter].repro_cull_cost);
  printf("Tot Milk:\t%f\n", arr_cow_costs[herd_counter].milk);
  printf("Tot Feed:\t%f\n", arr_cow_costs[herd_counter].feed);
  printf("Tot Net Return:\t%f\n\n", arr_cow_costs[herd_counter].net_return);
}

/*
 * Calcultate THIS Lacatation Discount Milk and Feed.
 */
void calculate_TLDMF(int iter_num)
{
  extern struct user_inputs user;
  extern int mim_track_incrementer;

  double sum_milk = 0;
  double sum_feed = 0;
  int mim_tracker = 0;
  int indices[NUM_ROWS]; // May not require NUM_ROWS elements.
  int num_indices = 0;   // Actual number indices in indices[NUM_ROWS].
  int i, index;

  mim_tracker = user.month_after_calving + mim_track_incrementer;
  if (mim_tracker <= NUM_MIM)
  {
    num_indices = get_this_lactation_indices(indices, mim_tracker);
    for (i = 0; i < num_indices; i++)
    {
      index = indices[i];
      //-printf("i: %d\n", index);
      // A temporary sum for milk.
      sum_milk = sum_milk + (curr_iter[index] * arr_milk_pxion[index] * arr_milk_produced[index]);

      // A temporary sum for feed.
      sum_feed = sum_feed + (curr_iter[index] * arr_feed_intake[index]) * (arr_O[index] * user.feed_price_production + arr_P[index] * user.feed_price_dry);
    }
    mim_track_incrementer++;
  }

  this_lactation_discount_milk[iter_num] = sum_milk * user.milk_price * 30.4 * user.extra_rest_of_lactation;
  this_lactation_discount_feed[iter_num] = -sum_feed * 30.4 * user.extra_rest_of_lactation;
}

/*
 * Calcultate NEXT Lacatation Discount Milk and Feed.
 */
void calculate_NLDMF(int iter_num)
{
  extern struct user_inputs user;
  extern int counter_16, counter_17;

  int index_start = (user.lactation + 1 - 1) * 232;
  int index_end = index_start + 232;

  int present = 0;
  int i;
  for (i = index_start; i < index_end; i++)
  {
    if (curr_iter[i] != 0)
    {
      present = 1;
      break;
    }
  }

  double sum_milk = 0;
  double sum_feed = 0;
  int l;
  if (!present)
  {
    counter_17 = 0;
  }
  else
  {
    if (counter_17 == user.last_dim_to_breed + 2)
    {
      counter_17 = 1;
      counter_16++;
    }
    else
    {
      counter_17++;
      if (counter_17 == 1)
      {
        counter_16 = user.lactation + 1;
      }
    }
  }

  l = counter_16;
  //-printf("%d:\t\t%d\t\t%d\n", iter_num+1, counter_17, l);

  if (l != 0)
  {
    if (l <= 10)
    { // lactation has a max value of 10.
      index_start = (l - 1) * 232;
      index_end = NUM_ROWS;
      for (i = index_start; i < index_end; i++)
      {
        sum_milk = sum_milk + (curr_iter[i] * arr_milk_pxion[i] * arr_milk_produced[i]);
        sum_feed = sum_feed + (curr_iter[i] * arr_feed_intake[i] * arr_milk_produced[i]);
      }
    }
  }

  next_lactation_discount_milk[iter_num] = sum_milk * user.milk_price * 30.4 * user.extra_next_lactations;
  next_lactation_discount_feed[iter_num] = -sum_feed * user.feed_price_production * 30.4 * user.extra_next_lactations;
}

/*
 * Get indices for THIS lactation calculations.
 */
int get_this_lactation_indices(int indices[], int mim_tracker)
{
  extern struct user_inputs user;

  int l_start = 0, n_start = 0;
  int index = 0, i = 0;
  int n;

  l_start = (user.lactation - 1) * 232;
  if (mim_tracker < 26)
  {
    index = l_start + mim_tracker - 1;
    indices[i] = index;
    i++;
    index = index + 23;
    indices[i] = index;
    i++;
    index = index + 22;
    n_start = 2;
  }
  else
  {
    index = l_start + 25 + (23 * (mim_tracker - 25) + 22);
    n_start = mim_tracker - 24;
  }

  for (n = n_start; (n <= mim_tracker - 2) && (n <= 9); n++)
  {
    indices[i] = index;
    i++;
    index = index + 22;
  }

  return i;
}

/*
 * Sumproduct of two arrays.
 */
double sumproduct2(double arr1[], double arr2[], int offset, int length)
{
  if (offset < 0)
  {
    offset = NUM_ROWS + offset;
  }

  double sum = 0;
  int i;
  int end = offset + length;
  for (i = offset; i < end; i++)
  {
    sum = sum + (arr1[i] * arr2[i]);
  }

  return sum;
}

/*
 * Sumproduct of two arrays.
 */
double sumproduct3(double arr1[], double arr2[], double arr3[], int offset, int length)
{
  if (offset < 0)
  {
    offset = NUM_ROWS + offset;
  }

  double sum = 0;
  int i;
  int end = offset + length;
  for (i = offset; i < end; i++)
  {
    sum = sum + (arr1[i] * arr2[i] * arr3[i]);
  }

  return sum;
}

/*
 * Array sum
 */
double
array_sum(double arr[], int offset, int length)
{
  if (offset < 0)
  {
    offset = NUM_ROWS + offset;
  }

  int i;
  double sum = 0;
  int end = offset + length;
  for (i = offset; i < end; i++)
  {
    sum = sum + arr[i];
  }

  return sum;
}

/*
 * Print array
 */
void print_array(double arr[], int length)
{
  int i;
  for (i = 0; i < length; i++)
  {
    printf("%d\t%f\n", i + 34, arr[i]);
  }
}

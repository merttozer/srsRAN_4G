/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2021 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsran/phy/utils/random.h"
#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include "srsran/phy/dft/dft.h"
#include "srsran/phy/utils/vector.h"

uint32_t N       = 256;
bool     forward = true;
bool     mirror  = false;
bool     norm    = false;
bool     dc      = false;

void usage(char* prog)
{
  printf("Usage: %s\n", prog);
  printf("\t-N Transform size [Default 256]\n");
  printf("\t-b Backwards transform first [Default Forwards]\n");
  printf("\t-m Mirror the transform freq bins [Default false]\n");
  printf("\t-n Normalize the transform output [Default false]\n");
  printf("\t-d Handle insertion/removal of null DC carrier internally [Default false]\n");
}

void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "Nbmnd")) != -1) {
    switch (opt) {
      case 'N':
        N = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'b':
        forward = false;
        break;
      case 'm':
        mirror = true;
        break;
      case 'n':
        norm = true;
        break;
      case 'd':
        dc = true;
        break;
      default:
        usage(argv[0]);
        exit(-1);
    }
  }
}

void print(cf_t* in, int len)
{
  for (int i = 0; i < len; i++) {
    float re = crealf(in[i]);
    float im = cimagf(in[i]);
    printf("%f+%fi, ", re, im);
  }
  printf("\n\n");
}

int test_dft(cf_t* in)
{
  int res = 0;

  srsran_dft_plan_t plan;
  if (forward) {
    srsran_dft_plan(&plan, N, SRSRAN_DFT_FORWARD, SRSRAN_DFT_COMPLEX);
  } else {
    srsran_dft_plan(&plan, N, SRSRAN_DFT_BACKWARD, SRSRAN_DFT_COMPLEX);
  }
  srsran_dft_plan_set_mirror(&plan, mirror);
  srsran_dft_plan_set_norm(&plan, norm);
  srsran_dft_plan_set_dc(&plan, dc);

  cf_t* out1 = srsran_vec_cf_malloc(N);
  cf_t* out2 = srsran_vec_cf_malloc(N);
  srsran_vec_cf_zero(out1, N);
  srsran_vec_cf_zero(out2, N);

  print(in, N);
  srsran_dft_run(&plan, in, out1);
  print(out1, N);

  srsran_dft_plan_t plan_rev;
  if (!forward) {
    srsran_dft_plan(&plan_rev, N, SRSRAN_DFT_FORWARD, SRSRAN_DFT_COMPLEX);
  } else {
    srsran_dft_plan(&plan_rev, N, SRSRAN_DFT_BACKWARD, SRSRAN_DFT_COMPLEX);
  }
  srsran_dft_plan_set_mirror(&plan_rev, mirror);
  srsran_dft_plan_set_norm(&plan_rev, norm);
  srsran_dft_plan_set_dc(&plan_rev, dc);

  srsran_dft_run(&plan_rev, out1, out2);
  print(out2, N);

  if (!norm) {
    cf_t n = N + 0 * I;
    for (int i = 0; i < N; i++)
      out2[i] /= n;
  }

  for (int i = 0; i < N; i++) {
    float diff = cabsf(in[i] - out2[i]);
    if (diff > 0.01)
      res = -1;
  }

  srsran_dft_plan_free(&plan);
  srsran_dft_plan_free(&plan_rev);
  free(out1);
  free(out2);

  return res;
}

int main(int argc, char** argv)
{
  srsran_random_t random_gen = srsran_random_init(0x1234);
  parse_args(argc, argv);
  cf_t* in = srsran_vec_cf_malloc(N);
  in[0]    = 0.0f;
  srsran_random_uniform_complex_dist_vector(random_gen, &in[1], N - 1, -1, 1);

  if (test_dft(in) != 0)
    return -1;

  free(in);
  srsran_random_free(random_gen);
  printf("Done\n");
  exit(0);
}

#include "common.h"
#include "log.h"
#include "matrix_funs.h"
#include <stdint.h>
#include <stdlib.h>

int main(void) {
  double complex original[3][3] = {
      {4 + 35 * I, 12 - 68 * I, -16 + 29 * I},
      {12 - 68 * I, 37 + 0 * I, -43 + 57 * I},
      {-16 + 1 * I, -43 - 27 * I, 98 + 0 * I},
  };
  printj("1. original matrix is as follows:\n");
  print_complex_matrix(*original, 3, 3);

  double complex ooh_stk[3][3] = {0};
  double complex *ooh_stk_ptr = &ooh_stk[0][0];
  multiply_with_transpose_stk(*original, 3, 3, &ooh_stk_ptr);
  printj(
      "2. original matrix multiplied with its transpose is as follows (Matrix "
      "A):\n");
  print_complex_matrix(ooh_stk_ptr, 3, 3);

  double complex cholesky_stk[3][3] = {0};
  double complex *cholesky_stk_ptr = &cholesky_stk[0][0];
  cholesky_decomposition_stk(ooh_stk_ptr, 3, &cholesky_stk_ptr);
  printj("3. L Part of the cholesky_decomposition result of Matrix A is as "
         "follows:\n");
  print_complex_matrix(cholesky_stk_ptr, 3, 3);

  fill_upper_triangular_stk(&cholesky_stk_ptr, 3);
  printj("4. Matrix L filled upper triangular as Matrix LR: \n");
  print_complex_matrix(cholesky_stk_ptr, 3, 3);

  double complex intermediate_stk[3][3] = {0};
  double complex *intermediate_stk_ptr = &intermediate_stk[0][0];
  intermediate_diagnal_inverse_matrix_stk(cholesky_stk_ptr, 3,
                                          &intermediate_stk_ptr);
  printj(
      "5. intermediate_diagnal_inverse_matrix of Matrix LR is as follows:\n");
  print_complex_matrix(intermediate_stk_ptr, 3, 3);

  double complex back[3][3] = {0};
  double complex *back_ptr = &back[0][0];
  backward_stk(*original, cholesky_stk_ptr, intermediate_stk_ptr, 3, 3,
               &back_ptr);
  printj("6. backward is as follows: \n");
  print_complex_matrix(back_ptr, 3, 3);

  double complex res[3][3] = {0};
  double complex *res_ptr = &res[0][0];
  forward_stk(back_ptr, cholesky_stk_ptr, intermediate_stk_ptr, 3, 3, &res_ptr);
  printj("7. res = Matrix A's inversion(aka A^-1): \n");
  print_complex_matrix(res_ptr, 3, 3);

  double complex res2[3][3] = {0};
  double complex *res2_ptr = &res2[0][0];
  get_invers_of_mat_multi_trans_mat_stk(*original, &res2_ptr);
  print_complex_matrix(res2_ptr, 3, 3);

  FINISH(0);
  return 0;
}

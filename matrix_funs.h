#ifndef INCLUDE_GUARD_MATRIX_FUNC_H
#define INCLUDE_GUARD_MATRIX_FUNC_H

#include <complex.h>

void multiply_with_transpose_stk(double complex *mat, int row, int col,
                                 double complex **stk_out);

double complex *multiply_with_transpose(double complex *mat, int row, int col);

void cholesky_decomposition_stk(double complex *her_mat, int dim,
                                double complex **stk_out);
double complex *cholesky_decomposition(double complex *her_mat, int dim);

double complex *naive_square_transpose(double complex *mat, int dim);

double complex *advanced_square_transpose(double complex *mat, int dim);

void intermediate_diagnal_inverse_matrix_stk(double complex *mat, int dim,
                                             double complex **stk_out);

void backward_stk(double complex *b, double complex *r, double complex *s,
                  int row, int col, double complex **stk_out);

void forward_stk(double complex *c, double complex *r, double complex *s,
                 int row, int col, double complex **stk_out);
void print_complex_matrix(double complex *mat, int row, int col);

void fill_upper_triangular_stk(double complex **mat, int dim);
double complex *fill_upper_triangular(double complex *mat, int dim);

void get_invers_of_mat_multi_trans_mat_stk(double complex *mat,
                                           double complex **out);
#endif // !INCLUDE_GUARD_MATRIX_FUNC_H

#include "matrix_funs.h"
#include "bsp_print.h"
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAT_ODR
#define MAT_ODR (3)
#endif /* ifndef MAT_ODR */

static double sat_int(int a, int n) {
  int res = 0;
  if (a > pow(2, n - 1)) {
    res = pow(2, n - 1);
  } else if (a < -pow(2, n - 1)) {
    res = -pow(2, n - 1);
  } else {
    res = a;
  }
  return res;
}

static double complex comp_sat(double complex a, int n) {
  double c = 0, d = 0;
  if (creal(a) > pow(2, n - 1)) {
    c = pow(2, n - 1);
  } else if (creal(a) < -pow(2, n - 1)) {
    c = -pow(2, n - 1);
  } else {
    c = (int)creal(a);
  }
  if (cimag(a) > pow(2, n - 1)) {
    d = pow(2, n - 1);
  } else if (cimag(a) < -pow(2, n - 1)) {
    d = -pow(2, n - 1);
  } else {
    d = (int)cimag(a);
  }
  return c + d * I;
}

void multiply_with_transpose_stk(double complex *mat, int row, int col,
                                 double complex **stk_out) {
  double complex *res = *stk_out;
  memset(res, 0, row * col);
  if (res == NULL) {
    printj("cannot alloc memories! \n");
  }
  memset(res, 0, sizeof(double complex) * row * row);

  for (int i = 0; i < row; ++i) {
    for (int j = 0; j < i + 1; ++j) {
      double complex tmp_sum = 0.0 + 0.0 * I;
      for (int k = 0; k < col; ++k) {
        tmp_sum += mat[i * col + k] * conj(mat[j * col + k]);
      }
      res[i * col + j] = comp_sat(tmp_sum, 32);
      res[j * col + i] = res[i * col + j];
    }
  }
  return;
}

void cholesky_decomposition_stk(double complex *her_mat, int dim,
                                double complex **stk_out) {
  double complex *res = *stk_out;
  memset(res, 0, dim * dim);

  for (int i = 0; i < dim; ++i) {
    for (int j = 0; j < i + 1; ++j) {
      double complex tmp_sum = 0.0 + 0.0 * I;
      int tmp_sqrt_in = 0.0;
      for (int k = 0; k < j; ++k) {
        tmp_sum += res[i * dim + k] * conj(res[j * dim + k]);
      }
      if (i == j) {
        tmp_sqrt_in = (int)creal(her_mat[i * dim + i]) - (int)creal(tmp_sum);
        if (tmp_sqrt_in <= 0) {
          tmp_sqrt_in = 1;
        }
        res[i * dim + j] = sqrt(tmp_sqrt_in);
        res[i * dim + j] = sat_int((int)creal(res[i * dim + j]), 16);
      } else {
        res[i * dim + j] =
            (1 / res[j * dim + j] * (her_mat[i * dim + j] - tmp_sum));
        res[i * dim + j] = comp_sat(res[i * dim + j], 16);
      }
    }
  }
  return;
}

void fill_upper_triangular_stk(double complex **mat, int dim) {
  for (int i = 0; i < dim; ++i) {
    for (int j = 0; j < i + 1; ++j) {
      (*mat)[j * dim + i] = (*mat)[i * dim + j];
    }
  }
  return;
}

void intermediate_diagnal_inverse_matrix_stk(double complex *mat, int dim,
                                             double complex **stk_out) {
  double complex *res = *stk_out;
  memset(res, 0, sizeof(double complex) * dim);
  for (int i = 0; i < dim; ++i) {
    res[i * dim + i] = 1 / mat[i * dim + i] * 16;
  }
  return;
}

void backward_stk(double complex *b, double complex *r, double complex *s,
                  int row, int col, double complex **stk_out) {
  double complex *res = *stk_out;
  for (int i = 0; i < col; ++i) {
    for (int j = 0; j < row; ++j) {
      double complex tmp_sum = 0.0 + 0.0 * I;
      for (int k = 0; k < col; ++k) {
        tmp_sum += r[j * col + k] * res[k * col + i];
      }
      res[j * col + i] = (b[j * col + i] - tmp_sum) * s[j * col + j];
      res[j * col + i] = comp_sat(res[j * col + i], 16);
    }
  }
  return;
}

void forward_stk(double complex *c, double complex *r, double complex *s,
                 int row, int col, double complex **stk_out) {
  double complex *res = *stk_out;
  for (int i = col - 1; i >= 0; --i) {
    for (int j = row - 1; j >= 0; --j) {
      double complex tmp_sum = 0.0 + -0.0 * I;
      for (int k = j + 1; k < row; ++k) {
        tmp_sum += conj(r[k * row + j]) * res[k * row + i];
      }
      res[j * row + i] = (c[j * row + i] - tmp_sum) * s[j * row + j];
      res[j * row + i] = comp_sat(res[j * row + i], 16);
    }
  }
  return;
}

void get_invers_of_mat_multi_trans_mat_stk(double complex *mat,
                                           double complex **out) {

  double complex amah[MAT_ODR][MAT_ODR] = {0};
  double complex *p_amah = &amah[0][0];
  multiply_with_transpose_stk(mat, MAT_ODR, MAT_ODR, &p_amah);

  double complex chol[MAT_ODR][MAT_ODR] = {0};
  double complex *p_chol = &chol[0][0];
  cholesky_decomposition_stk(p_amah, MAT_ODR, &p_chol);
  fill_upper_triangular_stk(&p_chol, MAT_ODR);

  double complex inte[MAT_ODR][MAT_ODR] = {0};
  double complex *p_inte = &inte[0][0];
  intermediate_diagnal_inverse_matrix_stk(p_chol, MAT_ODR, &p_inte);

  double complex back[MAT_ODR][MAT_ODR] = {0};
  double complex *p_back = &back[0][0];
  backward_stk(mat, p_chol, p_inte, MAT_ODR, MAT_ODR, &p_back);

  forward_stk(p_back, p_chol, p_inte, MAT_ODR, MAT_ODR, out);
}

double complex *multiply_with_transpose_heap(double complex *mat, int row,
                                             int col) {
  double complex *res =
      (double complex *)malloc(sizeof(double complex) * row * col);
  memset(res, 0, sizeof(double complex) * row * row);

  for (int i = 0; i < row; ++i) {
    for (int j = 0; j < i + 1; ++j) {
      double complex tmp_sum = 0.0 + 0.0 * I;
      for (int k = 0; k < col; ++k) {
        tmp_sum += mat[i * col + k] * conj(mat[j * col + k]);
      }
      res[i * col + j] = comp_sat(tmp_sum, 32);
      res[j * col + i] = res[i * col + j];
    }
  }

  return res;
}

double complex *cholesky_decomposition(double complex *her_mat, int dim) {
  double complex *res =
      (double complex *)malloc(sizeof(double complex) * dim * dim);
  memset(res, 0, sizeof(double complex) * dim * dim);

  for (int i = 0; i < dim; ++i) {
    for (int j = 0; j < i + 1; ++j) {
      double complex tmp_sum = 0.0 + 0.0 * I;
      int tmp_sqrt_in = 0.0;
      for (int k = 0; k < j; ++k) {
        tmp_sum += res[i * dim + k] * conj(res[j * dim + k]);
      }
      if (i == j) {
        tmp_sqrt_in = (int)creal(her_mat[i * dim + i]) - (int)creal(tmp_sum);
        if (tmp_sqrt_in <= 0) {
          tmp_sqrt_in = 1;
        }
        res[i * dim + j] = sqrt(tmp_sqrt_in);
        res[i * dim + j] = sat_int((int)creal(res[i * dim + j]), 16);
      } else {
        res[i * dim + j] =
            (1 / res[j * dim + j] * (her_mat[i * dim + j] - tmp_sum));
        res[i * dim + j] = comp_sat(res[i * dim + j], 16);
      }
    }
  }
  return res;
}

double complex *fill_upper_triangular(double complex *mat, int dim) {
  for (int i = 0; i < dim; ++i) {
    for (int j = 0; j < i + 1; ++j) {
      mat[j * dim + i] = mat[i * dim + j];
    }
  }
  return mat;
}

double complex *naive_square_transpose(double complex *mat, int dim) {
  for (int i = 1; i < dim; ++i) {
    for (int j = 0; j < i; ++j) {
      double complex tmp = mat[i * dim + j];
      mat[i * dim + j] = mat[j * dim + i];
      mat[j * dim + i] = tmp;
    }
  }
  return mat;
}

#define DEVIDE_THRESHOLD 4
static void transpose_tile(int row, int col, int dim, double complex *mat) {
  if (dim > DEVIDE_THRESHOLD) {
    transpose_tile(row, col, dim / 2, mat);
    transpose_tile(row, col + dim / 2, dim / 2, mat);
    transpose_tile(row + dim / 2, col, dim / 2, mat);
    transpose_tile(row + dim / 2, col + dim / 2, dim / 2, mat);
  } else {
    for (int a = 0; a < dim; a++) {
      for (int b = 0; b < dim; b++) {
        int tmp = mat[(row + a) * dim + col + b];
        mat[(row + a) * dim + col + b] = mat[(col + b) * dim + row + a];
        mat[(col + b) * dim + row + a] = tmp;
      }
    }
  }
}

static void transpose_tile_diag(int pos, int dim, double complex *mat) {
  if (dim > DEVIDE_THRESHOLD) {
    transpose_tile_diag(pos, dim / 2, mat);
    transpose_tile(pos, pos + dim / 2, dim / 2, mat);
    transpose_tile_diag(pos + dim / 2, dim / 2, mat);
  } else {
    /* swap values on either side of the first diagonal */
    for (int a = 1; a < dim; a++) {
      /* stop the inner loop when b == a */
      for (int b = 0; b < a; b++) {
        double complex tmp = mat[(pos + a) * dim + pos + b];
        mat[(pos + a) * dim + pos + b] = mat[(pos + b) * dim + pos + a];
        mat[(pos + b) * dim + pos + a] = tmp;
      }
    }
  }
}

double complex *advanced_square_transpose(double complex *mat, int dim) {
  transpose_tile_diag(0, dim, mat);
  return mat;
}

static int len_integer_part(double in) {
  int count = 0, integer = (int)in;
  while (integer != 0) {
    integer /= 10;
    ++count;
  }
  return count;
}

void reverse(char *str, int len) {
  int start = 0;
  int end = len - 1;
  while (start < end) {
    char temp = *(str + start);
    *(str + start) = *(str + end);
    *(str + end) = temp;
    start++;
    end--;
  }
}

void int_to_str(int x, char *str_out, int *index) {
  int i = 0;
  do {
    str_out[i++] = (x % 10) + '0';
    x = x / 10;
  } while (x);
  reverse(str_out, i);
  *index = i;
}

void double_to_str(double num, int left_align, int align_width, char *str_out) {
  int int_part = (int)num;
  double dec_part = num - int_part;

  int index = 0;
  int_to_str(int_part, str_out, &index);

  if (dec_part > 0.0001) {
    str_out[index++] = '.';

    dec_part = dec_part * 1000;
    int dec_part_int = (int)dec_part;
    dec_part = dec_part - dec_part_int;
    if (dec_part > 0.5) {
      dec_part_int++;
    }

    int_to_str(dec_part_int, str_out + index, &index);
    index += index;
  }

  if (align_width > index) {
    int diff = align_width - index;

    if (left_align) {
      for (int i = 0; i < diff; i++)
        str_out[index++] = ' ';
    } else {
      memmove(str_out + diff, str_out, index);
      for (int i = 0; i < diff; i++)
        str_out[i] = ' ';
      index += diff;
    }
  }

  str_out[index] = '\0';
}

double complex *multiply_with_transpose(double complex *mat, int row, int col) {
  double complex *res =
      (double complex *)malloc(sizeof(double complex) * row * row);
  if (res == NULL) {
    printj("cannot alloc memories! \n");
  }
  memset(res, 0, sizeof(double complex) * row * row);

  for (int i = 0; i < row; ++i) {
    for (int j = 0; j < i + 1; ++j) {
      double complex tmp_sum = 0.0 + 0.0 * I;
      for (int k = 0; k < col; ++k) {
        tmp_sum += mat[i * col + k] * conj(mat[j * col + k]);
      }
      res[i * col + j] = comp_sat(tmp_sum, 32);
      res[j * col + i] = res[i * col + j];
    }
  }
  return res;
}
void print_complex_matrix(double complex *mat, int row, int col) {
  int rmax = 0, imax = 0;
  for (int i = 0; i < row; ++i) {
    for (int j = 0; j < col; ++j) {
      int rlen = len_integer_part(creal(mat[i * row + j]));
      int ilen = len_integer_part(creal(mat[i * row + j]));
      if (rmax < rlen)
        rmax = rlen;
      if (imax < ilen)
        imax = ilen;
    }
  }
  for (int i = 0; i < row; ++i) {
    printj("    ");
    for (int j = 0; j < col; ++j) {

      char *rsign = (creal(mat[i * row + j]) < 0) ? "-" : "+";
      char *isign = (cimag(mat[i * row + j]) < 0) ? "-" : "+";
      double imag = (cimag(mat[i * row + j]) < 0) ? -cimag(mat[i * row + j])
                                                  : cimag(mat[i * row + j]);
      double real = (creal(mat[i * row + j]) < 0) ? -creal(mat[i * row + j])
                                                  : creal(mat[i * row + j]);
      // printj("%s %0*.3f %s %0*.3f * j    ", rsign, rmax + 4, real, isign,
      //        imax + 4, imag);
      char real_str[10] = {'0'};
      char imag_str[10] = {'0'};
      double_to_str(real, 0, 9, real_str);
      double_to_str(imag, 0, 9, imag_str);
      printj("%s %s %s %s * j", rsign, real_str, isign, imag_str);
    }
    printj("\n");
  }
  printj("\n");
}

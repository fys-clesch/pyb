#ifndef __IGYBA_HEADER_HPP__
#define __IGYBA_HEADER_HPP__

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <sys/time.h>
#include <iomanip>
#include <sstream>

/* Operating system definition. */
#if defined(__unix__) || defined(__LINUX__)
 #define ISLINUX
 #define DIRMOD ,0775)
#elif defined(__WIN32__) || defined(_WIN32) || defined(__MSDOS__)
 #define DIRMOD )
 #if defined(_WIN64) || defined(__x86_64__)
  #define ISWIN64
 #else
  #define ISWIN32
 #endif
#endif

/* Useful macros. */
#define MAXOF(a, b) ((a) > (b) ? (a) : (b))
#define MINOF(a, b) ((a) < (b) ? (a) : (b))
#define POW2(x) ((x) != 0. ? (x) * (x) : 0.)
#define PRINT_TOKEN_INT(tok) iprint(stdout, #tok " is %i\n", tok)
#define PRINT_TOKEN_DOUBLE(tok) iprint(stdout, #tok " is %g\n", tok)
#define ERR_ARG __FILE__,__LINE__,__func__

/* Other global definitions. */
#define STOP_AT_ERR 1
#define USE_RESTRICT 1
#define SHOW_WAIT_KEY 0
#define SHOW_MOUSE_CB 0
#define USE_CURSES 0

constexpr unsigned int FILENAME_MAX1 = FILENAME_MAX + 1;

#if USE_RESTRICT
 #define res_pt __restrict
#else
 #define res_pt
#endif

#ifndef M_PI
 #define M_PI 3.1415926535897932
#endif

#ifndef M_PI2
 #define M_PI2 6.2831853071795865
#else
 #undef M_PI2
 #define M_PI2 6.2831853071795865
#endif

/* Project name. */
const std::string PROJECT_NAME = "pyb";
const std::string PROJECT_MAJ_VERSION = "2";
const std::string PROJECT_MIN_VERSION = "0";
const std::string PROJECT_YEAR = "2019";
const std::string PROJECT_MONTH = "08";
const std::string PROJECT_DAY = "21";

/* Type definitions. */
typedef unsigned int uint;
typedef unsigned long int ulong;
typedef unsigned char uchar;

/* auxf.cpp */
int iprint(FILE *stream, const char *format, ...);
void error_msg(const char *res_pt msg,
               const char *res_pt file,
               int line,
               const char *res_pt func);
void warn_msg(const char *res_pt msg,
              const char *res_pt file,
              int line,
              const char *res_pt func);
void file_error_msg(const char *res_pt fname,
                    const char *res_pt file,
                    int line,
                    const char *res_pt func);
double *alloc_vec(const uint n);
double *alloc_mat(const uint row, const uint col);
double *realloc_mat(double *m, const uint row, const uint col);
int32_t *alloc_mat_int32(const uint row, const uint col);
uint16_t *alloc_mat_uint16(const uint row, const uint col);
int16_t *alloc_mat_int16(const uint row, const uint col);
uint8_t *alloc_mat_uint8(const uint row, const uint col);
int8_t *alloc_mat_int8(const uint row, const uint col);
float *alloc_mat_float(const uint row, const uint col);
double *alloc_3mat(const uint row, const uint col);
double *alloc_3matrix(const uint row, const uint col, const double init);
double *realloc_3mat(double *m, const uint row, const uint col);
void sincosd(const double alpha,
             double *res_pt cosd,
             double *res_pt sind) __attribute__((optimize(0)));
void sincos_sqrt(double alpha,
                 double *res_pt cosd,
                 double *res_pt sind);
void find_minmax(const double *res_pt const m,
                 uint *res_pt pxmin,
                 uint *res_pt pymin,
                 uint *res_pt pxmax,
                 uint *res_pt pymax,
                 const uint row,
                 const uint col,
                 double *res_pt vmin,
                 double *res_pt vmax);
double find_max(const double *res_pt m,
                uint *res_pt px,
                uint *res_pt py,
                const uint row,
                const uint col);
double find_min(const double *res_pt m,
                uint *res_pt px,
                uint *res_pt py,
                const uint row,
                const uint col);
double *alloc_matrix(const uint row,
                     const uint col,
                     const double init);
long double *alloc_ldmatrix(const uint row,
                            const uint col,
                            const long double init);
double *alloc_vector(const uint row, const double init);
int *alloc_intvector(const uint row, const int init);
uint *alloc_uintvector(const uint row, const uint init);
int *alloc_intmatrix(const uint row, const uint col, const int init);
uint *alloc_uintmatrix(const uint row, const uint col, const uint init);
uchar *alloc_ucharmatrix(const uint row, const uint col, const uchar init);
uchar *realloc_ucharmatrix(uchar *m,
                           const uint row,
                           const uint col,
                           const uchar init);
void uintfile2double(const char *res_pt target,
                    double *res_pt m, const uint row, const uint col);
void uintfile2uint(const char *res_pt target,
                   uint *res_pt m,
                   const uint row,
                   const uint col);
void uintfile2uchar(const char *res_pt target,
                    uchar *res_pt m,
                    const uint row,
                    const uint col);
uint intfile2double(const char *res_pt target,
                    double *res_pt m,
                    const uint row,
                    const uint col);
uint fpfile2double(const char *res_pt target,
                   double *res_pt out,
                   const uint row,
                   const uint col,
                   const bool check_cmmnts = false);
void count_entries(FILE *res_pt file,
                   uint *res_pt nrow,
                   uint *res_pt ncol,
                   const bool check_cmmnts = false,
                   uint *res_pt cmt_lines = nullptr);
uchar isfile(const char *szfile);
void create_dir(const char *fname);
void fp_error(const char *res_pt file, const int line,
            const char *res_pt fname);
void get_DateAndTime(std::string &str,
                     const bool only_date = false,
                     const bool w_msecs = false);
const std::string convert_Int2Str(const int i);
const std::string convert_Double2Str(const double x, const uint32_t prec = 2);
double map_Linear(const double x,
                  const double in_min,
                  const double in_max,
                  const double out_min,
                  const double out_max);
void transposeflip_Matrix(double *res_pt const dst,
                          const double *res_pt const src,
                          const uint nrows,
                          const uint ncols);

#endif

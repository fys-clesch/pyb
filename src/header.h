#ifndef HEADER_H
#define HEADER_H

/* operating system definition */
#if defined(__unix__) || defined(__LINUX__)
 #define ISLINUX
 #define DIRMOD ,0775)
#elif defined(__WIN32__) || defined(_WIN32) || defined(__MSDOS__)
 #define DIRMOD )
 #ifdef _WIN64
  #define ISWIN64
 #else
  #define ISWIN32
 #endif
#endif

/* useful macros */
#define MAXOF(a,b) ((a)>(b)?(a):(b))
#define MINOF(a,b) ((a)<(b)?(a):(b))
#define POW2(x) ((x)!=0.?(x)*(x):0.)
#define PRINT_TOKEN_INT(tok) fprintf(stdout, #tok " is %i\n", tok)
#define PRINT_TOKEN_DOUBLE(tok) fprintf(stdout, #tok " is %g\n", tok)
#define ERR_ARG __FILE__,__LINE__,__func__

/* other global definitions */
#define FILENAME_MAX1 (uint)(FILENAME_MAX+1)
#define STOP_AT_ERR 1
#define USE_RESTRICT 1
#define SHOW_WAIT_KEY 0
#define SHOW_MOUSE_CB 0
#define USE_CURSES 0
#define STRLEN_PARAMETER 30u

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

/* project name */
static const char PROJECT_NAME[] = "igyba";

/* type definitions */
typedef unsigned int uint;
typedef unsigned long int ulong;
typedef unsigned char uchar;

/* structures */
struct parameter_struct
{
char name[STRLEN_PARAMETER],
     unit[STRLEN_PARAMETER];
double init, /**< initial value */
       min,
       max,
       val; /**< recent value */
uchar log, /**< 0 = linear, 1 = log */
      fit; /**< 1 = fit this */
};
typedef struct parameter_struct parameter;

struct data2view_struct
{
double *res_pt rgb, /**< colour at a given point */
       *res_pt data, /**< data stored as x, data, z */
       *res_pt cb, /**< colour box colouring stripes */
       *res_pt rgbcb, /**< colour box colour at each stripe */
       min_norm, /**< minimum of data with -1 <= min_norm <= 0 */
       min_val, /**< minimum of data */
       max_norm, /**< maximum of data with 0 < max_norm <= 1 */
       /* if the absolute maximum value is positive, max_norm = 1., else min_norm = -1. */
       max_val, /**< maximum of data */
       wc[3], /**< world coordinates at the a mouse position */
       rot_y, rot_x,
       mov_y, mov_x, mov_lvl,
       zoom;
const double zoom_std,
             rot_y_std,
             rot_x_std;
uint nstrps, /**< number of stripes for the colour box */
     row, col; /**< number of the datapoints in each direction */
const uint std_win_x, std_win_y;
uchar allocd,
      filled,
      map_mode,
      update_animate;
};
typedef struct data2view_struct data2view;

struct data2minime_struct
{
double lambda,
       scl; /**< scaling factor pixel -> millimetre */
double *res_pt data,
       lin_reg_par[7]; /**< holds data of the linear regression fit:
       * 0. entry is the ssq
       * 1. entry is offset, a
       * 2. entry is multiplier, b
       * 3. entry is sigma of 1., siga
       * 4. entry is sigma of 2., sigb
       * 5. entry is sigma the dataset that is fitted, sigdat
       * 6. is the correlation between 1. and 2., r = covar / (siga * sigb)
       */
uchar *res_pt bad,
      allocd,
      filled,
      do_gnuplot;
uint row,
     col,
     ntot;
parameter fit_par[5];
};
typedef struct data2minime_struct data2minime;

/* auxf.c */
void error_msg(const char *res_pt msg, const char *res_pt file, int line, const char *res_pt func);
void warn_msg(const char *res_pt msg, const char *res_pt file, int line, const char *res_pt func);
void file_error_msg(const char *res_pt fname, const char *res_pt file, int line, const char *res_pt func);
double *alloc_vec(const uint n);
double *alloc_mat(const uint row, const uint col);
float *alloc_mat_sp(const uint row, const uint col);
double *alloc_3mat(const uint row, const uint col);
void sincosd(const double alpha,
			double *res_pt cosd, double *res_pt sind) __attribute__((optimize(0)));
void sincos_sqrt(double alpha,
				double *res_pt cosd, double *res_pt sind) __attribute__((optimize(1)));
void find_minmax(const double *res_pt m,
				uint *res_pt pxmin, uint *res_pt pymin,
				uint *res_pt pxmax, uint *res_pt pymax,
				const uint row, const uint col,
				double *res_pt vmin, double *res_pt vmax);
double find_max(const double *res_pt m, uint *res_pt px, uint *res_pt py, const uint row, const uint col);
double find_min(const double *res_pt m, uint *res_pt px, uint *res_pt py, const uint row, const uint col);
double *alloc_matrix(const uint row, const uint col, const double init);
long double *alloc_ldmatrix(const uint row, const uint col, const long double init);
double *alloc_vector(const uint row, const double init);
int *alloc_intvector(const uint row, const int init);
uint *alloc_uintvector(const uint row, const uint init);
int *alloc_intmatrix(const uint row, const uint col, const int init);
uint *alloc_uintmatrix(const uint row, const uint col, const uint init);
uchar *alloc_ucharmatrix(const uint row, const uint col, const uchar init);
void uintfile2double(const char *res_pt target, double *res_pt m, const uint row, const uint col);
void uintfile2uint(const char *res_pt target, uint *res_pt m, const uint row, const uint col);
void uintfile2uchar(const char *res_pt target, uchar *res_pt m, const uint row, const uint col);
uint intfile2double(const char *res_pt target, double *res_pt m, const uint row, const uint col);
uint fpfile2double(const char *res_pt target, double *res_pt m, const uint row, const uint col);
void count_entries(FILE *res_pt file, uint *res_pt nrow, uint *res_pt ncol);
uchar isfile(const char *szfile);
void create_dir(const char *fname);
void fp_error(const char *res_pt file, const int line, const char *res_pt fname);

#endif

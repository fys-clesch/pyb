#include <math.h>
#include <float.h>
#include <assert.h>
#include <fenv.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "header.hpp"

#if defined(ISLINUX)
 #ifndef __USE_BSD /**< for S_IFDIR */
  #define __USE_BSD
 #endif
#elif defined(ISWIN32) || defined(ISWIN64)
#endif

int iprint(FILE *stream, const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	int done = vfprintf(stream, format, arg);
	va_end(arg);
	return done;
}

void error_msg(const char *res_pt msg, const char *res_pt file, int line, const char *res_pt func)
{
	iprint(stderr,
			"\adire straits in file %s line %i from '%s':\n%s\n",
			file, line, func, msg);
	perror("interpreted as");
	errno = 0;
	#if STOP_AT_ERR
	iprint(stderr, "\npress enter to continue: ");
	fflush(stderr);
	getchar();
	#endif
}

void warn_msg(const char *res_pt msg, const char *res_pt file, int line, const char *res_pt func)
{
	iprint(stderr,
			"\acaught something in file %s line %i from '%s':\n%s\n",
			file, line, func, msg);
}

void file_error_msg(const char *res_pt fname, const char *res_pt file, int line, const char *res_pt func)
{
	iprint(stderr,
			"\afile error in %s line %i from '%s':\ncan't read or write '%s'\n",
			file, line, func, fname);
	perror("interpreted as");
	errno = 0;
	#if STOP_AT_ERR
	iprint(stderr, "\npress enter to continue: ");
	fflush(stderr);
	getchar();
	#endif
}

double *alloc_vec(const uint n)
{
	double *m = (double *)malloc(n * sizeof(double));
	if(NULL == m)
	{
		error_msg("memory allocation failed", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	return m;
}

double *alloc_mat(const uint row, const uint col)
{
	double *m = (double *)malloc(row * col * sizeof(double));
	if(NULL == m)
	{
		error_msg("memory allocation failed", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	return m;
}

double *realloc_mat(double *m, const uint row, const uint col)
{
	double *old = m;
	m = (double *)realloc(m, row * col * sizeof(double));
	if(NULL == m)
	{
		error_msg("memory reallocation failed", ERR_ARG);
		m = old;
	}
	return m;
}

int32_t *alloc_mat_int32(const uint row, const uint col)
{
	int32_t *m = (int32_t *)malloc(row * col * sizeof(int32_t));
	if(NULL == m)
	{
		error_msg("memory allocation failed", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	return m;
}

uint16_t *alloc_mat_uint16(const uint row, const uint col)
{
	uint16_t *m = (uint16_t *)malloc(row * col * sizeof(uint16_t));
	if(NULL == m)
	{
		error_msg("memory allocation failed", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	return m;
}

int16_t *alloc_mat_int16(const uint row, const uint col)
{
	int16_t *m = (int16_t *)malloc(row * col * sizeof(int16_t));
	if(NULL == m)
	{
		error_msg("memory allocation failed", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	return m;
}

uint8_t *alloc_mat_uint8(const uint row, const uint col)
{
	uint8_t *m = (uint8_t *)malloc(row * col * sizeof(uint8_t));
	if(NULL == m)
	{
		error_msg("memory allocation failed", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	return m;
}

int8_t *alloc_mat_int8(const uint row, const uint col)
{
	int8_t *m = (int8_t *)malloc(row * col * sizeof(int8_t));
	if(NULL == m)
	{
		error_msg("memory allocation failed", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	return m;
}

float *alloc_mat_float(const uint row, const uint col)
{
	float *m = (float *)malloc(row * col * sizeof(float));
	if(NULL == m)
	{
		error_msg("memory allocation failed", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	return m;
}

double *alloc_3mat(const uint row, const uint col)
{
	double *m = (double *) malloc(3 * row * col * sizeof(double));
	if(NULL == m)
	{
		error_msg("memory allocation failed", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	return m;
}

double *alloc_3matrix(const uint row, const uint col, const double init)
{
	double *m = (double *) malloc(3 * row * col * sizeof(double));
	if(NULL == m)
	{
		error_msg("memory allocation failed", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(uint i = 0; i < row * col; ++i)
		m[i] = init;
	return m;
}

double *realloc_3mat(double *m, const uint row, const uint col)
{
	double *old = m;
	m = (double *)realloc(m, 3 * row * col * sizeof(double));
	if(NULL == m)
	{
		error_msg("memory reallocation failed", ERR_ARG);
		m = old;
	}
	return m;
}

/** \brief Compute the sine and cosine of an angle alpha in one step.
 *
 * \param alpha const double The angle in radian.
 * \param cosd double *res_pt The cosine of alpha.
 * \param sind double *res_pt The sine of alpha.
 * \return void
 *
 * If available, uses assembler code to compute the sine and cosine in one step.
 */
void sincosd(const double alpha, double *res_pt cosd, double *res_pt sind)
{
	#if defined(__i386__) && !defined(NO_ASM)
	 #if defined __GNUC__
	  #define ASM_SINCOS
	  __asm__ __volatile__ ("fsincos" : "=t" (*cosd), "=u" (*sind) : "0" (alpha));
	 #elif defined _MSC_VER
	  #define ASM_SINCOS
	  #warning "assembler functionality not yet tested"
	  __asm fld alpha
	  __asm fsincos
	  __asm fstp *cosd
	  __asm fstp *sind
	 #endif
	#endif
	#ifndef ASM_SINCOS /**< fall-back */
	 #warning "no assembler sincos function available"
	 sincos_sqrt(alpha, cosd, sind);
	#endif
}

/** \brief Fall-back function for sincosd.
 *
 * \param alpha double The angle in radians.
 * \param cosd double *res_pt The cosine of alpha.
 * \param sind double *res_pt The sine of alpha.
 * \return void
 *
 * Safes about 25 percent compared to a standard call of cos and sin.
 */
void sincos_sqrt(double alpha, double *res_pt cosd, double *res_pt sind)
{
	*cosd = cos(alpha);
	const double t1 = sqrt(1. - (*cosd) * (*cosd));
	alpha = fabs(alpha);
	if(alpha < M_PI)
		*sind = t1;
	else if(alpha < M_PI2)
		*sind = -t1;
	else
	{
		alpha = fmod(alpha, M_PI2);
		if(alpha < M_PI)
			*sind = t1;
		else
			*sind = -t1;
	}
}

void find_minmax(const double *res_pt const m,
				uint *res_pt pxmin, uint *res_pt pymin,
				uint *res_pt pxmax, uint *res_pt pymax,
				const uint row, const uint col,
				double *res_pt vmin, double *res_pt vmax)
{
	double tmax = -DBL_MAX, tmin = DBL_MAX;
	if(pxmin != nullptr)
		for(uint i = 0; i < row; ++i)
			for(uint j = 0, icol = i * col; j < col; ++j)
			{
				if(m[icol + j] < tmin)
				{
					tmin = m[icol + j];
					*pxmin = i;
					*pymin = j;
				}
				if(m[icol + j] > tmax)
				{
					tmax = m[icol + j];
					*pxmax = i;
					*pymax = j;
				}
			}
	else
		for(uint i = 0; i < row * col; ++i)
		{
			if(m[i] < tmin)
				tmin = m[i];
			if(m[i] > tmax)
				tmax = m[i];
		}
	*vmax = tmax;
	*vmin = tmin;
}

double find_max(const double *res_pt m, uint *res_pt px, uint *res_pt py, const uint row, const uint col)
{
	double temp = -DBL_MAX;
	for(uint i = 0; i < row; ++i)
		for(uint j = 0; j < col; ++j)
			if(m[i * col + j] > temp)
			{
				temp = m[i * col + j];
				*px = i;
				*py = j;
			}

	return temp;
}

double find_min(const double *res_pt m, uint *res_pt px, uint *res_pt py, const uint row, const uint col)
{
	double temp = DBL_MAX;
	for(uint i = 0; i < row; ++i)
		for(uint j = 0; j < col; ++j)
			if(m[i * col + j] < temp)
			{
				temp = m[i * col + j];
				*px = i;
				*py = j;
			}

	return temp;
}

double *alloc_matrix(const uint row, const uint col, const double init)
{
	double *m = (double *)malloc((row * col) * sizeof(double));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(uint i = 0; i < (row * col); ++i)
		m[i] = init;
	return m;
}

long double *alloc_ldmatrix(const uint row, const uint col, const long double init)
{
	long double *m = (long double *)malloc((row * col) * sizeof(long double));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(uint i = 0; i < (row * col); ++i)
		m[i] = init;
	return m;
}

double *alloc_vector(const uint row, const double init)
{
	double *m = (double *)malloc(row * sizeof(double));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(uint i = 0; i < row; ++i)
		m[i] = init;
	return m;
}

int *alloc_intvector(const uint row, const int init)
{
	int *m = (int *)malloc(row * sizeof(int));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(uint i = 0; i < row; ++i)
		m[i] = init;
	return m;
}

uint *alloc_uintvector(const uint row, const uint init)
{
	uint *m = (uint *)malloc(row * sizeof(uint));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(uint i = 0; i < row; ++i)
		m[i] = init;
	return m;
}

int *alloc_intmatrix(const uint row, const uint col, const int init)
{
	int *m = (int *)malloc(row * col * sizeof(int));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(uint i = 0; i < row * col; ++i)
		m[i] = init;
	return m;
}

uint *alloc_uintmatrix(const uint row, const uint col, const uint init)
{
	uint *m = (uint *)malloc(row * col * sizeof(uint));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(uint i = 0; i < row * col; ++i)
		m[i] = init;
	return m;
}

uchar *alloc_ucharmatrix(const uint row, const uint col, const uchar init)
{
	uchar *m = (uchar *)malloc(row * col * sizeof(uchar));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(uint i = 0; i < row * col; ++i)
		m[i] = init;
	return m;
}

uchar *realloc_ucharmatrix(uchar *m, const uint row, const uint col, const uchar init)
{
	uchar *old = m;
	m = (uchar *)realloc(m, row * col * sizeof(uchar));
	if(NULL == m)
	{
		error_msg("memory reallocation failed. good bye.", ERR_ARG);
		m = old;
	}
	else
		for(uint i = 0; i < row * col; ++i)
			m[i] = init;
	return m;
}

void uintfile2double(const char *res_pt target, double *res_pt m, const uint row, const uint col)
{
	FILE *readfile = fopen(target, "r");
	if(readfile == NULL)
	{
		file_error_msg(target, ERR_ARG);
		exit(EXIT_FAILURE);
	}

	uint x = 0, y = 0, temp, i = 0;
	while(fscanf(readfile, "%9u", &temp) != EOF)
	{
		m[x * col + y] = (double)temp;
		if(y < (col - 1))
			y++;
		else
		{
			y = 0;
			x++;
		}
		i++;
	}
	assert(x == row && i == row * col);
	fclose(readfile);
	iprint(stdout, "copied %s to memory - %u data points\n", target, i);
}

void uintfile2uint(const char *res_pt target, uint *res_pt m, const uint row, const uint col)
{
	FILE *readfile = fopen(target, "r");
	if(readfile == NULL)
	{
		file_error_msg(target, ERR_ARG);
		exit(EXIT_FAILURE);
	}
	uint x = 0, y = 0, temp, i = 0;
	while(fscanf(readfile, "%9u", &temp) != EOF)
	{
		m[x * col + y] = temp;
		if(y < (col - 1))
			y++;
		else
		{
			y = 0;
			x++;
		}
		i++;
	}
	assert(x == row && i == row * col);
	fclose(readfile);
	iprint(stdout, "copied %s to memory - %u data points\n", target, i);
}

void uintfile2uchar(const char *res_pt target, uchar *res_pt m, const uint row, const uint col)
{
	FILE *readfile = fopen(target, "r");
	if(readfile == NULL)
	{
		file_error_msg(target, ERR_ARG);
		exit(EXIT_FAILURE);
	}
	uint x = 0, y = 0, temp, i = 0;
	while(fscanf(readfile, "%9u", &temp) != EOF)
	{
		m[x * col + y] = (uchar)temp;
		if(y < (col - 1))
			y++;
		else
		{
			y = 0;
			x++;
		}
		i++;
	}
	assert(x == row && i == row * col);
	fclose(readfile);
	iprint(stdout, "copied %s to memory - %u data points\n", target, i);
}

uint intfile2double(const char *res_pt target, double *res_pt m, const uint row, const uint col)
{
	FILE *readfile = fopen(target, "r");
	if(readfile == NULL)
	{
		file_error_msg(target, ERR_ARG);
		exit(EXIT_FAILURE);
	}
	uint i = 0, j = 0, s = 0;
	int temp;
	while(fscanf(readfile, "%10i", &temp) != EOF)
	{
		m[i * col + j] = (double)temp;
		if(j < (col - 1))
			j++;
		else
		{
			j = 0;
			i++;
		}
		s++;
	}
	fclose(readfile);
	assert(i == row && s == row * col);
	return s;
}

uint fpfile2double(const char *res_pt target, double *res_pt m, const uint row, const uint col)
{
	FILE *readfile = fopen(target, "r");
	if(readfile == NULL)
	{
		file_error_msg(target, ERR_ARG);
		exit(EXIT_FAILURE);
	}
	uint i = 0, j = 0, s = 0;
	double temp;
	while(fscanf(readfile, "%18lg", &temp) != EOF)
	{
		m[i * col + j] = temp;
		if(j < (col - 1))
			j++;
		else
		{
			j = 0;
			i++;
		}
		s++;
	}
	fclose(readfile);
	assert(i == row && s == row * col);
	return s;
}

/** \brief Counts numeric entries in a file.
 *
 * \param file FILE *res_pt The pointer to the file.
 * \param nrow uint *res_pt The number of rows of the file.
 * \param ncol uint *res_pt The number of columns.
 * \return void
 *
 * The file has to resemble a matrix. No additional text entries are allowed.
 * The file is checked for consistence during evaluation. The pointer to the current
 * position in the file will be reset to the initial position.
 */
void count_entries(FILE *res_pt file, uint *res_pt nrow, uint *res_pt ncol)
{
	uint i = 0, sum = 0;
	*ncol = 0;
	*nrow = 0; /**< Use this as an error indicator. */
	char tc, lbreak = 0, wspace = 0;
	rewind(file);
	while(fscanf(file, "%c", &tc) != EOF)
		if(tc == '\n')
		{
			if(lbreak)
				break; /**< This happens in case of an additional line break at the end of the file. */
			i++; /**< The counter. */
			if(!sum)
				*ncol = sum = i; /**< First line break. */
			else if((sum += *ncol) != i)
			{
				iprint(stderr, "lines are not of constant length at pos: %u\n", i);
				error_msg("reading error:", ERR_ARG);
				return;
			}
			lbreak = 1;
		}
		else if(tc == '0' || tc == '1' || tc == '2' || tc == '3' || tc == '4' ||
				tc == '5' || tc == '6' || tc == '7' || tc == '8' || tc == '9' ||
				tc == '.' || tc == '-' || tc == 'e' || tc == 'E')
			wspace = lbreak = 0; /**< Got an entry. */
		else if(tc == ' ' || tc == '\t')
		{
			if(wspace)
			{
				iprint(stderr, "too much whitespace between numbers at pos: %u\n", i);
				error_msg("reading error:", ERR_ARG);
				return;
			}
			i++; /**< The counter. */
			wspace = 1;
		}
		else if(tc == 13)
			continue; /**< Carriage return on Linux. */
		else
		{
			iprint(stderr, "last read character: '%c'\nASCII: %i at pos %u\n", tc, tc, i);
			error_msg("reading error:", ERR_ARG);
			return;
		}

	if(!lbreak)
		sum += *ncol; /**< Adds the last column to the total number in case of no final line break. */
	rewind(file);
	*nrow = sum / (*ncol);
}

uchar isfile(const char *szfile)
{
	struct stat statBuffer;
	return (stat(szfile, &statBuffer) >= 0 && /**< Make sure it exists... */
			!(statBuffer.st_mode & S_IFDIR)); /**< ...and it's not a directory. */
}

/** \brief Creates a directory to store the file if it doesn't exist.
 *
 * \param fname const char* Name of the directory.
 * \return void
 *
 */
void create_dir(const char *fname)
{
	uint i;
	if((i = strcspn(fname, "/")) < strlen(fname))
	{
		uint k = i++;
		for(; i < strlen(fname); i++)
			if(fname[i] == '/')
				k = i;

		char dirname[FILENAME_MAX1];
		snprintf(dirname, ++k, "%s", fname);
		mkdir(dirname DIRMOD;
	}
}

/** \brief Prints floating point exceptions.
 *
 * \param file const char* The name of the file which called this function.
 * \param line int The line where this function was called.
 * \param fname const char* The name of the function which called this function.
 * \return void
 *
 */
void fp_error(const char *res_pt file, const int line, const char *res_pt fname)
{
	int set_excepts = fetestexcept(FE_ALL_EXCEPT & (!FE_INVALID));
	if(set_excepts & FE_DIVBYZERO)
		error_msg("there was a division by zero", file, line, fname);
	if(set_excepts & FE_INEXACT)
		error_msg("sometimes double is not enough", file, line, fname);
	if(set_excepts & FE_INVALID)
		error_msg("there was an invalid operation", file, line, fname);
	if(set_excepts & FE_OVERFLOW)
		error_msg("overflow fp error. whatever that means.", file, line, fname);
	if(set_excepts & FE_UNDERFLOW)
		error_msg("underflow fp error. whatever that means.", file, line, fname);

	feclearexcept(FE_ALL_EXCEPT);
}

void get_DateAndTime(std::string &str, const bool only_date, const bool w_msecs)
{
		time_t rawtime;
		time(&rawtime);
		struct tm *timeinfo = localtime(&rawtime);
		char buf[128];
		if(only_date)
			strftime(buf, 128, "%Y%m%d", timeinfo);
		else
			strftime(buf, 128, "%Y%m%d_%H-%M-%S", timeinfo);
		str = buf;

		if(w_msecs)
		{
			char milli[8];
			timeval curTime;
			gettimeofday(&curTime, NULL);
			snprintf(milli, 8, "%03li", curTime.tv_usec / 1000);
			str = str + "-" + milli;
		}
}

const std::string convert_Int2Str(const int i)
{
	std::stringstream ss;
	ss << i;
	return ss.str();
}

const std::string convert_Double2Str(const double x, const uint32_t prec)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(prec) << x;
	return ss.str();
}

double map_Linear(const double x, const double in_min, const double in_max,
				const double out_min, const double out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void transposeflip_Matrix(double *res_pt const dst,
						const double *res_pt const src,
						const uint nrows, const uint ncols)
{
	for(uint i = 0; i < nrows; ++i)
	{
		const uint nri = (nrows - 1) - i,
		           inc = i * ncols;
		for(uint j = 0; j < ncols; ++j)
			dst[j * nrows + nri] = src[inc + j];
	}
}

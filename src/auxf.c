#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include <assert.h>
#include <fenv.h>

#include "header.h"

#if defined(ISLINUX)
 #ifndef __USE_BSD /**< for S_IFDIR */
  #define __USE_BSD
 #endif
#elif defined(ISWIN32) || defined(ISWIN64)
 #include <dir.h>
 #include <Windows.h>
#endif

#include <sys/stat.h>

void error_msg(const char *res_pt msg, const char *res_pt file, int line, const char *res_pt func)
{
	fprintf(stderr,
			"\adire straits in file %s line %i from '%s':\n%s\n",
			file, line, func, msg);
	perror("interpreted as");
	errno = 0;
	#if STOP_AT_ERR
	fprintf(stderr, "\npress enter to continue: ");
	fflush(stderr);
	getchar();
	#endif
}

void warn_msg(const char *res_pt msg, const char *res_pt file, int line, const char *res_pt func)
{
	fprintf(stderr,
			"\acaught something in file %s line %i from '%s':\n%s\n",
			file, line, func, msg);
}

void file_error_msg(const char *res_pt fname, const char *res_pt file, int line, const char *res_pt func)
{
	fprintf(stderr,
			"\afile error in %s line %i from '%s':\ncan't read or write '%s'\n",
			file, line, func, fname);
	perror("interpreted as");
	errno = 0;
	#if STOP_AT_ERR
	fprintf(stderr, "\npress enter to continue: ");
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
	double *m = (double *)malloc((row * col) * sizeof(double));
	if(NULL == m)
	{
		error_msg("memory allocation failed", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	return m;
}

float *alloc_mat_sp(const uint row, const uint col)
{
	float *m = (float *)malloc((row * col) * sizeof(float));
	if(NULL == m)
	{
		error_msg("memory allocation failed", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	return m;
}

double *alloc_3mat(const uint row, const uint col)
{
	double *m = (double *) malloc((3 * row * col) * sizeof(double));
	if(NULL == m)
	{
		error_msg("memory allocation failed", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	return m;
}

/** \brief compute the sine and cosine of an angle alpha in one step
 *
 * \param alpha const double the angle in radian
 * \param cosd double *res_pt the cosine of alpha
 * \param sind double *res_pt the sine of alpha
 * \return void
 *
 * if available, uses assembler code to compute the sine and cosine in one step
 */
void sincosd(const double alpha, double *res_pt cosd, double *res_pt sind)
{
	#if defined(__i386__)&&!defined(NO_ASM)
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

/** \brief fall-back function for sincosd
 *
 * \param alpha double the angle in radians
 * \param cosd double *res_pt the cosine of alpha
 * \param sind double *res_pt the sine of alpha
 * \return void
 *
 * safes about 25 percent compared to a standard call of cos and sin
 */
void sincos_sqrt(double alpha, double *res_pt cosd, double *res_pt sind)
{
	*cosd = cos(alpha);
	double t1 = sqrt(1. - (*cosd) * (*cosd));
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

void find_minmax(const double *res_pt m,
				uint *res_pt pxmin, uint *res_pt pymin,
				uint *res_pt pxmax, uint *res_pt pymax,
				const uint row, const uint col,
				double *res_pt vmin, double *res_pt vmax)
{
	double tmax = -DBL_MAX, tmin = DBL_MIN;
	uint i, j;
	if(pxmin != NULL)
		for(i = 0; i < row; i++)
			for(j = 0; j < col; j++)
			{
				if(m[i * col + j] < tmin)
				{
					tmin = m[i * col + j];
					*pxmin = i;
					*pymin = j;
				}
				if(m[i * col + j] > tmax)
				{
					tmax = m[i * col + j];
					*pxmax = i;
					*pymax = j;
				}
			}
	else
		for(i = 0; i < row; i++)
			for(j = 0; j < col; j++)
			{
				if(m[i * col + j] < tmin)
					tmin = m[i * col + j];
				if(m[i * col + j] > tmax)
					tmax = m[i * col + j];
			}
	*vmax = tmax;
	*vmin = tmin;
}

double find_max(const double *res_pt m, uint *res_pt px, uint *res_pt py, const uint row, const uint col)
{
	double temp = -DBL_MAX;
	uint i, j;
	for(i = 0; i < row; i++)
		for(j = 0; j < col; j++)
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
	uint i, j;
	for(i = 0; i < row; i++)
		for(j = 0; j < col; j++)
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
	uint i;
	double *m = (double *) malloc((row * col) * sizeof(double));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < (row * col); i++)
		m[i] = init;
	return m;
}

long double *alloc_ldmatrix(const uint row, const uint col, const long double init)
{
	uint i;
	long double *m = (long double *) malloc((row * col) * sizeof(long double));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < (row * col); i++)
		m[i] = init;
	return m;
}

double *alloc_vector(const uint row, const double init)
{
	uint i;
	double *m = (double *) malloc(row * sizeof(double));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < row; i++)
		m[i] = init;
	return m;
}

int *alloc_intvector(const uint row, const int init)
{
	uint i;
	int *m = (int *) malloc(row * sizeof(int));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < row; i++)
		m[i] = init;
	return m;
}

uint *alloc_uintvector(const uint row, const uint init)
{
	uint i;
	uint *m = (uint *) malloc(row * sizeof(uint));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < row; i++)
		m[i] = init;
	return m;
}

int *alloc_intmatrix(const uint row, const uint col, const int init)
{
	uint i;
	int *m = (int *) malloc((row * col) * sizeof(int));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < (row * col); i++)
		m[i] = init;
	return m;
}

uint *alloc_uintmatrix(const uint row, const uint col, const uint init)
{
	uint i;
	uint *m = (uint *) malloc((row * col) * sizeof(uint));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < (row * col); i++)
		m[i] = init;
	return m;
}

uchar *alloc_ucharmatrix(const uint row, const uint col, const uchar init)
{
	uint i;
	uchar *m = (uchar *) malloc((row * col) * sizeof(uchar));
	if(NULL == m)
	{
		error_msg("memory allocation failed. good bye.", ERR_ARG);
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < (row * col); i++)
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
	fprintf(stdout, "copied %s to memory - %u data points\n", target, i);
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
	fprintf(stdout, "copied %s to memory - %u data points\n", target, i);
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
	fprintf(stdout, "copied %s to memory - %u data points\n", target, i);
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

/** \brief counts numeric entries in a file
 *
 * \param file FILE *res_pt the pointer to the file
 * \param nrow uint *res_pt the number of rows of the file
 * \param ncol uint *res_pt the number of columns
 * \return void
 *
 * the file has to resemble a matrix. no additional text entries are allowed.
 * the file is checked for consistence during evaluation. the pointer to the current
 * position in the file will be reset to the initial position.
 */
void count_entries(FILE *res_pt file, uint *res_pt nrow, uint *res_pt ncol)
{
	uint i = 0, sum = 0;
	*ncol = 0;
	*nrow = 0; /**< use this as an error indicator */
	char tc, lbreak = 0, wspace = 0;
	rewind(file);
	while(fscanf(file, "%c", &tc) != EOF)
		if(tc == '\n')
		{
			if(lbreak)
				break; /**< this happens in case of an additional line break at the end of the file */
			i++; /**< the counter */
			if(!sum)
				*ncol = sum = i; /**< first line break */
			else if((sum += *ncol) != i)
			{
				fprintf(stderr, "lines are not of constant length at pos: %u\n", i);
				error_msg("reading error:", ERR_ARG);
				return;
			}
			lbreak = 1;
		}
		else if(tc == '0' || tc == '1' || tc == '2' || tc == '3' || tc == '4' ||
				tc == '5' || tc == '6' || tc == '7' || tc == '8' || tc == '9' ||
				tc == '.' || tc == '-' || tc == 'e' || tc == 'E')
			wspace = lbreak = 0; /**< got an entry */
		else if(tc == ' ' || tc == '\t')
		{
			if(wspace)
			{
				fprintf(stderr, "too much whitespace between numbers at pos: %u\n", i);
				error_msg("reading error:", ERR_ARG);
				return;
			}
			i++; /**< the counter */
			wspace = 1;
		}
		else if(tc == 13)
			continue; /**< carriage return on linux */
		else
		{
			fprintf(stderr, "last read character: '%c'\nASCII: %i at pos %u\n", tc, tc, i);
			error_msg("reading error:", ERR_ARG);
			return;
		}

	if(!lbreak)
		sum += *ncol; /**< adds the last column to the total number in case of no final line break */
	rewind(file);
	*nrow = sum / (*ncol);
}

uchar isfile(const char *szfile)
{
	struct stat statBuffer;
	return (stat(szfile, &statBuffer) >= 0 && /**< make sure it exists */
			!(statBuffer.st_mode & S_IFDIR)); /**< and it's not a directory */
}

/** \brief creates a directory to store the file if it doesn't exist
 *
 * \param fname const char* name of the directory
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

/** \brief prints floating point exceptions
 *
 * \param file const char* the name of the file which called this function
 * \param line int the line where this function was called
 * \param fname const char* the name of the function which called this function
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

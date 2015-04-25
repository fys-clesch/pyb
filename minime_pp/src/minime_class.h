#ifndef __MINIME_CLASS_HEADER_H__
#define __MINIME_CLASS_HEADER_H__

#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <atomic>

#include "../../src/header.hpp"
#include "fifo_class.h"

struct parameter
{
	std::string name, /**< Name of the parameter */
		 unit; /**< Unit of the parameter */
	double init, /**< Initial value */
		   minv, /**< Lower bound */
		   maxv, /**< Upper bound */
		   val, /**< Current value */
		   std_rel_err; /**< Error estimate, relative */
	bool log, /**< False: linear, True: log */
		 fit; /**< True means 'fit this' */
};

class minime
{
private:

	double lambda, /**< Wavelength in micrometer. */
		   scl; /**< Scaling factor pixel -> micrometer. */

	/**< The relaxed memory order is fine for the following atomic,
	as long as there is are no other possible side effects on other non-atomic
	variables. */
	std::atomic<bool> allocd,
	                  filled,
	                  do_gnuplot;

	uchar *res_pt bad;

	uint mnm_rows,
	     mnm_cols,
	     mnm_ntot;

	struct linear_reg_par
	{
		double ssq, /**< The sum of squares */
			   aoff, /**< Linear offset */
			   bmul, /**< Multiplier */
			   siga, /**< Sigma of a */
			   sigb, /**< Sigma of b */
			   sigdat, /**< Sigma of the dataset */
			   corr; /** The correlation between a and b: covar / (siga * sigb) */
	}linregpar;

	double *res_pt data;

	parameter fit_par_b[5],
	          fit_par_p[4];

	fifo ffc;

	static void fit_GaussianMultinormal(double *res_pt set, double *res_pt ssq);
	static void fit_BeamPropagation(double *res_pt set, double *res_pt ssq);
	void store_AllocatedMemory(const bool b);
	void store_FilledMemory(const bool b);
	bool load_AllocatedMemory(void);
	bool load_FilledMemory(void);
	bool load_UseGnuplot(void);
	double get_Wavelength(void);
	double get_PixelToUm(void);
	void get_CentroidBeamRadius(double *res_pt cen_x, double *res_pt cen_y,
								double *res_pt rad_x, double *res_pt rad_y,
								double *res_pt corr);
	void get_CentroidBeamCovariance(double *res_pt cen_x,
									double *res_pt cen_y,
									double *res_pt wxx, double *res_pt wyy,
									double *res_pt wxy);
	void get_GaussBeamMultinormal(const double wxz, const double wyz,
								const double corr,
								const double x_off, const double y_off,
								double *out);
	void get_GaussBeamMultinormalCovar(const double sxx, const double syy,
										const double sxy,
										const double x_off, const double y_off,
										double *out);
	void store_UseGnuplot(const bool b);
	void print_Parameter(const parameter *pp);

public:

	static parameter *alloc_Parameter(const uint n);

	minime(void);
	~minime(void);

	void set_Wavelength(const double wlen);
	void set_PixelToUm(const double pix2um);
	void set_Plotting(const bool do_it);
	void alloc_DataFromMemory(const uint nrows, const uint ncols,
						const uchar *const bad_in = nullptr);
	void fill_DataFromMemory(const double *res_pt data_in,
						const uchar *res_pt const bad_in = nullptr);
	void fit_GaussEllip(void);
	void fit_BeamProp(void);
	void fill_DataFromFile(const std::string &fname,
						const std::string &bad_fname = "");
};

#endif

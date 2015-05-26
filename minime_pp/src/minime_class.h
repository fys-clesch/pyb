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
protected:

	struct linear_reg_par
	{
		double ssq, /**< The sum of squares */
			   aoff, /**< Linear offset */
			   bmul, /**< Multiplier */
			   siga, /**< Sigma of a */
			   sigb, /**< Sigma of b */
			   sigdat, /**< Sigma of the dataset */
			   corr; /** The correlation between a and b: covar / (siga * sigb) */
	} linregpar;

	double *res_pt data;

	uint mnm_ntot; /**< Number of measurement points to be fitted. */

	fifo ffc;

	void print_Parameter(const parameter *pp)
	{
		iprint(stdout,
				"> parameter name: %s / %s\n" \
				"init: %g\n" \
				"min: %g\n" \
				"max: %g\n" \
				"val: %g\n" \
				"err: %g\n" \
				"log: %u\n" \
				"fit: %u\n",
				(*pp).name.c_str(), (*pp).unit.c_str(),
				(*pp).init, (*pp).minv, (*pp).maxv, (*pp).val,
				(*pp).std_rel_err,
				(*pp).log, (*pp).fit);
	}

	const parameter null_par = {"", "",
							0., 0., 0., 0., 0.,
							0, 0};

public:

	minime(void)
	{
		data = nullptr;
		linregpar.ssq =
		linregpar.aoff =
		linregpar.bmul =
		linregpar.siga =
		linregpar.sigb =
		linregpar.sigdat =
		linregpar.corr = 0.;

		mnm_ntot = 0;
	}

	~minime(void)
	{
		if(data != nullptr)
			free(data);
		#ifndef IGYBA_NDEBUG
		iprint(stdout, "'%s': memory released\n", __func__);
		#endif
	}

	static parameter *alloc_Parameter(const uint n)
	{
		parameter *m = static_cast<parameter *>(malloc(n * sizeof(parameter)));
		if(NULL == m)
		{
			error_msg("memory allocation failed. good bye.", ERR_ARG);
			exit(EXIT_FAILURE);
		}
		for(uint i = 0; i < n; ++i)
		{
			m[i].name = "";
			m[i].unit = "";
			m[i].init =
			m[i].minv =
			m[i].maxv =
			m[i].std_rel_err =
			m[i].val = 0.;
			m[i].log =
			m[i].fit = 0;
		}
		return m;
	}
};

class minime_profile : protected minime
{
private:

	double lambda, /**< Wavelength in micrometer. */
		   scl; /**< Scaling factor pixel -> micrometer. */

	/**< The relaxed memory order is fine for the following atomic,
	as long as there is are no other possible side effects on other non-atomic
	variables. */
	std::atomic<bool> allocd,
	                  filled,
	                  do_gnuplot,
	                  use_bad;

	uchar *res_pt bad;

	uint mnm_rows,
	     mnm_cols;

	parameter fit_par_b[5];

	static void fit_GaussianMultinormal(double *res_pt set, double *res_pt ssq);
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
	bool load_UseBad(void);
	void store_UseBad(const bool b);

public:

	minime_profile(void);
	~minime_profile(void);

	virtual void print_Parameter(const parameter *pp);
	void set_Wavelength(const double wlen);
	void set_PixelToUm(const double pix2um);
	void set_Plotting(const bool do_it);
	void alloc_DataFromMemory(const uint nrows, const uint ncols,
						const uchar *const bad_in = nullptr);
	void fill_DataFromMemory(const double *res_pt data_in,
						const uchar *res_pt const bad_in = nullptr);
	void fit_GaussEllip(void);
	void fill_DataFromFile(const std::string &fname,
						const std::string &bad_fname = "");
};

class minime_propagation : protected minime
{
private:

	double lambda; /**< Wavelength in micrometer. */

	bool do_gnuplot,
	     allocd_vec;

	double *zpnt, /**< Holds the relative measurement positions in millimeter. */
	       *wz_x, /**< Holds the beam radii in micrometer. */
	       *wz_y; /**< Holds the beam radii in micrometer. */

	parameter fit_par_p[4];

	static void fit_BeamPropagation(double *res_pt set, double *res_pt ssq);

	double get_BeamWidth(const uint n, const double z0, const double w0);

public:

	minime_propagation(void);
	~minime_propagation(void);

    void print_Members(void);
	void set_Wavelength(const double wlen);
	void set_Plotting(const bool do_it);
	void fit_BeamProp(void);
	void fill_DataFromFile(const std::string &fname);
	void store_AllocatedMemory(const bool b);
	bool load_AllocatedMemory(void);
};

#endif

#ifndef __FIFO_CLASS_HEADER__
#define __FIFO_CLASS_HEADER__

#include <string.h>
#include <stdlib.h>
#include <cstdio>
#include <chrono>
#include <thread>

#ifndef __FIFO_NO_OPENCV__
#include <opencv2/core/core.hpp>
#endif

#include "../../src/header.hpp"

#ifndef ISLINUX
#include "../../src/mingw.thread.h"
#endif

#if defined(ISLINUX)
 #ifndef __USE_BSD /**< for S_IFDIR */
  #define __USE_BSD
 #endif
 #include <sys/stat.h>
#elif defined(ISWIN32) || defined(ISWIN64)
 #include <dir.h>
#endif

class fifo
{
private:

	uint dat_rows,
	     dat_cols;

	bool use_contours;

	constexpr static uint print_prec = 15; /**< print-precision for output */

	std::string ax_x_title,
	            ax_y_title,
	            ax_z_title,
	            plot_title;

public:

	fifo(void);
	fifo(const uint nrows, const uint ncols);
	~fifo(void);

	void set_Dimensions(const uint nrows, const uint ncols);
	void set_AxisTitles(const std::string &xtitle,
						const std::string &ytitle,
						const std::string &ztitle = "z");
	void set_PlotTitle(const std::string &title);
	void set_UseContours(const bool set_it);
	static void print_HistogramFromUintFile(const std::string &target,
											const uint maxcount);
	void load_BeamProfileFilesToDoubleMemory(const std::string &inif,
													double *res_pt ccdtensor,
													const bool read_z_pos,
													double *res_pt z_pos);

	void plot_Data(const double *res_pt data,
					const bool auto_range = true,
					const double lo = 0.,
					const double hi = 0.);
	#ifndef __FIFO_NO_OPENCV__
	void plot_Data(const cv::Mat &mdata,
					const bool auto_range = true,
					const double lo = 0.,
					const double hi = 0.);
	void write_MatToFile(const cv::Mat &mat,
						const std::string &fname = "");
	#endif

	void write_Data_g(const std::string &fname,
					const uchar format,
					const double *res_pt data,
					const uint x_co = 0, const uint y_co = 0);

	void write_Bin_float(const std::string &fname, const float *res_pt data,
						const uint nrowscols = 0);
	void write_Bin_double(const std::string &fname, const double *res_pt data,
						const uint nrowscols = 0);
	void write_Bin_uint(const std::string &fname, const uint *res_pt data,
						const uint nrowscols = 0);
	void plot_Histogram(const std::string &fname, const uint steps);
};

#endif

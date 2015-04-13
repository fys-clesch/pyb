#ifndef __GRABBER_CLASS_HEADER_H__
#define __GRABBER_CLASS_HEADER_H__

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;

#include "../../src/header.hpp"
#include "../../viewer_pp/src/viewer_class.h"
#include "../../minime_pp/src/minime_class.h"
#include "../../src/threadhand_class.h"

#define DRAW_NOT_FINITE_SIGN_ATOM 0 /**< toggles between a sign to visualize
that the calculated moment is not finite */

class grabber
{
private:

	bool detct_settings,
	     bground_in,
	     bground_appl,
	     blur_appl,
	     finite_moments,
	     grab_frames,
	     roi_on,
	     mouse_drag,
	     saturated;

	std::atomic<bool> close_copy_thread,
	                  close_viewer_thread,
	                  close_minime_thread;

	std::atomic<uint32_t> work_roi_rows,
	                      work_roi_cols;

	std::atomic<double> gaussblur_sigma_x_atm,
	                    groundlift_sub_atm,
	                    gaussblur_min_atm,
	                    gaussblur_max_atm,
	                    groundlift_max_atm;

	std::atomic<uint> gaussblur_sze_atm,
	                  gaussblur_sze_min_atm,
	                  gaussblur_sze_max_atm;

	int in_chn,
	    in_depth,
	    px_mouse,
	    py_mouse;

	uint32_t in_rows,
	         in_cols;


	uint64_t frms;

	double pval,
	       max_pval,
	       pix2um_scale,
	       ell_theta,
	       ell_ecc,
	       gaussblur_min,
	       gaussblur_max,
	       gaussblur_sigma_x,
	       groundlift_max,
	       groundlift_sub;


	double *work_roi_arr,
	       *work_roi_arr_buf, /** @todo candidate for atomic? */
	       *work_roi_arr_tflip_buf, /** @todo candidate for atomic? */
	       *gldata_buf;

	constexpr static uint32_t saturated_thresh = 80 * 80;

	constexpr static uint16_t mat_typ = CV_32FC1;

	Mat fp_in, /**< a floating point copy of the input */
	    bground, /**< a background matrix of floating point type */
	    work, /**< the matrix that is to be worked on */
	    work_roi, /**< a copy of 'work' but with a specific AOI */
	    work_roi_tflip, /**< similar to 'work_roi' but flipped */
	    rgb, /**< The input in RGB format. To be drawn on. */
	    temp_CV_32FC3,
	    temp_CV_32FC1,
	    temp_CV_16UC3,
	    covar,
	    eigenv,
	    beam_parameter,
	    tbar_win_mat;

	Point2d centroid;

	Point_<int> start_roi,
	            end_roi;

	Rect_<int> roi_rect;

	std::string main_win_name,
	            tbar_win_name;

	uint gaussblur_sze_min,
	     gaussblur_sze_max;

	Size gaussblur_sze;


	threadhand event_CopyToViewer,
	           event_LaunchViewer,
	           event_LaunchMinime;

	minime mime;

	viewer beau;

	fifo ffc;

	void init_Mats(void);
	void fill_DataForViewer(const uint nrows, const uint ncols);

public:

	enum save_Im_type{RGB = 1, WORK, FP_IN, IN};

	Mat in;

	grabber(void);
	~grabber(void);
	void produce_Mat_Work(void);
	void update_Mats_RgbAndFp(void);
	void set_Pix2UmScale(const double scl);
	void get_Moments_own(void);
	void get_Moments(void);
	void draw_Moments(const bool chatty = true);
	void draw_Info(void);
	void set_Background(void);
	void unset_Background(void);
	static void cast_static_set_MouseEvent(const int event,
											const int x, const int y,
											const int flags,
											void *u);
	void draw_RoiRectangle(void);
	void set_MouseEvent(const int event, const int x, const int y,
						const int flags);
	void show_Im_RGB(void);
	void show_Trackbars(void);
	void show_Help(void);
	void show_Intro(void);
	void show_HelpOnCurses(void);
	Mat get_Mat_private(const enum save_Im_type mtype);
	void save_Image(const enum save_Im_type mtype,
					const std::string &win_title = "",
					const std::string &fmt = "png");
	void store_Image(const enum save_Im_type mtype,
					const std::string &fname = "");
	void draw_Crossline(void);
	void apply_RemoveBase(const double thresh = 0.);
	void set_MainWindowName(const std::string &name);
	void set_TrackbarWindowName(const std::string &name);
	void get_MainWindowName(std::string &name);
	std::string get_MainWindowName(void);
	void get_TrackbarWindowName(std::string &name);
	std::string get_TrackbarWindowName(void);
	Mat get_TrackbarWindowMat(void);
	Mat &get_TrackbarWindowMatPtr(void);
	void toggle_Smoothing(void);
	uint32_t get_nRows(void);
	uint32_t get_nCols(void);
	uint32_t get_nRowsROI(void);
	uint32_t get_nColsROI(void);
	void toggle_Grabbing(void);
	bool is_Grabbing(void);
	void increment_Frames(void);
	uint64_t get_Frames(void);
	void gnuplot_Image(const enum save_Im_type mtype,
						const std::string &win_title = "");
	void calculate_BeamRadius(void);

	static void copy_DataToViewer(void);
	static void schedule_Viewer(int argc, char **argv);
	static void schedule_Minime(const double wavelengthUm, const double pix2um);

	void launch_Minime(const double wavelengthUm, const double pix2um);
	void close_MinimeThread(void);
	bool signal_MinimeThreadIfWait(void);
	void close_CopyThread(void);
	bool signal_CopyThreadIfWait(void);
	void close_ViewerThread(void);
	bool signal_ViewerThreadIfWait(void);
	bool is_ViewerWindowRunning(void);
	void close_ViewerWindow(void);

	void create_TrackbarBlur(void);
	static void cast_static_SetTrackbarHandlerBlur(int i, void *ptr);
	void TrackbarHandlerBlur(int i);
	void create_TrackbarBlurSize(void);
	static void cast_static_SetTrackbarHandlerBlurSize(int i, void *ptr);
	void TrackbarHandlerBlurSize(int i);
	void create_TrackbarGroundlift(void);
	static void cast_static_SetTrackbarHandlerGroundlift(int i, void *ptr);
	void set_GroundliftAtomic(const double val);
	void TrackbarHandlerGroundlift(int i);

	void get_GroundliftRangeAtomic(double *res_pt gl_current,
									double *res_pt gl_max);
	uint get_KernelSizeAtomic(uint *res_pt sze_min, uint *res_pt sze_max);
	void set_KernelSizeAtomic(const uint i);
	double get_GaussBlurRangeAtomic(double *res_pt gb_min, double *res_pt gb_max);
	void set_GaussBlurAtomic(const double val);
	bool get_MouseDrag(void);
	void set_MouseDrag(const bool val);
	bool get_RoiActive(void);
	void set_RoiActive(const bool val);
	double get_PixelValue(void);
	void set_PixelValue(const double val);
	void copy_MousePosition(const int px, const int py);
	void set_StartRoi(const Point_<int> &val);
	void set_EndRoi(const Point_<int> &val);
	void set_RectRoi(const Rect_<int> &val);
	void get_RectRoi(int *res_pt sx, int *res_pt sy,
					int *res_pt rw, int *res_pt rh);
	double get_PixelValueWork(const int x, const int y);
	void get_StartRoi(int *res_pt px, int *res_pt py);

	void toggle_ViewerIdling(void);
	void toggle_ViewerMap3DMode(void);
	void toggle_ViewerRotation(void);

	void exchange_Atomics(void);
};

#endif

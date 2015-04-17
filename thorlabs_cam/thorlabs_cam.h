#ifndef __IGYBA_THORLABS_CAM_H__
#define __IGYBA_THORLABS_CAM_H__

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../src/header.hpp"
#include "../grabber/src/grabber_class.h"

#ifdef ISLINUX
 #include "ueye.h"
#elif defined(ISWIN32) || defined(ISWIN64)
 #include "uc480.h"
#endif

class thorlabs_cam : public grabber
{
private:

	HCAM pcam;

	SENSORINFO s_info;

	BOARDINFO b_info;

	int err,
	    memID; /**< grabber memory - buffer ID */

	char *im_mem; /**< pointer to buffer */

	uint32_t im_width,
	         im_max_width, /**< initial or maximal width */
	         im_height,
	         im_max_height, /**< initial or maximal height */
	         pix_clock,
	         im_min_width, /**< AOI minimum width */
	         im_min_height, /**< AOI minimum height */
	         im_inc_width, /**< AOI step size in width */
	         im_inc_height, /**< AOI step size in height */
	         im_aoi_width, /**< full AOI width */
	         im_aoi_height, /**< full AOI height */
	         im_aoi_width_start, /**< start position (in pixel) of the AOI */
	         im_aoi_height_start, /**< start position (in pixel) of the AOI */
	         sensor_aa_width, /**< exact active width of the sensor in um */
	         sensor_aa_height; /**< exact active height of the sensor in um */

	uint16_t pix_size, /**< pixel size encoded in an integer */
	         bits_p_pix, /**< the total number of bits occupied at one pixel.
	                          not all may be used,
	                          even when the value is e.g. 32 */
	         color_mod, /**< control the colour mode of the images */
	         color_mod_test,
	         color_mod_init;

	double dpix_size, /**< pixel size in um */
	       fps,
	       exp_time, /**< exposure time in ms */
	       exp_time_min,
	       exp_time_max,
	       exp_time_inc;

	std::atomic<double> dpix_size_atm,
	                    fps_atm,
	                    exp_time_atm,
	                    exp_time_min_atm,
	                    exp_time_max_atm,
	                    exp_time_inc_atm;

	bool err_break,
	     cntrl_exp_time,
	     supp_fine_inc_exp_time;

	Mat im_p,
	    infotbar_win_mat;

	std::string infotbar_win_name,
	            sensorname;

	const std::string trck_name_aoi_sw = "start width",
	                  trck_name_aoi_ww = "AOI width",
	                  trck_name_aoi_sh = "start height",
	                  trck_name_aoi_wh = "AOI height";

	bool get_Image(IplImage *ipl_im);
	Mat get_Mat(void);
	void show_Image(const char *win_name);
	void get_PixelClock(void);
	void get_Fps(void);
	void get_ExposureTime(void);
	void set_ExposureTime(double time);
	void set_ErrorReport(void);
	void get_CameraStatus(void);
	void get_GainBoost(void);
	void identify_CameraAOISettings(void);
	static void cast_static_SetTrackbarHandlerExposure(int i, void *ptr);
	void TrackbarHandlerExposure(int i);
	void set_CameraInfoWindowName(const std::string &name);
	void TrackbarHandlerStartAOIWidth(int i);
	static void cast_static_SetTrackbarHandlerStartAOIWidth(int i, void *ptr);
	void create_TrackbarStartAOIWidth(void);
	void set_MouseEvent(const int event, const int x, const int y,
						const int flags);

public:

	thorlabs_cam(void);
	~thorlabs_cam(void);
	void init_Camera(void);
	void set_ColourMode(void);
	void set_ExitMode(void);
	void set_Display(void);
	void alloc_ImageMem(void);
	void set_ImageMem(void);
	void set_ImageSize(void);
	void inquire_ImageMem(int *res_pt nx, int *res_pt ny,
				int *res_pt bits_p_pix,
				int *res_pt pixel_bit_pitch);
	bool caught_Error(void);
	void show_CameraTrackbars(void);
	double get_PixelPitch(void);
	std::string get_CameraInfoWindowName(void);
		bool get_Image(Mat &im);
	bool get_Image(void);
	static void cast_static_set_MouseEvent(const int event,
											const int x, const int y,
											const int flags,
											void *u);
	int get_Width(void);
	int get_Height(void);
	void create_TrackbarExposure(void);
	void draw_CameraInfo(void);
	/* The following functions are accessed by wxWidgets. */
	void get_ExposureTimesAtomic(double *res_pt time,
							double *res_pt min_time,
							double *res_pt max_time,
							double *res_pt inc_time);
	void set_ExposureTimeAtomic(double time);
	void exchange_ExposureTimeAtomic(void);
};

#endif

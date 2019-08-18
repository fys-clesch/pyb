#include "thorlabs_cam.h"
#include <assert.h>

thorlabs_cam::thorlabs_cam(void)
{
	/* SENSORINFO */
	s_info = SENSORINFO{
		0,
		"", '\0',
		0, 0,
		0, 0, 0, 0, 0,
		#if defined(ISWIN32) || defined(ISWIN64)
		0,
		#endif
		""};
	/* BOARDINFO */
	b_info = BOARDINFO{
		"", "", "", "",
		'\0', '\0',
		""
		};
	/* HCAM */
	pcam = (HCAM)NULL;
	/* int */
	err = memID = 0; /* 2 */
	/* char * */
	im_mem = (char *)NULL;
	/* double */
	exp_time =
	exp_time_inc =
	exp_time_max =
	exp_time_min =
	fps = 0.;
	dpix_size = -1.; /* 6 */
	/* bool */
	cntrl_exp_time =
	supp_fine_inc_exp_time =
	err_break = false; /* 3 */
	/* uint16_t */
	color_mod =
	color_mod_test = IS_CM_BAYER_RG8;
	color_mod_init =
	bits_p_pix =
	pix_size = 0; /* 5 */
	/* atomic<double> */
	fps_atm.store(0., std::memory_order_relaxed);
	dpix_size_atm.store(0., std::memory_order_relaxed);
	exp_time_atm.store(0., std::memory_order_relaxed);
	exp_time_min_atm.store(0., std::memory_order_relaxed);
	exp_time_max_atm.store(0., std::memory_order_relaxed);
	exp_time_inc_atm.store(0., std::memory_order_relaxed); /* 6 */
	/* uint32_t */
	pix_clock =
	im_width =
	im_max_width =
	im_height =
	im_max_height =
	im_min_width =
	im_min_height =
	im_inc_width =
	im_inc_height =
	sensor_aa_width =
	sensor_aa_height =
	im_aoi_width =
	im_aoi_height =
	im_aoi_width_start =
	im_aoi_height_start = 0; /* 15 */
	/* string */
	infotbar_win_name  = "camera information window";
	/* Mat */
	infotbar_win_mat = cv::Mat::zeros(150, 350, CV_8UC3);

	/* set the bits per pixel variable */
	if(color_mod == IS_CM_MONO8 ||
		color_mod == IS_CM_SENSOR_RAW8)
		bits_p_pix = 8;
	else if(color_mod == IS_CM_MONO12 ||
			color_mod == IS_CM_MONO16 ||
			color_mod == IS_CM_SENSOR_RAW12 ||
			color_mod == IS_CM_SENSOR_RAW16 ||
			color_mod == IS_CM_BGR555_PACKED ||
			color_mod == IS_CM_BGR565_PACKED)
		bits_p_pix = 16;
	else if(color_mod == IS_CM_RGB8_PACKED ||
			color_mod == IS_CM_BGR8_PACKED)
		bits_p_pix = 24;
	else if(color_mod == IS_CM_RGBA8_PACKED ||
			color_mod == IS_CM_BGRA8_PACKED ||
			color_mod == IS_CM_RGBY8_PACKED ||
			color_mod == IS_CM_BGRY8_PACKED ||
			color_mod == IS_CM_RGB10V2_PACKED ||
			color_mod == IS_CM_BGR10V2_PACKED)
		bits_p_pix = 32;
	else
	{
		bits_p_pix = 8;
		warn_msg("can't find right colour mode. setting bits per pixel to 8.",
				ERR_ARG);
	}
}

thorlabs_cam::~thorlabs_cam(void)
{
	im_p.release();

	/* Stop live video mode. */
	err = is_StopLiveVideo(pcam, IS_FORCE_VIDEO_STOP);
	if(err != IS_SUCCESS)
		warn_msg("error forcing live video stop", ERR_ARG);

	/* free image memory. */
	err = is_FreeImageMem(pcam, im_mem, memID);
	if(err != IS_SUCCESS)
		warn_msg("error freeing image memory", ERR_ARG);

	/* close / release camera handler. */
	err = is_ExitCamera(pcam);
	if(err != IS_SUCCESS)
		warn_msg("error releasing camera handler", ERR_ARG);

	#ifndef IGYBA_NDEBUG
	iprint(stdout, "'%s': memory released\n", __func__);
	#endif
}

void thorlabs_cam::init_Camera(void)
{
	/* Initialise the camera. */
	err = is_InitCamera(&pcam, NULL);
	if(err != IS_SUCCESS)
		err_break = true;
	#ifdef IGYBA_NDEBUG
	else
		iprint(stdout, "camera initialised successfully\n");
	#endif

	if(caught_Error())
	{
		error_msg("error in the camera initialisation detected.\n" \
				"is the camera connected?\nexiting.", ERR_ARG);
		exit(EXIT_FAILURE);
	}

	set_ErrorReport();

	/* Get board and sensor info. */
	err = is_GetSensorInfo(pcam, &s_info);
	if(err != IS_SUCCESS)
	{
		error_msg("error getting sensor info", ERR_ARG);
		err_break = true;
	}

	im_width = im_aoi_width = im_max_width = s_info.nMaxWidth;
	im_height = im_aoi_height = im_max_height = s_info.nMaxHeight;
	color_mod_init = s_info.nColorMode;
	sensorname = s_info.strSensorName;

	#ifdef ISLINUX
	warn_msg("pixel size not available on linux library 'ueye' yet. " \
			"relying on the identification of the sensor name.", ERR_ARG);
	#elif defined(ISWIN32) || defined(ISWIN64)
	pix_size = s_info.wPixelSize;
	#endif

	identify_CameraAOISettings();

	dpix_size = pix_size * 1e-8 * 1e6; /**< Into meters then into um. */
	dpix_size_atm.store(dpix_size, std::memory_order_relaxed);

	err = is_GetBoardInfo(pcam, &b_info);
	if(err != IS_SUCCESS)
	{
		error_msg("error getting board info", ERR_ARG);
		err_break = true;
	}

	get_PixelClock();
	get_Fps();
	get_ExposureTime();
	get_GainBoost();
	#define PR_C "%-23s: "
	iprint(stdout,
			PR_C "%s\n" \
			PR_C "%s\n" \
			PR_C "%s\n" \
			PR_C "%i\n" \
			PR_C "%i\n" \
			PR_C "%.2g\n" \
			PR_C "%i\n" \
			PR_C "%.2g\n" \
			PR_C "%s\n" \
			PR_C "%u\n" \
			PR_C "%g\n" \
			PR_C "%g\n" \
			PR_C "%g\n" \
			PR_C "%g\n",
			"camera ID", b_info.ID,
			"serial #", b_info.SerNo,
			"date", b_info.Date,
			"pixel size / 1e-8 m", pix_size,
			"width / pixel", im_width,
			"      / um", im_width * dpix_size,
			"height / pixel", im_height,
			"       / um", im_height * dpix_size,
			"colour mode", color_mod_init == IS_COLORMODE_MONOCHROME ?
			("MONOCHROME") : color_mod_init == IS_COLORMODE_BAYER ?
			("BAYER") : ("CBYCRY"),
			"pixel clock / MHz", pix_clock,
			"exposure time set / ms", exp_time,
			"              max / ms", exp_time_max,
			"              min / ms", exp_time_min,
			"              inc / ms", exp_time_inc);

	if(color_mod == IS_CM_MONO8 ||
		color_mod == IS_CM_SENSOR_RAW8)
		im_p = cv::Mat(cv::Size(im_width, im_height), CV_8UC1, 0.);
	else if(color_mod == IS_CM_MONO12 ||
			color_mod == IS_CM_MONO16 ||
			color_mod == IS_CM_SENSOR_RAW12 ||
			color_mod == IS_CM_SENSOR_RAW16 ||
			color_mod == IS_CM_BGR555_PACKED ||
			color_mod == IS_CM_BGR565_PACKED)
		im_p = cv::Mat(cv::Size(im_width, im_height), CV_16UC1, 0.);
	else if(color_mod == IS_CM_RGB8_PACKED ||
			color_mod == IS_CM_BGR8_PACKED)
		im_p = cv::Mat(cv::Size(im_width, im_height), CV_8UC3, 0.);
	else if(color_mod == IS_CM_RGBA8_PACKED ||
			color_mod == IS_CM_BGRA8_PACKED ||
			color_mod == IS_CM_RGBY8_PACKED ||
			color_mod == IS_CM_BGRY8_PACKED ||
			color_mod == IS_CM_RGB10V2_PACKED ||
			color_mod == IS_CM_BGR10V2_PACKED)
		im_p = cv::Mat(cv::Size(im_width, im_height), CV_8UC4, 0.);
	else
	{
		im_p = cv::Mat(cv::Size(im_width, im_height), CV_8UC1, 0.);
		warn_msg("can't find right colour mode. " \
				"setting Mat format to CV_8UC1.", ERR_ARG);
	}
}

void thorlabs_cam::set_ColourMode(void)
{
	err = is_SetColorMode(pcam, color_mod);
	if(err != IS_SUCCESS)
		error_msg("error setting colour mode", ERR_ARG);
	if(color_mod_test != is_SetColorMode(pcam, IS_GET_COLOR_MODE))
		iprint(stderr,
				"error setting colour mode to %u\n",
				color_mod_test);
}

/* Set to auto-release camera and memory if camera is disconnected on-the-fly. */
void thorlabs_cam::set_ExitMode(void)
{
	err = is_EnableAutoExit(pcam, IS_ENABLE_AUTO_EXIT);
	if(err != IS_SUCCESS)
		warn_msg("error setting auto-exit mode", ERR_ARG);
}

/* Set image mode to DIB = system memory. */
void thorlabs_cam::set_Display(void)
{
	err = is_SetDisplayMode(pcam, IS_SET_DM_DIB);
	if(err != IS_SUCCESS)
		error_msg("error setting DIB mode", ERR_ARG);
}

/* Allocate image memory. */
void thorlabs_cam::alloc_ImageMem(void)
{
	err = is_AllocImageMem(pcam,
							im_width, im_height, bits_p_pix, &im_mem, &memID);
	if(err != IS_SUCCESS)
	{
		error_msg("error allocating image memory", ERR_ARG);
		err_break = true;
	}
}

/* Reads out properties of the allocated image memory. */
void thorlabs_cam::inquire_ImageMem(int *res_pt nx, int *res_pt ny,
									int *res_pt bits_p_pix,
									int *res_pt pixel_bit_pitch)
{
	int xo, yo, bppo, pbpo;
	err = is_InquireImageMem(pcam, im_mem, memID, &xo, &yo, &bppo, &pbpo);
	if(err != IS_SUCCESS)
	{
		error_msg("error inquiring image memory", ERR_ARG);
		err_break = true;
	}
	iprint(stdout,
			"%s: %i\n" \
			"%s: %i\n" \
			"%s: %i\n" \
			"%s: %i\n",
			"x / entries", xo,
			"y / entries", yo,
			"bpp", bppo,
			"pbb", pbpo);
	if(nx)
		*nx = xo;
	if(ny)
		*ny = yo;
	if(bits_p_pix)
		*bits_p_pix = bppo;
	if(pixel_bit_pitch)
		*pixel_bit_pitch = pbpo;
}

/** \brief Activate image memory.
 *
 * \param void
 * \return void
 *
 */
void thorlabs_cam::set_ImageMem(void)
{
	err = is_SetImageMem(pcam, im_mem, memID);
	if(err != IS_SUCCESS)
	{
		error_msg("error activating image memory", ERR_ARG);
		err_break = true;
	}
}

void thorlabs_cam::set_ImageSize(void)
{
	/** Deprecated:
	 * err = is_SetImageSize(pcam, width, height);
	 */
	IS_RECT rectAOI;
	rectAOI.s32X =
	rectAOI.s32Y = 0;
	rectAOI.s32Width = im_width;
	rectAOI.s32Height = im_height;
	err = is_AOI(pcam, IS_AOI_IMAGE_SET_AOI, (void *)&rectAOI, sizeof(rectAOI));
	if(err != IS_SUCCESS)
		error_msg("error setting image size", ERR_ARG);
}

/** \brief Acquire a single image from the camera.
 *
 * \param ipl_im cv::IplImage*
 * \return bool
 *
 */
bool thorlabs_cam::get_Image(cv::IplImage *ipl_im)
{
	err = is_FreezeVideo(pcam, IS_WAIT);
	if(err == IS_SUCCESS)
	{
		cvSetData(ipl_im, im_mem, im_width); /* Sets imageData member. */
		return true;
	}
	handle_Error(err);
	return false;
}

bool thorlabs_cam::get_Image(cv::Mat &img)
{
	err = is_FreezeVideo(pcam, IS_WAIT);
	if(err == IS_SUCCESS)
	{
		/** @todo cv::Mat(int rows, int cols, int type, void* data, size_t step = AUTO_STEP) */
		im_p.data = (uchar *)im_mem;
		im_p.copyTo(img);
		return true;
	}
	handle_Error(err);
	return false;
}

bool thorlabs_cam::get_Image(void)
{
	err = is_FreezeVideo(pcam, IS_WAIT);
	if(err == IS_SUCCESS) /** @todo cv::Mat(int rows, int cols, int type, void* data, size_t step = AUTO_STEP) */
	{
		im_p.data = (uchar *)im_mem;
		return true;
	}
	handle_Error(err);
	return false;
}

void thorlabs_cam::handle_Error(const uchar err)
{
	if(err == IS_OUT_OF_MEMORY)
		error_msg("no memory available", ERR_ARG);
	else if(err == IS_BAD_STRUCTURE_SIZE)
		error_msg("an internal structure has an incorrect size", ERR_ARG);
	else if(err == IS_CANT_OPEN_DEVICE)
		error_msg("no camera connected or initialisation error", ERR_ARG);
	else if(err == IS_CANT_COMMUNICATE_WITH_DRIVER)
		error_msg("driver has not been loaded", ERR_ARG);
	else if(err == IS_TRANSFER_ERROR)
		warn_msg("transfer error", ERR_ARG);
	else if(err == IS_IO_REQUEST_FAILED)
		error_msg("an IO request from the uc480 driver failed", ERR_ARG);
	else if(err == IS_CAPTURE_RUNNING)
		warn_msg("capture operation in progress must be " \
				"terminated before starting another one", ERR_ARG);
	else
		warn_msg("unspecified error / warning during image capture", ERR_ARG);
}

cv::Mat thorlabs_cam::get_Mat(void)
{
	return im_p.clone();
}

int thorlabs_cam::get_Width(void)
{
	return im_width;
}

int thorlabs_cam::get_Height(void)
{
	return im_height;
}

bool thorlabs_cam::caught_Error(void)
{
	if(err_break)
		return true;
	else
		return false;
}

void thorlabs_cam::show_Image(const char *win_name)
{
	cv::imshow(win_name, im_p);
}

void thorlabs_cam::get_PixelClock(void)
{
	#ifdef ISLINUX
	pix_clock = is_GetUsedBandwidth(pcam);
	#else
	err = is_PixelClock(pcam,
						IS_PIXELCLOCK_CMD_GET,
						(void *)&pix_clock,
						sizeof(pix_clock));
	#endif
	if(err != IS_SUCCESS)
	{
		warn_msg("error getting pixel clock", ERR_ARG);
		err_break = true;
	}
}

void thorlabs_cam::get_Fps(void)
{
	err = is_GetFramesPerSecond(pcam, &fps);
	if(err != IS_SUCCESS)
	{
		warn_msg("error frame rate", ERR_ARG);
		err_break = true;
	}
}

double thorlabs_cam::get_PixelPitch(void)
{
	return dpix_size;
}

void thorlabs_cam::get_ExposureTime(void)
{
	double rng[3],
	       time;
	uint32_t ncaps;
	err = is_Exposure(pcam,
					IS_EXPOSURE_CMD_GET_CAPS,
					(void *)&ncaps,
					sizeof(ncaps));
	if(err != IS_SUCCESS)
	{
		error_msg("error getting exposure function modes", ERR_ARG);
		err_break = true;
	}
	else
	{
		if(ncaps & IS_EXPOSURE_CAP_EXPOSURE)
		{
			iprint(stdout, "exposure time setting supported\n");
			cntrl_exp_time = true;
		}
		else
			cntrl_exp_time = false;
		if(ncaps & IS_EXPOSURE_CAP_FINE_INCREMENT)
		{
			iprint(stdout, "exposure time fine increment supported\n");
			supp_fine_inc_exp_time = true;
		}
		else
			supp_fine_inc_exp_time = false;
		#ifndef ISLINUX
		if(ncaps & IS_EXPOSURE_CAP_LONG_EXPOSURE)
			iprint(stdout, "long exposure time supported\n");
		#endif
	}

	err = is_Exposure(pcam,
					IS_EXPOSURE_CMD_GET_EXPOSURE,
					(void *)&time,
					sizeof(time));
	if(err != IS_SUCCESS)
	{
		error_msg("error getting exposure time", ERR_ARG);
		err_break = true;
	}
	exp_time = time;
	err = is_Exposure(pcam,
					IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE,
					(void *)rng,
					sizeof(rng));
	if(err != IS_SUCCESS)
	{
		error_msg("error getting exposure time range", ERR_ARG);
		err_break = true;
	}
	exp_time_min = rng[0];
	exp_time_max = rng[1];
	exp_time_inc = rng[2];

	set_ExposureTimeAtomic(exp_time);
	exp_time_min_atm.store(exp_time_min, std::memory_order_relaxed);
	exp_time_max_atm.store(exp_time_max, std::memory_order_relaxed);
	exp_time_inc_atm.store(exp_time_inc, std::memory_order_relaxed);
}

void thorlabs_cam::set_ExposureTime(double time)
{
	if(time > exp_time_max)
		time = exp_time_max;
	else if(time < exp_time_min)
		time = exp_time_min;

	err = is_Exposure(pcam,
					IS_EXPOSURE_CMD_SET_EXPOSURE,
					(void *)&time,
					sizeof(time));
	if(err != IS_SUCCESS)
	{
		error_msg("error setting exposure time", ERR_ARG);
		err_break = true;
	}
	else
	{
		#ifndef IGYBA_NDEBUG
		iprint(stdout, "exposure time set to %g ms\n", time);
		#endif
		exp_time = time;
		set_ExposureTimeAtomic(exp_time);
	}
}

void thorlabs_cam::exchange_ExposureTimeAtomic(void)
{
	double time = exp_time_atm.load(std::memory_order_acquire);
	if(time != exp_time)
	{
		if(time > exp_time_max)
			time = exp_time_max;
		else if(time < exp_time_min)
			time = exp_time_min;

		err = is_Exposure(pcam,
						IS_EXPOSURE_CMD_SET_EXPOSURE,
						(void *)&time,
						sizeof(time));
		if(err != IS_SUCCESS)
		{
			error_msg("error setting exposure time", ERR_ARG);
			err_break = true;
		}
		else
		{
			#ifndef IGYBA_NDEBUG
			iprint(stdout, "exposure time set to %g ms\n", time);
			#endif
			exp_time = time;
			exp_time_atm.store(exp_time, std::memory_order_release);
		}
	}
}

void thorlabs_cam::set_ExposureTimeAtomic(double time)
{
	exp_time_atm.store(time, std::memory_order_relaxed);
}

void thorlabs_cam::get_ExposureTimesAtomic(double *res_pt time,
										double *res_pt min_time,
										double *res_pt max_time,
										double *res_pt inc_time)
{
	*time = exp_time_atm.load(std::memory_order_acquire);
	*min_time = exp_time_min_atm.load(std::memory_order_acquire);
	*max_time = exp_time_max_atm.load(std::memory_order_acquire);
	*inc_time = exp_time_inc_atm.load(std::memory_order_acquire);
}

void thorlabs_cam::get_CameraStatus(void)
{
	ulong status;
	status = is_CameraStatus(pcam,
							IS_LAST_CAPTURE_ERROR,
							IS_GET_STATUS);
	switch(status)
	{
	case IS_BAD_STRUCTURE_SIZE:
		error_msg("an internal structure has an incorrect size", ERR_ARG);
		break;
	case IS_CANT_COMMUNICATE_WITH_DRIVER:
		error_msg("no driver loaded, communication with camera failed", ERR_ARG);
		break;
	case IS_CANT_OPEN_DEVICE:
		error_msg("no camera connected or initialization error", ERR_ARG);
		break;
	case IS_INVALID_CAMERA_HANDLE:
		error_msg("invalid camera handle", ERR_ARG);
		break;
	case IS_OUT_OF_MEMORY:
		error_msg("no memory could be allocated", ERR_ARG);
		break;
	case IS_TIMED_OUT:
		warn_msg("an image capturing process could not be terminated within the allowable period", ERR_ARG);
		break;
	}
}

/** \brief Sets the error event logging from Thorlabs driver to off.
 *
 * \param void
 * \return void
 *
 * Note that errors can still be handled, only the dialogue box generated by
 * the Thorlabs driver (on Win systems) is disabled.
 */
void thorlabs_cam::set_ErrorReport(void)
{
	err = is_SetErrorReport(pcam, IS_DISABLE_ERR_REP);
	if(err != IS_SUCCESS)
	{
		error_msg("error disabling error report", ERR_ARG);
		err_break = true;
	}
}

void thorlabs_cam::get_GainBoost(void)
{
	err = is_SetGainBoost(pcam, IS_GET_SUPPORTED_GAINBOOST);
	if(err == IS_SET_GAINBOOST_ON)
	{
		err = is_SetGainBoost(pcam, IS_GET_GAINBOOST);
		if(err == IS_SET_GAINBOOST_ON)
			iprint(stdout,
					"analogue hardware gain boost feature available and switched on\n");
		else
			iprint(stdout,
					"analogue hardware gain boost feature available and switched off\n");
	}
	else if(err == IS_SET_GAINBOOST_OFF)
		iprint(stdout,
				"analogue hardware gain boost feature not available\n");
	else
		iprint(stdout,
				"error detecting analogue hardware gain boost feature\n");
}

void thorlabs_cam::TrackbarHandlerExposure(int i)
{
	const double out_min = 0., out_max = 100.;
	double res = map_Linear((double)i, out_min, out_max,
							exp_time_min, exp_time_max);
	(*this).set_ExposureTime(res);
}

void thorlabs_cam::cast_static_SetTrackbarHandlerExposure(int i, void *ptr)
{
	thorlabs_cam *cam = static_cast<thorlabs_cam *>(ptr);
	(*cam).TrackbarHandlerExposure(i);
}

void thorlabs_cam::create_TrackbarExposure(void)
{
	const std::string trck_name = "Exposure";
	const double out_min = 0., out_max = 100.;
	double res = map_Linear(exp_time, exp_time_min, exp_time_max,
							out_min, out_max);
	static int setting = res; /* Must be static, as its memory address is stored
	in the cv function. */
	assert(setting <= 100);
	/**
	 * 3 parameters are:
	 * The address of the variable that is changing when the trackbar is moved,
	 * the maximum value the trackbar can move,
	 * and the function that is called whenever the trackbar is moved.
	 */
	cv::createTrackbar(trck_name,
					infotbar_win_name,
					&setting,
					(int)out_max,
					thorlabs_cam::cast_static_SetTrackbarHandlerExposure,
					this);
}

void thorlabs_cam::draw_CameraInfo(void)
{
	infotbar_win_mat.setTo(0);
	const uint16_t lw = 1;
	double fscl = .45,
	       sx, sy;
	const int font = cv::FONT_HERSHEY_SIMPLEX;
	const cv::Scalar_<double> clr_txt(0., 255., 0.);
	static std::string info(128, '\0');
	#define putText_ARGS cv::Point2d(sx, sy), font, fscl, clr_txt, lw, cv::LINE_AA

	sx = 10.;
	sy = infotbar_win_mat.rows - 10.;
	get_Fps();
	info = "frame rate: " + convert_Double2Str(fps, 1) + " 1 / s";
	cv::putText(infotbar_win_mat, info, putText_ARGS);
	sy -= 20.;
	info = "exposure time: " + convert_Double2Str(exp_time, 3) + " ms";
	cv::putText(infotbar_win_mat, info, putText_ARGS);
	sy -= 20.;
	info = "(start / end) width: (" +
	convert_Int2Str(im_aoi_width_start) + ", " +
	convert_Int2Str(im_aoi_width) + ") pixel";
	cv::putText(infotbar_win_mat, info, putText_ARGS);
	sy -= 20.;
	info = "(start / end) height: (" +
	convert_Int2Str(im_aoi_height_start) + ", " +
	convert_Int2Str(im_aoi_height) + ") pixel";
	cv::putText(infotbar_win_mat, info, putText_ARGS);
	sy -= 20.;
	info = "sensor area: (" +
	convert_Int2Str(sensor_aa_width) + " * " +
	convert_Int2Str(sensor_aa_height) + ") um^2";
	cv::putText(infotbar_win_mat, info, putText_ARGS);
	sy -= 20.;
	info = "camera model: " + sensorname;
	cv::putText(infotbar_win_mat, info, putText_ARGS);
	#undef putText_ARGS
}

std::string thorlabs_cam::get_CameraInfo(void)
{
	std::string out;
	out = "Camera model: " + sensorname +
	"\nSensor area: (" +
	convert_Int2Str(sensor_aa_width) + " * " +
	convert_Int2Str(sensor_aa_height) +	") um^2\n" +
	"Pixel count: " +
	convert_Int2Str(im_aoi_width) + " * " +
	convert_Int2Str(im_aoi_height) +
	"\nPixel pitch: " + convert_Double2Str(dpix_size) + " um / pix";
	return out;
}

void thorlabs_cam::identify_CameraAOISettings(void)
{
	if(!sensorname.compare("DCC1240x") ||
		!sensorname.compare("DCC1240M") ||
		!sensorname.compare("DCC1240C") ||
		!sensorname.compare("DCC1240N") ||
		!sensorname.compare("DCC3240x") ||
		!sensorname.compare("DCC3240M") ||
		!sensorname.compare("DCC3240C") ||
		!sensorname.compare("DCC3240N"))
	{
		/* Sensor is
		e2v EV76C560ABT (monochrome) or
		e2v EV76C560ACT (color) or
		e2v EV76C661ABT (NIR). */
		im_min_width = 16;
		im_inc_width = 4;
		im_min_height = 4;
		im_inc_height = 2;
		sensor_aa_width = 6784;
		sensor_aa_height = 5427;
		if(!pix_size)
			pix_size = 530;
	}
	else if(!sensorname.compare("DCC1545M"))
	{
		/* Sensor is Aptina MT9M001. */
		im_min_width = 32;
		im_inc_width = 4;
		im_min_height = 4;
		im_inc_height = 2;
		sensor_aa_width = 6656;
		sensor_aa_height = 5325;
		if(!pix_size)
			pix_size = 520;
	}
	else if(!sensorname.compare("DCC1645C"))
	{
		/* Sensor is Aptina MT9M131. */
		im_min_width = 32;
		im_inc_width = 4;
		im_min_height = 4;
		im_inc_height = 2;
		sensor_aa_width = 4608; /* Exact sensitive area in um. */
		sensor_aa_height = 3686;
		if(!pix_size)
			pix_size = 360;
	}
	else if(!sensorname.compare("DCU223x") ||
			!sensorname.compare("DCU223C"))
	{
		/* Sensor is Sony ICX204AK. */
		im_min_width = 16;
		im_inc_width = 4;
		im_min_height = 120;
		im_inc_height = 2;
		sensor_aa_width = 4762;
		sensor_aa_height = 3571;
		if(!pix_size)
			pix_size = 465;
	}
	else if(!sensorname.compare("DCU223M"))
	{
		/* Sensor is Sony ICX204AL. */
		im_min_width = 16;
		im_inc_width = 4;
		im_min_height = 120;
		im_inc_height = 1;
		sensor_aa_width = 4762;
		sensor_aa_height = 3571;
		if(!pix_size)
			pix_size = 465;
	}
	else if(!sensorname.compare("DCU224C"))
	{
		/* Sensor is Sony ICX205AK. */
		im_min_width = 16;
		im_inc_width = 4;
		im_min_height = 120;
		im_inc_height = 2;
		sensor_aa_width = 5952;
		sensor_aa_height = 4762;
		if(!pix_size)
			pix_size = 465;
	}
	else if(!sensorname.compare("DCU224M"))
	{
		/* Sensor is Sony ICX205AL. */
		im_min_width = 16;
		im_inc_width = 4;
		im_min_height = 120;
		im_inc_height = 1;
		sensor_aa_width = 5952;
		sensor_aa_height = 4762;
		if(!pix_size)
			pix_size = 465;
	}
	else
		error_msg("error identifying the sensor name. " \
					"setting AOI is dangerous!", ERR_ARG);
}

void thorlabs_cam::set_CameraInfoWindowName(const std::string &name)
{
	infotbar_win_name = name;
}

std::string thorlabs_cam::get_CameraInfoWindowName(void)
{
	return infotbar_win_name;
}

void thorlabs_cam::show_CameraTrackbars(void)
{
	cv::imshow(infotbar_win_name, infotbar_win_mat);
}

void thorlabs_cam::TrackbarHandlerStartAOIWidth(int i)
{
	/* Get the current position. */
	assert(i == cv::getTrackbarPos(trck_name_aoi_sw, infotbar_win_name));
	/* Calculate the new one. */
	i -= i % im_inc_width;
	/* Set the new one. */
	if(i > 0)
	{
		im_aoi_width_start = i;
		cv::setTrackbarPos(trck_name_aoi_sw, infotbar_win_name, im_aoi_width_start);
	}
	/* Set the new total width. */
	const uint max_sze = im_max_width - im_aoi_width_start;
	if(im_aoi_width > max_sze)
		cv::setTrackbarPos(trck_name_aoi_ww, infotbar_win_name, max_sze);
	/* Update AOI. */
}

void thorlabs_cam::cast_static_SetTrackbarHandlerStartAOIWidth(int i, void *ptr)
{
	thorlabs_cam *cam = static_cast<thorlabs_cam *>(ptr);
	(*cam).TrackbarHandlerStartAOIWidth(i);
}

void thorlabs_cam::create_TrackbarStartAOIWidth(void)
{
	const int out_max = 100;
	static int setting = 0;

	cv::createTrackbar(trck_name_aoi_sw,
					infotbar_win_name,
					&setting,
					out_max,
					thorlabs_cam::cast_static_SetTrackbarHandlerStartAOIWidth,
					this);
}

void thorlabs_cam::cast_static_set_MouseEvent(const int event,
										const int x, const int y,
										const int flags, void *udata)
{
	thorlabs_cam *ptr = static_cast<thorlabs_cam *>(udata);
	(*ptr).set_MouseEvent(event, x, y, flags);
}

void thorlabs_cam::set_MouseEvent(const int event,
							const int x, const int y,
							const int flags)
{
	if(flags & cv::EVENT_FLAG_SHIFTKEY)
	{
		if(event == cv::EVENT_LBUTTONDOWN && !get_MouseDrag())
		{
			/* AOI selection begins. */
			set_RoiActive(false);
			set_EndRoi(cv::Point_<int>(x, y));
			set_StartRoi(cv::Point_<int>(x, y));
			set_MouseDrag(true);
		}
		else if(event == cv::EVENT_MOUSEMOVE && get_MouseDrag())
		{
			/* AOI being selected. */
			set_EndRoi(cv::Point_<int>(x, y));
		}
		else if(event == cv::EVENT_LBUTTONUP && get_MouseDrag())
		{
			set_EndRoi(cv::Point_<int>(x, y));
			int sx, sy;
			get_StartRoi(&sx, &sy);
			const int sw = x - sx,
			          sh = y - sy;
			set_RectRoi(cv::Rect_<int>(sx, sy, sw, sh));
			set_MouseDrag(false);
			if(sw <= 25 || sh <= 25 ||
				sx + abs(sw) >= (int)get_nCols() ||
				sy + abs(sh) >= (int)get_nRows())
				set_RoiActive(false);
			else
				set_RoiActive(true);
		}
	}
    else if(event == cv::EVENT_MOUSEMOVE || event == cv::EVENT_LBUTTONDOWN)
    {
    	set_MouseDrag(false);
		set_PixelValue(get_PixelValueWork(x, y));
		#if SHOW_MOUSE_CB
		iprint(stdout, "val: %g, pos: (%i, %i)\n", get_PixelValue(), x, y);
		#endif
		copy_MousePosition(x, y);
	}
	else if(event == cv::EVENT_RBUTTONDOWN)
	{
		#if SHOW_MOUSE_CB
		iprint(stdout, "r but: (%i, %i)\n", x, y);
		#endif
		set_PixelValue(0xDEADDEAD);
	}
	else if(event == cv::EVENT_MBUTTONDOWN)
	{
		#if SHOW_MOUSE_CB
		iprint(stdout, "m but: (%i, %i)\n", x, y);
		#endif
		set_PixelValue(0xDEADDEAD);
	}
	else if(event == cv::EVENT_MOUSEMOVE)
	{
		#if SHOW_MOUSE_CB
		iprint(stdout, "pos: (%i, %i)\n", x, y);
		#endif
		set_PixelValue(0xDEADDEAD);
	}
    else
	{
		copy_MousePosition(0, 0);
		set_PixelValue(0xDEADDEAD);
	}
}

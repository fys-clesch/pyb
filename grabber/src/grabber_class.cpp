#include "grabber_class.h"
#include <assert.h>

static grabber *g1ptr = nullptr;

grabber::grabber(void)
{
	g1ptr = static_cast<grabber *>(this);
	/* bool */
	grab_frames = true;
	bground_in =
	bground_appl =
	detct_settings =
	blur_appl =
	finite_moments =
	roi_on =
	mouse_drag =
	saturated = false; /* 9 */
	/* atomic<bool> */
	close_copy_thread.store(false, std::memory_order_relaxed);
	close_viewer_thread.store(false, std::memory_order_relaxed);
	wait_camera_thread.store(false, std::memory_order_relaxed);
	close_minime_thread.store(false, std::memory_order_relaxed); /* 4 */
	/* int */
	in_chn =
	in_depth = 0xDEADDEAD;
	px_mouse =
	py_mouse = 0; /* 4 */
	/* uint32_t */
	in_rows =
	in_cols = 0xDEADDEAD; /* 2 */
	/* atomic<uint32_t> */
	work_roi_rows.store(0, std::memory_order_relaxed);
	work_roi_cols.store(0, std::memory_order_relaxed);
	/* uint64_t */
	frms = 0; /* 1 */
	/* double */
	max_pval =
	pval =
	pix2um_scale =
	ell_ecc =
	ell_theta = 0xDEADDEAD;
	gaussblur_min = .25;
	gaussblur_max = 5.;
	gaussblur_sigma_x = 1.;
	groundlift_max = 40.;
	groundlift_sub = 0.; /* 10 */
	/* double * */
	work_roi_arr =
	work_roi_arr_buf =
	work_roi_arr_tflip_buf =
	gldata_buf = nullptr; /* 4 */
	/* atomic<double> */
	gaussblur_sigma_x_atm.store(gaussblur_sigma_x, std::memory_order_relaxed);
	groundlift_sub_atm.store(groundlift_sub, std::memory_order_relaxed);
	gaussblur_min_atm.store(gaussblur_min, std::memory_order_relaxed);
	gaussblur_max_atm.store(gaussblur_max, std::memory_order_relaxed);
	groundlift_max_atm.store(groundlift_max, std::memory_order_relaxed); /* 5 */
	/* Mat */
	covar = (Mat_<double>(2, 2) << 0., 0., 0., 0.);
	eigenv = (Mat_<double>(2, 1) << 0., 0.);
	beam_parameter = (Mat_<double>(3, 1) << 0., 0., 0.);
	tbar_win_mat = Mat::zeros(150, 350, CV_8UC3); /* 4 */
	/* Point2d */
	centroid = Point2d(0., 0.); /* 1 */
	/* Point_<int> */
	start_roi =
	end_roi = Point_<int>(0, 0);  /* 2 */
	/* string */
	main_win_name = "igyba - main window";
	tbar_win_name = "igyba - trackbar window"; /* 2 */
	/* uint */
	gaussblur_sze_min = 1;
	gaussblur_sze_max = 61; /* 2 */
	/* Size */
	gaussblur_sze = Size(3, 3); /* 1 */
	/* atomic<uint> */
	gaussblur_sze_atm.store(gaussblur_sze.width, std::memory_order_relaxed);
	gaussblur_sze_min_atm.store(gaussblur_sze_min, std::memory_order_relaxed);
	gaussblur_sze_max_atm.store(gaussblur_sze_max, std::memory_order_relaxed); /* 3 */
}

grabber::~grabber(void)
{
	in.release();

	fp_in.release();
	rgb.release();
	bground.release();
	work.release();
	work_roi.release();
	work_roi_tflip.release();
	temp_CV_32FC3.release();
	temp_CV_32FC1.release();
	temp_CV_16UC3.release();
	tbar_win_mat.release();
	covar.release();
	eigenv.release();

	if(work_roi_arr != nullptr)
		free(work_roi_arr);

	if(work_roi_arr_buf != nullptr)
		free(work_roi_arr_buf);

	if(work_roi_arr_tflip_buf != nullptr)
		free(work_roi_arr_tflip_buf);

	if(gldata_buf != nullptr)
		free(gldata_buf);

	covar.release();
	eigenv.release();
	beam_parameter.release();

	g1ptr = nullptr;

	#ifndef IGYBA_NDEBUG
	iprint(stdout, "'%s': memory released\n", __func__);
	#endif
}

void grabber::init_Mats(void)
{
	assert(in.data != NULL);

	in_chn = in.channels();
	in_depth = in.depth();

	switch(in_depth)
	{
		case CV_8U:
			max_pval = pow(2., 8) - 1.;
			break;
		case CV_8S:
			max_pval = pow(2., 7) - 1.;
			break;
		case CV_16U:
			max_pval = pow(2., 16) - 1.;
			break;
		case CV_16S:
			max_pval = pow(2., 15) - 1.;
			break;
		case CV_32S:
			max_pval = pow(2., 32) - 1.;
			break;
		case CV_32F:
			max_pval = FLT_MAX;
			break;
		case CV_64F:
			max_pval = DBL_MAX;
			break;
		default:
			error_msg("i'm probably colour-blind", ERR_ARG);
	}

	in_rows = in.rows;
	in_cols = in.cols;
	store_WorkRoiRows(in_rows);
	store_WorkRoiCols(in_cols);
	detct_settings = true;

	fp_in.create(in_rows, in_cols, mat_typ);
	work.create(in_rows, in_cols, mat_typ);
	work_roi.create(load_WorkRoiRows(),
					load_WorkRoiCols(),
					mat_typ);
	work_roi_tflip.create(load_WorkRoiCols(),
						load_WorkRoiRows(),
						mat_typ);
	bground.create(in_rows, in_cols, mat_typ);
	rgb.create(in_rows, in_cols, CV_8UC3);
	temp_CV_32FC3.create(in_rows, in_cols, CV_32FC3);
	temp_CV_32FC1.create(in_rows, in_cols, CV_32FC1);
	temp_CV_16UC3.create(in_rows, in_cols, CV_16UC3);
	work_roi_arr = alloc_matrix(load_WorkRoiRows(),
								load_WorkRoiCols(),
								0xDEADBEAF);
	work_roi_arr_buf = alloc_matrix(load_WorkRoiRows(),
									load_WorkRoiCols(),
									0xDEADBEAF);
	work_roi_arr_tflip_buf = alloc_matrix(load_WorkRoiCols(),
										load_WorkRoiRows(),
										0xDEADBEAF);
	gldata_buf = alloc_3matrix(load_WorkRoiRows(),
								load_WorkRoiCols(),
								0xDEADBEAF);
}

void grabber::set_Pix2UmScale(const double scl)
{
	pix2um_scale = scl;
}

void grabber::set_Background(void)
{
	if(in_chn == 1)
		in.convertTo(bground, mat_typ);
	else if(in_chn == 3)
	{
		Mat temp = in.clone();
		temp.convertTo(temp, CV_32FC3);
		cvtColor(temp, temp, CV_BGR2GRAY, 1);
		temp.convertTo(bground, mat_typ);
	}

	if(bground.type() != mat_typ)
		error_msg("trying to subtract a background image with a different Mat type", ERR_ARG);
	else
		bground_in = true;
}

void grabber::unset_Background(void)
{
	if(bground_in)
		bground = 0.;
	bground_in = false;
}

void grabber::update_Mats_RgbAndFp(void)
{
	if(!detct_settings)
		init_Mats();
	if(in_chn == 3)
	{
		in.convertTo(rgb, CV_8UC3);
		in.convertTo(temp_CV_32FC3, CV_32FC3);
		cvtColor(temp_CV_32FC3, temp_CV_32FC1, CV_BGR2GRAY, 1);
		if(temp_CV_32FC1.type() != CV_32FC1)
			warn_msg("Mat should be CV_32FC1. performance issues.", ERR_ARG);
	}
	else if(in_chn == 1)
	{
		in.convertTo(fp_in, mat_typ);
		double minv, maxv;
		minMaxLoc(in, &minv, &maxv);
		if(in.type() == CV_32F || in.type() == CV_64F)
		{
			in = in * 255. / maxv - minv; /**< This should not be done on
			integer-type Mat. */
			cvtColor(in, temp_CV_32FC3, CV_GRAY2BGR, 3);
			temp_CV_32FC3.convertTo(rgb, CV_8UC3);
		}
		else if(in.type() == CV_16U)
		{
			cvtColor(in, temp_CV_16UC3, CV_GRAY2BGR, 3);
			temp_CV_16UC3.convertTo(rgb, CV_8UC3);
		}
		else if(in.type() == CV_8U)
		{
			cvtColor(in, rgb, CV_GRAY2BGR, 3);
			if(rgb.type() != CV_8UC3)
				error_msg("Mat should be CV_8UC3. performance issues.",
							ERR_ARG);
		}
		else
		{
			error_msg("Mat type not implemented (yet). good bye.", ERR_ARG);
			exit(EXIT_FAILURE);
		}
	}
	if(fp_in.type() != mat_typ || rgb.type() != CV_8UC3)
		error_msg("wrong Mat type detected", ERR_ARG);
	produce_Mat_Work();
}

void grabber::produce_Mat_Work(void)
{
	if(bground_in)
	{
		work = fp_in - bground;
		double minv;
		minMaxLoc(work, &minv);
		if(minv < 0.)
			work = work - minv;
		bground_appl = true;
	}
	else
	{
		fp_in.copyTo(work);
		bground_appl = false;
	}
	uint unxm = in_rows,
	     unym = in_cols,
	     unx = unxm,
	     uny = unym;
	if(work.isContinuous())
	{
		uny *= unx;
		unx = 1;
	}
	if(blur_appl)
	{
		GaussianBlur(work, work,
					gaussblur_sze,
					gaussblur_sigma_x,
					gaussblur_sigma_x);
		if(groundlift_sub != 0.)
			for(uint i = 0; i < unx; ++i)
			{
				float *const m = work.ptr<float>(i);
				for(uint j = 0; j < uny; ++j)
					(m[j] <= groundlift_sub) ? m[j] = 0. :
						                       m[j] -= groundlift_sub;
			}
	}
	/* Process the ROI version of 'work'. */
	if(roi_on)
	{
		/* Attention here! The height is the number of rows. */
		store_WorkRoiRows(roi_rect.height);
		store_WorkRoiCols(roi_rect.width);
		work_roi = work(roi_rect).clone();
	}
	else
	{
		store_WorkRoiRows(in_rows);
		store_WorkRoiCols(in_cols);
		work.copyTo(work_roi);
	}
	/* The flipping takes place. */
	flip(work_roi.t(), work_roi_tflip, 1);
	unxm = load_WorkRoiRows();
	unym = load_WorkRoiCols();
	#ifndef IGYBA_NDEBUG
	if(roi_on) /**< Attention in the next line. */
		assert((int)unxm == roi_rect.height && (int)unym == roi_rect.width);
	assert((int)unxm == work_roi.rows && (int)unym == work_roi.cols);
	assert((int)unym == work_roi_tflip.rows && (int)unxm == work_roi_tflip.cols);
	#endif
	/** @todo If there's a bottleneck here, a parallelization can be introduced.
	 * One processor runs the normal matrix, the other one the flipped one.
	 * 2015-04-21: Implemented the following OpenMP section, but couldn't
	 * profile it with gprof - meaningless results. In addition, a (one) spurious
	 * error occurred, maybe because of a data race.
	 */
//	#pragma omp parallel sections private(uny, unx) firstprivate(unxm, unym)
//	{
//		#pragma omp section
//		{
			/* First treat the data in the normally ordered way. */
			if(work_roi.isContinuous())
			{
				uny = unxm * unym;
				unx = 1;
			}
			for(uint i = 0; i < unx; ++i)
			{
				const float *const m = work_roi.ptr<float>(i);
				for(uint j = 0; j < uny; ++j)
				{
					work_roi_arr_buf[i * uny + j] = (double)m[j];
					work_roi_arr[i * uny + j] = (double)m[j];
				}
			}
//		}
//		#pragma omp section
//		{
			/* Next treat the flipped data to be processed in the viewer. */
			if(work_roi_tflip.isContinuous())
			{
				uny = 1;
				unx = unxm * unym;
			}
			for(uint i = 0; i < uny; ++i)
			{
				const float *const m_tf = work_roi_tflip.ptr<float>(i);
				for(uint j = 0; j < unx; ++j)
					work_roi_arr_tflip_buf[i * unx + j] = (double)m_tf[j];
			}
//		}
//	}
}

/** \brief Calculates the first and second moments of the working matrix with
 * a specified ROI.
 *
 * \param void
 * \return void
 *
 * The code doesn't account for interleaving pixels.
 */
void grabber::get_Moments_own(void)
{
	double cen[2] = {0., 0.},
	       norm = 0.;
	const uint imax = load_WorkRoiRows(),
	           jmax = load_WorkRoiCols();
	for(uint i = 0; i < imax; i++)
	{
		const float *const m = work_roi.ptr<float>(i);
		for(uint j = 0; j < jmax; j++)
		{
			const double f = m[j];
			cen[0] += f * i;
			cen[1] += f * j;
			norm += f;
		}
	}
	if(fabs(norm) <= DBL_EPSILON)
	{
		warn_msg("norm is 0.", ERR_ARG);
		const double fscl = 1.5;
		putText(rgb, "norm is 0., no output",
				Point2d(.5 * in_cols, .5 * in_rows),
				FONT_HERSHEY_SIMPLEX, fscl,
				Scalar_<double>(255., 0., 0.), 1, CV_AA);
	}
	else
	{
		cen[0] /= norm;
		cen[1] /= norm;
		double rxx = 0., ryy = 0., rxy = 0.;
		for(uint i = 0; i < imax; i++)
		{
			const float *const m = work_roi.ptr<float>(i);
			for(uint j = 0; j < jmax; j++)
			{
				const double f = m[j],
							 x = i - cen[0],
							 y = j - cen[1];
				rxx += f * x * x;
				ryy += f * y * y;
				rxy += f * x * y;
			}
		}
		rxx /= norm;
		ryy /= norm;
		rxy /= norm;
		const double corr = rxy / (rxx * ryy);
		if(fabs(corr) > 1.)
		{
			iprint(stderr, "corr: %g\n", corr);
			warn_msg("the correlation is > 1.!", ERR_ARG);
		}
		/* xy is swapped: */
		{
			double temp = cen[1];
			cen[1] = cen[0];
			cen[0] = temp;
			temp = rxx;
			rxx = ryy;
			ryy = temp;
		}
		centroid = Point2d(cen[0], cen[1]);
		covar = (Mat_<double>(2, 2) << rxx, rxy, rxy, ryy);
		double eig[2],
			   temp = covar.at<double>(0, 0) - covar.at<double>(1, 1);
		if(fabs(temp) >= DBL_EPSILON)
			ell_theta = .5 * atan2(2. * rxy, temp) * 180. / M_PI;
		/* The eigenvalues of the covariance matrix give the variance along
		 * the main axis.
		 */
		eig[0] = eig[1] =
		.5 * (covar.at<double>(0, 0) + covar.at<double>(1, 1));
		temp = .5 * sqrt(4. * POW2(covar.at<double>(0, 1)) + POW2(temp));
		eig[0] += temp;
		eig[1] -= temp;
		eigenv.at<double>(0) = eig[0];
		eigenv.at<double>(1) = eig[1];

		eig[0] = sqrt(eig[0]);
		eig[1] = sqrt(eig[1]);
		if(eig[0] != 0. && eig[1] != 0.)
		{
			temp = eig[1] / eig[0];
			if(temp > 1.)
				ell_ecc = sqrt(1. - eig[0] / eig[1]);
			else
				ell_ecc = sqrt(1. - temp);
		}
		calculate_BeamRadius();
	}
}

/** \brief Calculates the first and second moment of the working matrix with
 * a specified ROI.
 *
 * \param void
 * \return void
 *
 * Calculation is done via calling OpenCV's 'moments' function. It might
 * be slower than 'get_Moments_own'.
 */
void grabber::get_Moments(void)
{
	Moments mom = moments(work_roi);

	centroid.x = mom.m10 / mom.m00;
	centroid.y = mom.m01 / mom.m00;
	double temp = mom.m11 / mom.m00 - centroid.x * centroid.y;
	covar = (Mat_<double>(2, 2) <<
	        mom.m20 / mom.m00 - POW2(centroid.x),
	        temp, temp,
	        mom.m02 / mom.m00 - POW2(centroid.y));

	temp = covar.at<double>(0, 0) - covar.at<double>(1, 1);

	if(!std::isfinite(temp) ||
		!std::isfinite(centroid.x) ||
		!std::isfinite(centroid.y))
		finite_moments = false;
	else
		finite_moments = true;

	if(fabs(temp) > DBL_EPSILON)
		ell_theta = .5 * atan2(2. * covar.at<double>(0, 1), temp) * 180. / M_PI;

	eigen(covar, eigenv);

	const double eig[2] = {
		sqrt(eigenv.at<double>(0)),
		sqrt(eigenv.at<double>(1))};

	if(eig[0] != 0. && eig[1] != 0.)
	{
		temp = eig[1] / eig[0];
		if(temp > 1.)
			ell_ecc = sqrt(1. - eig[0] / eig[1]);
		else
			ell_ecc = sqrt(1. - temp);
	}
	calculate_BeamRadius();
}

void grabber::draw_Moments(const bool chatty)
{
	if(chatty)
			iprint(stdout,
					"\ntheta / deg: %g\n" \
					"centroid / pix: (%g, %g)\n" \
					"covariance / pix^2: [[%g, %g], [%g, %g]]\n" \
					"eigenvalues / pix: %g, %g\n" \
					"beam radius along window axis / um: %g, %g\n" \
					"beam correlation / 1: %g\n" \
					"eccentricity / 1: %g\n",
					ell_theta,
					centroid.x, centroid.y,
					covar.at<double>(0, 0), covar.at<double>(0, 1),
					covar.at<double>(1, 0), covar.at<double>(1, 1),
					sqrt(eigenv.at<double>(0)), sqrt(eigenv.at<double>(1)),
					beam_parameter.at<double>(0), beam_parameter.at<double>(1),
					beam_parameter.at<double>(2),
					ell_ecc);

	const Scalar_<double> clr_ell(100., 100., 255.);
	uint16_t lw = 2;
	static Mat scl_sqrt_eigenv(eigenv.size(), eigenv.depth());
	sqrt(eigenv, scl_sqrt_eigenv);
	const Size_<double> minmaj_ax = Size_<double>(scl_sqrt_eigenv.at<double>(0),
												scl_sqrt_eigenv.at<double>(1));
	/* The square root of the eigenvalues, multiplied by the pixel to
	 * micro meter factor, gives the measure of the major and minor axis of
	 * the shown ellipse. However, this measure does not match the beam radius
	 * definition, which has an additional scaling factor of 2.
	 */
	scl_sqrt_eigenv *= pix2um_scale * 2.;

	if(!finite_moments)
	{
		iprint(stdout, "moments are not finite, can't draw an ellipse\n");
		Point p = Point(in_cols >> 1, in_rows >> 1);
		#if DRAW_NOT_FINITE_SIGN_ATOM
		Size s = Size(in_cols / 4, in_cols / 16);
		ellipse(rgb, p, s, 90, 0., 360., clr_ell, lw, CV_AA);
		ellipse(rgb, p, s, 0, 0., 360., clr_ell, lw, CV_AA);
		ellipse(rgb, p, s, 45, 0., 360., clr_ell, lw, CV_AA);
		ellipse(rgb, p, s, -45, 0., 360., clr_ell, lw, CV_AA);
		circle(rgb, p, 5., clr_ell, lw, CV_AA);
		#else
		circle(rgb, p, in_cols / 6, clr_ell, 4, CV_AA);
		const static Point pl = Point(p.x - in_cols / 16, in_rows / 2.4),
		                   pr = Point(p.x + in_cols / 16, pl.y),
		                   pfrown = Point(p.x, in_rows / 1.6);
		circle(rgb, pl, 3., clr_ell, 4, CV_AA);
		circle(rgb, pr, 3., clr_ell, 4, CV_AA);
		const Size sfrown = Size(in_cols / 9, in_cols / 18);
		ellipse(rgb, pfrown, sfrown, 0., -10., -170., clr_ell, 4, CV_AA);
		#endif
	}
	else if(saturated)
	{
		const uint shiftp = 15;
		const static Point p = Point(in_cols >> 1, in_rows >> 1),
		                   pl = Point(p.x - in_cols / 16, in_rows / 2.4),
		                   pl0 = Point(pl.x - shiftp, pl.y - shiftp),
		                   pl1 = Point(pl.x - shiftp, pl.y + shiftp),
		                   pl2 = Point(pl.x + shiftp, pl.y - shiftp),
		                   pl3 = Point(pl.x + shiftp, pl.y + shiftp),
		                   pr = Point(p.x + in_cols / 16, pl.y),
		                   pr0 = Point(pr.x - shiftp, pr.y - shiftp),
		                   pr1 = Point(pr.x - shiftp, pr.y + shiftp),
		                   pr2 = Point(pr.x + shiftp, pr.y - shiftp),
		                   pr3 = Point(pr.x + shiftp, pr.y + shiftp);
		circle(rgb, p, in_cols / 6, clr_ell, 4, CV_AA);
		line(rgb, pl0, pl3, clr_ell, 4, CV_AA);
		line(rgb, pl1, pl2, clr_ell, 4, CV_AA);
		line(rgb, pr0, pr3, clr_ell, 4, CV_AA);
		line(rgb, pr1, pr2, clr_ell, 4, CV_AA);
		const uint mh = 35,
		           mw = 20;
		const static Point pmouth = Point(p.x, in_rows / 1.75),
		                   pf0 = Point(pmouth.x, pmouth.y),
		                   pf1 = Point(pf0.x + mw, pf0.y + mh),
		                   pf2 = Point(pf1.x + mw, pf1.y - mh),
		                   pf3 = Point(pf2.x + mw, pf2.y + mh),
		                   pf4 = Point(pf3.x + mw, pf3.y - mh),
		                   pf1m = Point(pf0.x - mw, pf0.y + mh),
		                   pf2m = Point(pf1m.x - mw, pf1m.y - mh),
		                   pf3m = Point(pf2m.x - mw, pf2m.y + mh),
		                   pf4m = Point(pf3m.x - mw, pf3m.y - mh);
		rectangle(rgb, pf0, pf1, clr_ell, 4, CV_AA);
		rectangle(rgb, pf1, pf2, clr_ell, 4, CV_AA);
		rectangle(rgb, pf2, pf3, clr_ell, 4, CV_AA);
		rectangle(rgb, pf3, pf4, clr_ell, 4, CV_AA);

		rectangle(rgb, pf0, pf1m, clr_ell, 4, CV_AA);
		rectangle(rgb, pf1m, pf2m, clr_ell, 4, CV_AA);
		rectangle(rgb, pf2m, pf3m, clr_ell, 4, CV_AA);
		rectangle(rgb, pf3m, pf4m, clr_ell, 4, CV_AA);
	}
	else
	{
		static Point2d cen_draw;
		if(roi_on)
		{
			cen_draw.x = centroid.x + roi_rect.x;
			cen_draw.y = centroid.y + roi_rect.y;
		}
		else
			cen_draw = centroid;
		ellipse(rgb, cen_draw, minmaj_ax, ell_theta, 0., 360.,
				clr_ell, lw, CV_AA);
		circle(rgb, cen_draw, 1., clr_ell, lw, CV_AA);
	}

	double temp = 1. / 32. * in_rows;
	const double incy = 1. / 32. * in_rows,
				 sx = 1. / 128. * in_cols,
				 fscl = .45;
	const int font = FONT_HERSHEY_SIMPLEX;
	Scalar_<double> clr_txt(0., 255., 0.);
	lw = 1;

	static std::string infos(128, '\0');

	#define putText_ARGS Point2d(sx, temp), font, fscl, clr_txt, lw, CV_AA
	infos = "theta / deg: " + convert_Double2Str(ell_theta);
	putText(rgb, infos, putText_ARGS);
	temp += incy;
	infos = "eccentric / 1: " + convert_Double2Str(ell_ecc);
	putText(rgb, infos, putText_ARGS);
	temp += incy;
	infos = "centroid / pix: (" +
			convert_Double2Str(centroid.x) + ", " +
			convert_Double2Str(centroid.y) + ")";
	putText(rgb, infos, putText_ARGS);
	temp += incy;
	infos = "covariance / pix^2: [[" +
			convert_Double2Str(covar.at<double>(0, 0)) + ", " +
			convert_Double2Str(covar.at<double>(0, 1)) + "], [" +
			convert_Double2Str(covar.at<double>(1, 0)) + ", " +
			convert_Double2Str(covar.at<double>(1, 1)) + "]]";
	putText(rgb, infos, putText_ARGS);
	temp += incy;
	putText(rgb, "> beam radius", putText_ARGS);
	temp += incy;
	infos = "main axes / um: " +
			convert_Double2Str(scl_sqrt_eigenv.at<double>(0)) + ", " +
			convert_Double2Str(scl_sqrt_eigenv.at<double>(1));
	putText(rgb, infos, putText_ARGS);
	temp += incy;
	infos = "global axes / um: " +
			convert_Double2Str(beam_parameter.at<double>(0)) + ", " +
			convert_Double2Str(beam_parameter.at<double>(1));
	putText(rgb, infos, putText_ARGS);

	clr_txt = Scalar_<double>(255., 0., 0.);
	temp += incy;
	if(in_cols <= sqrt(covar.at<double>(0, 0)))
		putText(rgb, "sqrt(covar[0, 0]) > columns", putText_ARGS);
	temp += incy;
	if(in_rows <= sqrt(covar.at<double>(1, 1)))
		putText(rgb, "sqrt(covar[1, 1]) > rows", putText_ARGS);
	#undef putText_ARGS
}

/** \brief Draws information into the 'rgb' matrix.
 *
 * \param void
 * \return void
 *
 */
void grabber::draw_Info(void)
{
	/* Clear the trackbar window first. */
	tbar_win_mat.setTo(0);
	/* Some definitions. */
	const uint16_t lw = 1;
	double fscl = .45;
	const int font = FONT_HERSHEY_SIMPLEX;
	const Scalar_<double> clr_txt(0., 255., 0.);
	static std::string infos(128, '\0');
	#define putText_ARGS Point2d(sx, sy), font, fscl, clr_txt, lw, CV_AA
	double sx = 1. / 128. * in_cols,
	       sy = in_rows - 10.;
	/* Print the value under the mouse pointer. */
	if(pval != 0xDEADDEAD)
	{
		infos = "(" + convert_Int2Str(px_mouse) + ", " +
		convert_Int2Str(py_mouse) + "): " +
		convert_Double2Str(pval);
		putText(rgb, infos, putText_ARGS);
	}
	else
		putText(rgb, "out of focus", putText_ARGS);
	draw_RoiRectangle();
	/* Information regarding too high pixel values. */
	const double minval = max_pval,
	             maxval = max_pval;
	static Mat mmmat(in_rows, in_cols, CV_8UC1);
	inRange(fp_in, minval, maxval, mmmat);
	uint count_nz = countNonZero(mmmat);
	if(count_nz > saturated_thresh)
		saturated = true;
	else
		saturated = false;
	sx = 1. / 128. * in_cols;
	sy = in_rows >> 1;
	fscl = .6;
	infos = "# pixels at max: " + convert_Int2Str(count_nz);
	putText(rgb, infos, putText_ARGS);
	fscl = .45;
	#ifndef IGYBA_NDEBUG
	iprint(stdout, "# pixels at %g: %u\n",
			       max_pval, count_nz);
	#endif
	/* Information regarding the blurring. */
	if(blur_appl)
	{
		static std::string blur_info(128, '\0'),
		                   lift_info(128, '\0');

		blur_info = "blur matrix size: (" +
		convert_Int2Str(gaussblur_sze.width) + ", " +
		convert_Int2Str(gaussblur_sze.height) + ") " +
		"sigma: " + convert_Double2Str(gaussblur_sigma_x);
		lift_info = "groundlift: " + convert_Double2Str(groundlift_sub);

		sx = 10.;
		sy = tbar_win_mat.rows - 10.;
		putText(tbar_win_mat, blur_info, putText_ARGS);
		sy -= 20.;
		putText(tbar_win_mat, lift_info, putText_ARGS);
	}
	else
	{
		sx = 10.;
		sy = tbar_win_mat.rows - 10;
		putText(tbar_win_mat, "no blurring", putText_ARGS);
	}
	/* Information regarding the background correction. */
	sy = tbar_win_mat.rows - 10. - 2. * 20.;
	if(bground_appl)
		putText(tbar_win_mat, "background correction applied", putText_ARGS);
	else if(bground_in)
		putText(tbar_win_mat, "background loaded, not applied", putText_ARGS);
	else
		putText(tbar_win_mat, "no background correction", putText_ARGS);
	/* Information regarding the ROI. */
	sy = tbar_win_mat.rows - 10. - 3. * 20.;
	if(!roi_on && !mouse_drag)
		putText(tbar_win_mat, "no ROI selected", putText_ARGS);
	else if(mouse_drag)
		putText(tbar_win_mat, "selecting ROI...", putText_ARGS);
	else
	{
		infos = "(start / span) AOI width: (" +
		convert_Int2Str(roi_rect.x) + ", " +
		convert_Int2Str(roi_rect.width) + ") pixel";
		putText(tbar_win_mat, infos, putText_ARGS);
		sy -= 20.;
		infos = "(start / span) AOI height: (" +
		convert_Int2Str(roi_rect.y) + ", " +
		convert_Int2Str(roi_rect.height) + ") pixel";
		putText(tbar_win_mat, infos, putText_ARGS);
	}
	#undef putText_ARGS
}

/** \brief Same as draw_Info(void) but boiled-down for wxWidgets.
 *
 * \param void
 * \return void
 *
 */
void grabber::draw_InfoWxVersion(void)
{
	/* Some definitions. */
	const uint16_t lw = 1;
	double fscl = .45;
	const int font = FONT_HERSHEY_SIMPLEX;
	const Scalar_<double> clr_txt(0., 255., 0.);
	static std::string infos(128, '\0');
	#define putText_ARGS Point2d(sx, sy), font, fscl, clr_txt, lw, CV_AA
	double sx = 1. / 128. * in_cols,
	       sy = in_rows - 10.;
	/* Print the value under the mouse pointer. */
	if(pval != 0xDEADDEAD)
	{
		infos = "(" + convert_Int2Str(px_mouse) + ", " +
		convert_Int2Str(py_mouse) + "): " +
		convert_Double2Str(pval);
		putText(rgb, infos, putText_ARGS);
	}
	else
		putText(rgb, "out of focus", putText_ARGS);
	draw_RoiRectangle();
	/* Information regarding too high pixel values. */
	const double minval = max_pval,
	             maxval = max_pval;
	static Mat mmmat(in_rows, in_cols, CV_8UC1);
	inRange(fp_in, minval, maxval, mmmat);
	uint count_nz = countNonZero(mmmat);
	if(count_nz > saturated_thresh)
		saturated = true;
	else
		saturated = false;
	sx = 1. / 128. * in_cols;
	sy = in_rows >> 1;
	fscl = .6;
	infos = "# pixels at max: " + convert_Int2Str(count_nz);
	putText(rgb, infos, putText_ARGS);
	fscl = .45;
	#ifndef IGYBA_NDEBUG
	iprint(stdout, "# pixels at %g: %u\n",
			       max_pval, count_nz);
	#endif
	#undef putText_ARGS
}

void grabber::draw_RoiRectangle(void)
{
	if(mouse_drag)
		rectangle(rgb, start_roi, end_roi, Scalar(100., 100., 50.), 2);
	else if(roi_on)
		rectangle(rgb, roi_rect, Scalar(150., 100., 50.), 2);
}

void grabber::set_MouseEvent(const int event,
							const int x, const int y,
							const int flags)
{
	if(flags & EVENT_FLAG_SHIFTKEY)
	{
		if(event == EVENT_LBUTTONDOWN && !mouse_drag)
		{
			/* AOI selection begins. */
			roi_on = false;
			end_roi = start_roi = Point_<int>(x, y);
			mouse_drag = true;
		}
		else if(event == EVENT_MOUSEMOVE && mouse_drag)
		{
			/* AOI being selected. */
			end_roi = Point_<int>(x, y);
		}
		else if(event == EVENT_LBUTTONUP && mouse_drag)
		{
			end_roi = Point_<int>(x, y);
			roi_rect = Rect_<int>(start_roi.x, start_roi.y,
									x - start_roi.x,
									y - start_roi.y);
			mouse_drag = false;
			if(roi_rect.width <= 25 || roi_rect.height <= 25 ||
				roi_rect.x + abs(roi_rect.width) >= (int)in_cols ||
				roi_rect.y + abs(roi_rect.height) >= (int)in_rows)
				roi_on = false;
			else
				roi_on = true;
		}
	}
    else if(event == EVENT_MOUSEMOVE || event == EVENT_LBUTTONDOWN)
    {
    	mouse_drag = false;
    	px_mouse = x;
    	py_mouse = y;
		pval = (double)work.at<float>(py_mouse, px_mouse);
		#if SHOW_MOUSE_CB
		iprint(stdout, "val: %g, pos: (%i, %i)\n", pval, px_mouse, py_mouse);
		#endif
	}
	else if(event == EVENT_RBUTTONDOWN)
	{
		#if SHOW_MOUSE_CB
		iprint(stdout, "r but: (%i, %i)\n", x, y);
		#endif
		pval = 0xDEADDEAD;
	}
	else if(event == EVENT_MBUTTONDOWN)
	{
		#if SHOW_MOUSE_CB
		iprint(stdout, "m but: (%i, %i)\n", x, y);
		#endif
		pval = 0xDEADDEAD;
	}
	else if(event == EVENT_MOUSEMOVE)
	{
		#if SHOW_MOUSE_CB
		iprint(stdout, "pos: (%i, %i)\n", x, y);
		#endif
		pval = 0xDEADDEAD;
	}
    else
	{
		px_mouse = py_mouse = 0;
		pval = 0xDEADDEAD;
	}
}

void grabber::cast_static_set_MouseEvent(const int event,
										const int x, const int y,
										const int flags, void *udata)
{
	grabber *ptr = static_cast<grabber *>(udata);
	(*ptr).set_MouseEvent(event, x, y, flags);
}

void grabber::show_Im_RGB(void)
{
	imshow(main_win_name, rgb);
}

void grabber::show_Trackbars(void)
{
	imshow(tbar_win_name, tbar_win_mat);
}

Mat grabber::get_Mat_private(const save_Im_type mtype)
{
	if(mtype == save_Im_type::RGB)
		return rgb;
	else if(mtype == save_Im_type::WORK)
		return work;
	else if(mtype == save_Im_type::FP_IN)
		return fp_in;
	else
	{
		error_msg("wrong enumerator", ERR_ARG);
		return Mat::eye(in_rows, in_cols, mat_typ);
	}
}

void grabber::save_Image(const save_Im_type mtype,
						const std::string &fname,
						const std::string &fmt)
{
	std::string str, strc;
	if(fname.empty())
		get_DateAndTime(str);
	else
		str = fname;
	/* If necessary, remove the last '.fmt'. This makes sure that there are
		no files like abc.png.png. */
	const size_t found = str.rfind("." + fmt);
	if(found != std::string::npos)
		str.erase(str.begin() + found, str.end());
	/* Continue. */
	strc = str + "_info." + fmt;
	str += "." + fmt;
	iprint(stdout, "storing to '%s' . ", str.c_str());
	fflush(stdout);
	if(mtype == save_Im_type::RGB)
		imwrite(str, rgb);
	else if(mtype == save_Im_type::WORK)
		imwrite(str, work);
	else if(mtype == save_Im_type::FP_IN)
		imwrite(str, fp_in);
	else
		error_msg("wrong enumerator", ERR_ARG);

	draw_Info();
	imwrite(strc, tbar_win_mat);
	iprint(stdout, "done\n");
}

void grabber::store_Image(const save_Im_type mtype,
						const std::string &fname)
{
	std::string str;
	if(fname.empty())
	{
		get_DateAndTime(str);
		if(mtype == save_Im_type::RGB)
			str += "_rgb";
		else if(mtype == save_Im_type::WORK)
			str += "_work";
		else if(mtype == save_Im_type::FP_IN)
			str += "_fp";
		else
			error_msg("wrong enumerator", ERR_ARG);
		str = str + ".dat";
	}
	else
		str = fname;
	iprint(stdout, "storing to '%s' . ", str.c_str());
	fflush(stdout);
	FILE *writefile = fopen(str.c_str(), "w");
	if(writefile == NULL)
	{
		file_error_msg(str.c_str(), ERR_ARG);
		exit(EXIT_FAILURE);
	}

	if(mtype == save_Im_type::RGB)
		for(uint i = 0; i < (uint)rgb.rows; i++)
			for(uint j = 0; j < (uint)rgb.cols; j++)
			{
				const Vec3b m = rgb.at<Vec3b>(i, j);
				fprintf(writefile,
							"%u %u %hhu %hhu %hhu%s",
							i, j, m[0], m[1], m[2],
							j < (uint)rgb.cols - 1 ? "\n" : "\n\n");
			}
	else if(mtype == save_Im_type::WORK)
		for(uint i = 0; i < (uint)work.rows; i++)
		{
			const float *const m = work.ptr<float>(i);
			for(uint j = 0; j < (uint)work.cols; j++)
				fprintf(writefile,
							"%g%c",
							m[j],
							j < (uint)work.cols - 1 ? ' ' : '\n');
		}
	else if(mtype == save_Im_type::FP_IN)
		for(uint i = 0; i < (uint)fp_in.rows; i++)
		{
			const float *const m = fp_in.ptr<float>(i);
			for(uint j = 0; j < (uint)fp_in.cols; j++)
				fprintf(writefile,
							"%g%c",
							m[j],
							j < (uint)fp_in.cols - 1 ? ' ' : '\n');
		}
	else
		error_msg("wrong enumerator", ERR_ARG);

	iprint(stdout, "done\n");
	fclose(writefile);
}

void grabber::show_Help(void)
{
	#define putText_ARGS Point2d(sx, sy), font, fscl, clr_txt, lw, CV_AA
	static const std::string win_title = "Help me!";
	const double fscl = .45, wwidth = 800., wheight = 600.;;
	const int font = CV_FONT_HERSHEY_SIMPLEX;
	const uint16_t lw = 1, lt = CV_AA;
	const Scalar_<double> clr_txt(0., 255., 0.);
	double sx,
	       sy,
	       syinc1 = 15.,
	       syinc2 = 20.,
	       sxstart1 = 10.,
	       sxstart2 = 50.,
	       sxstart3 = 70.;
	namedWindow(win_title, CV_WINDOW_AUTOSIZE);
	Mat img(Size(wwidth, wheight), CV_8UC3, Scalar::all(0));
	{
		static const std::string text =
		"Welcome to the help function of 'igyba'";

		int baseline = 0;
		Size textSize = getTextSize(text, font, fscl, lw, &baseline);
		/* Centre the text. */
		Point textOrg((img.cols - textSize.width) >> 1,
						(img.rows + textSize.height) >> 1);

		sx = textOrg.x;
		sy = 15.;
		putText(img, text, putText_ARGS);
		sy = 20.;
		line(img, Point(sx, sy), Point(sx + 280., sy), clr_txt, lw, lt);
	}
	sx = sxstart1;
	sy = 45.;
	putText(img, "Keys that are supposed to work:", putText_ARGS);

	#define TO_NEXT_KEY sy += syinc2; sx = sxstart1;
	sy += 20.;
	putText(img, " 'r':", putText_ARGS);
	sx = sxstart2;
	putText(img, "saves the image that's grabbed when pressing 'r' and", putText_ARGS);
	sy += syinc1;
	putText(img, "subtracts it from the proceeding captures", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " 'R':", putText_ARGS);
	sx = sxstart2;
	putText(img, "if a background correction is applied, this removes it", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " 's':", putText_ARGS);
	sx = sxstart2;
	putText(img, "save the image that's currently displayed to the active directory", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " 'S':", putText_ARGS);
	sx = sxstart2;
	putText(img, "save the 'WORK' matrix as an image", putText_ARGS);
	sy += syinc1;
	putText(img, "to the active directory", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " 'o':", putText_ARGS);
	sx = sxstart2;
	putText(img, "toggles between a blurring of the acquired data", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " 'X':", putText_ARGS);
	sx = sxstart2;
	putText(img, "switches on the OpenGL rendering", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " 'F':", putText_ARGS);
	sx = sxstart2;
	putText(img, "starts a fitting procedure.", putText_ARGS);
	sy += syinc1;
	putText(img, "be sure that the input looks at least like a Gaussian.", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " 'p':", putText_ARGS);
	sx = sxstart2;
	putText(img, "toggle pause", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " 'P':", putText_ARGS);
	sx = sxstart2;
	putText(img, "plots the 'WORK' matrix via gnuplot", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " '1':", putText_ARGS);
	sx = sxstart2;
	putText(img, "save the 'RGB' matrix as an int-valued data file - (x y R G B) format", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " '2':", putText_ARGS);
	sx = sxstart2;
	putText(img, "save the 'WORK' matrix as a fp-valued data file - matrix format", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " '3':", putText_ARGS);
	sx = sxstart2;
	putText(img, "save the 'FP_IN' matrix as a fp-valued data file - matrix format", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " '0':", putText_ARGS);
	sx = sxstart2;
	putText(img, "reset the window size to the image size", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " 'F1':", putText_ARGS);
	sx = sxstart3;
	putText(img, "display this beautiful help text. press 'F1' if it's enough.", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " 'Esc':", putText_ARGS);
	sx = sxstart3;
	putText(img, "gets you out of here", putText_ARGS);
	TO_NEXT_KEY
	putText(img, " 'Shift':", putText_ARGS);
	sx = sxstart3;
	putText(img, "drag the mouse and specify a ROI. draw a small ROI to get rid of it.", putText_ARGS);
	#undef TO_NEXT_KEY

	sx = sxstart1;
	sy = wheight - 10.;
	putText(img, "clesch@fysik.dtu.dk takes the blame.", putText_ARGS);

	imshow(win_title, img);
	while(true)
	{
		uint32_t kctrl = waitKey(0);
		if(kctrl == 7340032 || kctrl == 1114046) /**< The latter happens on some
			builds of OpenCV. */
			break;
	}
	destroyWindow(win_title);
	#undef putText_ARGS
}

void grabber::show_Intro(void)
{
	#define putText_ARGS Point2d(sx, sy), font, fscl, clr_txt, lw, CV_AA
	static const std::string win_title = "Welcome";
	const double fscl = .45, wwidth = 550., wheight = 350.;
	const int font = CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC;
	const uint16_t lw = 1,
	               lt = CV_AA;
	const Scalar_<double> clr_txt(0., 255., 0.);
	double sx, sy;
	namedWindow(win_title, CV_WINDOW_AUTOSIZE);
	Mat img(Size(wwidth, wheight), CV_8UC3, Scalar::all(0.));
	{
		static const std::string text = "igyba -- igyba gets your beam analysed";

		int baseline = 0;
		Size textSize = getTextSize(text, font, fscl, lw, &baseline);
		/* Centre the text. */
		Point textOrg((img.cols - textSize.width) >> 1, (img.rows + textSize.height) >> 1);

		sx = textOrg.x;
		sy = 15.;
		putText(img, text, putText_ARGS);
		sy = 20.;
		line(img, Point(sx, sy), Point(sx + 300., sy), clr_txt, lw, lt);
	}
	sx = wwidth - 80.;
	sy = wheight - 25.;
	putText(img, "2015 RC2", putText_ARGS);
	sy = wheight - 5.;
	putText(img, "GPLv3", putText_ARGS);

	uint64_t seed = time(NULL);
	RNG rng(seed);
	if(true)
	{
		Point p1(wwidth * .5, wheight * .5);
		for(uint i = 0; i < 200; i++)
		{
			const Point p2(rng.gaussian(30.) + wwidth * .5,
							rng.gaussian(30.) + wheight * .5);
			const Scalar_<uint16_t> color = Scalar(rng.uniform(0, 255),
													rng.uniform(0, 255),
													rng.uniform(0, 255));
			line(img, p1, p2, color, lw, lt);
			p1 = p2;
		}
	}
	else
	{
		vector<Point> contour;
		for(uint i = 0; i < 100; i++)
		{
			double x = rng.gaussian(10.) + wwidth * .5,
				   y = rng.gaussian(10.) + wheight * .5;
			contour.push_back(Point(x, y));
		}
		const Point *pts = (const Point *)Mat(contour).data;
		const int npts = Mat(contour).rows;
		const bool draw_clsd = true;
		polylines(img, &pts, &npts, 1, draw_clsd, clr_txt, lw, lt);
	}

	imshow(win_title, img);
	waitKey(1200);
	destroyWindow(win_title);
	#undef putText_ARGS
}

#if USE_CURSES
#if !defined(__unix__) || !defined(__LINUX__)
#include "curses.h"
void grabber::show_HelpOnCurses(void)
{
#ifdef XCURSES
	Xinitscr(argc, argv);
#else
	initscr();
#endif
	nodelay(stdscr, TRUE);
	noecho();
	/** colours available:
	 * COLOR_BLACK, COLOR_RED, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN, COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE
	 */
	const int nwidth = 100, nheight = 50; /* owidth = COLS, oheight = LINES */
	resize_term(nheight, nwidth);
	if(has_colors())
	{
		start_color();
		uint i = 1;
		init_pair(i++, COLOR_GREEN, COLOR_BLACK);
		init_pair(i++, COLOR_GREEN, COLOR_WHITE);
		init_pair(i++, COLOR_RED, COLOR_BLACK);
	}
	while(getch() == ERR)
	{
		attrset(COLOR_PAIR(1) | A_NORMAL);
		mvaddstr(1, 1, "Welcome to the help function of 'igyba'");
		mvaddstr(2, 1, "---------------------------------------");
		mvaddstr(4, 1, "Following keys will get you a response:");
	}
	clear();
	endwin();
}
#endif
#endif

void grabber::draw_Crossline(void)
{
	Point2d pt1, pt2;
	LineIterator lit(work, pt1, pt2)
	#ifndef IGYBA_NDEBUG
	             , lit2 = lit;
	#else
	;
	#endif

	vector<float> buf;
	buf.reserve(lit.count);

	for(int i = 0; i < lit.count; i++, ++lit)
	{
		const uchar *ptr = *lit;
		buf.push_back((float)*ptr);
	}

	#ifndef IGYBA_NDEBUG
	for(int i = 0; i < lit2.count; i++, ++lit2)
	{
		float val = work.at<float>(lit2.pos());
		assert(buf[i] == val);
	}
	#endif
}

void grabber::apply_RemoveBase(const double thresh)
{
    if(thresh != 0.)
	{
		uint unx = in_rows,
		     uny = in_cols;
		if(work.isContinuous())
		{
			uny *= unx;
			unx = 1;
		}
		for(uint i = 0; i < unx; i++)
		{
			float *const m = work.ptr<float>(i);
			for(uint j = 0; j < uny; j++)
				if(m[j] < thresh)
					m[j] = 0.;
		}
	}
	else
	{
		Mat cropped = work(Rect(0, 0, 10, 10)).clone(); /**< x0, y0, x1, y1 */
		const Scalar_<double> sum_cropped = sum(cropped);
		const double non0_cropped = (double)countNonZero(cropped),
		             res = sum_cropped[0] / non0_cropped;
		work -= res;
	}
}

void grabber::set_MainWindowName(const std::string &name)
{
	main_win_name = name;
}

void grabber::set_TrackbarWindowName(const std::string &name)
{
	tbar_win_name = name;
}

Mat grabber::get_TrackbarWindowMat(void)
{
	return tbar_win_mat;
}

Mat &grabber::get_TrackbarWindowMatPtr(void)
{
	return tbar_win_mat;
}

void grabber::get_MainWindowName(std::string &name)
{
	name = main_win_name;
}

std::string grabber::get_MainWindowName(void)
{
	return main_win_name;
}

void grabber::get_TrackbarWindowName(std::string &name)
{
	name = tbar_win_name;
}

std::string grabber::get_TrackbarWindowName(void)
{
	return tbar_win_name;
}

uint32_t grabber::get_nRows(void)
{
	return in_rows;
}

double grabber::get_PixelValue(void)
{
	return pval;
}

void grabber::set_PixelValue(const double val)
{
	pval = val;
}

uint32_t grabber::get_nCols(void)
{
	return in_cols;
}

uint32_t grabber::get_nRowsROI(void)
{
	return work_roi_rows.load();
}

uint32_t grabber::get_nColsROI(void)
{
	return work_roi_cols.load();
}

/** \brief This function serves to fill the allocated memory with data. It is
called by the copy thread.
 *
 * \param nrows const uint
 * \param ncols const uint
 * \return void
 *
 * Watch out that there is no data race, as the thread is not blocking the
 * viewer.
 *
 */
void grabber::fill_DataForViewer(const uint nrows, const uint ncols)
{
	if(!beau.load_FilledMemory()) /**< This has to be done only when
		 i.e. after the memory is allocated and filled. */
		beau.init_Colorbox();

	double max_v,
	       min_v,
	       max_norm,
	       min_norm,
	       *data_buf = gldata_buf,
	       *rgb_buf = alloc_3mat(nrows, ncols);

	viewer::calc_DrawingData(work_roi_arr_tflip_buf, true,
							&max_v, &min_v,
							&max_norm, &min_norm,
							nrows, ncols,
							data_buf, rgb_buf);
	/* Tell the viewer that it can wait for new data. */
	beau.store_NewDataAvailable(true);
	/* Check if the viewer is alive... */
	if(is_ViewerWindowRunning())
		/* ...and waiting: */
		if(beau.event_SwapDataToViewer.check())
		{
			/* Then set the fresh data over. */
			beau.set_DrawingData(&max_v, &min_v, &max_norm, &min_norm,
									data_buf, rgb_buf);
			beau.store_FilledMemory(true);
			beau.store_NewDataAvailable(false);
			/* Let the viewer run again. */
			beau.event_SwapDataToViewer.signal();
		}
	free(rgb_buf);
}

/** \brief The thread launched before starting the main loop to copy the data
constantly to the viewer memory.
 *
 * \param void
 * \return void
 *
 */
void grabber::copy_DataToViewer(void)
{
	uint nrows, /**< The variables have to be outside of the inner scope. */
	     ncols;
	while(true)
	{
		/* Wait for the main loop to have data ready: */
		(*g1ptr).event_CopyToViewer.wait();
		/* In case we want to leave the whole application: */
		if((*g1ptr).close_copy_thread.load(std::memory_order_relaxed))
		{
			(*g1ptr).event_CopyToViewer.reset();
			break;
		}
		/* Watch out, the cv matrix is transposed! */
		nrows = (*g1ptr).load_WorkRoiCols();
		ncols = (*g1ptr).load_WorkRoiRows();
		/* First check if the viewer memory is allocated
		 * and set to right dimension:
		 */
		(*g1ptr).beau.alloc_DataFromMemory(nrows, ncols);
		/* Then fill the memory. */
		(*g1ptr).fill_DataForViewer(nrows, ncols);
		/* Go back and be ready to wait. */
		(*g1ptr).event_CopyToViewer.reset();
	}
}

void grabber::schedule_Viewer(int argc, char **argv)
{
	while(true)
	{
		/* Wait for the main loop to start the viewer: */
		(*g1ptr).event_LaunchViewer.wait();
		/* In case we want to leave: */
		if((*g1ptr).close_viewer_thread.load(std::memory_order_relaxed))
		{
			(*g1ptr).event_LaunchViewer.reset();
			break;
		}
		/* Fire up the viewer: */
		(*g1ptr).beau.launch_Freeglut(argc, argv, true);
		/* Go back and be ready to wait for the next round. */
		(*g1ptr).event_LaunchViewer.reset();
	}
}

void grabber::toggle_Smoothing(void)
{
	blur_appl = !blur_appl;
}

double grabber::get_GaussBlurRangeAtomic(double *res_pt gb_min,
										double *res_pt gb_max)
{
	*gb_min = gaussblur_min_atm.load(std::memory_order_relaxed);
	*gb_max = gaussblur_max_atm.load(std::memory_order_relaxed);
	return gaussblur_sigma_x_atm.load(std::memory_order_relaxed);
}

void grabber::set_GaussBlurAtomic(const double val)
{
	gaussblur_sigma_x_atm.store(val, std::memory_order_relaxed);
}

void grabber::TrackbarHandlerBlur(int i)
{
	const double out_min = 0., out_max = 50.;
	double res = map_Linear((double)i, out_min, out_max,
							gaussblur_min, gaussblur_max);
	gaussblur_sigma_x = res;
}

void grabber::cast_static_SetTrackbarHandlerBlur(int i, void *ptr)
{
	grabber *bptr = static_cast<grabber *>(ptr);
	(*bptr).TrackbarHandlerBlur(i);
}

void grabber::create_TrackbarBlur(void)
{
	const std::string trck_name = "Blur sigma";
	const double out_min = 0., out_max = 50.;

	double res = map_Linear(1., gaussblur_min, gaussblur_max, out_min, out_max);
	static int setting = res;
	assert(setting <= 50);

	createTrackbar(trck_name,
					tbar_win_name,
					&setting,
					(int)out_max,
					grabber::cast_static_SetTrackbarHandlerBlur,
					this);
}

uint grabber::get_KernelSizeAtomic(uint *res_pt sze_min, uint *res_pt sze_max)
{
	*sze_min = gaussblur_sze_min_atm.load(std::memory_order_relaxed);
	*sze_max = gaussblur_sze_max_atm.load(std::memory_order_relaxed);
	return gaussblur_sze_atm.load(std::memory_order_relaxed);
}

void grabber::set_KernelSizeAtomic(const uint i)
{
	gaussblur_sze_atm.store(i, std::memory_order_relaxed);
}

void grabber::TrackbarHandlerBlurSize(int i)
{
	uint res = (i << 1) + 1;
	gaussblur_sze = Size(res, res);
}

void grabber::cast_static_SetTrackbarHandlerBlurSize(int i, void *ptr)
{
	grabber *bptr = static_cast<grabber *>(ptr);
	(*bptr).TrackbarHandlerBlurSize(i);
}

void grabber::create_TrackbarBlurSize(void)
{
	const std::string trck_name = "Blur size";
	const int out_max = (gaussblur_sze_max - 1) >> 1;
	static int setting = (gaussblur_sze.width - 1) >> 1;

	createTrackbar(trck_name,
					tbar_win_name,
					&setting,
					out_max,
					grabber::cast_static_SetTrackbarHandlerBlurSize,
					this);
}

void grabber::set_GroundliftAtomic(const double val)
{
	groundlift_sub_atm.store(val, std::memory_order_relaxed);
}

void grabber::TrackbarHandlerGroundlift(int i)
{
	const double out_min = 0., out_max = 50.;
	double res = map_Linear((double)i, out_min, out_max, 0., groundlift_max);
	groundlift_sub = res;
}

void grabber::cast_static_SetTrackbarHandlerGroundlift(int i, void *ptr)
{
	grabber *bptr = static_cast<grabber *>(ptr);
	(*bptr).TrackbarHandlerGroundlift(i);
}

void grabber::create_TrackbarGroundlift(void)
{
	const std::string trck_name = "Groundlift";
	const double out_min = 0., out_max = 50.;

	double res = map_Linear(groundlift_sub, 0., groundlift_max,
							out_min, out_max);
	static int setting = res;
	assert(setting <= 50);

	createTrackbar(trck_name,
					tbar_win_name,
					&setting,
					out_max,
					grabber::cast_static_SetTrackbarHandlerGroundlift,
					this);
}

void grabber::get_GroundliftRangeAtomic(double *res_pt gl_current,
								double *res_pt gl_max)
{
	*gl_current = groundlift_sub_atm.load(std::memory_order_relaxed);
	*gl_max = groundlift_max_atm.load(std::memory_order_relaxed);
}

/** \brief This is the thread function for the minime class member.
 *
 * \param wavelengthUm const double The wavelength in micro meter.
 * \param pix2um const double The pixel to micro meter scaling given by the
 camera.
 * \return void
 *
 * This is the link between the grabber and minime thread, thus care has to
 * be taken with respect to race conditions.
 */
void grabber::launch_Minime(const double wavelengthUm, const double pix2um)
{
	const uint n_roi_rows = load_WorkRoiRows(),
	           n_roi_cols = load_WorkRoiCols();

	mime.set_Wavelength(wavelengthUm);
	mime.set_PixelToUm(pix2um);
	mime.set_Plotting(true);
	mime.alloc_DataFromMemory(n_roi_rows, n_roi_cols);

	if(work_roi_arr_buf == nullptr)
	{
		error_msg("buffer not allocated", ERR_ARG);
		return;
	}

	double *cpy = alloc_mat(n_roi_rows, n_roi_cols);
	memcpy(cpy, work_roi_arr_buf, n_roi_rows * n_roi_cols * sizeof(double));
	mime.fill_DataFromMemory(cpy);
	store_WaitCameraThread(false);
	free(cpy);

	mime.fit_GaussEllip();
	iprint(stdout, "fit is finished\n");
	store_WaitCameraThread(false); /**< Makes sure that an intermediate call to
	the fit function hangs up. It also makes sure that no other frames are
	grabbed between the call to the fit function and the finishing of an
	ongoing fit. */
}

void grabber::schedule_Minime(const double wavelengthUm, const double pix2um)
{
	static const double wlen = wavelengthUm,
	                    scl = pix2um;
	while(true)
	{
		/* Wait for the main loop to start minime: */
		(*g1ptr).event_LaunchMinime.wait();
		/* In case we want to leave: */
		if((*g1ptr).close_minime_thread.load(std::memory_order_relaxed))
		{
			(*g1ptr).event_LaunchMinime.reset();
			break;
		}
		/* Fire up minime: */
		(*g1ptr).launch_Minime(wlen, scl);
		/* Go back and be ready to wait for the next round. */
		(*g1ptr).event_LaunchMinime.reset();
	}
}

void grabber::close_MinimeThread(void)
{
	close_minime_thread.store(true, std::memory_order_relaxed);
	iprint(stdout, "waiting for minime to close .");
	while(true)
	{
		if(signal_MinimeThreadIfWait())
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(80));
		iprint(stdout, ".");
		fflush(stdout);
	}
	iprint(stdout, " done\n");
}

bool grabber::signal_MinimeThreadIfWait(void)
{
	if(event_LaunchMinime.check())
	{
		event_LaunchMinime.signal();
		return true;
	}
	else
		return false;
}

void grabber::wait_CameraThread(void)
{
	store_WaitCameraThread(true);
	iprint(stdout, "waiting for minime thread to get data .");
	while(true)
	{
		if(!load_WaitCameraThread())
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(80));
		iprint(stdout, ".");
		fflush(stdout);
	}
	iprint(stdout, " done\n");
}

void grabber::close_CopyThread(void)
{
	close_copy_thread.store(true, std::memory_order_relaxed);
	iprint(stdout, "waiting for copy thread to close .");
	while(true)
	{
		if(signal_CopyThreadIfWait())
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(80));
		iprint(stdout, ".");
		fflush(stdout);
	}
	iprint(stdout, " done\n");
}

bool grabber::signal_CopyThreadIfWait(void)
{
	if(event_CopyToViewer.check())
	{
		event_CopyToViewer.signal();
		return true;
	}
	else
		return false;
}

void grabber::close_ViewerThread(void)
{
	close_viewer_thread.store(true, std::memory_order_relaxed);
	iprint(stdout, "waiting for viewer thread to close .");
	while(true)
	{
		if(signal_ViewerThreadIfWait())
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(80));
		iprint(stdout, ".");
		fflush(stdout);
	}
	iprint(stdout, " done\n");
}

bool grabber::signal_ViewerThreadIfWait(void)
{
	if(event_LaunchViewer.check())
	{
		event_LaunchViewer.signal();
		return true;
	}
	else
		return false;
}

void grabber::toggle_Grabbing(void)
{
	grab_frames = !grab_frames;
}

bool grabber::is_Grabbing(void)
{
	return grab_frames;
}

void grabber::increment_Frames(void)
{
	++frms;
}

uint64_t grabber::get_Frames(void)
{
	return frms;
}

void grabber::gnuplot_Image(const save_Im_type mtype,
							const std::string &fname)
{
	std::string str;

	if(!fname.empty()) /**< If a name is not provided, a generic one will
		be generated. */
		ffc.set_FileName(fname);

	get_DateAndTime(str);

	std::replace(str.begin(), str.end(), '_', ' ');

	if(mtype == save_Im_type::RGB)
		str += " RGB";
	else if(mtype == save_Im_type::WORK)
		str += " Work";
	else if(mtype == save_Im_type::FP_IN)
		str += " Raw";
	else
	{
		error_msg("wrong enumerator", ERR_ARG);
		return;
	}

	ffc.set_AxisTitles("x / pixel", "y / pixel", "Intensity");
	ffc.set_PlotTitle(str);
	ffc.set_UseContours(true);

	switch(mtype)
	{
		case save_Im_type::RGB:
			ffc.plot_Data(rgb);
			break;
		case save_Im_type::WORK:
			ffc.plot_Data(work);
			break;
		case save_Im_type::FP_IN:
			ffc.plot_Data(fp_in);
			break;
	}
}

bool grabber::is_ViewerWindowRunning(void)
{
	return beau.load_IsRunning();
}

void grabber::close_ViewerWindow(void)
{
	beau.close_Freeglut();
}

void grabber::calculate_BeamRadius(void)
{
	/* Don't forget the factor of 2. for the Gaussian beam formalism. */
	beam_parameter.at<double>(0) = 2. * sqrt(covar.at<double>(0, 0));
	beam_parameter.at<double>(1) = 2. * sqrt(covar.at<double>(1, 1));
	beam_parameter.at<double>(2) =
	4. * covar.at<double>(0, 1) /
	(beam_parameter.at<double>(0) * beam_parameter.at<double>(1));

	beam_parameter.at<double>(0) *= pix2um_scale;
	beam_parameter.at<double>(1) *= pix2um_scale;
}

bool grabber::get_MouseDrag(void)
{
	return mouse_drag;
}

void grabber::set_MouseDrag(const bool val)
{
	mouse_drag = val;
}

bool grabber::get_RoiActive(void)
{
	return roi_on;
}

void grabber::set_RoiActive(const bool val)
{
	roi_on = val;
}

void grabber::copy_MousePosition(const int px, const int py)
{
	px_mouse = px;
	py_mouse = py;
}

void grabber::set_StartRoi(const Point_<int> &val)
{
	start_roi = val;
}

void grabber::set_EndRoi(const Point_<int> &val)
{
	end_roi = val;
}

void grabber::set_RectRoi(const Rect_<int> &val)
{
	roi_rect = val;
}

void grabber::get_RectRoi(int *res_pt sx, int *res_pt sy,
						int *res_pt rw, int *res_pt rh)
{
	*sx = roi_rect.x;
	*sy = roi_rect.y;
	*rw = roi_rect.width;
	*rh = roi_rect.height;
}

double grabber::get_PixelValueWork(const int x, const int y)
{
	return (double)work.at<float>(y, x);
}

void grabber::get_StartRoi(int *res_pt px, int *res_pt py)
{
	*px = start_roi.x;
	*py = start_roi.y;
}

void grabber::toggle_ViewerIdling(void)
{
	beau.toggle_Idling();
}

void grabber::save_ViewerScreenshot(const std::string &fname)
{
	beau.store_SaveScreen(true, fname);
}

void grabber::toggle_ViewerMap3DMode(void)
{
	beau.toggle_Map3DMode();
}

void grabber::toggle_ViewerRotation(void)
{
	beau.toggle_Rotation();
}

void grabber::exchange_Atomics(void)
{
	gaussblur_sigma_x = gaussblur_sigma_x_atm.load(std::memory_order_relaxed);
	groundlift_sub = groundlift_sub_atm.load(std::memory_order_relaxed);
	gaussblur_sze.width = gaussblur_sze_atm.load(std::memory_order_relaxed);
	gaussblur_sze.height = gaussblur_sze.width;
}

void grabber::store_WorkRoiRows(const uint n)
{
	work_roi_rows.store(n, std::memory_order_relaxed);
}

uint grabber::load_WorkRoiRows(void)
{
	return work_roi_rows.load(std::memory_order_relaxed);
}

void grabber::store_WorkRoiCols(const uint n)
{
	work_roi_cols.store(n, std::memory_order_relaxed);
}

uint grabber::load_WorkRoiCols(void)
{
	return work_roi_cols.load(std::memory_order_relaxed);
}

void grabber::store_WaitCameraThread(const bool b)
{
	wait_camera_thread.store(b, std::memory_order_relaxed);
}

bool grabber::load_WaitCameraThread(void)
{
	return wait_camera_thread.load(std::memory_order_relaxed);
}

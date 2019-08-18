#include "grabber_class.h"
#include "test.h"

void get_Moments(const cv::Mat &im, cv::Mat &out, const double scale, const bool chatty)
{
	im.copyTo(out);
	cv::Moments mom;
	if(im.channels() == 3)
	{
		cv::Mat im_gray;
		printf("Mat has 3 channels\n");
		cv::cvtColor(im, im_gray, cv::COLOR_BGR2GRAY);
		mom = moments(im_gray);
	}
	else if(im.channels() == 1)
	{
		printf("Mat has 1 channel\n");
		mom = moments(im);
	}
	else
	{
		error_msg("unknown format", ERR_ARG);
		return;
	}

	const double cen[2] = {mom.m10 / mom.m00, mom.m01 / mom.m00};
	const cv::Point2d centroid = cv::Point2d(cen[0], cen[1]);
	cv::Mat eig,
	    covar = (cv::Mat_<double>(2, 2) << mom.m20 / mom.m00 - POW2(cen[0]),
				mom.m11 / mom.m00 - cen[0] * cen[1],
				mom.m11 / mom.m00 - cen[0] * cen[1],
				mom.m02 / mom.m00 - POW2(cen[1]));
	eigen(covar, eig);

	cv::Mat beampar = (cv::Mat_<double>(3, 1) <<
					2. * sqrt(covar.at<double>(0, 0)),
					2. * sqrt(covar.at<double>(1, 1)),
					4. * covar.at<double>(0, 1));
	beampar.at<double>(2) /= (beampar.at<double>(0) * beampar.at<double>(1));

	sqrt(eig, eig);

	double theta = 0., ecc = 0.,
	       temp = covar.at<double>(0, 0) - covar.at<double>(1, 1);

	if(fabs(temp) >= DBL_EPSILON)
		theta = .5 * atan2(2. * covar.at<double>(1, 0), temp) * 180. / M_PI;

	if(eig.at<double>(0) != 0. && eig.at<double>(1) != 0.)
	{
		temp = eig.at<double>(1) / eig.at<double>(0);
		if(temp > 1.)
			ecc = sqrt(1. - eig.at<double>(0) / eig.at<double>(1));
		else
			ecc = sqrt(1. - temp);
	}
	draw_Moments(out, beampar, centroid, eig, covar, theta, scale, ecc, chatty);
}

void get_Moments_own(const cv::Mat &im, cv::Mat &out, const double scale, const bool chatty)
{
	cv::Mat work;
	im.copyTo(out);
	im.copyTo(work);
	if(im.channels() == 3)
	{
		cv::cvtColor(im, work, cv::COLOR_BGR2GRAY);
		work.convertTo(work, CV_32F);
	}
	else if(im.channels() == 1)
		work.convertTo(work, CV_32F);
	else
	{
		error_msg("unknown format", ERR_ARG);
		return;
	}
	double cen[2] = {0., 0.}, norm = 0.;

	for(uint i = 0; i < (uint)work.rows; i++)
	{
		const float *const m = work.ptr<float>(i); /**< Pay attention here for the type of the image! */
		for(uint j = 0; j < (uint)work.cols; j++)
		{
			const double f = m[j];
			cen[0] += f * i; /**< This doesn't account for */
			cen[1] += f * j; /**< interleaving pixels. */
			norm += f;
		}
	}

	const double sx = 1. / 128. * work.cols,
	             fscl = .45;
	const int font = cv::FONT_HERSHEY_SIMPLEX;
	const cv::Scalar_<double> clr_txt(0., 255., 0.);

	if(norm == 0.)
	{
		if(chatty)
			warn_msg("norm is 0.!", ERR_ARG);

		cv::putText(out, "norm is 0., no output",
					cv::Point2d(sx, out.rows - 4.), font, fscl, clr_txt, 1, cv::LINE_AA);
	}
	else
	{
		cen[0] /= norm;
		cen[1] /= norm;

		double rxx = 0., ryy = 0., rxy = 0.;

		for(uint i = 0; i < (uint)work.rows; i++)
		{
			const float *const m = work.ptr<float>(i);
			for(uint j = 0; j < (uint)work.cols; j++)
			{
				const double f = m[j],
							 x = i - cen[0],
							 y = j - cen[1];
				rxx += f * x * x; /**< This doesn't account for */
				ryy += f * y * y; /**< interleaving pixels. */
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
			warn_msg("the correlation is > 1!", ERR_ARG);
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

		cv::Point2d centroid = cv::Point2d(cen[0], cen[1]);
		cv::Mat covar = (cv::Mat_<double>(2, 2) << rxx, rxy, rxy, ryy),
		    eig;
		/* The eigenvalues of the covariance matrix give
		 * the variance along the principal (or main) axis:
		 */
		{
			double temp = .5 * sqrt(4. * POW2(rxy) + POW2(rxx - ryy)),
			       eig_[2];
			eig_[0] = eig_[1] = .5 * (rxx + ryy);
			eig_[0] += temp;
			eig_[1] -= temp;
			eig_[0] = sqrt(eig_[0]);
			eig_[1] = sqrt(eig_[1]);

			eig = (cv::Mat_<double>(2, 1) << eig_[0], eig_[1]);
		}
		/* These are the parameters along the global axes: */
		cv::Mat beampar = (cv::Mat_<double>(3, 1) <<
						2. * sqrt(rxx),
						2. * sqrt(ryy),
						4. * rxy);
		beampar.at<double>(2) /= (beampar.at<double>(0) * beampar.at<double>(1));

		double theta = 0., ecc = 0.,
			   temp = covar.at<double>(0, 0) - covar.at<double>(1, 1);

		if(fabs(temp) >= DBL_EPSILON)
			theta = .5 * atan2(2. * covar.at<double>(1, 0), temp) * 180. / M_PI;

		if(eig.at<double>(0) != 0. && eig.at<double>(1) != 0.)
		{
			temp = eig.at<double>(1) / eig.at<double>(0);
			if(temp > 1.)
				ecc = sqrt(1. - eig.at<double>(0) / eig.at<double>(1));
			else
				ecc = sqrt(1. - temp);
		}
		draw_Moments(out, beampar, centroid, eig, covar,
					theta, scale, ecc, chatty);
	}
}

void draw_Moments(cv::Mat &out, const cv::Mat &beampar,
				const cv::Point2d &centroid, const cv::Mat &eig,
				const cv::Mat &covar, const double &theta, const double &scale,
				const double &ecc,
				const bool chatty)
{
	const cv::Scalar_<double> clr_ell(100., 100., 255.);
	uint16_t lw = 2;
	ellipse(out, centroid,
			cv::Size_<double>(2.* eig.at<double>(0), 2.* eig.at<double>(1)),
			theta, 0., 360., clr_ell, lw, cv::LINE_AA);
	ellipse(out, centroid,
			cv::Size_<double>(eig.at<double>(0), eig.at<double>(1)),
			theta, 0., 360., clr_ell, lw, cv::LINE_AA);
	circle(out, centroid, 1., clr_ell, lw, cv::LINE_AA);
	line(out, centroid, cv::Point2d(eig.at<double>(0) + centroid.x, centroid.y),
		cv::Scalar(0, 255, 50), 2);
	line(out, centroid,
		cv::Point2d(beampar.at<double>(0) + centroid.x, centroid.y),
		cv::Scalar(255, 0, 50));

	beampar *= scale;

	if(chatty)
		iprint(stdout,
				"\ntheta / deg: %g\n" \
				"centroid / pix: (%g, %g)\n" \
				"covar / pix^2: [[%g, %g], [%g, %g]]\n" \
				"eigen / scl: %g, %g\n" \
				"beampar / scl: [(%g, %g), %g]\n" \
				"beampar over eigen: (%g, %g)\n" \
				"ecc / 1: %g\n",
				theta,
				centroid.x, centroid.y,
				covar.at<double>(0, 0), covar.at<double>(0, 1),
				covar.at<double>(1, 0), covar.at<double>(1, 1),
				eig.at<double>(0) * scale, eig.at<double>(1) * scale,
				beampar.at<double>(0), beampar.at<double>(1),
				beampar.at<double>(2),
				beampar.at<double>(0) / (eig.at<double>(0) * scale),
				beampar.at<double>(1) / (eig.at<double>(1) * scale),
				ecc);
	{
		/* Test if the beam parameter along the global axes
		 * can be transformed into the main axes:
		 */
		const double rxx = POW2(beampar.at<double>(0)) / 4.,
		             ryy = POW2(beampar.at<double>(1)) / 4.,
		             rxy =
		             beampar.at<double>(0) *
		             beampar.at<double>(1) *
		             beampar.at<double>(2) / 4.;
		cv::Mat covar_ = (cv::Mat_<double>(2, 2) << rxx, rxy, rxy, ryy),
		    eig_;
		if(chatty)
			iprint(stdout,
					"covar from beampar / pix^2: [[%g, %g], [%g, %g]]\n",
					covar_.at<double>(0, 0), covar_.at<double>(0, 1),
					covar_.at<double>(1, 0), covar_.at<double>(1, 1));
	}
	double temp = 1. / 32. * out.rows;
	const double incy = 1. / 32. * out.rows,
	             sx = 1. / 128. * out.cols,
	             fscl = .45;
	const int font = cv::FONT_HERSHEY_SIMPLEX;
	const cv::Scalar_<double> clr_txt(0., 255., 0.);
	lw = 1;
	cv::putText(out, "theta / deg: " + convert_Double2Str(theta),
			cv::Point2d(sx, temp), font, fscl, clr_txt, lw, cv::LINE_AA);
	temp += incy;
	cv::putText(out, "eccentric / 1: " + convert_Double2Str(ecc),
			cv::Point2d(sx, temp), font, fscl, clr_txt, lw, cv::LINE_AA);
	temp += incy;
	cv::putText(out, "centroid / pix: (" +
			convert_Double2Str(centroid.x) + ", " +
			convert_Double2Str(centroid.y) + ")",
			cv::Point2d(sx, temp), font, fscl, clr_txt, lw, cv::LINE_AA);
	temp += incy;
	eig *= scale;
	cv::putText(out, "eigen / um: " +
			convert_Double2Str(eig.at<double>(0))  + ", " +
			convert_Double2Str(eig.at<double>(1)),
			cv::Point2d(sx, temp), font, fscl, clr_txt, lw, cv::LINE_AA);
	temp += incy;
	cv::putText(out, "covar / pix^2: [[" +
			convert_Double2Str(covar.at<double>(0, 0)) + ", " +
			convert_Double2Str(covar.at<double>(0, 1)) + "], [" +
			convert_Double2Str(covar.at<double>(1, 0)) + ", " +
			convert_Double2Str(covar.at<double>(1, 1)) + "]]",
			cv::Point2d(sx, temp), font, fscl, clr_txt, lw, cv::LINE_AA);
	temp += incy;
	cv::putText(out, "beampar / um: [(" +
			convert_Double2Str(beampar.at<double>(0)) + ", " +
			convert_Double2Str(beampar.at<double>(1)) + "), " +
			convert_Double2Str(beampar.at<double>(2)) + "]",
			cv::Point2d(sx, temp), font, fscl, clr_txt, lw, cv::LINE_AA);
	temp += incy;
	cv::putText(out, "beampar / eigen: (" +
			convert_Double2Str(beampar.at<double>(0) /
								(eig.at<double>(0) * scale)) + ", " +
			convert_Double2Str(beampar.at<double>(1) /
								(eig.at<double>(1) * scale)) + ")",
			cv::Point2d(sx, temp), font, fscl, clr_txt, lw, cv::LINE_AA);
	temp += incy;
	const cv::Scalar_<double> clr_w(0., 255., 0.);
	if(out.cols < sqrt(covar.at<double>(0, 0)))
		cv::putText(out, "covar[0] > cols",
				cv::Point2d(sx, temp), font, fscl, clr_w, lw, cv::LINE_AA);
	temp += incy;
	if(out.rows < sqrt(covar.at<double>(1, 1)))
		cv::putText(out, "covar[1] > rows",
				cv::Point2d(sx, temp), font, fscl, clr_w, lw, cv::LINE_AA);
}

void set_MouseEvent(const int event, const int x, const int y,
					const int flags, void *udata)
{
    if(event == cv::EVENT_MOUSEMOVE || (event == cv::EVENT_LBUTTONDOWN && flags == cv::EVENT_FLAG_CTRLKEY))
    {
    	cv::Mat im = *(static_cast<cv::Mat *>(udata)); /**< C++ style pointer casting */
    	int ch = im.channels(), el_typ = im.depth();
    	if(ch == 3)
    	{
			if(el_typ == CV_8U)
			{
				cv::Vec<uint8_t, 3> intrgb = im.at< cv::Vec<uint8_t, 3> >(y, x);
				uint intw = (intrgb[0] + intrgb[1] + intrgb[2]) / 3;
				iprint(stdout, "  val: (%u, %u, %u) = %u, pos: (%i, %i)\n",
						intrgb[0], intrgb[1], intrgb[2], intw, x, y);
			}
			else if(el_typ == CV_8S)
			{
				cv::Vec<int8_t, 3> intrgb = im.at< cv::Vec<int8_t, 3> >(y, x);
				int intw = (intrgb[0] + intrgb[1] + intrgb[2]) / 3;
				iprint(stdout, "  val: (%i, %i, %i) = %i, pos: (%i, %i)\n",
						intrgb[0], intrgb[1], intrgb[2], intw, x, y);
			}
			else if(el_typ == CV_16U)
			{
				cv::Vec<uint16_t, 3> intrgb = im.at< cv::Vec<uint16_t, 3> >(y, x);
				uint32_t intw = (intrgb[0] + intrgb[1] + intrgb[2]) / 3;
				iprint(stdout, "  val: (%u, %u, %u) = %u, pos: (%i, %i)\n",
						intrgb[0], intrgb[1], intrgb[2], intw, x, y);
			}
			else if(el_typ == CV_16S)
			{
				cv::Vec<int16_t, 3> intrgb = im.at< cv::Vec<int16_t, 3> >(y, x);
				int32_t intw = (intrgb[0] + intrgb[1] + intrgb[2]) / 3;
				iprint(stdout, "  val: (%i, %i, %i) = %i, pos: (%i, %i)\n",
						intrgb[0], intrgb[1], intrgb[2], intw, x, y);
			}
			else if(el_typ == CV_32S)
			{
				cv::Vec<int32_t, 3> intrgb = im.at< cv::Vec<int32_t, 3> >(y, x);
				long int intw = (intrgb[0] + intrgb[1] + intrgb[2]) / 3;
				iprint(stdout, "  val: (%i, %i, %i) = %li, pos: (%i, %i)\n",
						intrgb[0], intrgb[1], intrgb[2], intw, x, y);
			}
			else if(el_typ == CV_32F)
			{
				cv::Vec<float, 3> intrgb = im.at<cv::Vec <float, 3> >(y, x);
				double intw = (intrgb[0] + intrgb[1] + intrgb[2]) / 3.;
				iprint(stdout, "  val: (%g, %g, %g) = %g, pos: (%i, %i)\n",
						intrgb[0], intrgb[1], intrgb[2], intw, x, y);
			}
			else if(el_typ == CV_64F)
			{
				cv::Vec<double, 3> intrgb = im.at<cv::Vec <double, 3> >(y, x);
				double intw = (intrgb[0] + intrgb[1] + intrgb[2]) / 3.;
				iprint(stdout, "  val: (%g, %g, %g) = %g, pos: (%i, %i)\n",
						intrgb[0], intrgb[1], intrgb[2], intw, x, y);
			}
			else
				iprint(stderr, "'%s': unrecognised format\n", __func__);
    	}
		else if(ch == 1)
		{
			if(el_typ == CV_8U)
			{
				uint8_t intw = im.at<uint8_t>(y, x);
				iprint(stdout, "  val: %u, pos: (%i, %i)\n", intw, x, y);
			}
			else if(el_typ == CV_8S)
			{
				int8_t intw = im.at<int8_t>(y, x);
				iprint(stdout, "  val: %i, pos: (%i, %i)\n", intw, x, y);
			}
			else if(el_typ == CV_16U)
			{
				uint16_t intw = im.at<uint16_t>(y, x);
				iprint(stdout, "  val: %u, pos: (%i, %i)\n", intw, x, y);
			}
			else if(el_typ == CV_16S)
			{
				int16_t intw = im.at<int16_t>(y, x);
				iprint(stdout, "  val: %i, pos: (%i, %i)\n", intw, x, y);
			}
			else if(el_typ == CV_32S)
			{
				int32_t intw = im.at<int32_t>(y, x);
				iprint(stdout, "  val: %i, pos: (%i, %i)\n", intw, x, y);
			}
			else if(el_typ == CV_32F)
			{
				float intw = im.at<float>(y, x);
				iprint(stdout, "  val: %g, pos: (%i, %i)\n", intw, x, y);
			}
			else if(el_typ == CV_64F)
			{
				double intw = im.at<double>(y, x);
				iprint(stdout, "  val: %g, pos: (%i, %i)\n", intw, x, y);
			}
			else
				iprint(stderr, "'%s': unrecognised format\n", __func__);
		}
		else
		{
			iprint(stderr, "'%s': unrecognised format\n", __func__);
			return;
		}
	}
	#ifndef IGYBA_NDEBUG
	else if(event == cv::EVENT_RBUTTONDOWN)
		iprint(stdout, "  r but: (%i, %i)\n", x, y);
	else if(event == cv::EVENT_MBUTTONDOWN)
		iprint(stdout, "  m but: (%i, %i)\n", x, y);
	else if(event == cv::EVENT_MOUSEMOVE)
		iprint(stdout, "  pos: (%i, %i)\n", x, y);
	iprint(stdout, "  event: %i, flag: %i, pos: (%i, %i)\n", event, flags, x, y);
	#endif
}

void test_pic1(void)
{
	cv::Mat im = cv::imread("CutDat.jpeg", cv::IMREAD_UNCHANGED);
	assert(im.data != NULL);

	uint height = im.rows,
	     width = im.cols,
	     step = im.step,
	     channels = im.channels();
	uint8_t *const data = im.data;
	iprint(stdout, "processing a %u x %u image with %u channels with step %u\n",
			height, width, channels, step);

	cv::imshow("Example 1", im);
	cv::moveWindow("Example 1", 0, 0);

	cv::waitKey(0);

	for(uint i = 0; i < height; i++)
		for(uint j = 0; j < width; j++)
			for(uint k = 0; k < channels; k++)
				data[i * step + j * channels + k] =
				255 - data[i * step + j * channels + k];
	cv::imshow("Example 1", im);
	cv::Mat out(im);
	cv::namedWindow("Example 1 Blur", cv::WINDOW_AUTOSIZE);
	cv::GaussianBlur(im, out, cv::Size(11, 11), 1., 1.);
	cv::imshow("Example 1 Blur", out);
	cv::waitKey(0);
	cv::destroyAllWindows();
}

void test_vid()
{
	iprint(stdout, "accessing camera.. ");
	fflush(stdout);

	cv::VideoCapture cap;
	cap.open(0);
	if(!cap.isOpened())
		iprint(stderr, "no access to camera\n");
	else
		iprint(stdout, "done\n");

	cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);

	uint i = true;
	while(true)
	{
		if(!cap.grab())
		{
			iprint(stderr, "no frame to grab\n");
			break;
		}
		else
		{
			cv::Mat frame, frame_mom;
			cap.retrieve(frame);
			get_Moments(frame, frame_mom, false);
			cv::imshow("Camera", frame_mom);
			frame.release();
			frame_mom.release();
		}
		i = cv::waitKey(30);
		iprint(stdout, "pressed key %u\n", i);
		if(i == 27)
			break;
	}
	cap.release();
	cv::destroyAllWindows();
	return;
}

void test_pic2(void)
{
	cv::Mat im2 =
	cv::imread("elohtyp.jpg", cv::IMREAD_UNCHANGED);
//	cv::imread("elohtypx.jpg", cv::CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat im_mom,
		im_mom_o;
	get_Moments(im2, im_mom);
	cv::imshow("draw_moments", im_mom);
	cv::moveWindow("draw_moments", 0, 0);
	cv::setMouseCallback("draw_moments", set_MouseEvent, &im2);
	uint i = cv::waitKey(0);
	while(i != 27)
	{
		iprint(stdout, "pressed key %u\n", i);
		i = cv::waitKey(0);
	}
	get_Moments_own(im2, im_mom_o);
	cv::imshow("draw_moments_own", im_mom_o);
	cv::moveWindow("draw_moments_own", 0, 0);
	cv::setMouseCallback("draw_moments_own", set_MouseEvent, &im2);
	i = cv::waitKey(0);
	while(i != 27)
	{
		iprint(stdout, "pressed key %u\n", i);
		i = cv::waitKey(0);
	}
	cv::destroyAllWindows();
}

void test_pic3(void)
{
	cv::Mat im2 = cv::imread("out.png", cv::IMREAD_UNCHANGED);
	cv::Mat im_mom,
	    im_mom_o;
	get_Moments(im2, im_mom);
	cv::imshow("draw_moments", im_mom);
	cv::setMouseCallback("draw_moments", set_MouseEvent, &im2);
	int i = cv::waitKey(0);
	while(i != 27)
	{
		iprint(stdout, "pressed key %i\n", i);
		i = cv::waitKey(0);
	}
	get_Moments_own(im2, im_mom_o);
	cv::imshow("draw_moments_own", im_mom_o);
	cv::setMouseCallback("draw_moments_own", set_MouseEvent, &im2);
	i = cv::waitKey(0);
	while(i != 27)
	{
		iprint(stdout, "pressed key %i\n", i);
		i = cv::waitKey(0);
	}
	cv::Mat im3 = im2;
	cv::GaussianBlur(im2, im3, cv::Size(21, 21), 2.);
	cv::cvtColor(im3, im3, cv::COLOR_BGR2GRAY);
	for(uint i = 0; i < (uint)im3.rows; i++)
	{
		uchar *const m = im3.ptr<uchar>(i); /**< Pay attention here for the type of the image! */
		for(uint j = 0; j < (uint)im3.cols; j++)
			if(m[j] < 20)
				m[j] = 0;
	}
	get_Moments_own(im3, im_mom_o);
	cv::imshow("draw_moments_own_blur", im_mom_o);
	cv::setMouseCallback("draw_moments_own_blur", set_MouseEvent, &im3);
	i = cv::waitKey(0);
	while(i != 27)
	{
		iprint(stdout, "pressed key %i\n", i);
		i = cv::waitKey(0);
	}
	cv::destroyAllWindows();
}

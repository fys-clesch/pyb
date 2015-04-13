#include "grabber_class.h"
#include "test.h"

void get_Moments(const Mat &im, Mat &out, const double scale, const bool chatty)
{
	im.copyTo(out);
	Moments mom;
	if(im.channels() == 3)
	{
		Mat im_gray;
		printf("Mat has 3 channels\n");
		cvtColor(im, im_gray, CV_BGR2GRAY);
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
	const Point2d centroid = Point2d(cen[0], cen[1]);
	Mat eig,
	    covar = (Mat_<double>(2, 2) << mom.m20 / mom.m00 - POW2(cen[0]),
				mom.m11 / mom.m00 - cen[0] * cen[1],
				mom.m11 / mom.m00 - cen[0] * cen[1],
				mom.m02 / mom.m00 - POW2(cen[1]));
	eigen(covar, eig);

	Mat beampar = (Mat_<double>(3, 1) <<
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

void get_Moments_own(const Mat &im, Mat &out, const double scale, const bool chatty)
{
	Mat work;
	im.copyTo(out);
	im.copyTo(work);
	if(im.channels() == 3)
	{
		cvtColor(im, work, CV_BGR2GRAY);
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
		const float *const m = work.ptr<float>(i); /**< pay attention here for the type of the image! */
		for(uint j = 0; j < (uint)work.cols; j++)
		{
			const double f = m[j];
			cen[0] += f * i; /**< this doesn't account for */
			cen[1] += f * j; /**< interleaving pixels */
			norm += f;
		}
	}

	const double sx = 1. / 128. * work.cols,
	             fscl = .45;
	const int font = FONT_HERSHEY_SIMPLEX;
	const Scalar_<double> clr_txt(0., 255., 0.);

	if(norm == 0.)
	{
		if(chatty)
			warn_msg("norm is 0.!", ERR_ARG);

		putText(out, "norm is 0., no output",
					Point2d(sx, out.rows - 4.), font, fscl, clr_txt, 1, CV_AA);
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
				rxx += f * x * x; /**< this doesn't account for */
				ryy += f * y * y; /**< interleaving pixels */
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

		Point2d centroid = Point2d(cen[0], cen[1]);
		Mat covar = (Mat_<double>(2, 2) << rxx, rxy, rxy, ryy),
		    eig;
		/* the eigenvalues of the covariance matrix give
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

			eig = (Mat_<double>(2, 1) << eig_[0], eig_[1]);
		}
		/* these are the parameters along the global axes: */
		Mat beampar = (Mat_<double>(3, 1) <<
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

void draw_Moments(Mat &out, const Mat &beampar,
				const Point2d &centroid, const Mat &eig,
				const Mat &covar, const double &theta, const double &scale,
				const double &ecc,
				const bool chatty)
{
	const Scalar_<double> clr_ell(100., 100., 255.);
	uint16_t lw = 2;
	ellipse(out, centroid,
			Size_<double>(2.* eig.at<double>(0), 2.* eig.at<double>(1)),
			theta, 0., 360., clr_ell, lw, CV_AA);
	ellipse(out, centroid,
			Size_<double>(eig.at<double>(0), eig.at<double>(1)),
			theta, 0., 360., clr_ell, lw, CV_AA);
	circle(out, centroid, 1., clr_ell, lw, CV_AA);
	line(out, centroid, Point2d(eig.at<double>(0) + centroid.x, centroid.y),
		Scalar(0, 255, 50), 2);
	line(out, centroid,
		Point2d(beampar.at<double>(0) + centroid.x, centroid.y),
		Scalar(255, 0, 50));

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
		/* test if the beam parameter along the global axes
		 * can be transformed into the main axes:
		 */
		const double rxx = POW2(beampar.at<double>(0)) / 4.,
		             ryy = POW2(beampar.at<double>(1)) / 4.,
		             rxy =
		             beampar.at<double>(0) *
		             beampar.at<double>(1) *
		             beampar.at<double>(2) / 4.;
		Mat covar_ = (Mat_<double>(2, 2) << rxx, rxy, rxy, ryy),
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
	const int font = FONT_HERSHEY_SIMPLEX;
	const Scalar_<double> clr_txt(0., 255., 0.);
	lw = 1;
	putText(out, "theta / deg: " + convert_Double2Str(theta),
			Point2d(sx, temp), font, fscl, clr_txt, lw, CV_AA);
	temp += incy;
	putText(out, "eccentric / 1: " + convert_Double2Str(ecc),
			Point2d(sx, temp), font, fscl, clr_txt, lw, CV_AA);
	temp += incy;
	putText(out, "centroid / pix: (" +
			convert_Double2Str(centroid.x) + ", " +
			convert_Double2Str(centroid.y) + ")",
			Point2d(sx, temp), font, fscl, clr_txt, lw, CV_AA);
	temp += incy;
	eig *= scale;
	putText(out, "eigen / um: " +
			convert_Double2Str(eig.at<double>(0))  + ", " +
			convert_Double2Str(eig.at<double>(1)),
			Point2d(sx, temp), font, fscl, clr_txt, lw, CV_AA);
	temp += incy;
	putText(out, "covar / pix^2: [[" +
			convert_Double2Str(covar.at<double>(0, 0)) + ", " +
			convert_Double2Str(covar.at<double>(0, 1)) + "], [" +
			convert_Double2Str(covar.at<double>(1, 0)) + ", " +
			convert_Double2Str(covar.at<double>(1, 1)) + "]]",
			Point2d(sx, temp), font, fscl, clr_txt, lw, CV_AA);
	temp += incy;
	putText(out, "beampar / um: [(" +
			convert_Double2Str(beampar.at<double>(0)) + ", " +
			convert_Double2Str(beampar.at<double>(1)) + "), " +
			convert_Double2Str(beampar.at<double>(2)) + "]",
			Point2d(sx, temp), font, fscl, clr_txt, lw, CV_AA);
	temp += incy;
	putText(out, "beampar / eigen: (" +
			convert_Double2Str(beampar.at<double>(0) /
								(eig.at<double>(0) * scale)) + ", " +
			convert_Double2Str(beampar.at<double>(1) /
								(eig.at<double>(1) * scale)) + ")",
			Point2d(sx, temp), font, fscl, clr_txt, lw, CV_AA);
	temp += incy;
	const Scalar_<double> clr_w(0., 255., 0.);
	if(out.cols < sqrt(covar.at<double>(0, 0)))
		putText(out, "covar[0] > cols",
				Point2d(sx, temp), font, fscl, clr_w, lw, CV_AA);
	temp += incy;
	if(out.rows < sqrt(covar.at<double>(1, 1)))
		putText(out, "covar[1] > rows",
				Point2d(sx, temp), font, fscl, clr_w, lw, CV_AA);
}

void set_MouseEvent(const int event, const int x, const int y,
					const int flags, void *udata)
{
    if(event == EVENT_MOUSEMOVE || (event == EVENT_LBUTTONDOWN && flags == EVENT_FLAG_CTRLKEY))
    {
    	Mat im = *(static_cast<Mat *>(udata)); /**< C++ style pointer casting */
    	int ch = im.channels(), el_typ = im.depth();
    	if(ch == 3)
    	{
			if(el_typ == CV_8U)
			{
				Vec<uint8_t, 3> intrgb = im.at< Vec<uint8_t, 3> >(y, x);
				uint intw = (intrgb[0] + intrgb[1] + intrgb[2]) / 3;
				iprint(stdout, "  val: (%u, %u, %u) = %u, pos: (%i, %i)\n",
						intrgb[0], intrgb[1], intrgb[2], intw, x, y);
			}
			else if(el_typ == CV_8S)
			{
				Vec<int8_t, 3> intrgb = im.at< Vec<int8_t, 3> >(y, x);
				int intw = (intrgb[0] + intrgb[1] + intrgb[2]) / 3;
				iprint(stdout, "  val: (%i, %i, %i) = %i, pos: (%i, %i)\n",
						intrgb[0], intrgb[1], intrgb[2], intw, x, y);
			}
			else if(el_typ == CV_16U)
			{
				Vec<uint16_t, 3> intrgb = im.at< Vec<uint16_t, 3> >(y, x);
				uint32_t intw = (intrgb[0] + intrgb[1] + intrgb[2]) / 3;
				iprint(stdout, "  val: (%u, %u, %u) = %u, pos: (%i, %i)\n",
						intrgb[0], intrgb[1], intrgb[2], intw, x, y);
			}
			else if(el_typ == CV_16S)
			{
				Vec<int16_t, 3> intrgb = im.at< Vec<int16_t, 3> >(y, x);
				int32_t intw = (intrgb[0] + intrgb[1] + intrgb[2]) / 3;
				iprint(stdout, "  val: (%i, %i, %i) = %i, pos: (%i, %i)\n",
						intrgb[0], intrgb[1], intrgb[2], intw, x, y);
			}
			else if(el_typ == CV_32S)
			{
				Vec<int32_t, 3> intrgb = im.at< Vec<int32_t, 3> >(y, x);
				long int intw = (intrgb[0] + intrgb[1] + intrgb[2]) / 3;
				iprint(stdout, "  val: (%i, %i, %i) = %li, pos: (%i, %i)\n",
						intrgb[0], intrgb[1], intrgb[2], intw, x, y);
			}
			else if(el_typ == CV_32F)
			{
				Vec<float, 3> intrgb = im.at<Vec <float, 3> >(y, x);
				double intw = (intrgb[0] + intrgb[1] + intrgb[2]) / 3.;
				iprint(stdout, "  val: (%g, %g, %g) = %g, pos: (%i, %i)\n",
						intrgb[0], intrgb[1], intrgb[2], intw, x, y);
			}
			else if(el_typ == CV_64F)
			{
				Vec<double, 3> intrgb = im.at<Vec <double, 3> >(y, x);
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
	else if(event == EVENT_RBUTTONDOWN)
		iprint(stdout, "  r but: (%i, %i)\n", x, y);
	else if(event == EVENT_MBUTTONDOWN)
		iprint(stdout, "  m but: (%i, %i)\n", x, y);
	else if(event == EVENT_MOUSEMOVE)
		iprint(stdout, "  pos: (%i, %i)\n", x, y);
	iprint(stdout, "  event: %i, flag: %i, pos: (%i, %i)\n", event, flags, x, y);
	#endif
}

void test_pic1(void)
{
	Mat im = imread("CutDat.jpeg", CV_LOAD_IMAGE_UNCHANGED);
	assert(im.data != NULL);

	uint height = im.rows,
	     width = im.cols,
	     step = im.step,
	     channels = im.channels();
	uint8_t *const data = im.data;
	iprint(stdout, "processing a %u x %u image with %u channels with step %u\n",
			height, width, channels, step);

	imshow("Example 1", im);
	moveWindow("Example 1", 0, 0);

	waitKey(0);

	for(uint i = 0; i < height; i++)
		for(uint j = 0; j < width; j++)
			for(uint k = 0; k < channels; k++)
				data[i * step + j * channels + k] =
				255 - data[i * step + j * channels + k];
	imshow("Example 1", im);
	Mat out(im);
	namedWindow("Example 1 Blur", CV_WINDOW_AUTOSIZE);
	GaussianBlur(im, out, Size(11, 11), 1., 1.);
	imshow("Example 1 Blur", out);
	waitKey(0);
	destroyAllWindows();
}

void test_vid()
{
	iprint(stdout, "accessing camera.. ");
	fflush(stdout);

	VideoCapture cap;
	cap.open(0);
	if(!cap.isOpened())
		iprint(stderr, "no access to camera\n");
	else
		iprint(stdout, "done\n");

	namedWindow("Camera", CV_WINDOW_AUTOSIZE);

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
			Mat frame, frame_mom;
			cap.retrieve(frame);
			get_Moments(frame, frame_mom, false);
			imshow("Camera", frame_mom);
			frame.release();
			frame_mom.release();
		}
		i = waitKey(30);
		iprint(stdout, "pressed key %u\n", i);
		if(i == 27)
			break;
	}
	cap.release();
	destroyAllWindows();
	return;
}

void test_pic2(void)
{
	Mat im2 =
	imread("elohtyp.jpg", CV_LOAD_IMAGE_UNCHANGED);
//	imread("elohtypx.jpg", CV_LOAD_IMAGE_UNCHANGED);
	Mat im_mom,
		im_mom_o;
	get_Moments(im2, im_mom);
	imshow("draw_moments", im_mom);
	moveWindow("draw_moments", 0, 0);
	setMouseCallback("draw_moments", set_MouseEvent, &im2);
	uint i = waitKey(0);
	while(i != 27)
	{
		iprint(stdout, "pressed key %u\n", i);
		i = waitKey(0);
	}
	get_Moments_own(im2, im_mom_o);
	imshow("draw_moments_own", im_mom_o);
	moveWindow("draw_moments_own", 0, 0);
	setMouseCallback("draw_moments_own", set_MouseEvent, &im2);
	i = waitKey(0);
	while(i != 27)
	{
		iprint(stdout, "pressed key %u\n", i);
		i = waitKey(0);
	}
	destroyAllWindows();
}

void test_pic3(void)
{
	Mat im2 = imread("out.png", CV_LOAD_IMAGE_UNCHANGED);
	Mat im_mom,
	    im_mom_o;
	get_Moments(im2, im_mom);
	imshow("draw_moments", im_mom);
	setMouseCallback("draw_moments", set_MouseEvent, &im2);
	int i = waitKey(0);
	while(i != 27)
	{
		iprint(stdout, "pressed key %i\n", i);
		i = waitKey(0);
	}
	get_Moments_own(im2, im_mom_o);
	imshow("draw_moments_own", im_mom_o);
	setMouseCallback("draw_moments_own", set_MouseEvent, &im2);
	i = waitKey(0);
	while(i != 27)
	{
		iprint(stdout, "pressed key %i\n", i);
		i = waitKey(0);
	}
	Mat im3 = im2;
	GaussianBlur(im2, im3, Size(21, 21), 2.);
	cvtColor(im3, im3, CV_BGR2GRAY);
	for(uint i = 0; i < (uint)im3.rows; i++)
	{
		uchar *const m = im3.ptr<uchar>(i); /**< pay attention here for the type of the image! */
		for(uint j = 0; j < (uint)im3.cols; j++)
			if(m[j] < 20)
				m[j] = 0;
	}
	get_Moments_own(im3, im_mom_o);
	imshow("draw_moments_own_blur", im_mom_o);
	setMouseCallback("draw_moments_own_blur", set_MouseEvent, &im3);
	i = waitKey(0);
	while(i != 27)
	{
		iprint(stdout, "pressed key %i\n", i);
		i = waitKey(0);
	}
	destroyAllWindows();
}

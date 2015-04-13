void set_MouseEvent(const int event, const int x, const int y, const int flags, void *udata);
void get_Moments(const Mat &im, Mat &out, const double scale = 1., const bool chatty = true);
void get_Moments_own(const Mat &im, Mat &out, const double scale = 1., const bool chatty = true);
void test_pic1(void);
void test_pic2(void);
void test_pic3(void);
void test_vid(void);
void draw_Moments(Mat &out, const Mat &beampar,
				const Point2d &centroid, const Mat &eig,
				const Mat &covar, const double &theta, const double &scale,
				const double &ecc,
				const bool chatty);

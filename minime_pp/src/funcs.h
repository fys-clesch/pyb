double *alloc_tensor(const uint row, const uint col, const uint depth);
void add_Matrix(double *m, const double a, const uint row, const uint col);
void sub_Matrices(double *res_pt m1, double *res_pt m2,
				const uint row, const uint col);
void smul_Matrix(const double scalar, double *m,
				const uint row, const uint col);
double sum_Matrix(const double *m, const uint row, const uint col);
double matrix_sumb(const double *res_pt m, const uchar *res_pt bad,
				const uint row, const uint col);
double centroid_xb(const double *res_pt m, const uchar *res_pt bad,
				const uint row, const uint col);
double centroid_yb(const double *res_pt m, const uchar *res_pt bad,
				const uint row, const uint col);
uint find_Bad(const char *res_pt target_bright, const char *res_pt target_dark,
			const bool to_bin, const double var_bwd, const bool noisy,
			const uint row, const uint col);
void startracker(const std::string &inif, const uint pics,
				const std::string &fout,
				const uint row, const uint col);
double sd_vardev(const double *res_pt m, double *res_pt mean,
				const bool out_var,
				const bool use_bessel_corr, const bool noisy,
				const uint row, const uint col);
void mem_histo(const double *res_pt target, const double maxcount,
			const uint steps, const std::string &fname,
			const uint row, const uint col);

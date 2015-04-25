#include "minime_class.h"
#include "funcs.h"
#include "minimizer_s.h"

static minime *d2m_global;
static double *sim;

void minime::fit_BeamPropagation(double *res_pt set, double *res_pt ssq)
{
	/* calculate linear offset and scaling */
	static bool init = false;
	double b = 0., S_tt = 0., S_sim = 0.;
	static double S_ccd = 0., sum = 0.;

	if(!init)
	{
		if((*d2m_global).bad != nullptr)
			for(uint i = 0; i < (*d2m_global).mnm_ntot; ++i)
				if(!(*d2m_global).bad[i])
				{
					S_ccd += (*d2m_global).data[i];
					sum++;
				}
		else
			for(uint i = 0; i < (*d2m_global).mnm_ntot; ++i)
			{
				S_ccd += (*d2m_global).data[i];
				sum++;
			}
		init = true;
	}

	/* wxz, wyz, corr, x_off, y_off */
	(*d2m_global).get_GaussBeamMultinormal(set[0], set[1],
											set[4],
											set[2], set[3],
											sim);

	if((*d2m_global).bad != nullptr)
		for(uint i = 0; i < (*d2m_global).mnm_ntot; ++i)
			if(!(*d2m_global).bad[i])
				S_sim += sim[i];
	else
		for(uint i = 0; i < (*d2m_global).mnm_ntot; ++i)
			S_sim += sim[i];

	const double sss = S_sim / sum;

	if((*d2m_global).bad != nullptr)
		for(uint i = 0; i < (*d2m_global).mnm_ntot; ++i)
			if(!(*d2m_global).bad[i])
			{
				const double t = sim[i] - sss;
				S_tt += t * t;
				b += t * (*d2m_global).data[i];
			}
	else
		for(uint i = 0; i < (*d2m_global).mnm_ntot; ++i)
		{
			const double t = sim[i] - sss;
			S_tt += t * t;
			b += t * (*d2m_global).data[i];
		}

	if(S_tt == 0.) /**< Happens if sim == 0. */
		b = 0.;
	else
		b /= S_tt;

	double siga = sqrt((1. + S_sim * S_sim / (sum * S_tt)) / sum),
	       sigb = sqrt(1. / S_tt);
	const double a = (S_ccd - S_sim * b) / sum,
	             covar = -S_sim / (sum * S_tt),
	             r = covar / (siga * sigb);

	*ssq = 0.;

	if((*d2m_global).bad != nullptr)
		for(uint i = 0; i < (*d2m_global).mnm_ntot; ++i)
			if(!(*d2m_global).bad[i])
			{
				const double t = (*d2m_global).data[i] -
				                 a - b * sim[i];
				*ssq += t * t;
			}
	else
		for(uint i = 0; i < (*d2m_global).mnm_ntot; ++i)
		{
			const double t = (*d2m_global).data[i] -
			                 a - b * sim[i];
			*ssq += t * t;
		}

	const double sigdat = sqrt(*ssq / (sum - 2.)); /* Sigma for data set,
	2 linear parameters */
	siga *= sigdat;
	sigb *= sigdat;

	(*d2m_global).linregpar.ssq = *ssq;
	(*d2m_global).linregpar.aoff = a;
	(*d2m_global).linregpar.bmul = b;
	(*d2m_global).linregpar.siga = siga;
	(*d2m_global).linregpar.sigb = sigb;
	(*d2m_global).linregpar.sigdat = sigdat;
	(*d2m_global).linregpar.corr = r;
}

void minime::fit_BeamProp(void)
{
	fit_par_p[0].name = "w_{0, x}";
	fit_par_p[0].unit = "um";
	fit_par_p[0].minv = 2.;
	fit_par_p[0].maxv = mnm_rows * .5;
	fit_par_p[0].fit = true;
	fit_par_p[0].init = 0.;
	fit_par_p[0].log = false;

	fit_par_p[1].name = "z_{0, x}";
	fit_par_p[1].unit = "mm";
	fit_par_p[1].minv = 2.;
	fit_par_p[1].maxv = mnm_cols * .5;
	fit_par_p[1].fit = true;
	fit_par_p[1].init = 0.;
	fit_par_p[1].log = false;

	fit_par_p[2].name = "w_{0, y}";
	fit_par_p[2].unit = "um";
	fit_par_p[2].minv = 0.;
	fit_par_p[2].maxv = 1. * mnm_rows;
	fit_par_p[2].fit = true;
	fit_par_p[2].init = 0.;
	fit_par_p[2].log = false;

	fit_par_p[3].name = "z_{0, y}";
	fit_par_p[3].unit = "mm";
	fit_par_p[3].minv = 0.;
	fit_par_p[3].maxv = 1. * mnm_cols;
	fit_par_p[3].fit = true;
	fit_par_p[3].init = 0.;
	fit_par_p[3].log = false;

	d2m_global = this;

	/* Waist and offset estimation */
	get_CentroidBeamRadius(&fit_par_p[2].init, &fit_par_p[3].init,
							&fit_par_p[0].init, &fit_par_p[1].init,
							&fit_par_p[4].init);
	iprint(stdout, "\n  <*** entry fit parameters ***>\n");
	for(parameter &p : fit_par_p)
		print_Parameter(&p);
	sim = alloc_mat(mnm_rows, mnm_cols);
	minimizer(fit_GaussianMultinormal, 5, fit_par_p, 1, true, true, mnm_ntot);
	iprint(stdout, "\n  <*** exit fit parameters ***>\n");
	for(parameter &p : fit_par_p)
		print_Parameter(&p);
	/* From the radii along the global axes, calculate the
	 * covariance matrix and its eigenvalues. This gives the measure
	 * along the main axes.
	 */
	const double fac = 1., /**< Use 4. to get the normal Gaussian definition */
	             covar_fin[4] = {
	             	fit_par_p[0].val * fit_par_p[0].val / fac,
	             	fit_par_p[0].val * fit_par_p[1].val * fit_par_p[4].val / fac,
	             	fit_par_p[0].val * fit_par_p[1].val * fit_par_p[4].val / fac,
	             	fit_par_p[1].val * fit_par_p[1].val / fac};
	double eigen_fin[2] = {},
		   temp = covar_fin[0] - covar_fin[3];

	eigen_fin[0] =
	eigen_fin[1] = .5 * (covar_fin[0] + covar_fin[3]);
	temp = .5 * sqrt(4. * covar_fin[1] * covar_fin[2] + temp * temp);
	eigen_fin[0] += temp;
	eigen_fin[1] -= temp;
	eigen_fin[0] = sqrt(eigen_fin[0]) * scl;
	eigen_fin[1] = sqrt(eigen_fin[1]) * scl;

	iprint(stdout, "\n> beam radius\n" \
			"main axes / um: %g, %g\n" \
			"global axes / um: %g, %g\n\n",
			eigen_fin[0], eigen_fin[1],
			fit_par_p[0].val * scl, fit_par_p[1].val * scl);

	if(load_UseGnuplot())
	{
		ffc.set_AxisTitles("x / pixel", "y / pixel", "Intensity / bit");
		ffc.set_PlotTitle("Input data");
		ffc.plot_Data(data);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		get_GaussBeamMultinormal(fit_par_p[0].val, fit_par_p[1].val,
								fit_par_p[4].val,
								fit_par_p[2].val, fit_par_p[3].val,
								sim);
		const double temp = covar_fin[0] - covar_fin[3],
		             ell_theta = .5 * atan2(2. * covar_fin[1], temp) *
		             180. / M_PI;
		smul_Matrix(linregpar.bmul, sim, mnm_rows, mnm_cols);
		add_Matrix(sim, linregpar.aoff, mnm_rows, mnm_cols);
		ffc.set_PlotTitle("Fit: " \
							"main (w_x, w_y) / {/Symbol m}m: (" +
							convert_Double2Str(eigen_fin[0]) + ", " +
							convert_Double2Str(eigen_fin[1]) + "); " +
							"global (w_x, w_y) / {/Symbol m}m: (" +
							convert_Double2Str(fit_par_p[0].val * scl) + ", " +
							convert_Double2Str(fit_par_p[1].val * scl) + "); " +
							"degrees: " +
							convert_Double2Str(ell_theta, 3));
		ffc.set_UseContours(true);
		ffc.plot_Data(sim);
		sub_Matrices(data, sim, mnm_rows, mnm_cols);
		ffc.set_PlotTitle("Residuum: Fit_{ij} - Data_{ij}");
		ffc.plot_Data(sim);
	}
	free(sim);
}

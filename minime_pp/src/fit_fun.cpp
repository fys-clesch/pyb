#include "minime_class.h"
#include "funcs.h"
#include "minimizer_s.h"

static minime *d2m_global;
static double *sim;

void minime::fit_GaussianMultinormal(double *res_pt set, double *res_pt ssq)
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
				double t = sim[i] - sss;
				S_tt += t * t;
				b += t * (*d2m_global).data[i];
			}
	else
		for(uint i = 0; i < (*d2m_global).mnm_ntot; ++i)
		{
			double t = sim[i] - sss;
			S_tt += t * t;
			b += t * (*d2m_global).data[i];
		}

	if(S_tt == 0.) /**< happens if sim == 0. */
		b = 0.;
	else
		b /= S_tt;

	double a = (S_ccd - S_sim * b) / sum,
	       siga = sqrt((1. + S_sim * S_sim / (sum * S_tt)) / sum),
	       sigb = sqrt(1. / S_tt),
	       covar = -S_sim / (sum * S_tt),
	       r = covar / (siga * sigb);

	*ssq = 0.;

	if((*d2m_global).bad != nullptr)
		for(uint i = 0; i < (*d2m_global).mnm_ntot; ++i)
			if(!(*d2m_global).bad[i])
			{
				double t = (*d2m_global).data[i] -
						   a - b * sim[i];
				*ssq += t * t;
			}
	else
		for(uint i = 0; i < (*d2m_global).mnm_ntot; ++i)
		{
			double t = (*d2m_global).data[i] -
					   a - b * sim[i];
			*ssq += t * t;
		}

	double sigdat = sqrt(*ssq / (sum - 2.)); /* sigma for data set, 2 linear parameters */
	siga *= sigdat;
	sigb *= sigdat;

	(*d2m_global).lin_reg_par[0] = *ssq;
	(*d2m_global).lin_reg_par[1] = a;
	(*d2m_global).lin_reg_par[2] = b;
	(*d2m_global).lin_reg_par[3] = siga;
	(*d2m_global).lin_reg_par[4] = sigb;
	(*d2m_global).lin_reg_par[5] = sigdat;
	(*d2m_global).lin_reg_par[6] = r;
}

void minime::fit_GaussEllip(void)
{
	fit_par[0].name = "x waist(z)";
	fit_par[0].unit =  "pixel";
	fit_par[0].min = 2.;
	fit_par[0].max = mnm_rows * .5;
	fit_par[0].fit = true;
	fit_par[0].init = 0.;
	fit_par[0].log = false;

	fit_par[1].name = "y waist(z)";
	fit_par[1].unit = "pixel";
	fit_par[1].min = 2.;
	fit_par[1].max = mnm_cols * .5;
	fit_par[1].fit = true;
	fit_par[1].init = 0.;
	fit_par[1].log = false;

	fit_par[2].name = "x offset";
	fit_par[2].unit = "pixel";
	fit_par[2].min = 0.;
	fit_par[2].max = 1. * mnm_rows;
	fit_par[2].fit = true;
	fit_par[2].init = 0.;
	fit_par[2].log = false;

	fit_par[3].name = "y offset";
	fit_par[3].unit = "pixel";
	fit_par[3].min = 0.;
	fit_par[3].max = 1. * mnm_cols;
	fit_par[3].fit = true;
	fit_par[3].init = 0.;
	fit_par[3].log = false;

	fit_par[4].name = "correlation";
	fit_par[4].unit = "1";
	fit_par[4].min = -1. + 2. * DBL_EPSILON;
	fit_par[4].max = 1. - 2. * DBL_EPSILON;
	fit_par[4].fit = true;
	fit_par[4].init = 0.;
	fit_par[4].log = false;

	d2m_global = this;

	/* waist and offset estimation */
	get_CentroidBeamRadius(&fit_par[2].init, &fit_par[3].init,
							&fit_par[0].init, &fit_par[1].init,
							&fit_par[4].init);

	iprint(stdout, "\n  <entry fit parameters>\n");
	for(parameter &p : fit_par)
		print_Parameter(&p);

	sim = alloc_mat(mnm_rows, mnm_cols);
	minimizer(fit_GaussianMultinormal, 5, fit_par, 1, true, false);

	iprint(stdout, "\n  <exit fit parameters>\n");
	for(parameter &p : fit_par)
		print_Parameter(&p);

	/* from the radii along the global axes, calculate the
	 * covariance matrix and its eigenvalues. this gives the measure
	 * along the main axes.
	 */
	const double fac = 1., /**< use 4. to get the normal Gaussian definition */
	             covar_fin[4] = {
	             	fit_par[0].val * fit_par[0].val / fac,
	             	fit_par[0].val * fit_par[1].val * fit_par[4].val / fac,
	             	fit_par[0].val * fit_par[1].val * fit_par[4].val / fac,
	             	fit_par[1].val * fit_par[1].val / fac};
	double eigen_fin[2] = {},
		   temp = covar_fin[0] - covar_fin[3];

	eigen_fin[0] =
	eigen_fin[1] = .5 * (covar_fin[0] + covar_fin[3]);
	temp = .5 * sqrt(4. * covar_fin[1] * covar_fin[2] + temp * temp);
	eigen_fin[0] += temp;
	eigen_fin[1] -= temp;
	eigen_fin[0] = sqrt(eigen_fin[0]) * scl;
	eigen_fin[1] = sqrt(eigen_fin[1]) * scl;

	iprint(stdout,
			"\n> beam radius\n" \
			"main axes / um: %g, %g\n" \
			"global axes / um: %g, %g\n\n",
			eigen_fin[0], eigen_fin[1],
			fit_par[0].val * scl, fit_par[1].val * scl);

	if(load_UseGnuplot())
	{
		ffc.set_AxisTitles("x / pixel", "y / pixel", "Intensity / bit");
		ffc.set_PlotTitle("Input data");
		ffc.plot_Data(data);

		std::this_thread::sleep_for(std::chrono::milliseconds(250));

		get_GaussBeamMultinormal(fit_par[0].val, fit_par[1].val,
								fit_par[4].val,
								fit_par[2].val, fit_par[3].val,
								sim);

		smul_Matrix(lin_reg_par[2], sim, mnm_rows, mnm_cols);
		add_Matrix(sim, lin_reg_par[1], mnm_rows, mnm_cols);
		ffc.set_PlotTitle("Fit: " \
							"main (w_x, w_y) / {/Symbol m}m: (" +
							convert_Double2Str(eigen_fin[0]) + ", " +
							convert_Double2Str(eigen_fin[1]) + "); " +
							"global (w_x, w_y) / {/Symbol m}m: (" +
							convert_Double2Str(fit_par[0].val * scl) + ", " +
							convert_Double2Str(fit_par[1].val * scl) + "); " +
							"correlation: " +
							convert_Double2Str(fit_par[4].val, 4));
		ffc.set_UseContours(true);
		ffc.plot_Data(sim);
		sub_Matrices(data, sim, mnm_rows, mnm_cols);
		ffc.set_PlotTitle("Residuum: Fit_{ij} - Data_{ij}");
//		ffc.plot_Data(sim);
	}
	free(sim);
}

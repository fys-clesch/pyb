#include "minime_class.h"
#include "funcs.h"
#include "minimizer_s.h"

static minime_propagation *d2m_global; /**< Pointer to the local minime_propagation
                                            class. This way, the SSQ function has
                                            direct access to the data. */
static double *sim;

void minime_propagation::fit_BeamPropSSQ(double *res_pt set,
                                         double *res_pt ssq)
{
    *ssq = 0.;
    for(uint i = 0; i < (*d2m_global).mnm_ntot; ++i)
    {
        /* Measurement point "i", estimate of z0, estimate of w0. */
        sim[i] = (*d2m_global).get_BeamWidth(i, set[0], set[1]);
        const double t = (*d2m_global).data[i] - sim[i];
        *ssq += t * t;
    }
}

void minime_propagation::fit_BeamProp(const double z0_init,
                                      const double z0_min,
                                      const double z0_max)
{
    fit_par_p[0].name = "z_{0, x}";
    fit_par_p[0].unit = "mm";
    fit_par_p[0].minv = z0_min;
    fit_par_p[0].maxv = z0_max;
    fit_par_p[0].fit = true;
    fit_par_p[0].init = z0_init;
    fit_par_p[0].log = false;

    fit_par_p[1].name = "w_{0, x}";
    fit_par_p[1].unit = "um";
    fit_par_p[1].minv = lambda;
    fit_par_p[1].maxv = lambda * 1e4;
    fit_par_p[1].fit = true;
    fit_par_p[1].init = lambda * 1e2;
    fit_par_p[1].log = true;

    fit_par_p[2].name = "z_{0, y}";
    fit_par_p[2].unit = fit_par_p[0].unit;
    fit_par_p[2].minv = fit_par_p[0].minv;
    fit_par_p[2].maxv = fit_par_p[0].maxv;
    fit_par_p[2].fit = fit_par_p[0].fit;
    fit_par_p[2].init = fit_par_p[0].init;
    fit_par_p[2].log = fit_par_p[0].log;

    fit_par_p[3].name = "w_{0, y}";
    fit_par_p[3].unit = fit_par_p[1].unit;
    fit_par_p[3].minv = fit_par_p[1].minv;
    fit_par_p[3].maxv = fit_par_p[1].maxv;
    fit_par_p[3].fit = fit_par_p[1].fit;
    fit_par_p[3].init = fit_par_p[1].init;
    fit_par_p[3].log = fit_par_p[1].log;

    d2m_global = this;

    /* Initial parameter estimation via linear regression? */

    iprint(stdout, "\n  <*** entry fit parameters ***>\n");
    for(parameter &p : fit_par_p)
        print_Parameter(&p);

    sim = alloc_vector(mnm_ntot, 0.);

    for(uint i = 0; i < mnm_ntot; ++i)
        data[i] = wz_x[i];
    minimizer(fit_BeamPropSSQ, 2, &fit_par_p[0], 1, true, true, mnm_ntot);
    for(uint i = 0; i < mnm_ntot; ++i)
        data[i] = wz_y[i];
    minimizer(fit_BeamPropSSQ, 2, &fit_par_p[2], 1, true, true, mnm_ntot);

    iprint(stdout, "\n  <*** exit fit parameters ***>\n");
    for(parameter &p : fit_par_p)
        print_Parameter(&p);

    free(sim);

    if(do_gnuplot)
    {
        double simx[4],
               simy[4];
        std::string title, timestr;
        get_DateAndTime(timestr);
        std::replace(timestr.begin(), timestr.end(), '_', ' ');
        title = "Mode estimation " + timestr;
        ffc.set_PlotTitle(title);
        ffc.set_Dimensions(mnm_ntot, 1);

        simx[0] = fit_par_p[0].val; /* z0 first. */
        simx[1] = fit_par_p[0].std_rel_err;
        simx[2] = fit_par_p[1].val; /* w0 */
        simx[3] = fit_par_p[1].std_rel_err;
        simy[0] = fit_par_p[2].val;
        simy[1] = fit_par_p[2].std_rel_err;
        simy[2] = fit_par_p[3].val;
        simy[3] = fit_par_p[3].std_rel_err;

        ffc.plot_BeamWidthsFit(wz_x, wz_y, zpnt, simx, simy, lambda);
    }
}

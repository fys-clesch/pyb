#include "gabe_class.h"
#include "funcs.h"

gabe::gabe(void)
{
    lambda =
    w0 =
    zr = 0.;
}

gabe::~gabe(void)
{
    #ifndef IGYBA_NDEBUG
    iprint(stdout, "'%s': memory released\n", __func__);
    #endif
}

void gabe::get_Waist_z(const double z, double *res_pt wz)
{
    *wz = w0 * sqrt(1. + POW2(z / zr));
}

void gabe::set_RayleighRange(double *res_pt zr_out)
{
    zr = M_PI * POW2(w0) / lambda;
    if(zr_out != NULL)
        *zr_out = zr;
}

void gabe::get_RayleighRange_Elliptic(const double wx0, const double wy0,
                    double *res_pt zxr, double *res_pt zyr)
{
    *zxr = M_PI * POW2(wx0) / lambda;
    *zyr = M_PI * POW2(wy0) / lambda;
}

double gabe::get_Waist_z(const double z, const double z0)
{
    constexpr double pisq = M_PI * M_PI;
    const double w0sq = w0 * w0,
                 wz = sqrt(w0sq + (z - z0) * (z - z0) *
                            lambda * lambda / (w0sq * pisq));
    return wz;
}

void gabe::get_Offset_z(const double wz, double *z)
{
    *z = zr * sqrt(POW2(wz / w0) - 1.);
}

double gabe::get_WavefrontRadius_z(const double z)
{
    if(z == 0.)
        return DBL_MAX;

    const double radz = z * (1. + POW2(zr / z));
    return radz;
}

void gabe::get_Waist_z_Elliptic(const double wx0, const double wy0, const double z,
                                const double zxr, const double zyr,
                                double *res_pt wxz, double *res_pt wyz)
{
    *wxz = wx0 * sqrt(1. + POW2(z / zxr));
    *wyz = wy0 * sqrt(1. + POW2(z / zyr));
}

double gabe::get_GaussianIntensity_Elliptic(const uint x, const uint y,
                                        const double wxz, const double wyz,
                                        const double x_off, const double y_off)
{
    double rwz = POW2((x - x_off) / wxz) +
                 POW2((y - y_off) / wyz),
           intens = 2. / (M_PI * wxz * wyz) * exp(-2. * rwz);

    return intens;
}
void gabe::get_GaussianIntensity_Elliptic_Array_z(const double wx0, const double wy0, const double z,
                                const double x_off, const double y_off, double *out,
                                const uint row, const uint col)
{
    double zxr, zyr, wxz, wyz;
    get_RayleighRange_Elliptic(wx0, wy0, &zxr, &zyr);
    get_Waist_z_Elliptic(wx0, wy0, z, zxr, zyr, &wxz, &wyz);
    #pragma omp parallel firstprivate(wxz, wyz, x_off, y_off) shared(out)
    {
        #pragma omp for schedule(static)
        for(uint x = 0; x < row; x++)
            for(uint y = 0; y < col; y++)
                out[x * col + y] = get_GaussianIntensity_Elliptic(x, y, wxz, wyz, x_off, y_off);
    }
}

void gabe::get_GaussianIntensity_Elliptic_Array(const double wxz, const double wyz,
                                                const double x_off, const double y_off, double *out,
                                                const uint row, const uint col)
{
    if(fabs(wxz) > DBL_MIN && fabs(wyz) > DBL_MIN)
    {
        const double norm = 2. / (M_PI * wxz * wyz);
        for(uint x = 0; x < row; x++)
            for(uint y = 0; y < col; y++)
            {
                const double tx = (x - x_off) / wxz,
                             ty = (y - y_off) / wyz;
                out[x * col + y] = norm * exp(-2. * (tx * tx + ty * ty));
            }
    }
    else
        for(uint i = 0; i < row * col; i++)
            out[i] = 0.;
}

/**
 * Calculates the Rayleigh range via waist development.
 *
 * z1 and z2 are the measurement points, zr* is the Rayleigh range in the respective direction.
 */
void gabe::get_RayleighRange_Via_2Widths(const double *res_pt m1,
                                        const double *res_pt m2,
                                        const uchar *res_pt bad,
                                        const double z1, const double z2,
                                        double *res_pt zrx, double *res_pt zry,
                                        const uint row, const uint col)
{
    double wz1 = get_BeamRadiusX(m1, bad, row, col),
           wz2 = get_BeamRadiusX(m2, bad, row, col);
    *zrx = sqrt((POW2(wz2 * z1) - POW2(z2 * wz1)) / (POW2(wz1) - POW2(wz2)));

    wz1 = get_BeamRadiusY(m1, bad, row, col);
    wz2 = get_BeamRadiusY(m2, bad, row, col);
    *zry = sqrt((POW2(wz2 * z1) - POW2(z2 * wz1)) / (POW2(wz1) - POW2(wz2)));

    iprint(stdout, "Rayleigh range in x direction: %g m\n\
            Rayleigh range in y direction: %g m\n", *zrx, *zry);
}

double gabe::get_BeamRadiusX(const double *res_pt m, const uchar *res_pt bad,
                            const uint row, const uint col, const bool noisy)
{
    double c_x = centroid_xb(m, bad, row, col),
           temp = 0., tempsum = 0.;

    for(uint i = 0; i < row; i++)
        for(uint j = 0; j < col; j++)
            if(!bad[i * col + j])
            {
                temp += m[i * col + j] * POW2(i - c_x);
                tempsum += m[i * col + j];
            }

    temp /= tempsum;

    if(temp <= 0.)
        error_msg("can't compute the radius, variance is negative", ERR_ARG);

    temp = 2. * sqrt(temp);

    if(noisy)
        iprint(stdout, "the beam radius in x direction is:\n %g pixel\n", temp);
    return temp;
}

double gabe::get_BeamRadiusY(const double *res_pt m, const uchar *res_pt bad,
                            const uint row, const uint col, const bool noisy)
{
    double c_y = centroid_yb(m, bad, row, col),
           temp = 0., tempsum = 0.;

    for(uint i = 0; i < row; i++)
        for(uint j = 0; j < col; j++)
            if(!bad[i * col + j])
            {
                temp += m[i * col + j] * POW2(j - c_y);
                tempsum += m[i * col + j];
            }

    temp /= tempsum;

    if(temp <= 0.)
        error_msg("can't compute the radius, variance is negative", ERR_ARG);

    temp = 2. * sqrt(temp);

    if(noisy)
        iprint(stdout, "the beam radius in y direction is:\n %g pixel\n", temp);
    return temp;
}

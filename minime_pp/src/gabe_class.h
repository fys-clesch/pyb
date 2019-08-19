#ifndef __GABE_CLASS_HEADER__
#define __GABE_CLASS_HEADER__

#include "../../src/header.hpp"
#include <math.h>
#include <float.h>

class gabe
{
private:

    double lambda,
           w0,
           zr;

public:

    gabe(void);
    ~gabe(void);

    void get_Waist_z(const double z, double *res_pt wz);
    void set_RayleighRange(double *res_pt zr_out);
    void get_RayleighRange_Elliptic(const double wx0, const double wy0,
                        double *res_pt zxr, double *res_pt zyr);
    double get_Waist_z(const double z, const double z0);
    void get_Offset_z(const double wz, double *z);
    double get_WavefrontRadius_z(const double z);
    void get_Waist_z_Elliptic(const double wx0, const double wy0, const double z,
                                    const double zxr, const double zyr,
                                    double *res_pt wxz, double *res_pt wyz);
    double get_GaussianIntensity_Elliptic(const uint x, const uint y,
                                            const double wxz, const double wyz,
                                            const double x_off, const double y_off);
    void get_GaussianIntensity_Elliptic_Array_z(const double wx0, const double wy0, const double z,
                                    const double x_off, const double y_off, double *out,
                                    const uint row, const uint col);
    void get_GaussianIntensity_Elliptic_Array(const double wxz, const double wyz,
                                                    const double x_off, const double y_off, double *out,
                                                    const uint row, const uint col);
    void get_RayleighRange_Via_2Widths(const double *res_pt m1, const double *res_pt m2, const uchar *res_pt bad,
                                            const double z1, const double z2,
                                            double *res_pt zrx, double *res_pt zry,
                                            const uint row, const uint col);

    double get_BeamRadiusX(const double *res_pt m, const uchar *res_pt bad,
                            const uint row, const uint col, const bool noisy = true);
    double get_BeamRadiusY(const double *res_pt m, const uchar *res_pt bad,
                            const uint row, const uint col, const bool noisy = true);
};

#endif

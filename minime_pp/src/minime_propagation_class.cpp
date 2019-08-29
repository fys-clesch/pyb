#include "minime_class.h"

minime_propagation::minime_propagation(void)
{
    lambda = 1.064;
    do_gnuplot = true;
    mnm_ntot = 1;
    zpnt =
    wz_x =
    wz_y = nullptr;

    allocd_vec = false;

    for(parameter &x : fit_par_p)
        x = null_par;
}

void minime_propagation::print_Members(void)
{
    iprint(stdout,
        "      Wavelength: %g\n"
        "     Use gnuplot: %u\n"
        "    Total points: %u\n"
        "Arrays allocated: %u\n",
        lambda, do_gnuplot, mnm_ntot, allocd_vec);

    iprint(stdout, "zpnt wz_x wz_y:\n");
    for(uint i = 0; i < mnm_ntot; ++i)
        iprint(stdout, "%g %g %g\n", zpnt[i], wz_x[i], wz_y[i]);

    for(parameter &x : fit_par_p)
        print_Parameter(&x);
}

minime_propagation::~minime_propagation(void)
{
    if(load_AllocatedMemory())
    {
        free(zpnt);
        free(wz_x);
        free(wz_y);
    }
    #ifndef NDEBUG_PYB
    iprint(stdout, "'%s': memory released\n", __func__);
    #endif
}

void minime_propagation::set_Wavelength(const double wlen)
{
    lambda = wlen;
}

void minime_propagation::set_Plotting(const bool do_it)
{
    do_gnuplot = do_it;
}

double minime_propagation::get_BeamWidth(const uint n, const double z0,
                                        const double w0)
{
    constexpr double pisq = M_PI * M_PI;
    const double zoff = (zpnt[n] - z0) * 1e3,
                 w0sq = w0 * w0,
                 wz = sqrt(w0sq + zoff * zoff *
                            lambda * lambda / (w0sq * pisq));
    return wz; /**< Return value must be in micrometer. */
}

void minime_propagation::fill_DataFromFile(const std::string &fname)
{
    uint tmp_rows, tmp_cols;
    double *tmp_data = nullptr;

    FILE *readfile = fopen(fname.c_str(), "r");
    if(readfile == NULL)
        file_error_msg(fname.c_str(), ERR_ARG);
    count_entries(readfile, &tmp_rows, &tmp_cols, true);
    fclose(readfile);

    if(tmp_cols != 3)
        error_msg("wrong number of rows in the file storing the beam radii",
                    ERR_ARG);

    mnm_ntot = tmp_rows;
    tmp_data = alloc_mat(mnm_ntot, tmp_cols);
    wz_x = alloc_vec(mnm_ntot);
    wz_y = alloc_vec(mnm_ntot);
    zpnt = alloc_vec(mnm_ntot);
    data = alloc_vec(mnm_ntot);

    store_AllocatedMemory(true);

    fpfile2double(fname.c_str(), tmp_data, mnm_ntot, tmp_cols, true);

    for(uint i = 0; i < mnm_ntot; ++i)
    {
        zpnt[i] = tmp_data[i * 3 + 0];
        wz_x[i] = tmp_data[i * 3 + 1];
        wz_y[i] = tmp_data[i * 3 + 2];
    }
    free(tmp_data);
}

void minime_propagation::store_AllocatedMemory(const bool b)
{
    allocd_vec = b;
}

bool minime_propagation::load_AllocatedMemory(void)
{
    return allocd_vec;
}

#include "minime_class.h"
#include "funcs.h"

double *alloc_tensor(const uint row, const uint col, const uint depth)
{
    double *m = (double *)malloc((row * col * depth) * sizeof(double));
    if(NULL == m)
    {
        error_msg("memory allocation failed. good bye.", ERR_ARG);
        exit(EXIT_FAILURE);
    }
    for(ulong i = 0; i < (row * col * depth); i++)
        m[i] = 0.;
    return m;
}

void add_Matrix(double *m, const double a, const uint row, const uint col)
{
    for(uint i = 0; i < row * col; i++)
        m[i] += a;
}

void sub_Matrices(double *res_pt m1, double *res_pt m2, const uint row, const uint col)
{
    for(uint i = 0; i < row * col; i++)
        m2[i] -= m1[i];
}

void smul_Matrix(const double scalar, double *m, const uint row, const uint col)
{
    for(uint i = 0; i < row * col; i++)
        m[i] *= scalar;
}

double sum_Matrix(const double *m, const uint row, const uint col)
{
    double temp = 0.;
    for(uint i = 0; i < row * col; i++)
        temp += m[i];
    return temp;
}

double matrix_sumb(const double *res_pt m, const uchar *res_pt bad, const uint row, const uint col)
{
    double temp = 0.;
    for(uint i = 0; i < row * col; i++)
        if(!bad[i])
            temp += m[i];
    return temp;
}

double centroid_xb(const double *res_pt m, const uchar *res_pt bad, const uint row, const uint col)
{
    double temp = 0.;
    for(uint i = 0; i < row; i++)
        for(uint j = 0; j < col; j++)
            if(!bad[i * col + j])
                temp += m[i * col + j] * i;

    temp /= matrix_sumb(m, bad, row, col);
    return temp;
}

double centroid_yb(const double *res_pt m, const uchar *res_pt bad, const uint row, const uint col)
{
    double temp = 0.;
    for(uint i = 0; i < row; i++)
        for(uint j = 0; j < col; j++)
            if(!bad[i * col + j])
                temp += m[i * col + j] * j;

    temp /= matrix_sumb(m, bad, row, col);
    return temp;
}

/**
 * target_bright: file where a very bright picture is stored in.
 * target_dark: file where a very dark picture is stored in.
 * If to_bin = 1: Write picture to binary file.
 * var_bwd is the sigma bandwidth.
 */
uint find_Bad(const char *res_pt target_bright, const char *res_pt target_dark,
            const bool to_bin, const double var_bwd, const bool noisy,
            const uint row, const uint col)
{
    fifo ff1(row, col);

    double *m_bright = alloc_matrix(row, col, 0.),
           *m_dark = alloc_matrix(row, col, 0.);

    uintfile2double(target_bright, m_bright, row, col);
    uintfile2double(target_dark, m_dark, row, col);

    double mean_bright,
           var_bright = sd_vardev(m_bright, &mean_bright, 1, 1, 0, row, col),
           max_bright = mean_bright + var_bwd * var_bright,
           min_bright = mean_bright - var_bwd * var_bright;

    double mean_dark,
           var_dark = sd_vardev(m_dark, &mean_dark, 1, 1, 0, row, col),
           max_dark = mean_dark + var_bwd * var_dark,
           min_dark = mean_dark - var_bwd * var_dark;

    if(noisy)
        iprint(stdout, "%g * var_bright: %g\nmean_bright: %g\n " \
                "%g * var_dark: %g\nmean_dark: %g\n",
                var_bwd, var_bwd * var_bright, mean_bright,
                var_bwd, var_bwd * var_dark, mean_dark);

    const std::string badfile = "bad/badfile.dat";
    create_dir(badfile.c_str());

    FILE *writefile = fopen(badfile.c_str(), "w");

    if(writefile == NULL)
    {
        file_error_msg(badfile.c_str(), ERR_ARG);
        exit(EXIT_FAILURE);
    }

    uint i = 0;

    for(uint x = 0; x < row; x++)
        for(uint y = 0; y < col; y++)
        {
            if(m_bright[x * col + y] < min_bright ||
                m_bright[x * col + y] > max_bright ||
                m_dark[x * col + y] < min_dark ||
                m_dark[x * col + y] > max_dark)
            {
                fprintf(writefile, "0");
                i++;
            }
            else
                fprintf(writefile, "1");

            if(y < (col - 1))
                fprintf(writefile, " ");
            if(y == (col - 1) && x != (row - 1))
                fprintf(writefile, "\n");
        }
    fclose(writefile);
    free(m_bright);
    free(m_dark);

    if(to_bin)
    {
        uint *bin_bright = alloc_uintmatrix(row, col, 0),
             *bin_dark = alloc_uintmatrix(row, col, 0);

        uintfile2uint(target_bright, bin_bright, row, col);
        uintfile2uint(target_dark, bin_dark, row, col);

        ff1.write_Bin_uint("bad/bright.bin", bin_bright);
        ff1.write_Bin_uint("bad/dark.bin", bin_dark);
        free(bin_bright);
        free(bin_dark);
    }

    iprint(stdout, "found %u bad pixels. %g %% are the good ones.\n",
            i, (1. - (double) i / (row * col)) * 100.);
    return i;
}

/* inif: textfile with storage place of (background) pics to be averaged */
void startracker(const std::string &inif, const uint pics,
                const std::string &fout,
                const uint row, const uint col)
{
    const uint ntot = row * col;
    fifo ff1(row, col);
    double *allinone = alloc_tensor(row, col, pics);
    ff1.load_BeamProfileFilesToDoubleMemory(inif, allinone, 0, (double *)NULL);

    double *avg = alloc_matrix(row, col, 0.);

    for(uint i = 0; i < pics; i++)
        for(uint j = 0; j < row; j++)
            for(uint k = 0; k < col; k++)
                avg[j * col + k] += allinone[i * ntot + j * col + k];
    free(allinone);

    FILE *streaky = fopen(fout.c_str(), "w");
    if(streaky == NULL)
    {
        file_error_msg(fout.c_str(), ERR_ARG);
        exit(EXIT_FAILURE);
    }

    for(uint j = 0; j < row; j++)
        for(uint k = 0; k < col; k++)
        {
            fprintf(streaky, "%.0f", round(avg[j * col + k] / pics));
            if(k < (col - 1))
                fprintf(streaky, " ");
            if(k == (col - 1) && j != (row - 1))
                fprintf(streaky, "\n");
        }
    fclose(streaky);
    free(avg);
}

/**
 * calculates 2 error properties of the data in 'mat'
 * out_var = 1: calculate variance s^2
 * use_bessel_corr = 1: with Bessel's correction
 */
double sd_vardev(const double *res_pt m, double *res_pt mean, const bool out_var,
                const bool use_bessel_corr, const bool noisy,
                const uint row, const uint col)
{
    uint j, i,
         ntot = row * col;
    *mean = sum_Matrix(m, row, col) / ntot;
    double res = 0.;

    for(i = 0; i < row; i++)
        for(j = 0; j < col; j++)
            res += POW2(m[i * col + j] - (*mean));

    if(!use_bessel_corr)
        res /= ntot;
    else
        res /= (ntot - 1.);

    if(!out_var)
        res = sqrt(res);

    if(noisy)
    {
        if(!out_var && !use_bessel_corr)
            iprint(stdout, "variance sigma^2_(n):"); /**< maximum-likelihood */
        if(!out_var && use_bessel_corr)
            iprint(stdout, "variance sigma^2_(n-1):"); /**< unbiased */
        if(out_var && !use_bessel_corr)
            iprint(stdout, "standard deviation sigma_(n):");
        if(out_var && use_bessel_corr)
            iprint(stdout, "standard deviation sigma_(n-1):");
        iprint(stdout, " %g\n", res);
    }
    return res;
}

/**
 * steps is the number of digitized values, if it's for example 100,
 * then there are 100 different (possible)
 * histogram values. steps has to be an integer value.
 * maxcount: highest digit to count (i.e. 2^14 = 16384).
 */
void mem_histo(const double *res_pt target, const double maxcount,
            const uint steps, const std::string &fname,
            const uint row, const uint col)
{
    uint *histo = alloc_uintvector(steps, 0);

    for(uint i = 0; i < row * col; i++)
    {
        double z = target[i] / maxcount * steps;
        uint zi = lround(z);
        if(zi > steps)
            error_msg("maxcount is too low", ERR_ARG);

        histo[zi]++;
    }
    FILE *writefile = fopen(fname.c_str(), "w");
    if(writefile == NULL)
    {
        file_error_msg(fname.c_str(), ERR_ARG);
        exit(EXIT_FAILURE);
    }
    for(uint i = 0; i < steps; i++)
        fprintf(writefile, "%u %u\n", i, histo[i]);

    fclose(writefile);
    free(histo);
}

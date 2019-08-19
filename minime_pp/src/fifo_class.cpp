#include "fifo_class.h"

fifo::fifo(void)
{
    dat_rows =
    dat_cols = 0;
    ax_x_title = "";
    ax_y_title = "";
    ax_z_title = "";
    plot_title = "";
    file_name = "";
    file_name_suffix = "";
    use_contours = false;
}

fifo::fifo(const uint nrows, const uint ncols) : dat_rows(nrows),
dat_cols(ncols)
{
    ax_x_title = "";
    ax_y_title = "";
    ax_z_title = "";
    plot_title = "";
    file_name = "";
    file_name_suffix = "";
}

fifo::~fifo(void)
{
    #ifndef IGYBA_NDEBUG
    iprint(stdout, "'%s': memory released\n", __func__);
    #endif
}

void fifo::set_Dimensions(const uint nrows, const uint ncols)
{
    dat_rows = nrows;
    dat_cols = ncols;
}

void fifo::set_AxisTitles(const std::string &xtitle, const std::string &ytitle,
                        const std::string &ztitle)
{
    ax_x_title = xtitle;
    ax_y_title = ytitle;
    ax_z_title = ztitle;
}

void fifo::set_FileName(const std::string &fname)
{
    file_name = fname;
}

void fifo::set_PlotTitle(const std::string &title)
{
    plot_title = title;
}

void fifo::set_FileNameSuffix(const std::string &suffix)
{
    file_name_suffix = suffix;
}

void fifo::set_UseContours(const bool set_it)
{
    use_contours = set_it;
}

void fifo::print_HistogramFromUintFile(const std::string &target,
                                    const uint maxcount)
{
    uint *histo = alloc_uintvector(maxcount, 0);

    std::string histo_file = target;
    uint found = histo_file.find_last_of(".");
    histo_file = histo_file.substr(0, found) + "_histogram.dat";

    iprint(stdout, "'%s': will create '%s'\n", __func__, histo_file.c_str());

    FILE *readfile = fopen(target.c_str(), "r"),
         *writefile = fopen(histo_file.c_str(), "w");

    if(readfile == NULL)
        file_error_msg(target.c_str(), ERR_ARG);
    if(writefile == NULL)
        file_error_msg(histo_file.c_str(), ERR_ARG);

    uint temp;
    while(fscanf(readfile, "%9u", &temp) != EOF)
        histo[temp]++;

    fclose(readfile);

    for(uint i = 0; i < maxcount; i++)
    {
        fprintf(writefile, "%u %u", i, histo[i]);
        if(i < (maxcount - 1))
            fprintf(writefile, "\n");
    }
    fclose(writefile);
    free(histo);
}

void fifo::load_BeamProfileFilesToDoubleMemory(const std::string &inif,
                                            double *res_pt ccdtensor,
                                            const bool read_z_pos,
                                            double *res_pt z_pos)
{
    FILE *readfile = fopen(inif.c_str(), "r");
    if(readfile == NULL)
    {
        file_error_msg(inif.c_str(), ERR_ARG);
        exit(EXIT_FAILURE);
    }

    double *pic = alloc_mat(dat_rows, dat_cols);
    char *fname = (char *)malloc(256 * sizeof(char));
    uint i = 0;
    if(read_z_pos)
        while(fscanf(readfile, "%255s %16lf", fname, &z_pos[i]) != EOF)
        {
            uintfile2double(fname, pic, dat_rows, dat_cols);
            memcpy(&ccdtensor[i * dat_rows * dat_cols],
                    pic,
                    dat_rows * dat_cols * sizeof(double));
            ++i;
        }
    else
        while(fscanf(readfile, "%255s", fname) != EOF)
        {
            uintfile2double(fname, pic, dat_rows, dat_cols);
            memcpy(&ccdtensor[i * dat_rows * dat_cols],
                    pic,
                    dat_rows * dat_cols * sizeof(double));
            ++i;
        }
    iprint(stdout, "%u CCD profiles smoothly loaded into memory\n", i);
    fclose(readfile);
    free(pic);
    free(fname);
}

void fifo::write_Data_g(const std::string &fname, const uchar format,
                        const double *res_pt data,
                        const uint x_co, const uint y_co)
{
    create_dir(fname.c_str());

    FILE *wfile;
    if(format == 4 || format == 6)
        wfile = fopen(fname.c_str(), "wb");
    else
        wfile = fopen(fname.c_str(), "w");

    if(wfile == NULL)
    {
        file_error_msg(fname.c_str(), ERR_ARG);
        exit(EXIT_FAILURE);
    }

    switch(format)
    {
        float *res_pt tsp;
        double *res_pt tdp;
        uint x, y;
        case(0): /* xyz. */
            for(x = 0 + x_co; x < dat_rows - x_co; x++)
                for(y = 0 + y_co; y < dat_cols - y_co; y++)
                    fprintf(wfile, "%u %u %.*g\n", x, y,
                                print_prec, data[x * dat_cols + y]);
            break;
        case(1): /* xyz with tabs. */
            for(x = 0 + x_co; x < dat_rows - x_co; x++)
                for(y = 0 + y_co; y < dat_cols - y_co; y++)
                    fprintf(wfile, "%u\t%u\t%.*g\n", x, y,
                                print_prec, data[x * dat_cols + y]);
            break;
        case(2): /* Matrix without last linefeed. */
            for(x = 0 + x_co; x < dat_rows - x_co; x++)
                for(y = 0 + y_co; y < dat_cols - y_co; y++)
                {
                    fprintf(wfile, "%.*g", print_prec, data[x * dat_cols + y]);
                    if(y < (dat_cols - 1 - y_co))
                        fprintf(wfile, " ");
                    if(y == (dat_cols - 1 - y_co) && x != (dat_rows - 1 - x_co))
                        fprintf(wfile, "\n");
                }
            break;
        case(3): /* gnuplot: xyz with linefeed after each block. */
            for(x = 0 + x_co; x < dat_rows - x_co; x++)
                for(y = 0 + y_co; y < dat_cols - y_co; y++)
                {
                    fprintf(wfile, "%u %u %g\n", x, y, data[x * dat_cols + y]);
                    if(y == (dat_cols - 1 - y_co))
                        fprintf(wfile, "\n");
                }
            break;
        case(4): /* gnuplot: whole binary float matrix with header. */
            tsp = alloc_mat_float(dat_rows + 1, dat_cols + 1);
            for(x = 0; x < dat_rows + 1; x++)
                for(y = 0; y < dat_cols + 1; y++)
                    if(!y && !x)
                        tsp[0] = dat_cols;
                    else if(!x && y != 0)
                        tsp[x * (dat_cols + 1) + y] = y - 1;
                    else if(!y && x != 0)
                        tsp[x * (dat_cols + 1) + y] = x - 1;
                    else
                        tsp[x * (dat_cols + 1) + y] = (float)data[(x - 1) *
                        dat_cols + y - 1];

            write_Bin_float(fname, tsp, (dat_rows + 1) * (dat_cols + 1));
            free(tsp);
            break;
        case(5): /* Whole ASCII matrix with header. */
            for(x = 0; x < dat_rows + 1; x++)
                for(y = 0; y < dat_cols + 1; y++)
                {
                    if(!y && !x)
                        fprintf(wfile, "%u", dat_cols);
                    else if(!x && y != 0)
                        fprintf(wfile, "%u", y - 1);
                    else if(!y && x != 0)
                        fprintf(wfile, "%u", x - 1);
                    else
                        fprintf(wfile, "%g", data[(x - 1) * dat_cols + y - 1]);

                    if(y == dat_cols)
                        fprintf(wfile, "\n");
                    else
                        fprintf(wfile, " ");
                }
            break;
        case(6): /* Whole binary matrix with header. */
            tdp = alloc_mat(dat_rows + 1, dat_cols + 1);
            for(x = 0; x < dat_rows + 1; x++)
                for(y = 0; y < dat_cols + 1; y++)
                    if(!y && !x)
                        tdp[0] = dat_cols;
                    else if(!x && y != 0)
                        tdp[x * (dat_cols + 1) + y] = y - 1;
                    else if(!y && x != 0)
                        tdp[x * (dat_cols + 1) + y] = x - 1;
                    else
                        tdp[x * (dat_cols + 1) + y] = data[(x - 1) *
                        dat_cols + y - 1];

            write_Bin_double(fname, tdp, (dat_rows + 1) * (dat_cols + 1));
            free(tdp);
            break;
        default:
            error_msg("wrong conversion number", ERR_ARG);
            break;
    }
    fclose(wfile);
}

void fifo::write_Bin_float(const std::string &fname, const float *res_pt data,
                        const uint nrowscols)
{
    uint nn;
    if(nrowscols == 0)
        nn = dat_rows * dat_cols;
    else
        nn = nrowscols;

    create_dir(fname.c_str());

    FILE *outfile = fopen(fname.c_str(), "wb");
    if(outfile == NULL)
    {
        file_error_msg(fname.c_str(), ERR_ARG);
        exit(EXIT_FAILURE);
    }

    fwrite(data, sizeof(float), nn, outfile);
    fclose(outfile);
}

void fifo::write_Bin_double(const std::string &fname, const double *res_pt data,
                            const uint nrowscols)
{
    uint nn;
    if(nrowscols == 0)
        nn = dat_rows * dat_cols;
    else
        nn = nrowscols;

    create_dir(fname.c_str());

    FILE *outfile = fopen(fname.c_str(), "wb");
    if(outfile == NULL)
    {
        file_error_msg(fname.c_str(), ERR_ARG);
        exit(EXIT_FAILURE);
    }

    fwrite(data, sizeof(double), nn, outfile);
    fclose(outfile);
}

void fifo::write_Bin_uint(const std::string &fname, const uint *res_pt data,
                        const uint nrowscols)
{
    uint nn;
    if(nrowscols == 0)
        nn = dat_rows * dat_cols;
    else
        nn = nrowscols;
    create_dir(fname.c_str());
    FILE *outfile = fopen(fname.c_str(), "wb");
    if(outfile == NULL)
    {
        file_error_msg(fname.c_str(), ERR_ARG);
        exit(EXIT_FAILURE);
    }
    fwrite(data, sizeof(uint), nn, outfile);
    fclose(outfile);
}

void fifo::write_Bin_int32(const std::string &fname,
                        const int32_t *res_pt data,
                        const uint nrowscols)
{
    uint nn;
    if(nrowscols == 0)
        nn = dat_rows * dat_cols;
    else
        nn = nrowscols;
    create_dir(fname.c_str());
    FILE *outfile = fopen(fname.c_str(), "wb");
    if(outfile == NULL)
    {
        file_error_msg(fname.c_str(), ERR_ARG);
        exit(EXIT_FAILURE);
    }
    fwrite(data, sizeof(int32_t), nn, outfile);
    fclose(outfile);
}

void fifo::write_Bin_uint16(const std::string &fname,
                            const uint16_t *res_pt data,
                            const uint nrowscols)
{
    uint nn;
    if(nrowscols == 0)
        nn = dat_rows * dat_cols;
    else
        nn = nrowscols;
    create_dir(fname.c_str());
    FILE *outfile = fopen(fname.c_str(), "wb");
    if(outfile == NULL)
    {
        file_error_msg(fname.c_str(), ERR_ARG);
        exit(EXIT_FAILURE);
    }
    fwrite(data, sizeof(uint16_t), nn, outfile);
    fclose(outfile);
}

void fifo::plot_Data(const double *res_pt data,
                    const bool auto_range,
                    const double lo, const double hi)
{
    std::string fname_tmp = std::tmpnam(nullptr);
    /* If any forward or backward slashes appear in the temp name,
    remove them. */
    fname_tmp.erase(std::remove(fname_tmp.begin(), fname_tmp.end(), '/'),
                    fname_tmp.end());
    fname_tmp.erase(std::remove(fname_tmp.begin(), fname_tmp.end(), '\\'),
                    fname_tmp.end());

    const std::string tmpdat = fname_tmp + ".dat",
                      cmdtmp = fname_tmp,
                      syscall = "gnuplot " + cmdtmp;

    if(file_name.empty())
        mkdir("plot" DIRMOD;

    write_Data_g(tmpdat, 4, data);

    FILE *gnufile = fopen(cmdtmp.c_str(), "w");
    if(gnufile == NULL)
    {
        file_error_msg(cmdtmp.c_str(), ERR_ARG);
        exit(EXIT_FAILURE);
    }

    fprintf(gnufile,
            "set term pngcairo dl 1 font \",10\" size 800, 600\n"
            "set pm3d implicit at s\n"
            "unset surface\n"
            "set colorbox user origin .8, .6 size .04, .3\n"
            "set cntrlabel onecolor\n"
            "%cset contour base\n"
            "%cset cntrparam 8\n"
            "set ticslevel .2\n"
            "set grid x, y, z\n"
            "set xtics offset -.4, -.2\n"
            "set ytics offset .8, -.2\n"
            "set xr [0 : %u]\n"
            "set yr [0 : %u]\n"
            "set format z '%%g'\n",
            use_contours ? ' ' : '#',
            use_contours ? ' ' : '#',
            dat_rows - 1,
            dat_cols - 1);
    if(!auto_range)
        fprintf(gnufile, "set zr [%g : %g]\n", lo, hi);

    fprintf(gnufile,
            "set zlabel '%s' rotate parallel offset -3, 0\n"
            "set ylabel '%s'\n"
            "set xlabel '%s' offset 0, -.4\n",
            ax_z_title.c_str(),
            ax_y_title.c_str(),
            ax_x_title.c_str());

    if(!plot_title.empty())
        fprintf(gnufile, "set title '%s'\n", plot_title.c_str());

    std::string timebuf;
    if(file_name.empty())
    {
        get_DateAndTime(timebuf);
        timebuf += file_name_suffix;
        timebuf += ".png";
        fprintf(gnufile, "set out 'plot/%s'\n", timebuf.c_str());
    }
    else
        fprintf(gnufile, "set out '%s%s'\n",
                file_name.c_str(), file_name_suffix.c_str());

    fprintf(gnufile,
            "splot '%s' binary matrix using 2:1:3 w l ls 7 palette t ''\n"
            "#splot '%s' matrix using 2:1:3 w l ls 7 palette t ''\n",
            tmpdat.c_str(), tmpdat.c_str());

    fclose(gnufile);
    iprint(stdout, "gnuplot message: ");
    fflush(stdout);
    const int i = system(syscall.c_str());
    if(i)
        warn_msg("\ngnuplot command returned '1' -- the plotting went wrong!",
                ERR_ARG);
    else
    {
        if(file_name.empty())
            iprint(stdout, "plotted 'plot/%s'\n", timebuf.c_str());
        else
            iprint(stdout, "plotted '%s%s'\n",
                    file_name.c_str(), file_name_suffix.c_str());
    }

    remove(tmpdat.c_str());
    remove(cmdtmp.c_str());
}

/** \brief Plots a histogram of a given file.
 *
 * \param fname const std::string& Name of the file to be printed.
 * \param steps const uint The number of integer steps for the histogram.
 * \return void
 *
 */
void fifo::plot_Histogram(const std::string &fname, const uint steps)
{
    std::string fname_tmp = std::tmpnam(nullptr);
    /* If any forward or backward slashes appear in the temp name,
    remove them. */
    fname_tmp.erase(std::remove(fname_tmp.begin(), fname_tmp.end(), '/'),
                    fname_tmp.end());
    fname_tmp.erase(std::remove(fname_tmp.begin(), fname_tmp.end(), '\\'),
                    fname_tmp.end());

    const std::string cmdtmp = fname_tmp,
                      syscall = "gnuplot " + cmdtmp;

    if(file_name.empty())
        mkdir("plot" DIRMOD;

    FILE *gnufile = fopen(cmdtmp.c_str(), "w");
    if(gnufile == NULL)
    {
        file_error_msg(cmdtmp.c_str(), ERR_ARG);
        exit(EXIT_FAILURE);
    }
    fprintf(gnufile,
            "set term pngcairo\n" \
            "set angles radians\n" \
            "set style function lines\n" \
            "set grid\n" \
            "set style line 1 lt 1 lw 2 pt 1\n" \
            "set style line 2 lt 2 lw 2 pt 2\n" \
            "set style fill pattern border\n"\
            "set xrange [1 : %u]\n", steps);
    fprintf(gnufile,
            "set ylabel '%s' rotate parallel\n" \
            "set xlabel '%s'\n",
            ax_y_title.c_str(),
            ax_x_title.c_str());

    if(!plot_title.empty())
        fprintf(gnufile, "set title '%s'\n", plot_title.c_str());

    std::string timebuf;
    if(file_name.empty())
    {
        get_DateAndTime(timebuf);
        fprintf(gnufile, "set out 'plot/%s.png'\n", timebuf.c_str());
    }
    else
        fprintf(gnufile, "set out '%s'\n", file_name.c_str());

    fprintf(gnufile, "plot '%s' w filledcurves\n", fname.c_str());

    fclose(gnufile);
    iprint(stdout, "gnuplot message: ");
    fflush(stdout);
    const int i = system(syscall.c_str());
    if(i)
        warn_msg("\ngnuplot command returned '1' -- the plotting went wrong!",
                ERR_ARG);
    else
    {
        if(file_name.empty())
            iprint(stdout, "plotted 'plot/%s.png'\n", timebuf.c_str());
        else
            iprint(stdout, "plotted '%s'\n", file_name.c_str());
    }

    remove(cmdtmp.c_str());
}

#ifndef __FIFO_NO_OPENCV__
void fifo::plot_Data(const cv::Mat &mdata,
                    const bool auto_range,
                    const double lo, const double hi)
{
    if(mdata.channels() != 1 && mdata.channels() != 3)
    {
        error_msg("wrong number of channels in the input matrix", ERR_ARG);
        return;
    }
    std::string fname_tmp = std::tmpnam(nullptr);
    fname_tmp.erase(std::remove(fname_tmp.begin(), fname_tmp.end(), '/'),
                    fname_tmp.end());
    fname_tmp.erase(std::remove(fname_tmp.begin(), fname_tmp.end(), '\\'),
                    fname_tmp.end());
    const std::string tmpdat = fname_tmp + ".dat",
                      cmdtmp = fname_tmp,
                      syscall = "gnuplot " + cmdtmp;

    if(file_name.empty())
        mkdir("plot" DIRMOD;

    write_MatToFile(mdata, tmpdat);

    FILE *gnufile = fopen(cmdtmp.c_str(), "w");
    if(gnufile == NULL)
    {
        file_error_msg(cmdtmp.c_str(), ERR_ARG);
        return;
    }

    fprintf(gnufile,
            "set term pngcairo dl 1 font \",10\" size 800, 600\n\n" \
            "set pm3d implicit at s\n" \
            "unset surface\n" \
            "set colorbox user origin .8, .6 size .04, .3\n" \
            "set cntrlabel onecolor\n" \
            "%cset contour base\n" \
            "%cset cntrparam\n" \
            "set ticslevel .2\n" \
            "set grid x, y, z\n" \
            "set xtics offset -.4, -.2\n" \
            "set ytics offset .8, -.2\n" \
            "set xr [0 : %u]\n" \
            "set yr [0 : %u]\n" \
            "set format z '%%g'\n",
            use_contours ? ' ' : '#',
            use_contours ? ' ' : '#',
            mdata.cols - 1, /**< Rows and columns are swapped. */
            mdata.rows - 1);
    if(!auto_range)
        fprintf(gnufile, "set zr [%g : %g]\n", lo, hi);

    fprintf(gnufile,
            "set zlabel '%s' rotate parallel offset -3, 0\n" \
            "set ylabel '%s'\n" \
            "set xlabel '%s' offset 0, -.4\n",
            ax_z_title.c_str(),
            ax_y_title.c_str(),
            ax_x_title.c_str());

    if(!plot_title.empty())
        fprintf(gnufile,
            "set title '%s'\n",
            plot_title.c_str());

    std::string timebuf;
    if(file_name.empty())
    {
        get_DateAndTime(timebuf);
        timebuf += file_name_suffix;
        timebuf += ".png";
        fprintf(gnufile, "set out 'plot/%s'\n", timebuf.c_str());
    }
    else
        fprintf(gnufile, "set out '%s%s'\n",
                file_name.c_str(), file_name_suffix.c_str());

    if(mdata.channels() == 1)
        fprintf(gnufile, "splot '%s' matrix u 1:2:3 w l ls 7 t ''\n",
                tmpdat.c_str());
    else if(mdata.channels() == 3)
        fprintf(gnufile, /**! (x,y,z,r,g,b) */
                "splot '%s' using 3:4:(0):0:1:2 w rgbimage t ''\n",
                tmpdat.c_str());

    fclose(gnufile);
    fprintf(stdout, "gnuplot message: ");
    fflush(stdout);
    const int i = system(syscall.c_str());
    if(i)
        fprintf(stderr, "\ngnuplot command returned '1' -- the plotting went wrong!\n");
    else
    {
        if(file_name.empty())
            iprint(stdout, "plotted 'plot/%s'\n", timebuf.c_str());
        else
            iprint(stdout, "plotted '%s%s'\n",
                    file_name.c_str(), file_name_suffix.c_str());
    }

    remove(tmpdat.c_str());
    remove(cmdtmp.c_str());
}

void fifo::write_MatToFile(const cv::Mat &mat, const std::string &fname,
                        const bool use_bin)
{
    std::string fname_n;

    if(fname.empty())
    {
        std::string timebuf;
        get_DateAndTime(timebuf, true);
        timebuf += ".dat";

        fname_n = timebuf;
    }
    else
        fname_n = fname;

    FILE *wfile = fopen(fname_n.c_str(), "w");

    if(wfile == NULL)
    {
        file_error_msg(fname_n.c_str(), ERR_ARG);
        return;
    }

    const uint chn = mat.channels(),
               bits_d = mat.depth(),
               mat_rows = mat.rows,
               mat_cols = mat.cols;

    #define IJ_FOR_LOOP \
    for(uint i = 0; i < mat_rows; ++i) \
        for(uint j = 0; j < mat_cols; ++j)

    #define SINGLE_CHANNEL_ARG(__MAT_TYPE__) \
    mat.at<__MAT_TYPE__>(i, j), \
    j < mat_cols - 1 ? ' ' : '\n'

    #define SINGLE_CHANNEL_BIT_ARG(__MAT_TYPE__) \
    for(uint i = 0; i < mat_rows + 1; ++i) \
        for(uint j = 0; j < mat_cols + 1; ++j) \
            if(!j && !i) \
                temp[0] = mat_cols; \
            else if(!i && j != 0) \
                temp[i * (mat_cols + 1) + j] = j - 1; \
            else if(!j && i != 0) \
                temp[i * (mat_cols + 1) + j] = i - 1; \
            else \
                temp[i * (mat_cols + 1) + j] = mat.at<__MAT_TYPE__>(i, j)

    #define TRIPPLE_CHANNEL_VAR(__MAT_TYPE__) \
    const cv::Vec<__MAT_TYPE__, 3> rgb = \
    mat.at<cv::Vec<__MAT_TYPE__, 3>>(i, j);

    #define TRIPPLE_CHANNEL_ARG \
    rgb[0], rgb[1], rgb[2], i, j, \
    j < mat_cols - 1 ? "\n" : "\n\n"

    /** @todo Add a binary output option. */
    if(use_bin)
    {
        if(chn == 1)
        {
            if(bits_d == CV_32S)
            {
                int32_t *temp = alloc_mat_int32(mat_rows + 1, mat_cols + 1);
                SINGLE_CHANNEL_BIT_ARG(int32_t);
                write_Bin_int32(fname_n.c_str(), temp,
                                    (mat_rows + 1) * (mat_cols + 1));
                free(temp);
            }
            else if(bits_d == CV_32F)
            {
                float *temp = alloc_mat_float(mat_rows + 1, mat_cols + 1);
                SINGLE_CHANNEL_BIT_ARG(float);
                write_Bin_float(fname_n.c_str(), temp,
                                    (mat_rows + 1) * (mat_cols + 1));
                free(temp);
            }
            else if(bits_d == CV_64F)
            {
                double *temp = alloc_mat(mat_rows + 1, mat_cols + 1);
                SINGLE_CHANNEL_BIT_ARG(double);
                write_Bin_double(fname_n.c_str(), temp,
                                    (mat_rows + 1) * (mat_cols + 1));
                free(temp);
            }
        }
    }
    else
    {
        if(chn == 1)
        {
            if(bits_d == CV_8U)
            {
                IJ_FOR_LOOP
                fprintf(wfile, "%hhu%c", SINGLE_CHANNEL_ARG(uint8_t));
            }
            else if(bits_d == CV_8S)
            {
                IJ_FOR_LOOP
                fprintf(wfile, "%hhi%c", SINGLE_CHANNEL_ARG(int8_t));
            }
            else if(bits_d == CV_16U)
            {
                IJ_FOR_LOOP
                fprintf(wfile, "%hu%c", SINGLE_CHANNEL_ARG(uint16_t));
            }
            else if(bits_d == CV_16S)
            {
                IJ_FOR_LOOP
                fprintf(wfile, "%hi%c", SINGLE_CHANNEL_ARG(int16_t));
            }
            else if(bits_d == CV_32S)
            {
                IJ_FOR_LOOP
                fprintf(wfile, "%i%c", SINGLE_CHANNEL_ARG(int32_t));
            }
            else if(bits_d == CV_32F)
            {
                IJ_FOR_LOOP
                fprintf(wfile, "%g%c", SINGLE_CHANNEL_ARG(float));
            }
            else if(bits_d == CV_64F)
            {
                IJ_FOR_LOOP
                fprintf(wfile, "%g%c", SINGLE_CHANNEL_ARG(double));
            }
            else
                warn_msg("unrecognised format\n", ERR_ARG);
        }
        else if(chn == 3)
        {
            if(bits_d == CV_8U)
            {
                IJ_FOR_LOOP
                {
                    TRIPPLE_CHANNEL_VAR(uint8_t)
                    fprintf(wfile, "%hhu %hhu %hhu %u %u%s", TRIPPLE_CHANNEL_ARG);
                }
            }
            else if(bits_d == CV_8S)
            {
                IJ_FOR_LOOP
                {
                    TRIPPLE_CHANNEL_VAR(int8_t)
                    fprintf(wfile, "%hhi %hhi %hhi %u %u%s", TRIPPLE_CHANNEL_ARG);
                }
            }
            else if(bits_d == CV_16U)
            {
                IJ_FOR_LOOP
                {
                    TRIPPLE_CHANNEL_VAR(uint16_t)
                    fprintf(wfile, "%hu %hu %hu %u %u%s", TRIPPLE_CHANNEL_ARG);
                }
            }
            else if(bits_d == CV_16S)
            {
                IJ_FOR_LOOP
                {
                    TRIPPLE_CHANNEL_VAR(int16_t)
                    fprintf(wfile, "%hi %hi %hi %u %u%s", TRIPPLE_CHANNEL_ARG);
                }
            }
            else if(bits_d == CV_32S)
            {
                IJ_FOR_LOOP
                {
                    TRIPPLE_CHANNEL_VAR(int32_t)
                    fprintf(wfile, "%i %i %i %u %u%s", TRIPPLE_CHANNEL_ARG);
                }
            }
            else if(bits_d == CV_32F)
            {
                IJ_FOR_LOOP
                {
                    TRIPPLE_CHANNEL_VAR(float)
                    fprintf(wfile, "%g %g %g %u %u%s", TRIPPLE_CHANNEL_ARG);
                }
            }
            else if(bits_d == CV_64F)
            {
                IJ_FOR_LOOP
                {
                    TRIPPLE_CHANNEL_VAR(double)
                    fprintf(wfile, "%g %g %g %u %u%s", TRIPPLE_CHANNEL_ARG);
                }
            }
            else
                warn_msg("unrecognised format\n", ERR_ARG);
        }
        else
            error_msg("unknown matrix format", ERR_ARG);
    }

    #undef IJ_FOR_LOOP
    #undef SINGLE_CHANNEL_ARG
    #undef TRIPPLE_CHANNEL_VAR
    #undef TRIPPLE_CHANNEL_ARG

    fclose(wfile);
}
#endif

/** \brief
 *
 * \param data_x const double *res_pt The measurements points in x in
 micrometer.
 * \param data_y const double *res_pt The measurements points in y in
 micrometer.
 * \param z_pnts_mm const double *res_pt The points along the axis of
 propagation in millimeter.
 * \param res_x const double *res_pt The results from the fit:
 z_{0, x}, w_{0, x} in micrometer.
 * \param res_y const double *res_pt The results from the fit:
 z_{0, y}, w_{0, y} in micrometer.
 * \param lambda_um const double The wavelength in micrometer.
 * \return void
 *
 */
void fifo::plot_BeamWidthsFit(const double *res_pt data_x,
                            const double *res_pt data_y,
                            const double *res_pt z_pnts_mm,
                            const double *res_pt res_x,
                            const double *res_pt res_y,
                            const double lambda_um)
{
    std::string fname_tmp = std::tmpnam(nullptr);
    fname_tmp.erase(std::remove(fname_tmp.begin(), fname_tmp.end(), '/'),
                    fname_tmp.end());
    fname_tmp.erase(std::remove(fname_tmp.begin(), fname_tmp.end(), '\\'),
                    fname_tmp.end());
    const std::string cmdtmp = fname_tmp,
                      tmpdat = fname_tmp + ".dat",
                      syscall = "gnuplot " + cmdtmp;

    if(file_name.empty())
        mkdir("plot" DIRMOD;

    /* Save the data to a temporary file. */
    {
        FILE *tmpfile = fopen(tmpdat.c_str(), "w");
        if(tmpfile == NULL)
        {
            file_error_msg(tmpdat.c_str(), ERR_ARG);
            exit(EXIT_FAILURE);
        }
        for(uint i = 0; i < dat_rows; ++i)
            fprintf(tmpfile, "%g %g %g\n", z_pnts_mm[i], data_x[i], data_y[i]);
        fclose(tmpfile);
    }

    FILE *gnufile = fopen(cmdtmp.c_str(), "w");
    if(gnufile == NULL)
    {
        file_error_msg(cmdtmp.c_str(), ERR_ARG);
        exit(EXIT_FAILURE);
    }
    fprintf(gnufile,
            "set term pngcairo size 800, 600\n"
            "set grid\n"
            "set key outside horizontal top center Left reverse nobox width 2 spacing 1\n"
            "f(x, lambda, z0, w0) = "
            "w0 * sqrt(1. + ((x - z0) / (w0**2 * pi / lambda))**2)\n"
            "set xr [:]\n"
            "set yr [:]\n"
            "set label 1 "
            "sprintf(\"z_{0x}: (%%.3g +- %%.3g) mm, w_{x} = (%%.3g +- %%.3g) "
            "{/Symbol m}m\", %g, %g, %g, %g) at graph .1, .9\n"
            "set label 2 "
            "sprintf(\"z_{0y}: (%%.3g +- %%.3g) mm, w_{y} = (%%.3g +- %%.3g) "
            "{/Symbol m}m\", %g, %g, %g, %g) at graph .1, .85\n"
            "set yl 'Beam radius / {/Symbol m}m' rotate parallel\n"
            "set xl 'Propagation / mm'\n",
            res_x[0], res_x[1] * res_x[0], res_x[2], res_x[3] * res_x[2],
            res_y[0], res_y[1] * res_y[0], res_y[2], res_y[3] * res_y[2]);

    if(!plot_title.empty())
        fprintf(gnufile, "set title '%s'\n", plot_title.c_str());

    std::string timebuf;
    if(file_name.empty())
    {
        get_DateAndTime(timebuf);
        fprintf(gnufile, "set out 'plot/%s.png'\n", timebuf.c_str());
    }
    else
        fprintf(gnufile, "set out '%s'\n", file_name.c_str());

    fprintf(gnufile,
            "plot "
            "f(x * 1e3, %g, %g * 1e3, %g) w l t 'w_{x}(z)', "
            "f(x * 1e3, %g, %g * 1e3, %g) w l t 'w_{y}(z)', "
            "'%s' using ($1):($2) w p t 'Data_{x}', "
            "'%s' using ($1):($3) w p t 'Data_{y}'\n",
            lambda_um, res_x[0], res_x[2],
            lambda_um, res_y[0], res_y[2],
            tmpdat.c_str(), tmpdat.c_str());

    fclose(gnufile);
    iprint(stdout, "gnuplot message: ");
    fflush(stdout);
    const int i = system(syscall.c_str());
    if(i)
        warn_msg("\ngnuplot command returned '1' -- the plotting went wrong!",
                ERR_ARG);
    else
    {
        if(file_name.empty())
            iprint(stdout, "plotted 'plot/%s.png'\n", timebuf.c_str());
        else
            iprint(stdout, "plotted '%s'\n", file_name.c_str());
    }

    remove(cmdtmp.c_str());
    remove(tmpdat.c_str());
}

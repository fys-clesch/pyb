/*
    igyba -- igyba gets your beam analysed
    Copyright (C) 2015  Clemens Sch\"afermeier

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/** @todo add Zernike fit to cope with a tilted background */

#include "../src/header.hpp"
#include "../thorlabs_cam/thorlabs_cam.h"
#ifndef IGYBA_NDEBUG
#include "../grabber/src/test.h"
#endif

using namespace cv;

static const std::string main_win_title = "igyba - Thorlabs cameras";

int main(int argc, char **argv)
{
    iprint(stdout, "%s read:\n", PROJECT_NAME.c_str());
        for(int i = 0; i < argc; i++)
            iprint(stdout,
                   "  argv[%i] = '%s'\n%c",
                   i,
                   argv[i],
                   (i == argc - 1) ? '\n' : ' ');

    thorlabs_cam t_cam;
    t_cam.init_Camera();

    t_cam.show_Intro();
    t_cam.show_Help();

    t_cam.set_ColourMode();
    t_cam.set_ExitMode();
    t_cam.set_Display();
    t_cam.alloc_ImageMem();
    t_cam.set_ImageMem();
    t_cam.set_ImageSize();
    t_cam.inquire_ImageMem((int *)NULL, (int *)NULL, (int *)NULL, (int *)NULL);

    if(t_cam.caught_Error())
    {
        error_msg("error(s) in the camera initialisation detected. " \
                  "messages sent to 'stderr'.\n" \
                  "is the camera connected?\nexiting.",
                  ERR_ARG);
        return EXIT_FAILURE;
    }
    else if(!t_cam.get_Image())
    {
        error_msg("can't get an image from thorlabs_cam instance. good bye.",
                  ERR_ARG);
        return EXIT_FAILURE;
    }

    t_cam.get_Image(t_cam.in);
    t_cam.set_Pix2UmScale(t_cam.get_PixelPitch());
    t_cam.update_Mats_RgbAndFp();
    t_cam.set_MainWindowName(main_win_title);
    /* Track bar setup. */
    namedWindow(t_cam.get_TrackbarWindowName(), CV_WINDOW_AUTOSIZE);
    t_cam.show_Trackbars();
    namedWindow(t_cam.get_CameraInfoWindowName(), CV_WINDOW_AUTOSIZE);
    t_cam.show_CameraTrackbars();
    /* Main window setup. */
    namedWindow(t_cam.get_MainWindowName(),
                CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);

    setMouseCallback(t_cam.get_MainWindowName(),
                     t_cam.cast_static_set_MouseEvent,
                     &t_cam);
    /* Camera track bars. */
    t_cam.create_TrackbarExposure();
    /* Image processing track bars. */
    t_cam.create_TrackbarBlur();
    t_cam.create_TrackbarBlurSize();
    t_cam.create_TrackbarGroundlift();

    t_cam.show_Im_RGB();

    std::thread thread_Viewer(grabber::schedule_Viewer, argc, argv);
    std::thread thread_Copy(grabber::copy_DataToViewer);
    std::thread thread_Minime(grabber::schedule_Minime,
                              1.064,
                              t_cam.get_PixelPitch());

    uint32_t kctrl = 0;
    for(;;)
    {
        if(t_cam.is_Grabbing())
        {
            if(!t_cam.get_Image(t_cam.in))
            {
                error_msg("can't get an image from thorlabs_cam instance",
                          ERR_ARG);
                break;
            }
            t_cam.increment_Frames();
        }

        switch(kctrl)
        {
            case 'r': // 1048690
                t_cam.set_Background();
                break;
            case 'R': // 1114194
                t_cam.unset_Background();
                break;
            case 's': // 1048691
                t_cam.save_Image(t_cam.save_Im_type::RGB);
                break;
            case 'S': // 1114195
                t_cam.save_Image(t_cam.save_Im_type::WORK);
                break;
            case '1': // 1048625
                t_cam.store_Image(t_cam.save_Im_type::RGB);
                break;
            case '2': // 1048626
                t_cam.store_Image(t_cam.save_Im_type::WORK);
                break;
            case '3': // 1048627
                t_cam.store_Image(t_cam.save_Im_type::FP_IN);
                break;
            case 7340032: /**< F1 or 1114046 */
                t_cam.show_Help();
                break;
            case '0': // 1048624
                resizeWindow(t_cam.get_MainWindowName(),
                             t_cam.get_Width(), t_cam.get_Height());
                break;
            case 'o': // 1048603
                t_cam.toggle_Smoothing();
                break;
            case 'X': // 1114200
                t_cam.signal_ViewerThreadIfWait();
                break;
            case 'F': // 1114182
                t_cam.signal_MinimeThreadIfWait();
                t_cam.wait_CameraThread();
                break;
            case 'p': // 1048688
                t_cam.toggle_Grabbing();
                break;
            case 'P':
                t_cam.gnuplot_Image(t_cam.save_Im_type::WORK);
                break;
            default:
                t_cam.update_Mats_RgbAndFp();
                t_cam.get_Moments();
                t_cam.signal_CopyThreadIfWait();

                t_cam.draw_Moments(false);
                t_cam.draw_Info();
                t_cam.draw_CameraInfo();
                t_cam.show_Im_RGB();
                t_cam.show_CameraTrackbars();
                t_cam.show_Trackbars();
//              Mat im_mom_o;
//              get_Moments_own(t_cam.get_Mat_private(t_cam.WORK),
//                                  im_mom_o, t_cam.get_PixelPitch(), false);
//              get_Moments(t_cam.get_Mat_private(t_cam.WORK),
//                              im_mom_o, t_cam.get_PixelPitch(), false);
//              imshow("draw_moments_own", im_mom_o);
        }

        #if SHOW_WAIT_KEY
        iprint(stdout, "kctrl: %u\n", kctrl);
        #endif

        kctrl = waitKey(20);
        if(kctrl == 27 || kctrl == 1048603)
            break;

        HWND *hwnd =
        static_cast<HWND *>(cvGetWindowHandle(main_win_title.c_str()));
        if(hwnd == nullptr)
            break;
    }

    destroyWindow(t_cam.get_TrackbarWindowName());
    destroyWindow(t_cam.get_CameraInfoWindowName());
    destroyWindow(t_cam.get_MainWindowName());

    iprint(stdout, "i made %lu turns\n", t_cam.get_Frames());

    /* First stop minime. */
    t_cam.close_MinimeThread();
    thread_Minime.join();

    /* Next stop reloading data into the viewer. */
    t_cam.close_CopyThread();
    thread_Copy.join();

    /* Then close, if necessary, the display. */
    if(t_cam.is_ViewerWindowRunning())
        t_cam.close_ViewerWindow();

    /* Finally close the actual thread. */
    t_cam.close_ViewerThread();
    thread_Viewer.join();

    /* Wait for a second to rest - and maybe to flush memory. */
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return EXIT_SUCCESS;
}

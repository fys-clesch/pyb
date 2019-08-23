/***************************************************************
 * Name:      igyba_wxMain.h
 * Purpose:   Defines Application Frame
 * Author:    Clemens Sch\"afermeier (clemens@fh-muenster.de)
 * Created:   2015-04-03
 * Copyright: Clemens Sch\"afermeier (clemens@fh-muenster.de)
 * License:
 *
 **************************************************************/

#ifndef IGYBA_WXMAIN_H
#define IGYBA_WXMAIN_H

//(*Headers(igyba_wxFrame)
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/gbsizer.h>
#include <wx/led.h>
#include <wx/menu.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
//*)

#include "../src/header.hpp"
#if USE_IDS_DRIVER
 #include "../ids_cam/ids_cam.h"
#else
 #include "../thorlabs_cam/thorlabs_cam.h"
#endif

class igyba_wxFrame : public wxFrame
{

private:

    #if USE_IDS_DRIVER
     ids_cam
    #else
     thorlabs_cam
    #endif
    t_cam;

    char **mb_argv;

    int m_argc;

    threadhand event_Cam;

    std::thread thread_Cam;

    std::atomic<bool> close_cam_thread,
                      select_roi,
                      force_quit;

    std::atomic<uint32_t> btn_state;

    wxString fname_img_out,
             fname_dat_out,
             fname_gnu_out,
             dirname_bin;

    enum wx_button_clicked
    {
        ERROR_BTN, NONE_BTN,
        SAVE_RGB_BTN, SAVE_WORK_BTN, SAVE_FP_BTN,
        STORE_RGB_BTN, STORE_WORK_BTN, STORE_FP_BTN,
        ACQ_SET_BACKGROUND, UNSET_BACKGROUND,
        TOGGLE_IDLING,
        MAKE_GNUPLOT,
        TOGGLE_SMOOTHING,
        START_VIEWER,
        START_MINIME,
        RESIZE_CAM_WINDOW,
        CLOSE_CAM_WINDOW,
        REMOVE_AOI
    };

        //(*Handlers(igyba_wxFrame)
        void OnQuit(wxCommandEvent &event);
        void OnAbout(wxCommandEvent &event);
        void OnButtonSaveImgRGBClick(wxCommandEvent& event);
        void OnButtonSaveImgWorkClick(wxCommandEvent& event);
        void OnButtonSaveImgFPClick(wxCommandEvent& event);
        void OnButtonSaveDataRGBClick(wxCommandEvent& event);
        void OnButtonSaveDataWorkClick(wxCommandEvent& event);
        void OnButtonSaveDataFPClick(wxCommandEvent& event);
        void OnToggleButtonSmoothingToggle(wxCommandEvent& event);
        void OnToggleButtonBackgroundToggle(wxCommandEvent& event);
        void OnButtonDecExpTimeClick(wxCommandEvent& event);
        void OnButtonIncExpTimeClick(wxCommandEvent& event);
        void OnSliderKernelSizeCmdScroll(wxScrollEvent& event);
        void OnToggleButtonFrameGrabToggle(wxCommandEvent& event);
        void OnToggleButtonViewerToggle(wxCommandEvent& event);
        void OnButtonStartClick(wxCommandEvent& event);
        void OnCloseMainFrame(wxCloseEvent& event);
        void OnButtonGnuplotClick(wxCommandEvent& event);
        void OnButtonResizeCamWinClick(wxCommandEvent& event);
        void OnButtonQuitClick(wxCommandEvent& event);
        void OnSliderExpTimeCmdScroll(wxScrollEvent& event);
        void OnSliderGroundliftCmdScroll(wxScrollEvent& event);
        void OnButtonDecGroundliftClick(wxCommandEvent& event);
        void OnButtonIncGroundliftClick(wxCommandEvent& event);
        void OnSliderStdDevCmdScroll(wxScrollEvent& event);
        void OnButtonDecStdDevClick(wxCommandEvent& event);
        void OnButtonIncStdDevClick(wxCommandEvent& event);
        void OnButtonDecKernelSizeClick(wxCommandEvent& event);
        void OnButtonIncKernelSizeClick(wxCommandEvent& event);
        void OnButtonMinimeClick(wxCommandEvent& event);
        void OnToggleButtonAOIToggle(wxCommandEvent& event);
        void OnToggleButtonMapViewerToggle(wxCommandEvent& event);
        void OnToggleButtonViewerAnimationToggle(wxCommandEvent& event);
        void OnToggleButtonViewerRotationToggle(wxCommandEvent& event);
        void OnButtonViewerScreenshotClick(wxCommandEvent& event);
        //*)

        //(*Identifiers(igyba_wxFrame)
        static const long ID_BUTTON_SAVE_IMG_RGB;
        static const long ID_BUTTON_SAVE_IMG_WORK;
        static const long ID_BUTTON_SAVE_IMG_FP;
        static const long ID_BUTTON_SAVE_DATA_RGB;
        static const long ID_BUTTON_SAVE_DATA_WORK;
        static const long ID_BUTTON_SAVE_DATA_FP;
        static const long ID_BUTTON_GNUPLOT;
        static const long ID_TEXTCTRL_OUTPUT_INFO;
        static const long ID_PANEL_OUTPUT;
        static const long ID_TOGGLEBUTTON_VIEWER;
        static const long ID_TOGGLEBUTTON_VIEWER_ANIMATION;
        static const long ID_TOGGLEBUTTON_MAP_VIEWER;
        static const long ID_TOGGLEBUTTON_VIEWER_ROTATION;
        static const long ID_BUTTON_VIEWER_SCREENSHOT;
        static const long ID_PANEL_VIEWER;
        static const long ID_BUTTON_MINIME;
        static const long ID_PANEL_MINIME;
        static const long ID_NOTEBOOK_THREADS;
        static const long ID_PANEL_THREADS;
        static const long ID_BUTTON_RESIZE_CAM_WIN;
        static const long ID_TEXTCTRL_AOI;
        static const long ID_TOGGLEBUTTON_AOI;
        static const long ID_PANEL_AOI_WIN;
        static const long ID_NOTEBOOK_MAIN;
        static const long ID_TOGGLEBUTTON_FRAMEGRAB;
        static const long ID_STATICLINE2;
        static const long ID_BUTTON_START;
        static const long ID_LED_MAIN;
        static const long ID_BUTTON_QUIT;
        static const long ID_PANEL_MAIN;
        static const long ID_TOGGLEBUTTON_BACKGROUND;
        static const long ID_STATICTEXT_EXP_TIME_DISP;
        static const long ID_BUTTON_DEC_EXP_TIME;
        static const long ID_SLIDER_EXP_TIME;
        static const long ID_BUTTON_INC_EXP_TIME;
        static const long ID_TEXTCTRL_CAM_INFO;
        static const long ID_PANEL_CAMERA;
        static const long ID_TOGGLEBUTTON_SMOOTHING;
        static const long ID_STATICTEXT_KERNEL_SIZE;
        static const long ID_BUTTON_DEC_KERNEL_SIZE;
        static const long ID_SLIDER_KERNEL_SIZE;
        static const long ID_BUTTON_INC_KERNEL_SIZE;
        static const long ID_STATICTEXT_STD_DEV;
        static const long ID_BUTTON_DEC_STD_DEV;
        static const long ID_SLIDER_STD_DEV;
        static const long ID_BUTTON_INC_STD_DEV;
        static const long ID_STATICTEXT_GROUNDLIFT;
        static const long ID_BUTTON_DEC_GROUNDLIFT;
        static const long ID_SLIDER_GROUNDLIFT;
        static const long ID_BUTTON_INC_GROUNDLIFT;
        static const long ID_PANEL_IMG_MANIP;
        static const long ID_NOTEBOOK_CAM_IMG;
        static const long idMenuQuit;
        static const long idMenuAbout;
        static const long ID_STATUSBAR_MAIN;
        //*)

        //(*Declarations(igyba_wxFrame)
        wxButton* ButtonDecExpTime;
        wxButton* ButtonDecGroundlift;
        wxButton* ButtonDecKernelSize;
        wxButton* ButtonDecStdDev;
        wxButton* ButtonGnuplot;
        wxButton* ButtonIncExpTime;
        wxButton* ButtonIncGroundlift;
        wxButton* ButtonIncKernelSize;
        wxButton* ButtonIncStdDev;
        wxButton* ButtonMinime;
        wxButton* ButtonQuit;
        wxButton* ButtonResizeCamWin;
        wxButton* ButtonSaveDataFP;
        wxButton* ButtonSaveDataRGB;
        wxButton* ButtonSaveDataWork;
        wxButton* ButtonSaveImgFP;
        wxButton* ButtonSaveImgRGB;
        wxButton* ButtonSaveImgWork;
        wxButton* ButtonStart;
        wxButton* ButtonViewerScreenshot;
        wxLed* LedMain;
        wxNotebook* NotebookCamImg;
        wxNotebook* NotebookMain;
        wxNotebook* NotebookThreads;
        wxPanel* PanelAOIWin;
        wxPanel* PanelCamera;
        wxPanel* PanelImgManip;
        wxPanel* PanelMain;
        wxPanel* PanelMinime;
        wxPanel* PanelOutput;
        wxPanel* PanelThreads;
        wxPanel* PanelViewer;
        wxSlider* SliderExpTime;
        wxSlider* SliderGroundlift;
        wxSlider* SliderKernelSize;
        wxSlider* SliderStdDev;
        wxStaticLine* StaticLine2;
        wxStaticText* StaticTextExpTimeDisp;
        wxStaticText* StaticTextGroundlift;
        wxStaticText* StaticTextKernelSize;
        wxStaticText* StaticTextStdDev;
        wxStatusBar* StatusBarMain;
        wxTextCtrl* TextCtrlAOI;
        wxTextCtrl* TextCtrlCamInfo;
        wxTextCtrl* TextCtrlOutputInfo;
        wxToggleButton* ToggleButtonAOI;
        wxToggleButton* ToggleButtonBackground;
        wxToggleButton* ToggleButtonFrameGrab;
        wxToggleButton* ToggleButtonMapViewer;
        wxToggleButton* ToggleButtonSmoothing;
        wxToggleButton* ToggleButtonViewer;
        wxToggleButton* ToggleButtonViewerAnimation;
        wxToggleButton* ToggleButtonViewerRotation;
        //*)

        DECLARE_EVENT_TABLE()

    int launch_Cam(int argc, char **argv);
    void close_CamThread(void);
    bool signal_CamThreadIfWait(void);
    void init_SliderExpTime(void);
    void init_SliderGroundlift(void);
    void init_SliderStdDev(void);
    void init_SliderKernelSize(void);
    void update_TextExpTime(const double val = -1.);
    void update_TextOutputInfo(const wxString &str);
    void update_TextAOI(void);
    void update_TextCamInfo(const std::string &str);
    void update_TextGroundlift(const double val = -1.);
    void update_TextStdDev(const double val = -1.);
    void update_TextKernelSize(const uint val = 0);
    void set_MouseEvent(const int event, const int x, const int y,
                        const int flags);

    static void cast_static_set_MouseEvent(const int event,
                                            const int x, const int y,
                                            const int flags,
                                            void *u);
    static void schedule_CamThread(int argc, char **argv);

    static wxString get_wxBuildInfo(void);

    void store_ButtonState(const uint32_t wxb);
    uint32_t load_ButtonState(void);
    void store_CloseCamState(const bool b);
    bool load_CloseCamState(void);
    void store_SelectRoi(const bool b);
    bool load_SelectRoi(void);
    void store_ForceQuit(const bool b);
    bool load_ForceQuit(void);

public:

    igyba_wxFrame(wxWindow* parent,
                  wxWindowID id = -1);
    igyba_wxFrame(int argc,
                  wchar_t **argv,
                  wxWindow *parent,
                  wxWindowID id = -1);
    virtual ~igyba_wxFrame(void);

};

#endif // IGYBA_WXMAIN_H

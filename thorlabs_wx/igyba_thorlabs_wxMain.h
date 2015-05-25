/***************************************************************
 * Name:      igyba_thorlabs_wxMain.h
 * Purpose:   Defines Application Frame
 * Author:    Clemens Schäfermeier (clesch@fysik.dtu.dk)
 * Created:   2015-04-03
 * Copyright: Clemens Schäfermeier ()
 * License:
 *
 * http://zetcode.com/gui/wxwidgets/layoutmanagement/
 * http://docs.wxwidgets.org/trunk/overview_helloworld.html
 **************************************************************/

#ifndef IGYBA_THORLABS_WXMAIN_H
#define IGYBA_THORLABS_WXMAIN_H

//(*Headers(igyba_thorlabs_wxFrame)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/menu.h>
#include <wx/textctrl.h>
#include <wx/led.h>
#include <wx/tglbtn.h>
#include <wx/statline.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/gbsizer.h>
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/statusbr.h>
//*)

#include "../thorlabs_cam/thorlabs_cam.h"

class igyba_thorlabs_wxFrame : public wxFrame
{

private:

	thorlabs_cam t_cam;

	char **mb_argv;

	int m_argc;

	threadhand event_Cam;

	std::thread thread_Cam;

	std::atomic<bool> close_cam_thread,
					  select_roi;

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

		//(*Handlers(igyba_thorlabs_wxFrame)
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

		//(*Identifiers(igyba_thorlabs_wxFrame)
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

		//(*Declarations(igyba_thorlabs_wxFrame)
		wxButton* ButtonSaveImgRGB;
		wxPanel* PanelCamera;
		wxButton* ButtonGnuplot;
		wxButton* ButtonDecExpTime;
		wxPanel* PanelOutput;
		wxToggleButton* ToggleButtonViewerRotation;
		wxButton* ButtonDecGroundlift;
		wxButton* ButtonIncKernelSize;
		wxTextCtrl* TextCtrlAOI;
		wxTextCtrl* TextCtrlCamInfo;
		wxButton* ButtonIncGroundlift;
		wxButton* ButtonSaveImgFP;
		wxStaticText* StaticTextExpTimeDisp;
		wxToggleButton* ToggleButtonBackground;
		wxSlider* SliderKernelSize;
		wxPanel* PanelMinime;
		wxButton* ButtonSaveDataRGB;
		wxToggleButton* ToggleButtonAOI;
		wxToggleButton* ToggleButtonMapViewer;
		wxPanel* PanelViewer;
		wxButton* ButtonMinime;
		wxButton* ButtonSaveDataWork;
		wxStaticLine* StaticLine2;
		wxButton* ButtonIncExpTime;
		wxButton* ButtonViewerScreenshot;
		wxNotebook* NotebookMain;
		wxSlider* SliderGroundlift;
		wxTextCtrl* TextCtrlOutputInfo;
		wxStaticText* StaticTextGroundlift;
		wxButton* ButtonIncStdDev;
		wxLed* LedMain;
		wxPanel* PanelThreads;
		wxSlider* SliderStdDev;
		wxPanel* PanelImgManip;
		wxPanel* PanelMain;
		wxToggleButton* ToggleButtonSmoothing;
		wxButton* ButtonQuit;
		wxStaticText* StaticTextKernelSize;
		wxToggleButton* ToggleButtonViewerAnimation;
		wxButton* ButtonResizeCamWin;
		wxPanel* PanelAOIWin;
		wxNotebook* NotebookThreads;
		wxButton* ButtonDecKernelSize;
		wxStaticText* StaticTextStdDev;
		wxButton* ButtonStart;
		wxSlider* SliderExpTime;
		wxButton* ButtonSaveImgWork;
		wxToggleButton* ToggleButtonFrameGrab;
		wxStatusBar* StatusBarMain;
		wxToggleButton* ToggleButtonViewer;
		wxButton* ButtonSaveDataFP;
		wxNotebook* NotebookCamImg;
		wxButton* ButtonDecStdDev;
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

public:

	igyba_thorlabs_wxFrame(int argc, wchar_t **argv, wxWindow *parent,
							wxWindowID id = -1);
	virtual ~igyba_thorlabs_wxFrame(void);

};

#endif // IGYBA_THORLABS_WXMAIN_H

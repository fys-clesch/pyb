#include "igyba_wxMain.h"

#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/aboutdlg.h>

//(*InternalHeaders(igyba_wxFrame)
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/string.h>
//*)

/** @todo Add a mode matching call. */

static igyba_wxFrame *itw1ptr = nullptr;

//(*IdInit(igyba_wxFrame)
const long igyba_wxFrame::ID_BUTTON_SAVE_IMG_RGB = wxNewId();
const long igyba_wxFrame::ID_BUTTON_SAVE_IMG_WORK = wxNewId();
const long igyba_wxFrame::ID_BUTTON_SAVE_IMG_FP = wxNewId();
const long igyba_wxFrame::ID_BUTTON_SAVE_DATA_RGB = wxNewId();
const long igyba_wxFrame::ID_BUTTON_SAVE_DATA_WORK = wxNewId();
const long igyba_wxFrame::ID_BUTTON_SAVE_DATA_FP = wxNewId();
const long igyba_wxFrame::ID_BUTTON_GNUPLOT = wxNewId();
const long igyba_wxFrame::ID_TEXTCTRL_OUTPUT_INFO = wxNewId();
const long igyba_wxFrame::ID_PANEL_OUTPUT = wxNewId();
const long igyba_wxFrame::ID_TOGGLEBUTTON_VIEWER = wxNewId();
const long igyba_wxFrame::ID_TOGGLEBUTTON_VIEWER_ANIMATION = wxNewId();
const long igyba_wxFrame::ID_TOGGLEBUTTON_MAP_VIEWER = wxNewId();
const long igyba_wxFrame::ID_TOGGLEBUTTON_VIEWER_ROTATION = wxNewId();
const long igyba_wxFrame::ID_BUTTON_VIEWER_SCREENSHOT = wxNewId();
const long igyba_wxFrame::ID_PANEL_VIEWER = wxNewId();
const long igyba_wxFrame::ID_BUTTON_MINIME = wxNewId();
const long igyba_wxFrame::ID_PANEL_MINIME = wxNewId();
const long igyba_wxFrame::ID_NOTEBOOK_THREADS = wxNewId();
const long igyba_wxFrame::ID_PANEL_THREADS = wxNewId();
const long igyba_wxFrame::ID_BUTTON_RESIZE_CAM_WIN = wxNewId();
const long igyba_wxFrame::ID_TEXTCTRL_AOI = wxNewId();
const long igyba_wxFrame::ID_TOGGLEBUTTON_AOI = wxNewId();
const long igyba_wxFrame::ID_PANEL_AOI_WIN = wxNewId();
const long igyba_wxFrame::ID_NOTEBOOK_MAIN = wxNewId();
const long igyba_wxFrame::ID_TOGGLEBUTTON_FRAMEGRAB = wxNewId();
const long igyba_wxFrame::ID_STATICLINE2 = wxNewId();
const long igyba_wxFrame::ID_BUTTON_START = wxNewId();
const long igyba_wxFrame::ID_LED_MAIN = wxNewId();
const long igyba_wxFrame::ID_BUTTON_QUIT = wxNewId();
const long igyba_wxFrame::ID_PANEL_MAIN = wxNewId();
const long igyba_wxFrame::ID_TOGGLEBUTTON_BACKGROUND = wxNewId();
const long igyba_wxFrame::ID_STATICTEXT_EXP_TIME_DISP = wxNewId();
const long igyba_wxFrame::ID_BUTTON_DEC_EXP_TIME = wxNewId();
const long igyba_wxFrame::ID_SLIDER_EXP_TIME = wxNewId();
const long igyba_wxFrame::ID_BUTTON_INC_EXP_TIME = wxNewId();
const long igyba_wxFrame::ID_TEXTCTRL_CAM_INFO = wxNewId();
const long igyba_wxFrame::ID_PANEL_CAMERA = wxNewId();
const long igyba_wxFrame::ID_TOGGLEBUTTON_SMOOTHING = wxNewId();
const long igyba_wxFrame::ID_STATICTEXT_KERNEL_SIZE = wxNewId();
const long igyba_wxFrame::ID_BUTTON_DEC_KERNEL_SIZE = wxNewId();
const long igyba_wxFrame::ID_SLIDER_KERNEL_SIZE = wxNewId();
const long igyba_wxFrame::ID_BUTTON_INC_KERNEL_SIZE = wxNewId();
const long igyba_wxFrame::ID_STATICTEXT_STD_DEV = wxNewId();
const long igyba_wxFrame::ID_BUTTON_DEC_STD_DEV = wxNewId();
const long igyba_wxFrame::ID_SLIDER_STD_DEV = wxNewId();
const long igyba_wxFrame::ID_BUTTON_INC_STD_DEV = wxNewId();
const long igyba_wxFrame::ID_STATICTEXT_GROUNDLIFT = wxNewId();
const long igyba_wxFrame::ID_BUTTON_DEC_GROUNDLIFT = wxNewId();
const long igyba_wxFrame::ID_SLIDER_GROUNDLIFT = wxNewId();
const long igyba_wxFrame::ID_BUTTON_INC_GROUNDLIFT = wxNewId();
const long igyba_wxFrame::ID_PANEL_IMG_MANIP = wxNewId();
const long igyba_wxFrame::ID_NOTEBOOK_CAM_IMG = wxNewId();
const long igyba_wxFrame::idMenuQuit = wxNewId();
const long igyba_wxFrame::idMenuAbout = wxNewId();
const long igyba_wxFrame::ID_STATUSBAR_MAIN = wxNewId();
//*)

BEGIN_EVENT_TABLE(igyba_wxFrame, wxFrame)
    //(*EventTable(igyba_wxFrame)
    //*)
    EVT_COMMAND_SCROLL(ID_SLIDER_EXP_TIME, igyba_wxFrame::OnSliderExpTimeCmdScroll)
    EVT_COMMAND_SCROLL(ID_SLIDER_GROUNDLIFT, igyba_wxFrame::OnSliderGroundliftCmdScroll)
    EVT_COMMAND_SCROLL(ID_SLIDER_KERNEL_SIZE, igyba_wxFrame::OnSliderKernelSizeCmdScroll)
    EVT_COMMAND_SCROLL(ID_SLIDER_STD_DEV, igyba_wxFrame::OnSliderStdDevCmdScroll)
END_EVENT_TABLE()

igyba_wxFrame::igyba_wxFrame(int argc,
                             wchar_t **argv,
                             wxWindow *parent,
                             wxWindowID id)
{
    /* global pointer */
    itw1ptr = static_cast<igyba_wxFrame *>(this);
    /* int */
    m_argc = argc; /* 1 */
    /* char ** */
    mb_argv = (char **)calloc(m_argc, sizeof(char *));
    for(int i = 0; i < m_argc; i++)
    {
        mb_argv[i] = (char *)calloc(wcslen(argv[i]) + 1, sizeof(char));
        wcstombs(mb_argv[i], argv[i], wcslen(argv[i]));
    }
    /* wxString */
    std::wstring str(argv[0]);
    size_t found = str.find_last_of(L"/\\");
    dirname_bin = wxString(str.substr(0, found));
    /* atomic<bool> */
    select_roi.store(false, std::memory_order_relaxed);
    force_quit.store(false, std::memory_order_relaxed);
    close_cam_thread.store(false, std::memory_order_relaxed); /* 3 */
    /* atomic<uint32_t> */
    btn_state.store(NONE_BTN, std::memory_order_relaxed); /* 1 */

    //(*Initialize(igyba_wxFrame)
    wxBoxSizer* BoxSizerInnerMain;
    wxBoxSizer* BoxSizerMain;
    wxBoxSizer* BoxSizerThreads;
    wxFlexGridSizer* FlexGridSizerStart;
    wxGridBagSizer* GridBagSizerExpTime;
    wxGridBagSizer* GridBagSizerGroundlift;
    wxGridBagSizer* GridBagSizerKernelSize;
    wxGridBagSizer* GridBagSizerStdDev;
    wxMenu* Menu1;
    wxMenu* Menu2;
    wxMenuBar* MenuBarMain;
    wxMenuItem* MenuItemAbout;
    wxMenuItem* MenuItemQuit;
    wxStaticBoxSizer* StaticBoxSizerAOI;
    wxStaticBoxSizer* StaticBoxSizerAOIWin;
    wxStaticBoxSizer* StaticBoxSizerCamInfo;
    wxStaticBoxSizer* StaticBoxSizerCamera;
    wxStaticBoxSizer* StaticBoxSizerControlsMain;
    wxStaticBoxSizer* StaticBoxSizerExpTime;
    wxStaticBoxSizer* StaticBoxSizerGroundlift;
    wxStaticBoxSizer* StaticBoxSizerImgManip;
    wxStaticBoxSizer* StaticBoxSizerKernelSize;
    wxStaticBoxSizer* StaticBoxSizerMinime;
    wxStaticBoxSizer* StaticBoxSizerOutput;
    wxStaticBoxSizer* StaticBoxSizerOutputInfo;
    wxStaticBoxSizer* StaticBoxSizerSaveData;
    wxStaticBoxSizer* StaticBoxSizerSaveImg;
    wxStaticBoxSizer* StaticBoxSizerStdDev;
    wxStaticBoxSizer* StaticBoxSizerViewer;
    wxStaticBoxSizer* StaticBoxSizerViewerDispSet;
    wxStaticBoxSizer* StaticBoxSizerViewerOutput;

    Create(parent, wxID_ANY, _("igyba 4 fingers"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    {
    	wxIcon FrameIcon;
    	FrameIcon.CopyFromBitmap(wxBitmap(wxImage(_T("icon.ico"))));
    	SetIcon(FrameIcon);
    }
    BoxSizerMain = new wxBoxSizer(wxHORIZONTAL);
    PanelMain = new wxPanel(this, ID_PANEL_MAIN, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_MAIN"));
    BoxSizerInnerMain = new wxBoxSizer(wxVERTICAL);
    NotebookMain = new wxNotebook(PanelMain, ID_NOTEBOOK_MAIN, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_MAIN"));
    PanelOutput = new wxPanel(NotebookMain, ID_PANEL_OUTPUT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_OUTPUT"));
    StaticBoxSizerOutput = new wxStaticBoxSizer(wxVERTICAL, PanelOutput, _("Controls"));
    StaticBoxSizerSaveImg = new wxStaticBoxSizer(wxHORIZONTAL, PanelOutput, _("Save image"));
    ButtonSaveImgRGB = new wxButton(PanelOutput, ID_BUTTON_SAVE_IMG_RGB, _("Display"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_IMG_RGB"));
    ButtonSaveImgRGB->SetToolTip(_("Save the displayed image"));
    StaticBoxSizerSaveImg->Add(ButtonSaveImgRGB, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSaveImgWork = new wxButton(PanelOutput, ID_BUTTON_SAVE_IMG_WORK, _("Work"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_IMG_WORK"));
    ButtonSaveImgWork->SetToolTip(_("Save the working image"));
    StaticBoxSizerSaveImg->Add(ButtonSaveImgWork, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSaveImgFP = new wxButton(PanelOutput, ID_BUTTON_SAVE_IMG_FP, _("Raw FP"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_IMG_FP"));
    ButtonSaveImgFP->SetToolTip(_("Save the raw, floating point, single channel image"));
    StaticBoxSizerSaveImg->Add(ButtonSaveImgFP, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerOutput->Add(StaticBoxSizerSaveImg, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerSaveData = new wxStaticBoxSizer(wxHORIZONTAL, PanelOutput, _("Save data"));
    ButtonSaveDataRGB = new wxButton(PanelOutput, ID_BUTTON_SAVE_DATA_RGB, _("Raw 3C"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_DATA_RGB"));
    ButtonSaveDataRGB->SetToolTip(_("Save the raw 3 channel image data"));
    StaticBoxSizerSaveData->Add(ButtonSaveDataRGB, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSaveDataWork = new wxButton(PanelOutput, ID_BUTTON_SAVE_DATA_WORK, _("Work"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_DATA_WORK"));
    ButtonSaveDataWork->SetToolTip(_("Save the working image data"));
    StaticBoxSizerSaveData->Add(ButtonSaveDataWork, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSaveDataFP = new wxButton(PanelOutput, ID_BUTTON_SAVE_DATA_FP, _("Raw 1C"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_DATA_FP"));
    ButtonSaveDataFP->SetToolTip(_("Save the raw 1 channel image data"));
    StaticBoxSizerSaveData->Add(ButtonSaveDataFP, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerOutput->Add(StaticBoxSizerSaveData, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonGnuplot = new wxButton(PanelOutput, ID_BUTTON_GNUPLOT, _("Make gnuplot"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_GNUPLOT"));
    ButtonGnuplot->SetToolTip(_("Plot current processed image data"));
    StaticBoxSizerOutput->Add(ButtonGnuplot, 0, wxALL|wxEXPAND, 5);
    StaticBoxSizerOutputInfo = new wxStaticBoxSizer(wxHORIZONTAL, PanelOutput, _("Output"));
    TextCtrlOutputInfo = new wxTextCtrl(PanelOutput, ID_TEXTCTRL_OUTPUT_INFO, _("Displays last saved data"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH|wxSTATIC_BORDER, wxDefaultValidator, _T("ID_TEXTCTRL_OUTPUT_INFO"));
    TextCtrlOutputInfo->SetMaxLength(512);
    TextCtrlOutputInfo->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
    StaticBoxSizerOutputInfo->Add(TextCtrlOutputInfo, 1, wxALL|wxEXPAND, 5);
    StaticBoxSizerOutput->Add(StaticBoxSizerOutputInfo, 0, wxALL|wxEXPAND, 5);
    PanelOutput->SetSizer(StaticBoxSizerOutput);
    StaticBoxSizerOutput->Fit(PanelOutput);
    StaticBoxSizerOutput->SetSizeHints(PanelOutput);
    PanelThreads = new wxPanel(NotebookMain, ID_PANEL_THREADS, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_THREADS"));
    BoxSizerThreads = new wxBoxSizer(wxVERTICAL);
    NotebookThreads = new wxNotebook(PanelThreads, ID_NOTEBOOK_THREADS, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_THREADS"));
    PanelViewer = new wxPanel(NotebookThreads, ID_PANEL_VIEWER, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_VIEWER"));
    StaticBoxSizerViewer = new wxStaticBoxSizer(wxVERTICAL, PanelViewer, _("Controls"));
    ToggleButtonViewer = new wxToggleButton(PanelViewer, ID_TOGGLEBUTTON_VIEWER, _("Launch viewer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON_VIEWER"));
    ToggleButtonViewer->SetToolTip(_("Toggle 3D live plot for the current AOI"));
    StaticBoxSizerViewer->Add(ToggleButtonViewer, 0, wxALL|wxEXPAND, 5);
    StaticBoxSizerViewerDispSet = new wxStaticBoxSizer(wxVERTICAL, PanelViewer, _("Display settings"));
    ToggleButtonViewerAnimation = new wxToggleButton(PanelViewer, ID_TOGGLEBUTTON_VIEWER_ANIMATION, _("Stop animation"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON_VIEWER_ANIMATION"));
    ToggleButtonViewerAnimation->Disable();
    ToggleButtonViewerAnimation->SetToolTip(_("Toggle animation on/off"));
    StaticBoxSizerViewerDispSet->Add(ToggleButtonViewerAnimation, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ToggleButtonMapViewer = new wxToggleButton(PanelViewer, ID_TOGGLEBUTTON_MAP_VIEWER, _("Show 2D map"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON_MAP_VIEWER"));
    ToggleButtonMapViewer->Disable();
    ToggleButtonMapViewer->SetToolTip(_("Toggle 3D/2D mode"));
    StaticBoxSizerViewerDispSet->Add(ToggleButtonMapViewer, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ToggleButtonViewerRotation = new wxToggleButton(PanelViewer, ID_TOGGLEBUTTON_VIEWER_ROTATION, _("Start rotation"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON_VIEWER_ROTATION"));
    ToggleButtonViewerRotation->Disable();
    ToggleButtonViewerRotation->SetToolTip(_("Toggle rotation in 3D mode"));
    StaticBoxSizerViewerDispSet->Add(ToggleButtonViewerRotation, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerViewer->Add(StaticBoxSizerViewerDispSet, 0, wxALL|wxEXPAND, 5);
    StaticBoxSizerViewerOutput = new wxStaticBoxSizer(wxVERTICAL, PanelViewer, _("Output"));
    ButtonViewerScreenshot = new wxButton(PanelViewer, ID_BUTTON_VIEWER_SCREENSHOT, _("Save screenshot"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_VIEWER_SCREENSHOT"));
    ButtonViewerScreenshot->Disable();
    StaticBoxSizerViewerOutput->Add(ButtonViewerScreenshot, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerViewer->Add(StaticBoxSizerViewerOutput, 0, wxALL|wxEXPAND, 5);
    PanelViewer->SetSizer(StaticBoxSizerViewer);
    StaticBoxSizerViewer->Fit(PanelViewer);
    StaticBoxSizerViewer->SetSizeHints(PanelViewer);
    PanelMinime = new wxPanel(NotebookThreads, ID_PANEL_MINIME, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_MINIME"));
    StaticBoxSizerMinime = new wxStaticBoxSizer(wxVERTICAL, PanelMinime, _("Controls"));
    ButtonMinime = new wxButton(PanelMinime, ID_BUTTON_MINIME, _("Launch minime"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MINIME"));
    ButtonMinime->SetToolTip(_("Start a fitting routine for the current AOI"));
    StaticBoxSizerMinime->Add(ButtonMinime, 0, wxALL|wxEXPAND, 5);
    PanelMinime->SetSizer(StaticBoxSizerMinime);
    StaticBoxSizerMinime->Fit(PanelMinime);
    StaticBoxSizerMinime->SetSizeHints(PanelMinime);
    NotebookThreads->AddPage(PanelViewer, _("Viewer"), true);
    NotebookThreads->AddPage(PanelMinime, _("Minime"), false);
    BoxSizerThreads->Add(NotebookThreads, 1, wxALL|wxEXPAND, 0);
    PanelThreads->SetSizer(BoxSizerThreads);
    BoxSizerThreads->Fit(PanelThreads);
    BoxSizerThreads->SetSizeHints(PanelThreads);
    PanelAOIWin = new wxPanel(NotebookMain, ID_PANEL_AOI_WIN, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_AOI_WIN"));
    StaticBoxSizerAOIWin = new wxStaticBoxSizer(wxVERTICAL, PanelAOIWin, _("Controls"));
    ButtonResizeCamWin = new wxButton(PanelAOIWin, ID_BUTTON_RESIZE_CAM_WIN, _("Resize window"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_RESIZE_CAM_WIN"));
    ButtonResizeCamWin->SetToolTip(_("Resizes the camera display window"));
    StaticBoxSizerAOIWin->Add(ButtonResizeCamWin, 0, wxALL|wxEXPAND, 5);
    StaticBoxSizerAOI = new wxStaticBoxSizer(wxVERTICAL, PanelAOIWin, _("Area of interest"));
    TextCtrlAOI = new wxTextCtrl(PanelAOIWin, ID_TEXTCTRL_AOI, _("No AOI selected"), wxDefaultPosition, wxSize(-1,35), wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH|wxTE_CENTRE|wxSTATIC_BORDER, wxDefaultValidator, _T("ID_TEXTCTRL_AOI"));
    TextCtrlAOI->SetMaxLength(256);
    TextCtrlAOI->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
    StaticBoxSizerAOI->Add(TextCtrlAOI, 1, wxALL|wxEXPAND, 5);
    ToggleButtonAOI = new wxToggleButton(PanelAOIWin, ID_TOGGLEBUTTON_AOI, _("Draw rectangle"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON_AOI"));
    ToggleButtonAOI->SetToolTip(_("Click and draw a rectangle in the camera window"));
    StaticBoxSizerAOI->Add(ToggleButtonAOI, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerAOIWin->Add(StaticBoxSizerAOI, 0, wxALL|wxEXPAND, 5);
    PanelAOIWin->SetSizer(StaticBoxSizerAOIWin);
    StaticBoxSizerAOIWin->Fit(PanelAOIWin);
    StaticBoxSizerAOIWin->SetSizeHints(PanelAOIWin);
    NotebookMain->AddPage(PanelOutput, _("Output"), true);
    NotebookMain->AddPage(PanelThreads, _("Threads"), false);
    NotebookMain->AddPage(PanelAOIWin, _("AOI && Window"), false);
    BoxSizerInnerMain->Add(NotebookMain, 1, wxALL|wxEXPAND, 0);
    StaticBoxSizerControlsMain = new wxStaticBoxSizer(wxVERTICAL, PanelMain, _("Main controls"));
    ToggleButtonFrameGrab = new wxToggleButton(PanelMain, ID_TOGGLEBUTTON_FRAMEGRAB, _("Idle frame grab"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON_FRAMEGRAB"));
    ToggleButtonFrameGrab->SetToolTip(_("Toggle frame grabbing"));
    StaticBoxSizerControlsMain->Add(ToggleButtonFrameGrab, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxSHAPED, 5);
    StaticLine2 = new wxStaticLine(PanelMain, ID_STATICLINE2, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE2"));
    StaticBoxSizerControlsMain->Add(StaticLine2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxSHAPED, 5);
    FlexGridSizerStart = new wxFlexGridSizer(1, 3, 0, 0);
    ButtonStart = new wxButton(PanelMain, ID_BUTTON_START, _("Start"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_START"));
    ButtonStart->Disable();
    FlexGridSizerStart->Add(ButtonStart, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LedMain = new wxLed(PanelMain,ID_LED_MAIN,wxColour(128,128,128),wxColour(0,255,0),wxColour(255,0,0),wxDefaultPosition,wxDefaultSize);
    LedMain->Disable();
    LedMain->SwitchOff();
    FlexGridSizerStart->Add(LedMain, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxSHAPED|wxFIXED_MINSIZE, 5);
    ButtonQuit = new wxButton(PanelMain, ID_BUTTON_QUIT, _("Quit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_QUIT"));
    ButtonQuit->Disable();
    ButtonQuit->SetToolTip(_("Quit the application"));
    FlexGridSizerStart->Add(ButtonQuit, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerControlsMain->Add(FlexGridSizerStart, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizerInnerMain->Add(StaticBoxSizerControlsMain, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxSHAPED, 5);
    PanelMain->SetSizer(BoxSizerInnerMain);
    BoxSizerInnerMain->Fit(PanelMain);
    BoxSizerInnerMain->SetSizeHints(PanelMain);
    BoxSizerMain->Add(PanelMain, 0, wxALL|wxEXPAND, 0);
    NotebookCamImg = new wxNotebook(this, ID_NOTEBOOK_CAM_IMG, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_CAM_IMG"));
    PanelCamera = new wxPanel(NotebookCamImg, ID_PANEL_CAMERA, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_CAMERA"));
    StaticBoxSizerCamera = new wxStaticBoxSizer(wxVERTICAL, PanelCamera, _("Controls"));
    ToggleButtonBackground = new wxToggleButton(PanelCamera, ID_TOGGLEBUTTON_BACKGROUND, _("Acquire background"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON_BACKGROUND"));
    ToggleButtonBackground->SetToolTip(_("Toggle background subtraction"));
    StaticBoxSizerCamera->Add(ToggleButtonBackground, 0, wxALL|wxEXPAND, 5);
    StaticBoxSizerExpTime = new wxStaticBoxSizer(wxVERTICAL, PanelCamera, _("Exposure time"));
    StaticTextExpTimeDisp = new wxStaticText(PanelCamera, ID_STATICTEXT_EXP_TIME_DISP, _("Time / ms"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_EXP_TIME_DISP"));
    StaticBoxSizerExpTime->Add(StaticTextExpTimeDisp, 1, wxALL|wxEXPAND, 5);
    GridBagSizerExpTime = new wxGridBagSizer(0, 0);
    ButtonDecExpTime = new wxButton(PanelCamera, ID_BUTTON_DEC_EXP_TIME, _("-"), wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_BUTTON_DEC_EXP_TIME"));
    GridBagSizerExpTime->Add(ButtonDecExpTime, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderExpTime = new wxSlider(PanelCamera, ID_SLIDER_EXP_TIME, 0, 0, 100, wxDefaultPosition, wxSize(150,-1), 0, wxDefaultValidator, _T("ID_SLIDER_EXP_TIME"));
    SliderExpTime->SetToolTip(_("Changes the exposure time of the camera"));
    GridBagSizerExpTime->Add(SliderExpTime, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonIncExpTime = new wxButton(PanelCamera, ID_BUTTON_INC_EXP_TIME, _("+"), wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_BUTTON_INC_EXP_TIME"));
    GridBagSizerExpTime->Add(ButtonIncExpTime, wxGBPosition(0, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerExpTime->Add(GridBagSizerExpTime, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerCamera->Add(StaticBoxSizerExpTime, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxSHAPED, 5);
    StaticBoxSizerCamInfo = new wxStaticBoxSizer(wxHORIZONTAL, PanelCamera, _("Information"));
    TextCtrlCamInfo = new wxTextCtrl(PanelCamera, ID_TEXTCTRL_CAM_INFO, _("Loading..."), wxDefaultPosition, wxSize(-1,50), wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH|wxSTATIC_BORDER, wxDefaultValidator, _T("ID_TEXTCTRL_CAM_INFO"));
    TextCtrlCamInfo->SetMaxLength(512);
    TextCtrlCamInfo->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
    StaticBoxSizerCamInfo->Add(TextCtrlCamInfo, 1, wxALL|wxEXPAND, 5);
    StaticBoxSizerCamera->Add(StaticBoxSizerCamInfo, 0, wxALL|wxEXPAND, 5);
    PanelCamera->SetSizer(StaticBoxSizerCamera);
    StaticBoxSizerCamera->Fit(PanelCamera);
    StaticBoxSizerCamera->SetSizeHints(PanelCamera);
    PanelImgManip = new wxPanel(NotebookCamImg, ID_PANEL_IMG_MANIP, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_IMG_MANIP"));
    StaticBoxSizerImgManip = new wxStaticBoxSizer(wxVERTICAL, PanelImgManip, _("Controls"));
    ToggleButtonSmoothing = new wxToggleButton(PanelImgManip, ID_TOGGLEBUTTON_SMOOTHING, _("Enable manipulation"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON_SMOOTHING"));
    ToggleButtonSmoothing->SetToolTip(_("Toggle smoothing and groundlift"));
    StaticBoxSizerImgManip->Add(ToggleButtonSmoothing, 0, wxALL|wxEXPAND, 5);
    StaticBoxSizerKernelSize = new wxStaticBoxSizer(wxVERTICAL, PanelImgManip, _("Kernel size"));
    StaticTextKernelSize = new wxStaticText(PanelImgManip, ID_STATICTEXT_KERNEL_SIZE, _("Size / pixel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_KERNEL_SIZE"));
    StaticBoxSizerKernelSize->Add(StaticTextKernelSize, 1, wxALL|wxEXPAND, 5);
    GridBagSizerKernelSize = new wxGridBagSizer(0, 0);
    ButtonDecKernelSize = new wxButton(PanelImgManip, ID_BUTTON_DEC_KERNEL_SIZE, _("-"), wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_BUTTON_DEC_KERNEL_SIZE"));
    ButtonDecKernelSize->Disable();
    GridBagSizerKernelSize->Add(ButtonDecKernelSize, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderKernelSize = new wxSlider(PanelImgManip, ID_SLIDER_KERNEL_SIZE, 0, 0, 100, wxDefaultPosition, wxSize(150,-1), 0, wxDefaultValidator, _T("ID_SLIDER_KERNEL_SIZE"));
    SliderKernelSize->Disable();
    GridBagSizerKernelSize->Add(SliderKernelSize, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonIncKernelSize = new wxButton(PanelImgManip, ID_BUTTON_INC_KERNEL_SIZE, _("+"), wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_BUTTON_INC_KERNEL_SIZE"));
    ButtonIncKernelSize->Disable();
    GridBagSizerKernelSize->Add(ButtonIncKernelSize, wxGBPosition(0, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerKernelSize->Add(GridBagSizerKernelSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerImgManip->Add(StaticBoxSizerKernelSize, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxSHAPED, 5);
    StaticBoxSizerStdDev = new wxStaticBoxSizer(wxVERTICAL, PanelImgManip, _("Standard deviation"));
    StaticTextStdDev = new wxStaticText(PanelImgManip, ID_STATICTEXT_STD_DEV, _("Width / pixel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_STD_DEV"));
    StaticBoxSizerStdDev->Add(StaticTextStdDev, 1, wxALL|wxEXPAND, 5);
    GridBagSizerStdDev = new wxGridBagSizer(0, 0);
    ButtonDecStdDev = new wxButton(PanelImgManip, ID_BUTTON_DEC_STD_DEV, _("-"), wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_BUTTON_DEC_STD_DEV"));
    ButtonDecStdDev->Disable();
    GridBagSizerStdDev->Add(ButtonDecStdDev, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderStdDev = new wxSlider(PanelImgManip, ID_SLIDER_STD_DEV, 0, 0, 100, wxDefaultPosition, wxSize(150,-1), 0, wxDefaultValidator, _T("ID_SLIDER_STD_DEV"));
    SliderStdDev->Disable();
    GridBagSizerStdDev->Add(SliderStdDev, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonIncStdDev = new wxButton(PanelImgManip, ID_BUTTON_INC_STD_DEV, _("+"), wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_BUTTON_INC_STD_DEV"));
    ButtonIncStdDev->Disable();
    GridBagSizerStdDev->Add(ButtonIncStdDev, wxGBPosition(0, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerStdDev->Add(GridBagSizerStdDev, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerImgManip->Add(StaticBoxSizerStdDev, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxSHAPED, 5);
    StaticBoxSizerGroundlift = new wxStaticBoxSizer(wxVERTICAL, PanelImgManip, _("Groundlift"));
    StaticTextGroundlift = new wxStaticText(PanelImgManip, ID_STATICTEXT_GROUNDLIFT, _("Lift / counts"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_GROUNDLIFT"));
    StaticBoxSizerGroundlift->Add(StaticTextGroundlift, 1, wxALL|wxEXPAND, 5);
    GridBagSizerGroundlift = new wxGridBagSizer(0, 0);
    ButtonDecGroundlift = new wxButton(PanelImgManip, ID_BUTTON_DEC_GROUNDLIFT, _("-"), wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_BUTTON_DEC_GROUNDLIFT"));
    ButtonDecGroundlift->Disable();
    GridBagSizerGroundlift->Add(ButtonDecGroundlift, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderGroundlift = new wxSlider(PanelImgManip, ID_SLIDER_GROUNDLIFT, 0, 0, 100, wxDefaultPosition, wxSize(150,-1), 0, wxDefaultValidator, _T("ID_SLIDER_GROUNDLIFT"));
    SliderGroundlift->Disable();
    GridBagSizerGroundlift->Add(SliderGroundlift, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonIncGroundlift = new wxButton(PanelImgManip, ID_BUTTON_INC_GROUNDLIFT, _("+"), wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_BUTTON_INC_GROUNDLIFT"));
    ButtonIncGroundlift->Disable();
    GridBagSizerGroundlift->Add(ButtonIncGroundlift, wxGBPosition(0, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerGroundlift->Add(GridBagSizerGroundlift, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizerImgManip->Add(StaticBoxSizerGroundlift, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxSHAPED, 5);
    PanelImgManip->SetSizer(StaticBoxSizerImgManip);
    StaticBoxSizerImgManip->Fit(PanelImgManip);
    StaticBoxSizerImgManip->SetSizeHints(PanelImgManip);
    NotebookCamImg->AddPage(PanelCamera, _("Camera"), true);
    NotebookCamImg->AddPage(PanelImgManip, _("Image manipulation"), false);
    BoxSizerMain->Add(NotebookCamImg, 1, wxALL|wxEXPAND, 0);
    SetSizer(BoxSizerMain);
    MenuBarMain = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItemQuit = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItemQuit);
    MenuBarMain->Append(Menu1, _("&File"));
    Menu2 = new wxMenu();
    MenuItemAbout = new wxMenuItem(Menu2, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(MenuItemAbout);
    MenuBarMain->Append(Menu2, _("H&elp"));
    SetMenuBar(MenuBarMain);
    StatusBarMain = new wxStatusBar(this, ID_STATUSBAR_MAIN, 0, _T("ID_STATUSBAR_MAIN"));
    int __wxStatusBarWidths_1[1] = { -1 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    StatusBarMain->SetFieldsCount(1,__wxStatusBarWidths_1);
    StatusBarMain->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(StatusBarMain);
    BoxSizerMain->Fit(this);
    BoxSizerMain->SetSizeHints(this);

    Connect(ID_BUTTON_SAVE_IMG_RGB,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonSaveImgRGBClick);
    Connect(ID_BUTTON_SAVE_IMG_WORK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonSaveImgWorkClick);
    Connect(ID_BUTTON_SAVE_IMG_FP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonSaveImgFPClick);
    Connect(ID_BUTTON_SAVE_DATA_RGB,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonSaveDataRGBClick);
    Connect(ID_BUTTON_SAVE_DATA_WORK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonSaveDataWorkClick);
    Connect(ID_BUTTON_SAVE_DATA_FP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonSaveDataFPClick);
    Connect(ID_BUTTON_GNUPLOT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonGnuplotClick);
    Connect(ID_TOGGLEBUTTON_VIEWER,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnToggleButtonViewerToggle);
    Connect(ID_TOGGLEBUTTON_VIEWER_ANIMATION,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnToggleButtonViewerAnimationToggle);
    Connect(ID_TOGGLEBUTTON_MAP_VIEWER,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnToggleButtonMapViewerToggle);
    Connect(ID_TOGGLEBUTTON_VIEWER_ROTATION,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnToggleButtonViewerRotationToggle);
    Connect(ID_BUTTON_VIEWER_SCREENSHOT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonViewerScreenshotClick);
    Connect(ID_BUTTON_MINIME,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonMinimeClick);
    Connect(ID_BUTTON_RESIZE_CAM_WIN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonResizeCamWinClick);
    Connect(ID_TOGGLEBUTTON_AOI,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnToggleButtonAOIToggle);
    Connect(ID_TOGGLEBUTTON_FRAMEGRAB,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnToggleButtonFrameGrabToggle);
    Connect(ID_BUTTON_START,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonStartClick);
    Connect(ID_BUTTON_QUIT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonQuitClick);
    Connect(ID_TOGGLEBUTTON_BACKGROUND,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnToggleButtonBackgroundToggle);
    Connect(ID_BUTTON_DEC_EXP_TIME,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonDecExpTimeClick);
    Connect(ID_BUTTON_INC_EXP_TIME,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonIncExpTimeClick);
    Connect(ID_TOGGLEBUTTON_SMOOTHING,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnToggleButtonSmoothingToggle);
    Connect(ID_BUTTON_DEC_KERNEL_SIZE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonDecKernelSizeClick);
    Connect(ID_BUTTON_INC_KERNEL_SIZE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonIncKernelSizeClick);
    Connect(ID_BUTTON_DEC_STD_DEV,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonDecStdDevClick);
    Connect(ID_BUTTON_INC_STD_DEV,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonIncStdDevClick);
    Connect(ID_BUTTON_DEC_GROUNDLIFT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonDecGroundliftClick);
    Connect(ID_BUTTON_INC_GROUNDLIFT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&igyba_wxFrame::OnButtonIncGroundliftClick);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&igyba_wxFrame::OnQuit);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&igyba_wxFrame::OnAbout);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&igyba_wxFrame::OnCloseMainFrame);
    //*)
    /* Proper way to cast: wxCommandEventHandler(igyba_wxFrame::OnAbout) */

    thread_Cam = std::thread(igyba_wxFrame::schedule_CamThread,
                             m_argc,
                             mb_argv);
    event_Cam.signal();
    LedMain->Enable();
}

igyba_wxFrame::~igyba_wxFrame(void)
{
    //(*Destroy(igyba_wxFrame)
    //*)
    for(int i = 0; i < m_argc; i++)
        free(mb_argv[i]);
    free(mb_argv);
    #ifndef IGYBA_NDEBUG
    iprint(stdout, "'%s': memory released\n", __func__);
    #endif
}

void igyba_wxFrame::update_TextExpTime(const double val)
{
    static double tmin, tmax, tinc;
    double time = val;
    static bool once = false;
    if(!once || val == -1.)
    {
        once = true;
        t_cam.get_ExposureTimesAtomic(&time, &tmin, &tmax, &tinc);
    }
    wxString out;
    out << "Time " << time << " ms";
    StaticTextExpTimeDisp->SetLabel(out);
}

void igyba_wxFrame::update_TextOutputInfo(const wxString &str)
{
    TextCtrlOutputInfo->SetLabel(str);
}

void igyba_wxFrame::update_TextAOI(void)
{
    wxString out;
    if(!t_cam.get_RoiActive() && !t_cam.get_MouseDrag())
        out << "No AOI selected";
    else if(t_cam.get_MouseDrag())
        out << "Selecting AOI...";
    else
    {
        int sx,
            sy,
            sw,
            sh;
        t_cam.get_RectRoi(&sx,
                          &sy,
                          &sw,
                          &sh);
        out << "AOI width (start / span): (" <<
               sx << ", " <<
               sw << ") pixel\n" <<
               "AOI height (start / span): (" <<
               sy << ", " <<
               sh << ") pixel";
    }
    TextCtrlAOI->SetLabel(out);
}

void igyba_wxFrame::update_TextCamInfo(const std::string &str)
{
    wxString out(str);
    if(out.IsEmpty())
        TextCtrlCamInfo->SetLabel("Loading...");
    else
        TextCtrlCamInfo->SetLabel(out);
}

void igyba_wxFrame::update_TextGroundlift(const double val)
{
    static double gl_max;
    double gl = val;
    static bool once = false;
    if(!once || val == -1.)
    {
        once = true;
        t_cam.get_GroundliftRangeAtomic(&gl, &gl_max);
    }
    wxString out;
    out << "Lift " << gl << " counts";
    StaticTextGroundlift->SetLabel(out);
}

void igyba_wxFrame::update_TextStdDev(const double val)
{
    static double gb_min, gb_max;
    double gb = val;
    static bool once = false;
    if(!once || val == -1.)
    {
        once = true;
        gb = t_cam.get_GaussBlurRangeAtomic(&gb_min, &gb_max);
    }
    wxString out;
    out << "Width " << gb << " pixel";
    StaticTextStdDev->SetLabel(out);
}

void igyba_wxFrame::update_TextKernelSize(const uint val)
{
    static uint sze_min, sze_max;
    uint sze = val;
    static bool once = false;
    if(!once || !val)
    {
        once = true;
        sze = t_cam.get_KernelSizeAtomic(&sze_min, &sze_max);
    }
    wxString out;
    out << "Width " << sze << " pixel";
    StaticTextKernelSize->SetLabel(out);
}

int igyba_wxFrame::launch_Cam(int argc, char **argv)
{
    static const std::string main_win_title = "igyba - IDS cameras";
    const double wavelen_um = 1.064;

    iprint(stdout, "%s read:\n", PROJECT_NAME.c_str());
        for(int i = 0; i < argc; ++i)
            iprint(stdout,
                   "  argv[%i] = '%s'\n%c",
                   i,
                   argv[i],
                   (i == argc - 1) ? '\n' : ' ');

    (*itw1ptr).t_cam.init_Camera();
    (*itw1ptr).t_cam.set_PixelClock("min");

    (*itw1ptr).t_cam.show_Intro();

    (*itw1ptr).t_cam.set_ColourMode();
    (*itw1ptr).t_cam.set_ExitMode();
    (*itw1ptr).t_cam.set_Display();
    (*itw1ptr).t_cam.alloc_ImageMem();
    (*itw1ptr).t_cam.set_ImageMem();
    (*itw1ptr).t_cam.set_ImageSize();
    (*itw1ptr).t_cam.inquire_ImageMem((int *)NULL,
                                      (int *)NULL,
                                      (int *)NULL,
                                      (int *)NULL);

    if((*itw1ptr).t_cam.caught_Error())
    {
        error_msg("error(s) in the camera initialisation detected. " \
                  "messages sent to 'stderr'.\n" \
                  "is the camera connected?\nexiting.",
                  ERR_ARG);
        return EXIT_FAILURE;
    }
    else if(!(*itw1ptr).t_cam.get_Image())
    {
        error_msg("can't get an image from camera instance. good bye.",
                  ERR_ARG);
        return EXIT_FAILURE;
    }

    update_TextExpTime();
    update_TextGroundlift();
    update_TextKernelSize();
    update_TextStdDev();

    (*itw1ptr).t_cam.get_Image((*itw1ptr).t_cam.in);
    (*itw1ptr).t_cam.set_Pix2UmScale((*itw1ptr).t_cam.get_PixelPitch());
    (*itw1ptr).t_cam.update_Mats_RgbAndFp();
    (*itw1ptr).t_cam.set_MainWindowName(main_win_title);

    update_TextCamInfo((*itw1ptr).t_cam.get_CameraInfo());

    /* Main window setup */
    cv::namedWindow((*itw1ptr).t_cam.get_MainWindowName(), cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);

    cv::setMouseCallback((*itw1ptr).t_cam.get_MainWindowName(),
                         cast_static_set_MouseEvent,
                         itw1ptr);

    /* Camera slider bars */
    init_SliderExpTime();
    /* Image processing slider bars */
    init_SliderStdDev();
    init_SliderKernelSize();
    init_SliderGroundlift();

    (*itw1ptr).t_cam.show_Im_RGB();

    std::thread thread_Viewer(grabber::schedule_Viewer, argc, argv);

    std::thread thread_Copy(grabber::copy_DataToViewer);
    std::thread thread_Minime(grabber::schedule_Minime,
                              wavelen_um,
                              (*itw1ptr).t_cam.get_PixelPitch());

    LedMain->SwitchOn();
    ButtonQuit->Enable();

    uint32_t c_btn_state = NONE_BTN;
    for(;;)
    {
        if((*itw1ptr).t_cam.is_Grabbing())
        {
            if(!(*itw1ptr).t_cam.get_Image((*itw1ptr).t_cam.temp_in))
            {
                warn_msg("can't get an image from camera instance. " \
                         "using last successfully captured image.",
                         ERR_ARG);
                (*itw1ptr).t_cam.increment_lost_Frames();
            }
            else
            {
                (*itw1ptr).t_cam.temp_in.copyTo((*itw1ptr).t_cam.in);
                (*itw1ptr).t_cam.increment_Frames();
            }
        }
        (*itw1ptr).t_cam.exchange_Atomics();
        (*itw1ptr).t_cam.exchange_ExposureTimeAtomic();
        c_btn_state = btn_state.exchange(NONE_BTN,
                                         std::memory_order_acquire);
        switch(c_btn_state)
        {
            case REMOVE_AOI:
                (*itw1ptr).t_cam.set_RoiActive(false);
                break;
            case ACQ_SET_BACKGROUND:
                (*itw1ptr).t_cam.set_Background();
                break;
            case UNSET_BACKGROUND:
                (*itw1ptr).t_cam.unset_Background();
                break;
            case SAVE_RGB_BTN:
                (*itw1ptr).t_cam.save_Image((*itw1ptr).t_cam.save_Im_type::RGB,
                                            fname_img_out.ToStdString());
                break;
            case SAVE_WORK_BTN:
                (*itw1ptr).t_cam.save_Image((*itw1ptr).t_cam.save_Im_type::WORK,
                                            fname_img_out.ToStdString());
                break;
            case SAVE_FP_BTN:
                (*itw1ptr).t_cam.save_Image((*itw1ptr).t_cam.save_Im_type::FP_IN,
                                            fname_img_out.ToStdString());
                break;
            case STORE_RGB_BTN:
                (*itw1ptr).t_cam.store_Image((*itw1ptr).t_cam.save_Im_type::RGB,
                                             fname_dat_out.ToStdString());
                break;
            case STORE_WORK_BTN:
                (*itw1ptr).t_cam.store_Image((*itw1ptr).t_cam.save_Im_type::WORK,
                                             fname_dat_out.ToStdString());
                break;
            case STORE_FP_BTN:
                (*itw1ptr).t_cam.store_Image((*itw1ptr).t_cam.save_Im_type::FP_IN,
                                             fname_dat_out.ToStdString());
                break;
            case RESIZE_CAM_WINDOW:
                cv::resizeWindow((*itw1ptr).t_cam.get_MainWindowName(),
                                 (*itw1ptr).t_cam.get_Width(),
                                 (*itw1ptr).t_cam.get_Height());
                break;
            case TOGGLE_SMOOTHING:
                (*itw1ptr).t_cam.toggle_Smoothing();
                break;
            case START_VIEWER:
                (*itw1ptr).t_cam.signal_ViewerThreadIfWait();
                break;
            case START_MINIME:
                (*itw1ptr).t_cam.signal_MinimeThreadIfWait();
                (*itw1ptr).t_cam.wait_CameraThread();
                break;
            case TOGGLE_IDLING:
                (*itw1ptr).t_cam.toggle_Grabbing();
                break;
            case MAKE_GNUPLOT:
                (*itw1ptr).t_cam.gnuplot_Image((*itw1ptr).t_cam.save_Im_type::WORK,
                                               fname_gnu_out.ToStdString());
                break;
            case CLOSE_CAM_WINDOW:
                /** @todo If the GUI holds on 'waiting for camera ...', then
                it is because the mouse callback has interfered with the
                closing action of the window. If this happens despite the next
                call, a flag can be added in the mouse callback that makes sure
                that the callback is blocked when closing the window. */
                store_ButtonState(CLOSE_CAM_WINDOW);
                cv::destroyWindow((*itw1ptr).t_cam.get_MainWindowName());
                break;
            default:
                (*itw1ptr).t_cam.update_Mats_RgbAndFp();
                (*itw1ptr).t_cam.get_Moments();
                (*itw1ptr).t_cam.signal_CopyThreadIfWait();
                (*itw1ptr).t_cam.draw_Moments(false);
                (*itw1ptr).t_cam.draw_InfoWxVersion();
                (*itw1ptr).t_cam.show_Im_RGB();
        }

        cv::waitKey(20);
        if(c_btn_state == CLOSE_CAM_WINDOW)
            break;

        HWND *hwnd =
        static_cast<HWND *>(cvGetWindowHandle(main_win_title.c_str()));
        if(hwnd == nullptr)
            break;
    }

    if(c_btn_state != CLOSE_CAM_WINDOW)
        LedMain->SwitchOff();

    iprint(stdout, "i made %lu turns\n", (*itw1ptr).t_cam.get_Frames());
    iprint(stdout, "i lost %lu turns\n", (*itw1ptr).t_cam.get_lost_Frames());

    /* First stop minime. */
    (*itw1ptr).t_cam.close_MinimeThread();
    thread_Minime.join();

    /* Next stop reloading data into the viewer. */
    (*itw1ptr).t_cam.close_CopyThread();
    thread_Copy.join();

    /* Then close, if necessary, the display. */
    if((*itw1ptr).t_cam.is_ViewerWindowRunning())
        (*itw1ptr).t_cam.close_ViewerWindow();

    /* Finally close the actual thread. */
    (*itw1ptr).t_cam.close_ViewerThread();
    thread_Viewer.join();

    /* Wait for second to rest - and maybe to flush memory */
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return EXIT_SUCCESS;
}

void igyba_wxFrame::schedule_CamThread(int argc, char **argv)
{
    while(true)
    {
        /* Wait for the main loop to start the thread: */
        (*itw1ptr).event_Cam.wait();
        /* In case we want to leave: */
        if((*itw1ptr).load_CloseCamState())
        {
            (*itw1ptr).event_Cam.reset();
            break;
        }
        /* Fire up the thread: */
        if((*itw1ptr).launch_Cam(argc, argv) == EXIT_FAILURE)
        {
            (*itw1ptr).store_ForceQuit(true);
            (*itw1ptr).event_Cam.reset();
            break;
        }
        /* Go back and be ready to wait for the next round. */
        (*itw1ptr).event_Cam.reset();
    }
}

void igyba_wxFrame::close_CamThread(void)
{
    store_CloseCamState(true);
    iprint(stdout, "waiting for camera to close .");
    while(true)
    {
        if(signal_CamThreadIfWait())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        iprint(stdout, ".");
        fflush(stdout);
    }
    iprint(stdout, " done\n");
}

bool igyba_wxFrame::signal_CamThreadIfWait(void)
{
    if(event_Cam.check())
    {
        event_Cam.signal();
        return true;
    }
    else
        return false;
}

void igyba_wxFrame::cast_static_set_MouseEvent(const int event,
                                               const int x,
                                               const int y,
                                               const int flags,
                                               void *udata)
{
    igyba_wxFrame *ptr = static_cast<igyba_wxFrame *>(udata);
    (*ptr).set_MouseEvent(event, x, y, flags);
}

/** \brief  This function controls the mouse events on the drawing
 * window from OpenCV. Whatever happens in here, happens in the cam
 * thread.
 *
 * \param event const int
 * \param x const int
 * \param y const int
 * \param flags const int
 * \return void
 *
 */
void igyba_wxFrame::set_MouseEvent(const int event,
                                   const int x,
                                   const int y,
                                   const int flags)
{
    if(flags & cv::EVENT_FLAG_SHIFTKEY || load_SelectRoi())
    {
        if(event == cv::EVENT_LBUTTONDOWN && !t_cam.get_MouseDrag())
        {
            /* AOI selection begins. */
            t_cam.set_RoiActive(false);
            t_cam.set_EndRoi(cv::Point_<int>(x, y));
            t_cam.set_StartRoi(cv::Point_<int>(x, y));
            t_cam.set_MouseDrag(true);
        }
        else if(event == cv::EVENT_MOUSEMOVE && t_cam.get_MouseDrag())
        {
            /* AOI being selected. */
            t_cam.set_EndRoi(cv::Point_<int>(x, y));
        }
        else if(event == cv::EVENT_LBUTTONUP && t_cam.get_MouseDrag())
        {
            t_cam.set_EndRoi(cv::Point_<int>(x, y));
            int sx, sy;
            t_cam.get_StartRoi(&sx, &sy);
            const int sw = x - sx,
                      sh = y - sy;
            t_cam.set_RectRoi(cv::Rect_<int>(sx, sy, sw, sh));
            t_cam.set_MouseDrag(false);
            if(sw <= 25 || sh <= 25 ||
                sx + abs(sw) >= (int)t_cam.get_nCols() ||
                sy + abs(sh) >= (int)t_cam.get_nRows())
            {
                t_cam.set_RoiActive(false);
                ToggleButtonAOI->SetValue(false);
                store_SelectRoi(false);
            }
            else
            {
                t_cam.set_RoiActive(true);
                ToggleButtonAOI->SetValue(false);
                ToggleButtonAOI->SetLabel("Remove AOI");
                ToggleButtonAOI->SetToolTip("Click to remove AOI");
                store_SelectRoi(false);
            }
        }
    }
    else if(event == cv::EVENT_MOUSEMOVE || event == cv::EVENT_LBUTTONDOWN)
    {
        t_cam.set_MouseDrag(false);
        t_cam.set_PixelValue(t_cam.get_PixelValueWork(x, y));
        t_cam.copy_MousePosition(x, y);
    }
    else if(event == cv::EVENT_RBUTTONDOWN ||
            event == cv::EVENT_MBUTTONDOWN)
    {
        t_cam.set_PixelValue(0xDEADDEAD);
    }
    else
    {
        t_cam.copy_MousePosition(0, 0);
        t_cam.set_PixelValue(0xDEADDEAD);
    }
    update_TextAOI();
}

wxString igyba_wxFrame::get_wxBuildInfo(void)
{
    wxString wxbuild(wxVERSION_STRING);
    #if defined(__WXMSW__)
    wxbuild << _T("-Windows");
    #elif defined(__UNIX__)
    wxbuild << _T("-Linux");
    #endif
    #if wxUSE_UNICODE
    wxbuild << _T("-Unicode");
    #else
    wxbuild << _T("-ANSI");
    #endif
    return wxbuild;
}

/* Buttons etc. */

void igyba_wxFrame::OnToggleButtonBackgroundToggle(wxCommandEvent& event)
{
    if(ToggleButtonBackground->GetValue())
    {
        ToggleButtonBackground->SetLabel("Unload background");
        store_ButtonState(ACQ_SET_BACKGROUND);
    }
    else
    {
        ToggleButtonBackground->SetLabel("Acquire background");
        store_ButtonState(UNSET_BACKGROUND);
    }
}

void igyba_wxFrame::OnButtonDecExpTimeClick(wxCommandEvent& event)
{
    static double tmin, tmax, tinc;
    static bool once = false;
    const double out_min = SliderExpTime->GetMin(),
                 out_max = SliderExpTime->GetMax();
    if(!once)
    {
        once = true;
        double time;
        t_cam.get_ExposureTimesAtomic(&time, &tmin, &tmax, &tinc);
    }
    const int curval = SliderExpTime->GetValue();
    if(curval > SliderExpTime->GetMin())
    {
        SliderExpTime->SetValue(curval - 1);
        const double res = map_Linear((double)(curval - 1),
                            out_min, out_max,
                            tmin, tmax);
        t_cam.set_ExposureTimeAtomic(res);
        update_TextExpTime(res);
    }
}

void igyba_wxFrame::OnButtonSaveImgRGBClick(wxCommandEvent& event)
{
    std::string str;
    get_DateAndTime(str);
    str += "_rgb";
    wxString def_fname(str);
    static wxString def_dirname = dirname_bin;
    wxFileDialog saveDialog(this, "Save RGB image",
                            def_dirname, def_fname,
                            "Image file (*.png)|*.png",
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveDialog.ShowModal() == wxID_CANCEL)
        return; /* The user changed her idea... */
    /* Save the last directory. */
    def_dirname = saveDialog.GetDirectory();
    /* Save the full name of the file. */
    fname_img_out = saveDialog.GetPath();
    update_TextOutputInfo("Saving image to:\n" + fname_img_out);

    store_ButtonState(SAVE_RGB_BTN);
}

void igyba_wxFrame::OnButtonSaveImgWorkClick(wxCommandEvent& event)
{
    std::string str;
    get_DateAndTime(str);
    str += "_work";
    wxString def_fname(str);
    static wxString def_dirname = dirname_bin;
    wxFileDialog saveDialog(this, "Save Work image",
                            def_dirname, def_fname,
                            "Image file (*.png)|*.png",
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveDialog.ShowModal() == wxID_CANCEL)
        return;
    def_dirname = saveDialog.GetDirectory();
    fname_img_out = saveDialog.GetPath();
    update_TextOutputInfo("Saving image to:\n" + fname_img_out);

    store_ButtonState(SAVE_WORK_BTN);
}

void igyba_wxFrame::OnButtonSaveImgFPClick(wxCommandEvent& event)
{
    std::string str;
    get_DateAndTime(str);
    str += "_fp";
    wxString def_fname(str);
    static wxString def_dirname = dirname_bin;
    wxFileDialog saveDialog(this, "Save FP image",
                            def_dirname, def_fname,
                            "Image file (*.png)|*.png",
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveDialog.ShowModal() == wxID_CANCEL)
        return;
    def_dirname = saveDialog.GetDirectory();
    fname_img_out = saveDialog.GetPath();
    update_TextOutputInfo("Saving image to:\n" + fname_img_out);

    store_ButtonState(SAVE_FP_BTN);
}

void igyba_wxFrame::OnButtonViewerScreenshotClick(wxCommandEvent& event)
{
    std::string str;
    get_DateAndTime(str);
    str += "_viewer";
    wxString def_fname(str);
    static wxString def_dirname = dirname_bin;
    wxFileDialog saveDialog(this, "Save viewer screenshot",
                            def_dirname, def_fname,
                            "Image file (*.png)|*.png",
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveDialog.ShowModal() == wxID_CANCEL)
        return;
    def_dirname = saveDialog.GetDirectory();
    fname_img_out = saveDialog.GetPath();
    update_TextOutputInfo("Saving screenshot to:\n" + fname_img_out);

    t_cam.save_ViewerScreenshot(fname_img_out.ToStdString());
}

void igyba_wxFrame::OnButtonSaveDataRGBClick(wxCommandEvent& event)
{
    std::string str;
    get_DateAndTime(str);
    str += "_raw_3_ch";
    wxString def_fname(str);
    static wxString def_dirname = dirname_bin;
    wxFileDialog saveDialog(this, "Save Raw triple channel data",
                            def_dirname, def_fname,
                            "Data file (*.dat)|*.dat",
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveDialog.ShowModal() == wxID_CANCEL)
        return;
    def_dirname = saveDialog.GetDirectory();
    fname_dat_out = saveDialog.GetPath();
    update_TextOutputInfo("Saving data to:\n" + fname_dat_out);

    store_ButtonState(STORE_RGB_BTN);
}

void igyba_wxFrame::OnButtonSaveDataWorkClick(wxCommandEvent& event)
{
    std::string str;
    get_DateAndTime(str);
    str += "_work";
    wxString def_fname(str);
    static wxString def_dirname = dirname_bin;
    wxFileDialog saveDialog(this, "Save Work data",
                            def_dirname, def_fname,
                            "Data file (*.dat)|*.dat",
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveDialog.ShowModal() == wxID_CANCEL)
        return;
    def_dirname = saveDialog.GetDirectory();
    fname_dat_out = saveDialog.GetPath();
    update_TextOutputInfo("Saving data to:\n" + fname_dat_out);

    store_ButtonState(STORE_WORK_BTN);
}

void igyba_wxFrame::OnButtonSaveDataFPClick(wxCommandEvent& event)
{
    std::string str;
    get_DateAndTime(str);
    str += "_raw_1_ch";
    wxString def_fname(str);
    static wxString def_dirname = dirname_bin;
    wxFileDialog saveDialog(this, "Save Raw single channel data",
                            def_dirname, def_fname,
                            "Data file (*.dat)|*.dat",
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveDialog.ShowModal() == wxID_CANCEL)
        return;
    def_dirname = saveDialog.GetDirectory();
    fname_dat_out = saveDialog.GetPath();
    update_TextOutputInfo("Saving data to:\n" + fname_dat_out);

    store_ButtonState(STORE_FP_BTN);
}

void igyba_wxFrame::OnToggleButtonSmoothingToggle(wxCommandEvent& event)
{
    store_ButtonState(TOGGLE_SMOOTHING);
    if(ToggleButtonSmoothing->GetValue())
    {
        ToggleButtonSmoothing->SetLabel("Disable manipulation");
        SliderKernelSize->Enable();
        SliderStdDev->Enable();
        SliderGroundlift->Enable();
        ButtonDecKernelSize->Enable();
        ButtonDecStdDev->Enable();
        ButtonDecGroundlift->Enable();
        ButtonIncKernelSize->Enable();
        ButtonIncStdDev->Enable();
        ButtonIncGroundlift->Enable();
    }
    else
    {
        ToggleButtonSmoothing->SetLabel("Enable manipulation");
        SliderKernelSize->Disable();
        SliderStdDev->Disable();
        SliderGroundlift->Disable();
        ButtonDecKernelSize->Disable();
        ButtonDecStdDev->Disable();
        ButtonDecGroundlift->Disable();
        ButtonIncKernelSize->Disable();
        ButtonIncStdDev->Disable();
        ButtonIncGroundlift->Disable();
    }
}

void igyba_wxFrame::OnQuit(wxCommandEvent &event)
{
    Close();
}

void igyba_wxFrame::OnAbout(wxCommandEvent &event)
{
    wxString msg,
             author;
    author << "Clemens Sch\x00E4" << "fermeier";
    msg << "Called " + PROJECT_NAME + " " +
           PROJECT_MAJ_VERSION + "." + PROJECT_MIN_VERSION + "\n" +
           "Build with tons of key strokes. And with\n" +
           "- OpenCV " <<
           CV_MAJOR_VERSION << "." <<
           CV_MINOR_VERSION << "." <<
           CV_SUBMINOR_VERSION <<
           "\n- freeglut " \
           "2.0." << FREEGLUT_VERSION_2_0 <<
           "\n- OpenMP" \
           "\n- gcc " <<
           __GNUC__ << "." <<
           __GNUC_MINOR__ << "." <<
           __GNUC_PATCHLEVEL__ <<
           "\n- " + get_wxBuildInfo() +
           "\n(C) " + author + ", " + PROJECT_YEAR + ", clemens@fh-muenster.de";
    wxMessageBox(msg, "Nice to see you here!");

    wxAboutDialogInfo info;
    wxString version;
    version = "igyba " + PROJECT_MAJ_VERSION + "." + PROJECT_MIN_VERSION;
    info.SetName(version);
    info.SetDescription("This program does something great.");
    info.SetCopyright("(C) 2019," + author + "<clemens@fh-muenster.de>");
}

void igyba_wxFrame::OnButtonIncExpTimeClick(wxCommandEvent& event)
{
    static double tmin, tmax, tinc;
    static bool once = false;
    const double out_min = SliderExpTime->GetMin(),
                 out_max = SliderExpTime->GetMax();
    if(!once)
    {
        once = true;
        double time;
        t_cam.get_ExposureTimesAtomic(&time, &tmin, &tmax, &tinc);
    }
    const int curval = SliderExpTime->GetValue();
    if(curval < SliderExpTime->GetMax())
    {
        SliderExpTime->SetValue(curval + 1);
        const double res = map_Linear((double)(curval + 1),
                            out_min, out_max,
                            tmin, tmax);
        t_cam.set_ExposureTimeAtomic(res);
        update_TextExpTime(res);
    }
}

void igyba_wxFrame::OnSliderKernelSizeCmdScroll(wxScrollEvent& event)
{
    uint res = ((uint)SliderKernelSize->GetValue() << 1) + 1;
    assert(res & 1);
    t_cam.set_KernelSizeAtomic(res);
    update_TextKernelSize(res);
}

void igyba_wxFrame::OnToggleButtonFrameGrabToggle(wxCommandEvent& event)
{
    store_ButtonState(TOGGLE_IDLING);
    if(ToggleButtonFrameGrab->GetValue())
    {
        ToggleButtonFrameGrab->SetLabel("Continue frame grab");
    }
    else
        ToggleButtonFrameGrab->SetLabel("Idle frame grab");
}

void igyba_wxFrame::OnToggleButtonViewerToggle(wxCommandEvent& event)
{
    if(ToggleButtonViewer->GetValue())
    {
        store_ButtonState(START_VIEWER);
        ToggleButtonViewerAnimation->Enable();
        ToggleButtonMapViewer->Enable();
        ToggleButtonViewerRotation->Enable();
        ButtonViewerScreenshot->Enable();
        ToggleButtonViewer->SetLabel("Close viewer");
    }
    else
    {
        t_cam.close_ViewerWindow();
        ToggleButtonViewerAnimation->Disable();
        ToggleButtonMapViewer->Disable();
        ToggleButtonViewerRotation->Disable();
        ButtonViewerScreenshot->Disable();
        ToggleButtonViewer->SetLabel("Launch viewer");
    }
}

void igyba_wxFrame::OnButtonStartClick(wxCommandEvent& event)
{

}

void igyba_wxFrame::OnCloseMainFrame(wxCloseEvent& event)
{
    store_ButtonState(CLOSE_CAM_WINDOW);
    close_CamThread();
    thread_Cam.join();
    Destroy();
}

void igyba_wxFrame::OnButtonGnuplotClick(wxCommandEvent& event)
{
    std::string str;
    get_DateAndTime(str);
    wxString def_fname(str);
    static wxString def_dirname = dirname_bin;
    wxFileDialog saveDialog(this, "Save gnuplot",
                            def_dirname, def_fname,
                            "Image file (*.png)|*.png",
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveDialog.ShowModal() == wxID_CANCEL)
        return;
    def_dirname = saveDialog.GetDirectory();
    fname_gnu_out = saveDialog.GetPath();
    update_TextOutputInfo("Saving plot to:\n" + fname_gnu_out);

    store_ButtonState(MAKE_GNUPLOT);
}

void igyba_wxFrame::OnButtonResizeCamWinClick(wxCommandEvent& event)
{
    store_ButtonState(RESIZE_CAM_WINDOW);
}

void igyba_wxFrame::OnButtonQuitClick(wxCommandEvent& event)
{
    const wxPoint pt = wxGetMousePosition();
    const int mx = pt.x,
              my = pt.y,
              ans = wxMessageBox("Continue closing?",
                                "Please confirm", wxICON_QUESTION | wxYES_NO,
                                NULL, mx, my);
    if(ans == wxYES)
        Close();
    else
        return;
}

void igyba_wxFrame::OnSliderExpTimeCmdScroll(wxScrollEvent& event)
{
    const double out_min = SliderExpTime->GetMin(),
                 out_max = SliderExpTime->GetMax();
    static double tmin, tmax;
    static bool once = false;
    if(!once)
    {
        once = true;
        double time, tinc;
        t_cam.get_ExposureTimesAtomic(&time, &tmin, &tmax, &tinc);
    }
    const double res = map_Linear((double)SliderExpTime->GetValue(),
                                out_min, out_max,
                                tmin, tmax);
    t_cam.set_ExposureTimeAtomic(res);
    update_TextExpTime(res);
}

void igyba_wxFrame::init_SliderExpTime(void)
{
    const double out_min = 0.,
                 out_max = 99.;
    double time, tmax, tmin, tinc;
    t_cam.get_ExposureTimesAtomic(&time, &tmin, &tmax, &tinc);
    const double res = map_Linear(time, tmin, tmax, out_min, out_max);
    static int setting = res;
    assert(setting < 100);
    SliderExpTime->SetRange((int)out_min, (int)out_max);
    SliderExpTime->SetValue(setting);
}

void igyba_wxFrame::OnSliderGroundliftCmdScroll(wxScrollEvent& event)
{
    const double out_min = SliderGroundlift->GetMin(),
                 out_max = SliderGroundlift->GetMax();
    static double gl, gl_max;
    static bool once = false;
    if(!once)
    {
        once = true;
        t_cam.get_GroundliftRangeAtomic(&gl, &gl_max);
    }
    double res = map_Linear((double)SliderGroundlift->GetValue(),
                            out_min, out_max,
                            0., gl_max);
    t_cam.set_GroundliftAtomic(res);
    update_TextGroundlift(res);
}

void igyba_wxFrame::init_SliderStdDev(void)
{
    const double out_min = 0.,
                 out_max = 99.;
    double std_min, std_max, cur_val;
    cur_val = t_cam.get_GaussBlurRangeAtomic(&std_min, &std_max);
    double res = map_Linear(cur_val, std_min, std_max, out_min, out_max);
    static int setting = res;
    assert(setting < 100);
    SliderStdDev->SetRange((int)out_min, (int)out_max);
    SliderStdDev->SetValue(setting);
}

void igyba_wxFrame::init_SliderKernelSize(void)
{
    const uint out_min = 1;
    uint sze_min, sze_max, cur_val,
         out_max;
    cur_val = t_cam.get_KernelSizeAtomic(&sze_min, &sze_max);
    out_max = (sze_max - 1) >> 1;
    SliderKernelSize->SetRange(out_min, out_max);
    SliderKernelSize->SetValue((cur_val - 1) >> 1);
}


void igyba_wxFrame::init_SliderGroundlift(void)
{
    const double out_min = 0.,
                 out_max = 99.;
    double gl, gl_max;
    t_cam.get_GroundliftRangeAtomic(&gl, &gl_max);
    double res = map_Linear(gl, 0., gl_max, out_min, out_max);
    static int setting = res;
    assert(setting < 100);
    SliderGroundlift->SetRange((int)out_min, (int)out_max);
    SliderGroundlift->SetValue(setting);
}

void igyba_wxFrame::OnButtonDecGroundliftClick(wxCommandEvent& event)
{
    static double gl_max;
    static bool once = false;
    const double out_min = SliderGroundlift->GetMin(),
                 out_max = SliderGroundlift->GetMax();
    if(!once)
    {
        once = true;
        double gl;
        t_cam.get_GroundliftRangeAtomic(&gl, &gl_max);
    }
    const int curval = SliderGroundlift->GetValue();
    if(curval > SliderGroundlift->GetMin())
    {
        SliderGroundlift->SetValue(curval - 1);
        const double res = map_Linear((double)(curval - 1),
                            out_min, out_max,
                            0., gl_max);
        t_cam.set_GroundliftAtomic(res);
        update_TextGroundlift(res);
    }
}

void igyba_wxFrame::OnButtonIncGroundliftClick(wxCommandEvent& event)
{
    static double gl_max;
    static bool once = false;
    const double out_min = SliderGroundlift->GetMin(),
                 out_max = SliderGroundlift->GetMax();
    if(!once)
    {
        once = true;
        double gl;
        t_cam.get_GroundliftRangeAtomic(&gl, &gl_max);
    }
    const int curval = SliderGroundlift->GetValue();
    if(curval < SliderGroundlift->GetMax())
    {
        SliderGroundlift->SetValue(curval + 1);
        const double res = map_Linear((double)(curval + 1),
                                      out_min,
                                      out_max,
                                      0.,
                                      gl_max);
        t_cam.set_GroundliftAtomic(res);
        update_TextGroundlift(res);
    }
}

void igyba_wxFrame::OnSliderStdDevCmdScroll(wxScrollEvent& event)
{
    const double out_min = SliderStdDev->GetMin(),
             out_max = SliderStdDev->GetMax();
    static double gb_min, gb_max;
    static bool once = false;
    if(!once)
    {
        once = true;
        t_cam.get_GaussBlurRangeAtomic(&gb_min, &gb_max);
    }
    double res = map_Linear((double)SliderStdDev->GetValue(),
                            out_min, out_max,
                            gb_min, gb_max);
    t_cam.set_GaussBlurAtomic(res);
    update_TextStdDev(res);
}

void igyba_wxFrame::OnButtonDecStdDevClick(wxCommandEvent& event)
{
    static double gb_min, gb_max;
    static bool once = false;
    const double out_min = SliderStdDev->GetMin(),
                 out_max = SliderStdDev->GetMax();
    if(!once)
    {
        once = true;
        t_cam.get_GaussBlurRangeAtomic(&gb_min, &gb_max);
    }
    const int curval = SliderStdDev->GetValue();
    if(curval > SliderStdDev->GetMin())
    {
        SliderStdDev->SetValue(curval - 1);
        const double res = map_Linear((double)(curval - 1),
                                      out_min,
                                      out_max,
                                      gb_min,
                                      gb_max);
        t_cam.set_GaussBlurAtomic(res);
        update_TextStdDev(res);
    }
}

void igyba_wxFrame::OnButtonIncStdDevClick(wxCommandEvent& event)
{
    static double gb_min, gb_max;
    static bool once = false;
    const double out_min = SliderStdDev->GetMin(),
                 out_max = SliderStdDev->GetMax();
    if(!once)
    {
        once = true;
        t_cam.get_GaussBlurRangeAtomic(&gb_min, &gb_max);
    }
    const int curval = SliderStdDev->GetValue();
    if(curval < SliderStdDev->GetMax())
    {
        SliderStdDev->SetValue(curval + 1);
        const double res = map_Linear((double)(curval + 1),
                                      out_min,
                                      out_max,
                                      gb_min,
                                      gb_max);
        t_cam.set_GaussBlurAtomic(res);
        update_TextStdDev(res);
    }
}

void igyba_wxFrame::OnButtonDecKernelSizeClick(wxCommandEvent& event)
{
    int curval = SliderKernelSize->GetValue();
    if(curval > SliderKernelSize->GetMin())
    {
        SliderKernelSize->SetValue(curval - 1);
        curval = (curval << 1) + 1;
        t_cam.set_KernelSizeAtomic(curval - 2);
        update_TextKernelSize(curval - 2);
    }
}

void igyba_wxFrame::OnButtonIncKernelSizeClick(wxCommandEvent& event)
{
    int curval = SliderKernelSize->GetValue();
    if(curval < SliderKernelSize->GetMax())
    {
        SliderKernelSize->SetValue(curval + 1);
        curval = (curval << 1) + 1;
        t_cam.set_KernelSizeAtomic(curval + 2);
        update_TextKernelSize(curval + 2);
    }
}

void igyba_wxFrame::OnButtonMinimeClick(wxCommandEvent& event)
{
    if(wxMessageBox("Start fit routine?",
                    "Please confirm",
                    wxICON_QUESTION | wxYES_NO) != wxYES)
        return;
    else
        store_ButtonState(START_MINIME);
}

void igyba_wxFrame::OnToggleButtonAOIToggle(wxCommandEvent& event)
{
    if(ToggleButtonAOI->GetValue())
    {
        if(ToggleButtonAOI->GetLabel().Cmp("Draw rectangle") == 0)
            store_SelectRoi(true);
        else
        {
            store_ButtonState(REMOVE_AOI);
            ToggleButtonAOI->SetLabel("Draw rectangle");
            ToggleButtonAOI->SetValue(false);
            TextCtrlAOI->SetLabel("No AOI selected");
        }
    }
}

void igyba_wxFrame::OnToggleButtonMapViewerToggle(wxCommandEvent& event)
{
    if(ToggleButtonMapViewer->GetValue())
        ToggleButtonMapViewer->SetLabel("Show 3D map");
    else
        ToggleButtonMapViewer->SetLabel("Show 2D map");
    t_cam.toggle_ViewerMap3DMode();
}

void igyba_wxFrame::OnToggleButtonViewerAnimationToggle(wxCommandEvent& event)
{
    if(ToggleButtonViewerAnimation->GetValue())
        ToggleButtonViewerAnimation->SetLabel("Start animation");
    else
        ToggleButtonViewerAnimation->SetLabel("Stop animation");
    t_cam.toggle_ViewerIdling();
}

void igyba_wxFrame::OnToggleButtonViewerRotationToggle(wxCommandEvent& event)
{
    if(ToggleButtonViewerRotation->GetValue())
        ToggleButtonViewerRotation->SetLabel("Stop rotation");
    else
        ToggleButtonViewerRotation->SetLabel("Start rotation");
    t_cam.toggle_ViewerRotation();
}

/** \brief The operation is ordered to happen before an acquire operation, serving as a
 * synchronisation point for other accesses to memory that may have visible side effects
 * on the loading thread.
 *
 * \param void
 * \return void
 *
 */
void igyba_wxFrame::store_ButtonState(const uint32_t wxb)
{
    btn_state.store(wxb, std::memory_order_release);
}

void igyba_wxFrame::store_CloseCamState(const bool b)
{
    close_cam_thread.store(b, std::memory_order_release);
}

void igyba_wxFrame::store_ForceQuit(const bool b)
{
    force_quit.store(b, std::memory_order_release);
}

void igyba_wxFrame::store_SelectRoi(const bool b)
{
    select_roi.store(b, std::memory_order_release);
}

/** \brief The operation is ordered to happen once all accesses to memory in the releasing
 * thread (that have visible side effects on the loading thread) have happened.
 *
 * \param void
 * \return void
 *
 */
uint32_t igyba_wxFrame::load_ButtonState(void)
{
    return btn_state.load(std::memory_order_acquire);
}

bool igyba_wxFrame::load_ForceQuit(void)
{
    return force_quit.load(std::memory_order_acquire);
}

bool igyba_wxFrame::load_CloseCamState(void)
{
    return close_cam_thread.load(std::memory_order_acquire);
}

bool igyba_wxFrame::load_SelectRoi(void)
{
    return select_roi.load(std::memory_order_acquire);
}

void igyba_wxFrame::OnTextCtrlCamInfoText(wxCommandEvent& event)
{

}

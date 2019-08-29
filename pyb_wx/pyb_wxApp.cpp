#include "pyb_wxApp.h"

//(*AppHeaders
#include "pyb_wxMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(pyb_wxApp)

bool pyb_wxApp::OnInit(void)
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
        pyb_wxFrame* Frame = new pyb_wxFrame(wxApp::argc,
                                                 wxApp::argv,
                                                 0);
        Frame->Show();
        SetTopWindow(Frame);
    }
    //*)
    return wxsOK;
}

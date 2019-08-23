#include "igyba_wxApp.h"

//(*AppHeaders
#include "igyba_wxMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(igyba_wxApp)

bool igyba_wxApp::OnInit(void)
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
        igyba_wxFrame* Frame = new igyba_wxFrame(wxApp::argc,
                                                 wxApp::argv,
                                                 0);
        Frame->Show();
        SetTopWindow(Frame);
    }
    //*)
    return wxsOK;
}

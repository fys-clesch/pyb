#include "igyba_thorlabs_wxApp.h"

//(*AppHeaders
#include "igyba_thorlabs_wxMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(igyba_thorlabs_wxApp)

bool igyba_thorlabs_wxApp::OnInit(void)
{
	//(*AppInitialize
	bool wxsOK = true;
	wxInitAllImageHandlers();
	if ( wxsOK )
	{
		igyba_thorlabs_wxFrame* Frame = new igyba_thorlabs_wxFrame(0);
		Frame->Show();
		SetTopWindow(Frame);
	}
	//*)
	return wxsOK;
}

/***************************************************************
 * Name:      igyba_wxApp.h
 * Purpose:   Defines Application Class
 * Author:    Clemens Schäfermeier (clemens@fh-muenster.de)
 * Created:   2015-04-03
 * Copyright: Clemens Schäfermeier ()
 * License:
 **************************************************************/

#ifndef IGYBA_WXAPP_H
#define IGYBA_WXAPP_H

#include <wx/app.h>

class igyba_wxApp : public wxApp
{
    public:
        virtual bool OnInit(void);
};

#endif // IGYBA_WXAPP_H

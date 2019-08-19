/***************************************************************
 * Name:      igyba_thorlabs_wxApp.h
 * Purpose:   Defines Application Class
 * Author:    Clemens Schäfermeier (clesch@fysik.dtu.dk)
 * Created:   2015-04-03
 * Copyright: Clemens Schäfermeier ()
 * License:
 **************************************************************/

#ifndef IGYBA_THORLABS_WXAPP_H
#define IGYBA_THORLABS_WXAPP_H

#include <wx/app.h>

class igyba_thorlabs_wxApp : public wxApp
{
    public:
        virtual bool OnInit(void);
};

#endif // IGYBA_THORLABS_WXAPP_H

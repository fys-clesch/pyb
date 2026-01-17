/***************************************************************
 * Name:      pyb_wxApp.h
 * Purpose:   Defines Application Class
 * Author:    Clemens Schaefermeier
 * Created:   2015-04-03
 * Copyright: Clemens Schaefermeier
 * License:   GPL
 **************************************************************/

#ifndef PYB_WXAPP_H
#define PYB_WXAPP_H

#include <wx/app.h>

class pyb_wxApp : public wxApp
{
    public:
        virtual bool OnInit(void);
};

#endif // PYB_WXAPP_H

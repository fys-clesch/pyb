// Name:        led.h
// Purpose:     wxLed class
// Author:      Thomas Monjalon
// Created:     09/06/2005
// Revision:    09/06/2005
// Licence:     wxWidgets
// mod   by:    Jonas Zinn
// mod date:    24/03/2012

#ifndef _WX_LED_H_
#define _WX_LED_H_

#include <wx/window.h>
#include <wx/bitmap.h>
#include <wx/dcclient.h>
#include <wx/thread.h>

class wxLed : public wxWindow
{
    public:
        wxLed(wxWindow *parent, wxWindowID id = wxID_ANY, wxColour disableColour = wxColour(128, 128, 128), wxColour onColour = wxColour(00, 255, 00), wxColour offColour = wxColour(255, 00, 00), const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize);
        wxLed();

        ~wxLed() ;

        bool Create(wxWindow *parent, wxWindowID id = wxID_ANY, wxColour disableColour = wxColour(128, 128, 128), wxColour onColour = wxColour(00, 255, 00), wxColour offColour = wxColour(255, 00, 00), const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize);

        void Enable(void) ;

        void Disable(void);

        void Switch(void);

        void SwitchOn(void);

        void SwitchOff(void);

        void SetOnColour(wxColour rgb);

        void SetOffColour(wxColour rgb);

        void SetDisableColour(wxColour rgb);

        void SetOnOrOff(bool on);

        bool IsEnabled(void);

        bool IsOn(void);

    protected:

        wxColour m_On;
        wxColour m_Off;
        wxColour m_Disable;
        wxBitmap *m_bitmap;
        wxMutex m_mutex ;
        bool m_isEnable;
        bool m_isOn;

        void OnPaint(wxPaintEvent &event);

        virtual void SetBitmap(wxString color);

    private:

        DECLARE_EVENT_TABLE()
} ;

#endif // _WX_LED_H_

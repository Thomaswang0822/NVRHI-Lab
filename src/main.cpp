#include "frontend/my_app.h"

#include <wx/wx.h>

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(wxID_PRINT, MyFrame::OnHello)
EVT_MENU(wxID_EXIT, MyFrame::OnExit)
EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
wxEND_EVENT_TABLE()

#ifdef WIN32
wxIMPLEMENT_APP(MyApp);
#else
wxIMPLEMENT_APP_CONSOLE(MyApp);
#endif // WIN32

// WIN32_LEAN_AND_MEAN excludes rarely-used Windows headers (like winsock.h)
// to avoid conflicts with wxWidgets which includes winsock2.h.
// Without this, we get redefinition errors for sockaddr, fd_set, etc.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "graphics/device_manager.h"
#include "graphics/basic_renderer.h"
#include "frontend/my_app.h"

#include <wx/wx.h>

#ifdef WIN32
wxIMPLEMENT_APP(MyApp);
#else
wxIMPLEMENT_APP_CONSOLE(MyApp);
#endif // WIN32

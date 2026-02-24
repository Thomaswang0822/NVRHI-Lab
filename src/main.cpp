#include "graphics/device_manager.h"
#include "frontend/my_app.h"

#include <wx/wx.h>

#ifdef WIN32
wxIMPLEMENT_APP(MyApp);
#else
wxIMPLEMENT_APP_CONSOLE(MyApp);
#endif // WIN32

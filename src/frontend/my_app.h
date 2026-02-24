#pragma once

#include <wx/wx.h>
#include <memory>

// Forward declaration
namespace nvrhi_lab {
    class DeviceManager;
}

class MyApp : public wxApp {
public:
    virtual bool OnInit() override;
    virtual int OnExit() override;

private:
    std::unique_ptr<nvrhi_lab::DeviceManager> m_DeviceManager;
};

class MyFrame : public wxFrame {
public:
    MyFrame();
    void SetDeviceManager(nvrhi_lab::DeviceManager* deviceManager);

private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    nvrhi_lab::DeviceManager* m_DeviceManager = nullptr;

    wxDECLARE_EVENT_TABLE();
};

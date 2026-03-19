#pragma once

#include <wx/wx.h>
#include <memory>

namespace nvrhi_lab {
    class DeviceManager;
    class BasicRenderer;
}

class MyApp : public wxApp {
public:
    virtual bool OnInit() override;
    virtual int OnExit() override;

private:
    std::unique_ptr<nvrhi_lab::DeviceManager> m_DeviceManager;
    std::unique_ptr<nvrhi_lab::BasicRenderer> m_Renderer;
};

class MyFrame : public wxFrame {
public:
    MyFrame();
    void SetDeviceManager(nvrhi_lab::DeviceManager* deviceManager);
    void SetRenderer(nvrhi_lab::BasicRenderer* renderer);

private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnRenderTimer(wxTimerEvent& event);

    nvrhi_lab::DeviceManager* m_DeviceManager = nullptr;
    nvrhi_lab::BasicRenderer* m_Renderer = nullptr;
    wxTimer* m_RenderTimer = nullptr;

    wxDECLARE_EVENT_TABLE();
};

#include "my_app.h"
#include "../graphics/device_manager.h"
#include "../graphics/basic_renderer.h"

#include <wx/msgdlg.h>

bool MyApp::OnInit() {
    // Create the main window
    MyFrame* frame = new MyFrame();
    frame->Show(true);
    
    // Create and initialize the device manager
    m_DeviceManager = std::make_unique<nvrhi_lab::DeviceManager>();
    
    nvrhi_lab::DeviceManagerDesc desc;
    desc.backend = nvrhi_lab::GraphicsBackend::D3D12;  // Try D3D12 first
    desc.enableValidation = true;
    desc.enableDebugLayer = true;
    
    // Get the window handle
    void* windowHandle = reinterpret_cast<void*>(frame->GetHandle());
    
    if (!m_DeviceManager->Initialize(desc, windowHandle)) {
        // Try D3D11 as fallback
        desc.backend = nvrhi_lab::GraphicsBackend::D3D11;
        if (!m_DeviceManager->Initialize(desc, windowHandle)) {
            wxMessageBox("Failed to initialize graphics device!", "Error", wxOK | wxICON_ERROR);
            return false;
        }
    }
    
    // Pass device manager to frame
    frame->SetDeviceManager(m_DeviceManager.get());
    
    // Show success message
    wxString msg;
    msg.Printf("Successfully initialized NVRHI with %s backend!\nResolution: %dx%d\nBackbuffers: %d",
               m_DeviceManager->GetBackendName(),
               m_DeviceManager->GetWidth(),
               m_DeviceManager->GetHeight(),
               m_DeviceManager->GetBackBufferCount());
    frame->SetStatusText(msg);
    
    // Create and initialize BasicRenderer
    m_Renderer = std::make_unique<nvrhi_lab::BasicRenderer>();
    if (!m_Renderer->Initialize(m_DeviceManager.get())) {
        wxMessageBox("Failed to initialize BasicRenderer!", "Error", wxOK | wxICON_ERROR);
        return false;
    }
    
    frame->SetRenderer(m_Renderer.get());
    
    return true;
}

int MyApp::OnExit() {
    m_Renderer.reset();
    m_DeviceManager.reset();
    return wxApp::OnExit();
}

// Frame implementation
MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "NVRHI Lab", wxDefaultPosition, wxSize(1280, 720)),
      m_RenderTimer(this)
{
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(wxID_PRINT, "&Hello...\tCtrl-H", "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Initializing...");
    
}

void MyFrame::SetDeviceManager(nvrhi_lab::DeviceManager* deviceManager) {
    m_DeviceManager = deviceManager;
}

void MyFrame::SetRenderer(nvrhi_lab::BasicRenderer* renderer) {
    m_Renderer = renderer;
    m_RenderTimer.Start(16);  // ~60 FPS
}

void MyFrame::OnExit(wxCommandEvent& event) {
    if (m_RenderTimer.IsRunning()) {
        m_RenderTimer.Stop();
    }
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("NVRHI Lab - A graphics learning laboratory using NVRHI", 
                 "About NVRHI Lab", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent& event) {
    if (m_DeviceManager && m_DeviceManager->IsInitialized()) {
        wxString msg;
        msg.Printf("Device Manager is initialized!\nBackend: %s\nSize: %dx%d",
                   m_DeviceManager->GetBackendName(),
                   m_DeviceManager->GetWidth(),
                   m_DeviceManager->GetHeight());
        wxLogMessage(msg);
    } else {
        wxLogMessage("Device Manager is NOT initialized!");
    }
}

void MyFrame::OnRenderTimer(wxTimerEvent& event) {
    if (m_DeviceManager && m_Renderer) {
        m_DeviceManager->BeginFrame();
        m_Renderer->Render();
        m_DeviceManager->Present();
    }
}

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_PRINT, MyFrame::OnHello)
    EVT_MENU(wxID_EXIT, MyFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
    EVT_TIMER(wxID_ANY, MyFrame::OnRenderTimer)
wxEND_EVENT_TABLE()

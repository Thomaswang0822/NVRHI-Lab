# Architecture

## Project Structure

```
NVRHI-Lab/
├── src/
│   ├── main.cpp              - Application entry point
│   ├── frontend/             - wxWidgets GUI layer
│   │   ├── my_app.h/cpp     - Main application and window frame
│   │   └── ...
│   ├── graphics/             - NVRHI integration layer
│   │   ├── device_manager.h/cpp   - NVRHI device and swap chain management
│   │   ├── basic_renderer.h/cpp   - Triangle rendering for Phase 1
│   │   ├── scene_object.h/cpp     - Base class for renderable objects
│   │   ├── target.h/cpp           - Interactive target objects
│   │   ├── background_wall.h/cpp   - Background geometry
│   │   └── camera.h/cpp          - Camera and view/projection matrices
│   ├── common/              - Shared utilities and helpers
│   ├── textures/            - Texture (纹理) examples and experiments
│   ├── buffers/             - Buffer (缓冲区) examples and experiments
│   ├── pipelines/           - Pipeline state management
│   └── bindings/           - Resource binding systems
├── build/                  - CMake working directory
├── bin/                    - Executables and runtime resources
├── docs/                   - Documentation
├── thirdparty/             - Third-party libraries
│   ├── NVRHI/             - NVRHI graphics abstraction layer
│   └── wxWidgets/         - GUI framework
└── .opencode/             - AI agent context and skills
```

## Component Interactions

### High-Level Flow

```
wxWidgets Main Loop
        ↓
    Timer Trigger
        ↓
GraphicsRenderer::render()
        ↓
DeviceManager::beginFrame()
        ↓
SceneObject::render() (for each object)
        ↓
Command List Submission
        ↓
DeviceManager::present()
```

### Key Components

**DeviceManager**
- Creates NVRHI device for D3D11/D3D12/Vulkan
- Manages swap chain and backbuffers
- Handles frame synchronization

**BasicRenderer**
- Renders triangle for Phase 1 testing
- Owns pipeline state, shaders, vertex buffer
- Records command lists

**SceneObject (Base Class)**
- Owns resources (buffers, textures, pipeline states)
- Implements `render()` method
- Provides interface for different object types

**Camera**
- Maintains view and projection matrices
- Supports orbit or static views
- Passes matrices to shaders via constant buffers

## Data Flow

**Initialization:**
```
Application Startup → Backend Selection → DeviceManager Creation
→ Window Handle → Swap Chain Setup → BasicRenderer Initialization
→ Render Loop Start
```

**Frame Render:**
```
Timer Tick → BeginFrame() → BasicRenderer::Render()
→ Command List Submission → Present()
```

## Backend Abstraction

All graphics code uses NVRHI API - backend-specific differences are abstracted:

- **Device creation**: `nvrhi::d3d11::createDevice()`, `nvrhi::d3d12::createDevice()`, `nvrhi::vulkan::createDevice()`
- **Resources**: Same API for all backends (textures, buffers, etc.)
- **Commands**: `nvrhi::CommandList` interface is backend-agnostic
- **Pipelines**: Same pipeline state objects across backends

Backend switching only requires app restart - no code changes needed.

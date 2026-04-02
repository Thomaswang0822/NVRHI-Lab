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
│   │   ├── platform/          - Platform abstraction (D3D12Context, etc.)
│   │   │   ├── platform_context.h - Interface
│   │   │   ├── d3d12_context.h/cpp - D3D12 implementation
│   │   │   └── ...                 - Future: D3D11, Vulkan
│   │   ├── device_manager.h/cpp - Pure NVRHI device management
│   │   ├── basic_renderer.h/cpp - Triangle rendering for Phase 1
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
DeviceManager::BeginFrame()
        ↓
BasicRenderer::Render()
        ↓
Command List Submission
        ↓
DeviceManager::Present()
```

### Key Components

**IPlatformContext**
- Interface for platform-specific device creation
- Implementations: D3D12Context (raw D3D12/DXGI), future: D3D11Context, VulkanContext
- Isolates all raw backend API calls

**DeviceManager**
- Uses IPlatformContext to create NVRHI device
- Pure NVRHI API after initialization - no raw D3D12/D3D11/Vulkan pointers
- Manages frame timing and presentation

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
→ Render Timer Start
```

**Frame Render:**
```
Timer Tick → BeginFrame() → BasicRenderer::Render()
→ Command List Submission → Present()
```

## Backend Abstraction

Graphics code uses pure NVRHI API - backend-specific differences isolated:

- **Device creation**: IPlatformContext implementations create raw objects, return NVRHI device
- **Resources**: Same NVRHI API for all backends (textures, buffers, etc.)
- **Commands**: `nvrhi::CommandList` interface is backend-agnostic
- **Platform isolation**: `platform/` folder contains backend implementations

Backend switching only requires app restart - no code changes needed.

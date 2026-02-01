# Tech Stack

## Core
- **Graphics Abstraction**: NVRHI (NV Rendering Hardware Interface)
- **Build System**: CMake
- **Language**: C++20

## Programming Languages (User Background)
- **Primary**: C++, CMake, CUDA, HLSL (familiar)

## Tools
- **Version Control**: Git
- **IDE**: TBD

## Frontend/UI
- **Framework**: wxWidgets
- **Requirements met**: Modern look, NOT ImGui
- **Key characteristics**: Pure C++ (no external build tools), clean and navigable codebase, smaller footprint than Qt
- **Rationale**: "Code-like" framework - user can navigate source to understand features and debug issues

## Development Environment
- **OS**: Windows (x64 only for NVRHI)
- **Platform**: May support Linux in future (NVRHI supports it)

## Documentation Storage
- `.opencode/contexts/` folder for AI agent context (essential for session continuity)
- `.opencode/skills/` folder for "quickly look at what we have so far" after creating a new session

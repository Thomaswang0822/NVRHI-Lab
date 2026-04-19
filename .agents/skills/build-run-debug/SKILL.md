---
name: build-run-debug
description: "Builds the project, runs the app with specified backend, and checks for errors. Invoke when user asks to build, run, test, or debug the application, or when verifying changes work across all backends."
---

# Build, Run, Debug

This skill provides a unified workflow for building the NVRHI-Lab project, running it with different graphics backends, and checking for errors.

## When to Use

Invoke this skill when:
- User asks to "build the project" or "rebuild"
- User asks to "run the app" or "test the app"
- User wants to "debug" or "check for errors"
- After making code changes that need verification
- When testing across all three backends (D3D11, D3D12, Vulkan)

## Build Commands

### Prerequisites
- Run from project root directory (`F:\repos\NVRHI-Lab`)
- Visual Studio must be installed with MSBuild component

### Incremental Build
```powershell
$msbuild = & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe
& $msbuild build\NVRHI-Lab.slnx /p:Configuration=Debug /p:Platform=x64 /m:4
```

### Full Rebuild
```powershell
& $msbuild build\NVRHI-Lab.slnx /t:Rebuild /p:Configuration=Debug /p:Platform=x64 /m:4
```

## Run Commands

### CRITICAL: Working Directory
**The app MUST be run from `build/` directory**, not project root or `bin/Debug/`.

Why: Shaders are compiled to `build/shaders/` and the app looks for `shaders/` relative to working directory.

### Basic Run (Single Backend)
```powershell
# From build/ directory
Push-Location build
& ..\bin\Debug\NVRHI-Lab.exe --backend=d3d12
Pop-Location
```

### Run with Frame Limit (Automated Testing)
```powershell
# Exit after N frames - useful for automated testing
Push-Location build
& ..\bin\Debug\NVRHI-Lab.exe --backend=vulkan --frameNum=10 2>&1
Pop-Location
```

### Test All Backends
```powershell
Push-Location build
& ..\bin\Debug\NVRHI-Lab.exe --backend=d3d11 --frameNum=5 2>&1
& ..\bin\Debug\NVRHI-Lab.exe --backend=d3d12 --frameNum=5 2>&1
& ..\bin\Debug\NVRHI-Lab.exe --backend=vulkan --frameNum=5 2>&1
Pop-Location
```

## Error Checking

### Build Errors
1. Check MSBuild output for `error C[0-9]+:` patterns
2. Common build errors:
   - Missing includes → Check header paths
   - Linker errors → Check library dependencies
   - Shader compilation errors → Check HLSL syntax

### Runtime Errors
Watch for `[Error]` prefix in output:
- `Cannot create a binding layout with visibility = None` → Add `.setVisibility(nvrhi::ShaderType::Vertex)`
- `Shader file not found` → Check shader compilation output in `build/shaders/`
- Vulkan validation errors → Check descriptor set bindings

### Exit Codes
- `0` = Success
- Non-zero = Failure (check error output)

## Workflow

### After Code Changes
1. Build the project (incremental)
2. If build fails, fix errors and rebuild
3. Run with `--frameNum=5` for quick verification
4. Test all three backends if changes affect rendering

### Before Committing
1. Full rebuild to ensure clean build
2. Test all three backends with `--frameNum=10`
3. Verify no `[Error]` messages in output

## Notes

- **Why MSBuild instead of `cmake --build`**: Consistency with Visual Studio workflow
- **Why not VS "Rebuild All"**: Race condition with CMake custom build steps causes `error MSB8066`
- **Working directory**: **MUST be `build/`** - shaders are at `build/shaders/`
- **Logging**: All errors go to stderr (non-blocking, no popup dialogs)
- **Exit codes**: 0 = success, non-zero = failure

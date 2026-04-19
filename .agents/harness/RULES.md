# Rules (Harness)

This file is the primary project harness - rules that govern how the AI agent should behave when working on this project.

## Language & Communication
- When explaining concepts, **always include Mandarin translation** in parentheses
- Example: "Texture (纹理) with mipmaps"
- Keep explanations concise and direct

## Code Coverage
- **Comprehensive concept coverage**: If NVRHI supports 8 primitive types, implement all 8
- **Stop at NVRHI level**: Don't dive into backend implementation details (D3D12/Vulkan internals)
- **Backend-agnostic focus**: Focus on concepts, not platform-specific implementation

## Code Style
- Follow existing code conventions in the codebase
- Use libraries/frameworks already present in the project
- Add **concise and useful comments at important locations** (not every line needs comments)
- Comments should explain "why" not "what" (code should be self-documenting for "what")
- **CRITICAL: Always add comments for magical macros/flags** explaining:
  - What the macro/flag does
  - Why it's needed (what problem it solves)
  - Example: `// WIN32_LEAN_AND_MEAN excludes rarely-used Windows headers to avoid winsock.h/winsock2.h conflicts`

## Execution Mode
- Before making non-trivial file changes, ask for confirmation
- Run lint/typecheck commands if available after changes
- Only commit when explicitly asked
- User performs key build tasks: CMake generate, build execution, test running
- AI prepares code and configuration, user executes build commands

## Git Workflow
- User has several years of experience but asks AI "what should I do"
- Guide on git operations when needed
- Never push unless explicitly requested

## Decision Making
- When user asks AI to make a decision (e.g., "you decide", "make the call"), present both the decision AND the reasoning that led to it
- Explanation should be concise but complete enough to justify the choice
- This applies to architectural decisions, tool choices, implementation approaches, and any other decision-making scenarios

## Resource Management

### RAII Principle
**Resources are acquired in constructors and released in destructors.** Let object lifetime manage resource lifetime.

| Pattern | Usage |
|---------|-------|
| `WRL::ComPtr<T>` | Auto-Release for COM objects |
| `std::unique_ptr<T>` | Single-owner smart pointer |
| `nvrhi::DeviceHandle` | NVRHI's ref-counted handles |
| Value members | Objects that live with their owner (e.g., `wxTimer m_Timer`) |

**Anti-patterns**:
```cpp
// ❌ BAD: Manual memory + async callback
wxTimer* m_Timer = new wxTimer(this);
m_Timer->Start(16);
// Timer can fire after this object destroyed!

// ✅ GOOD: Value member
wxTimer m_Timer{this};  // Lives with owner
m_Timer.Start(16);      // Auto-stopped on destruction
```

### Ownership Rules
- **Raw pointers passed to NVRHI** → NVRHI owns (via AddRef), don't Release
- **ComPtr/RefCountPtr** → Auto-release on destruction
- **Value members** → Lifetime tied to containing object

## Build & Debug Workflow

**Use the `build-run-debug` skill** for building, running, and debugging the application. Invoke it when:
- User asks to build, run, test, or debug the app
- After making code changes that need verification
- Testing across all three backends (D3D11, D3D12, Vulkan)

Key points (detailed workflow in skill):
- Use MSBuild (via `vswhere`) for consistency with Visual Studio
- Avoid VS "Rebuild All" - causes race conditions with CMake custom build steps
- Test all three backends after rendering changes
- Use `--frameNum=N` for automated testing

## Debugging & Logging

### wxWidgets Logging
```cpp
#include <wx/log.h>

wxLogError("Failed to create resource");        // Error dialog + log
wxLogWarning("Something unexpected");            // Warning dialog + log
wxLogMessage("Debug info: value=%d", value);    // Info log only
wxLogDebug("Debug: ptr=%p", ptr);               // Debug build only
```

## Documentation Philosophy

### Context Files Purpose
Context files (in `.agents/contexts/`) are for **session continuity** - keeping information essential for the AI agent to stay up-to-date when starting a new session.

**Target audience**: These context files are **agent-facing only** - intended for use by the AI agent and the user through the AI. They are not user-facing documentation.

### Context Files Maintenance (CRITICAL)
**The AI agent is solely responsible for keeping context files up-to-date.** This is NOT the user's responsibility.

**When to update context files:**
- After completing significant milestones (e.g., finishing a phase, implementing a backend)
- When version numbers change (sync with `docs/version-log.md`)
- When task statuses change (update checkboxes in IMPLEMENTATION_PLAN.md)
- Before ending a session if any progress was made

**Cross-reference rule:**
- Always check `docs/version-log.md` for the latest version and completed work
- Context files must never lag behind user-facing documentation
- If discrepancy found, update context files immediately

### Agent-Facing Context (`.agents/contexts/`)
**Target audience**: AI agent continuing the work

| Include | Exclude |
|---------|---------|
| Decisions affecting future work | Implementation details ("how") |
| Constraints (C++20, static libs) | One-time setup facts |
| Conventions (Mandarin translations) | Historical context |
| Learning boundaries | File paths, code snippets |

**Principle**: If it affects how AI should behave or decisions it must respect → Include. Otherwise → Exclude.

### User-Facing Docs (`docs/`)
**Target audience**: Someone learning NVRHI concepts through this project

| Include | Exclude |
|---------|---------|
| **Concepts** - What is a pipeline state? | Implementation details |
| **Why** - Why explicit resource barriers? | Code that can be read in source |
| **Patterns** - Render loop structure | Step-by-step tutorials |
| **NVRHI API insights** | Build instructions (belongs in README) |

**Principle**: Focus on WHAT and WHY, minimize HOW. Code snippets illustrate concepts, not copy-paste implementations.

### File Naming Conventions
- When referring to header and implementation files together, omit the extension: `device_manager`
- This implies both `device_manager.h` and `device_manager.cpp`
- Do NOT use slash: ~~`device_manager.h/cpp`~~ (looks like path separator)

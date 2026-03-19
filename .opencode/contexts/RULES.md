# Rules

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

## Documentation Philosophy

### Agent-Facing Context (`.opencode/contexts/`)
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

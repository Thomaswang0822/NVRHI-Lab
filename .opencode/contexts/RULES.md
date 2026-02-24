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

### User-Facing Docs (docs/)
- **Focus on concepts**, not implementation details
- Explain WHAT and WHY, minimize HOW
- Code snippets should illustrate concepts, not be copy-paste implementations
- Use analogies and high-level explanations
- Target audience: Users of the application who want to understand NVRHI concepts

### Agent-Facing Docs (.opencode/contexts/)
- **Implementation details are welcome here**
- Record architectural decisions, file locations, patterns
- Include specific function names and design choices
- Target audience: AI agents continuing the work

### File Naming Conventions
- When referring to header and implementation files together, omit the extension: `device_manager`
- This implies both `device_manager.h` and `device_manager.cpp`
- Do NOT use slash: ~~`device_manager.h/cpp`~~ (looks like path separator)

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
- **NO comments** unless specifically requested

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

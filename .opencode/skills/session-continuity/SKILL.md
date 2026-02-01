---
name: session-continuity
description: This skill should be used when the AI agent needs to quickly review all project context files to get up-to-date when starting a new session, when project context has been lost, or when the AI agent needs to update context files with new decisions or information.
---

# Session Continuity

## Purpose

Maintain session continuity by reviewing and updating context files that contain essential project information.

## When to Use

### Reading Context

Use this skill when:
- Starting a new session and needing to understand the project context
- Project context has been lost or needs to be refreshed
- Need to review project design, choices, rules, or technical stack

### Writing Context

Write to context files when:
- User explicitly requests with prompts like "write to our continuity log", "update context", "note this decision", or similar
- User forgets to instruct but confirms when asked "Should I update the context files with this decision?"

Write context files only for information that affects how the AI agent should behave or what decisions must be respected in future work. Refer to `.opencode/CONTEXT_PHILOSOPHY.md` for guidance on what to include vs. exclude.

## How to Use

### Reading Context

Run the bundled script to review all context files:

```bash
scripts/review-context.sh
```

The script reads all `.md` files from `.opencode/contexts/` directory, providing comprehensive project context including:

- Project design and philosophy
- Choices made (backends, C++ standard, etc.)
- Learning purpose and boundaries
- Rules and conventions
- Technical stack

Alternatively, manually read the context files from `.opencode/contexts/` directory.

### Writing Context

When updating context files:

1. Determine which file to update based on content type:
   - **CHOICES_MADE.md**: Technical decisions, backend selections, build configurations, optional features
   - **IMPLEMENTATION_PLAN.md**: Project structure, learning sequence, feature modules, milestones
   - **LEARNING_PURPOSE.md**: Learning goals, boundaries, what to learn vs. not learn
   - **PROJECT_DESIGN.md**: Overview, philosophy, primary focus, learning path
   - **RULES.md**: Communication rules, code conventions, execution mode, git workflow
   - **TECH_STACK.md**: Core technologies, languages, tools, development environment

2. Read the existing file first to understand current content

3. Add new information at the appropriate location (maintain existing structure and formatting)

4. Write the updated file preserving the existing style and organization

5. Confirm the update with the user after completion

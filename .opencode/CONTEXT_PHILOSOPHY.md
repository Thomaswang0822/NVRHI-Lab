# Context Philosophy

## Purpose
Context files (in `.opencode/contexts/`) are for **session continuity** - keeping information essential for the AI agent to stay up-to-date when starting a new session.

## Target Audience
These context files are **agent-facing only** - intended for use by the AI agent (opencode) and the user through the AI. They are not user-facing documentation.

- Files in `.opencode/` folder are for the AI agent + user collaboration only
- Traditional user-facing documentation (readme, user guides, API docs) will be created separately in the future
- When creating user-facing docs, relevant information may be copied from these context files and reformatted appropriately

## File Organization
- `.opencode/contexts/` - Essential context for new sessions
- `.opencode/skills/` - Quick reference for "what we have so far" (e.g., available features, progress)

## See Also
- `RULES.md` - Documentation guidelines (what to include/exclude for agent-facing vs user-facing docs)

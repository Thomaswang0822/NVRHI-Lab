# Context Philosophy

## Purpose
Context files (in `.opencode/contexts/`) are for **session continuity** - keeping information essential for the AI agent to stay up-to-date when starting a new session.

## What to Include
- **Project decisions** that affect ongoing work (e.g., all 3 backends enabled, C++20 standard)
- **Learning boundaries** (stop at NVRHI API, not backend internals)
- **Code conventions** (no comments, Mandarin translations in explanations)
- **Technical choices** that constrain future implementation (static libraries, not dynamic)
- **Work preferences** (ask before non-trivial changes, git workflow expectations)

## What NOT to Include
- **One-time setup facts** that won't change (repository name, license type, gitignore template)
- **Detailed implementation how-tos** (CMake snippets, file paths) - these should be in code or implementation docs
- **Static environment details** (specific VS versions, exact folder paths unless relevant)
- **Historical context** about initial planning phases (that's what session transcripts are for)

## Principle
If the information affects how the AI should behave or what decisions it must respect in future work, include it. If it's just "nice to know" or reference material, exclude it.

## File Organization
- `.opencode/contexts/` - Essential context for new sessions
- `.opencode/skills/` - Quick reference for "what we have so far" (e.g., available features, progress)

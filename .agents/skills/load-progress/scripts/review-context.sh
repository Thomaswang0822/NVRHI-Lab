#!/bin/bash

# Session Continuity - Quick Context Review
# Reads all context markdown files to get up-to-date with project state

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONTEXT_DIR="$SCRIPT_DIR/../../../contexts"
SEPARATOR="========================================"

echo ""
echo "$SEPARATOR"
echo "SESSION CONTINUITY - CONTEXT REVIEW"
echo "$SEPARATOR"
echo ""

for file in "$CONTEXT_DIR"/*.md; do
    if [ -f "$file" ]; then
        filename=$(basename "$file")
        echo ""
        echo ">>> $filename <<<"
        echo "$SEPARATOR"
        cat "$file"
        echo ""
    fi
done

echo "$SEPARATOR"
echo "END OF CONTEXT REVIEW"
echo "$SEPARATOR"

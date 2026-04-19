#!/bin/bash

# Session Continuity - Quick Context Review
# Reads all context and harness markdown files to get up-to-date with project state

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONTEXT_DIR="$SCRIPT_DIR/../../../contexts"
HARNESS_DIR="$SCRIPT_DIR/../../../harness"
SEPARATOR="========================================"

echo ""
echo "$SEPARATOR"
echo "SESSION CONTINUITY - CONTEXT REVIEW"
echo "$SEPARATOR"
echo ""

echo ""
echo ">>> HARNESS (Project Rules) <<<"
echo "$SEPARATOR"
for file in "$HARNESS_DIR"/*.md; do
    if [ -f "$file" ]; then
        filename=$(basename "$file")
        echo ""
        echo "--- $filename ---"
        cat "$file"
        echo ""
    fi
done

echo "$SEPARATOR"
echo ""
echo ">>> CONTEXT FILES <<<"
echo "$SEPARATOR"
for file in "$CONTEXT_DIR"/*.md; do
    if [ -f "$file" ]; then
        filename=$(basename "$file")
        echo ""
        echo "--- $filename ---"
        cat "$file"
        echo ""
    fi
done

echo "$SEPARATOR"
echo "END OF CONTEXT REVIEW"
echo "$SEPARATOR"

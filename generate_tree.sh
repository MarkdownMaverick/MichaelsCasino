#!/bin/bash

# Configuration
OUTPUT="PROJECT_TREE.md"
TREE_DEPTH=4
EXCLUDE_DIRS=".git"

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Generate project structure
echo -e "${BLUE}Generating project tree...${NC}"
tree -a -L "$TREE_DEPTH" -I "$EXCLUDE_DIRS" > "$OUTPUT"

# Add summary section
{
  echo ''
  echo 'To run this script, use the following commands:'
  echo 'chmod +x generate_tree.sh'
  echo './generate_tree.sh'
  echo ''
  echo '```'

  echo '```'
} >> "$OUTPUT"

echo -e "${GREEN}✓ Project tree saved to $OUTPUT${NC}"
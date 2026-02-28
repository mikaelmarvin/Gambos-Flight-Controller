#!/bin/bash
# Single entry point for devcontainer setup.
# - Run with no args (postCreate): configure build, fix compile_commands paths, add Starship to .bashrc.
# - Run with --post-start: only fix compile_commands paths (called on every container start).

set -e
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

if [[ "${1:-}" == "--post-start" ]]; then
    # Only fix compile_commands paths (container may have started with host-built paths)
    ./project/scripts/fix-compile-commands-for-container.sh
    exit 0
fi

# --- Full setup (postCreate) ---

# 1. Generate build and compile_commands.json
./project/scripts/configure.sh || true

# 2. Fix paths in compile_commands.json for container
./project/scripts/fix-compile-commands-for-container.sh || true

# 3. Add Starship prompt to ~/.bashrc (idempotent)
STARSHIP_LINE='[ -n "$BASH_VERSION" ] && command -v starship >/dev/null 2>&1 && eval "$(starship init bash)"'
if ! grep -q 'starship init bash' ~/.bashrc 2>/dev/null; then
    echo "" >> ~/.bashrc
    echo "# Starship prompt (gambos-sw devcontainer)" >> ~/.bashrc
    echo "$STARSHIP_LINE" >> ~/.bashrc
    echo "Added Starship to ~/.bashrc"
fi

echo "Devcontainer setup done. Open a new terminal for Starship prompt."

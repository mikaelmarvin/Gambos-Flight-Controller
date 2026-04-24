#!/bin/bash
# Single entry point for devcontainer setup.
# - Run with no args (postCreate): configure build, fix compile_commands paths, add Starship to .bashrc.
# - Run with --post-start: fix compile_commands paths + ensure Starship in ~/.bashrc (every start).

set -e
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

ensure_starship_bashrc() {
    # Idempotent: persistent dev-home volumes may predate image skel or omit this block.
    local line='[ -n "$BASH_VERSION" ] && command -v starship >/dev/null 2>&1 && eval "$(starship init bash)"'
    if ! grep -q 'starship init bash' "${HOME}/.bashrc" 2>/dev/null; then
        echo "" >> "${HOME}/.bashrc"
        echo "# Starship prompt (gambos devcontainer)" >> "${HOME}/.bashrc"
        echo "$line" >> "${HOME}/.bashrc"
        echo "Added Starship to ~/.bashrc"
    fi
}

if [[ "${1:-}" == "--post-start" ]]; then
    bash software/project/scripts/fix-compile-commands-for-container.sh
    ensure_starship_bashrc || true
    exit 0
fi

# --- Full setup (postCreate) ---

# 1. Generate build dirs + compile_commands.json for both presets (matches software/.clangd path routing:
#    app/board custom → build/custom, devkit → build/devkit). Same as build.sh per preset.
bash software/project/scripts/build.sh custom || true
bash software/project/scripts/build.sh devkit || true

# 2. Fix paths in compile_commands.json for container
bash software/project/scripts/fix-compile-commands-for-container.sh || true

# 3. Starship in ~/.bashrc (idempotent; also runs on every postStart for old volumes)
ensure_starship_bashrc || true

echo "Devcontainer setup done. Open a new terminal for Starship prompt."

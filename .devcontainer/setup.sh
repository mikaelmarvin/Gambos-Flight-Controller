#!/bin/bash
# Single entry point for devcontainer setup.
# - Run with no args (postCreate): configure build, add Starship to .bashrc.
# - Run with --post-start: ensure Starship in ~/.bashrc (every start).

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
    ensure_starship_bashrc || true
    exit 0
fi

# --- Full setup (postCreate) ---

# Configure + build custom (compile_commands.json for clangd).
bash software/project/scripts/build.sh custom

ensure_starship_bashrc || true

echo "Devcontainer setup done. Open a new terminal for Starship prompt."

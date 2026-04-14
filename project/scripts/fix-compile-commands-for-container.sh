#!/usr/bin/env bash
# Rewrite compile_commands.json so paths use the container workspace.
# Helps clangd / Go to Definition when the tree was configured on the host
# (e.g. /home/user/gambos-sw) and you open it under /workspace/project/gambos-sw.
# Run from repo root; safe to run multiple times.
#
# Usage: ./project/scripts/fix-compile-commands-for-container.sh [custom|devkit|all]
#   custom | devkit — only that preset under project/build/<preset>/
#   all (default)  — update each of project/build/custom and project/build/devkit
#                    when compile_commands.json exists
#
# Override destination: CONTAINER_WORKSPACE=/path ./project/scripts/...

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
CONTAINER_WORKSPACE="${CONTAINER_WORKSPACE:-/workspace/project/gambos-sw}"
MODE="${1:-all}"

usage() {
	echo "Usage: $0 [custom|devkit|all]" >&2
	echo "  Rewrite compile_commands.json under project/build/<preset>/ to use" >&2
	echo "  CONTAINER_WORKSPACE (default: ${CONTAINER_WORKSPACE})." >&2
	exit 2
}

case "$MODE" in
custom | devkit | all) ;;
-h | --help)
	echo "Usage: $0 [custom|devkit|all]" >&2
	echo "  Default: all — fixes project/build/custom and project/build/devkit when present." >&2
	exit 0
	;;
*) usage ;;
esac

# Args: path to compile_commands.json, label for messages
# Returns: 0 on success or benign skip, 1 on parse / rewrite error
fix_one() {
	local cc_json="$1"
	local label="$2"

	if [[ ! -f "$cc_json" ]]; then
		echo "Skip ${label}: not found (${cc_json})"
		return 0
	fi

	local first_dir
	# -m1: single match avoids grep|head SIGPIPE under pipefail
	first_dir=$(grep -oPm1 '"directory":\s*"\K[^"]+' "$cc_json" || true)
	if [[ -z "$first_dir" ]]; then
		echo "Could not parse directory from: ${cc_json}" >&2
		return 1
	fi

	if [[ "$first_dir" == /workspace/* ]]; then
		echo "OK ${label}: already uses container paths."
		return 0
	fi

	local host_root
	if [[ "$first_dir" =~ ^(.*/gambos-sw) ]]; then
		host_root="${BASH_REMATCH[1]}"
	else
		echo "Could not detect host repo root from: ${first_dir} (${cc_json})" >&2
		return 1
	fi

	if [[ "$host_root" == "$CONTAINER_WORKSPACE" ]]; then
		echo "OK ${label}: already uses container paths."
		return 0
	fi

	sed -i "s|${host_root}|${CONTAINER_WORKSPACE}|g" "$cc_json"
	echo "Updated ${label}: ${host_root} -> ${CONTAINER_WORKSPACE}"
	return 0
}

ERR=0

if [[ "$MODE" == "all" ]]; then
	fix_one "${REPO_ROOT}/project/build/custom/compile_commands.json" "custom" || ERR=1
	fix_one "${REPO_ROOT}/project/build/devkit/compile_commands.json" "devkit" || ERR=1
else
	cc_json="${REPO_ROOT}/project/build/${MODE}/compile_commands.json"
	if [[ ! -f "$cc_json" ]]; then
		echo "Not found: ${cc_json}. Run: cd project && cmake --preset ${MODE}" >&2
		exit 0
	fi
	fix_one "$cc_json" "$MODE" || ERR=1
fi

if [[ "$ERR" -ne 0 ]]; then
	exit 1
fi

echo "Done. Reload the window (Ctrl+Shift+P -> Developer: Reload Window) so clangd picks up changes."

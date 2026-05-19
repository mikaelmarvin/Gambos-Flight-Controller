#!/usr/bin/env bash
# Rewrite compile_commands.json so paths use the container workspace.
# Helps clangd / Go to Definition when the tree was configured on the host
# (e.g. /home/user/repo) and you open it under a bind-mounted path (e.g. /workspace/gambos).
# Run from repo root; safe to run multiple times.
#
# Usage: ./software/project/scripts/fix-compile-commands-for-container.sh [devkit|custom|all]
#   devkit           — software/project/build/devkit/compile_commands.json
#   custom           — software/project/build/custom/compile_commands.json
#   all (default)    — both devkit and custom
#
# Override destination: CONTAINER_WORKSPACE=/path ./software/project/scripts/...

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
CONTAINER_WORKSPACE="${CONTAINER_WORKSPACE:-$REPO_ROOT}"
MODE="${1:-all}"

print_help() {
	local stream="${1:-stdout}"
	local out="/dev/stdout"
	[[ "$stream" == "stderr" ]] && out="/dev/stderr"

	cat >"$out" <<EOF
Usage: $0 [devkit|custom|all]
  Rewrite compile_commands.json under software/project/build/{devkit,custom}/ to use
  CONTAINER_WORKSPACE (default: ${CONTAINER_WORKSPACE}).
  Default mode: all
EOF
}

usage() {
	print_help stderr
	exit 2
}

case "$MODE" in
devkit | custom | all) ;;
-h | --help)
	print_help
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

	local ok_msg="OK ${label}: already uses container paths."
	if [[ "$first_dir" == /workspace/* ]]; then
		echo "$ok_msg"
		return 0
	fi

	local host_root
	if [[ "$first_dir" == *"/software/project/"* ]]; then
		host_root="${first_dir%%/software/project/*}"
	elif [[ "$first_dir" == *"/project/"* ]]; then
		# Legacy layout (repo root contained project/ without software/)
		host_root="${first_dir%%/project/*}"
	else
		echo "Could not detect host repo root from: ${first_dir} (${cc_json})" >&2
		return 1
	fi

	if [[ "$host_root" == "$CONTAINER_WORKSPACE" ]]; then
		echo "$ok_msg"
		return 0
	fi

	sed -i "s|${host_root}|${CONTAINER_WORKSPACE}|g" "$cc_json"
	echo "Updated ${label}: ${host_root} -> ${CONTAINER_WORKSPACE}"
	return 0
}

ERR=0
targets=()
case "$MODE" in
devkit) targets=("devkit") ;;
custom) targets=("custom") ;;
all) targets=("devkit" "custom") ;;
esac

for target in "${targets[@]}"; do
	fix_one "${REPO_ROOT}/software/project/build/${target}/compile_commands.json" "${target}" || ERR=1
done

if [[ "$ERR" -ne 0 ]]; then
	exit 1
fi

echo "Done. Reload the window (Ctrl+Shift+P -> Developer: Reload Window) so clangd picks up changes."

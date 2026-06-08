#!/usr/bin/env bash
# Runs on the HOST via devcontainer.json "initializeCommand" (before the dev container is created).
# Limits pile-up from this compose project + Dev Containers' per-config vsc-*-uid wrapper images.
# Does NOT run "docker compose down", does NOT remove volumes, does NOT stop a running container.

set -euo pipefail
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${REPO_ROOT}"

if ! command -v docker >/dev/null 2>&1; then
    exit 0
fi

COMPOSE_FILE="${REPO_ROOT}/docker-compose.yml"
PROJECT_NAME="$(
    grep -m1 '^name:' "${COMPOSE_FILE}" 2>/dev/null | awk '{print $2}' | tr -d '\r' \
        || basename "${REPO_ROOT}" | tr '[:upper:]' '[:lower:]'
)"
SLUG="$(basename "${REPO_ROOT}" | tr '[:upper:]' '[:lower:]')"
COMPOSE=(docker compose -f "${COMPOSE_FILE}" --project-name "${PROJECT_NAME}")

# Stopped containers for this compose project only (does not stop a running dev container).
"${COMPOSE[@]}" rm -f 2>/dev/null || true

if docker container prune --help 2>/dev/null | grep -q -- '--filter'; then
    docker container prune -f --filter "label=com.docker.compose.project=${PROJECT_NAME}" 2>/dev/null || true
fi

if docker image prune --help 2>/dev/null | grep -q -- '--filter'; then
    docker image prune -f --filter "label=com.docker.compose.project=${PROJECT_NAME}" 2>/dev/null || true
fi

mapfile -t vsc_repos < <(docker images --format '{{.Repository}}' | grep -E "^vsc-${SLUG}-" | sort -u || true)
if ((${#vsc_repos[@]} > 1)); then
    for repo in "${vsc_repos[@]}"; do
        if docker ps -a --filter "ancestor=${repo}:latest" -q | grep -q .; then
            continue
        fi
        docker rmi "${repo}:latest" 2>/dev/null || true
    done
fi

if docker builder prune --help 2>/dev/null | grep -q -- '--filter'; then
    docker builder prune -f --filter "until=336h" 2>/dev/null || true
fi

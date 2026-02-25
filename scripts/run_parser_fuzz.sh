#!/usr/bin/env bash
set -euo pipefail

FUZZ_BINARY=${1:-build/ivy_parser_fuzz}

"${FUZZ_BINARY}" -max_total_time=60 -timeout=2 -rss_limit_mb=1024

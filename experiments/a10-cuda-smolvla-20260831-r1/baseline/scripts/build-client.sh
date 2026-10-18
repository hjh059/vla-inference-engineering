#!/usr/bin/env bash
set -euo pipefail

workspace=/root/vla-smoke-c01
build_dir="$workspace/build-formal-a10-20260831"
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

c++ -std=c++17 -O2 -Wall -Wextra \
  -I"$build_dir/proto-gen/serving" \
  "$script_dir/fixed_request_client.cpp" \
  "$build_dir/proto-gen/serving/vla.pb.cc" \
  -lprotobuf -lzmq \
  -o "$build_dir/fixed_request_client"

sha256sum "$build_dir/fixed_request_client"

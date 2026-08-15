#!/bin/bash
set -e

script_dir="$(cd "$(dirname "$0")" && pwd)"
project_root="$script_dir/.."
cd "$project_root"

app_subpath=""
jobs=$(nproc)
rebuild=0
uorb_generate=0
unit_test=0
while getopts "a:j:rut" opt; do
    case $opt in
        a) app_subpath="$OPTARG" ;;
        j) jobs="$OPTARG" ;;
        r) rebuild=1 ;;
        u) uorb_generate=1 ;;
        t) unit_test=1 ;;
        *) echo "Usage: $0 -a <app_subpath> [-j jobs] [-r]" >&2; exit 1 ;;
    esac
done

if [ -z "$app_subpath" ]; then
    echo "Error: -a <app_subpath> is required." >&2
    exit 1
fi

app_dir="apps/$app_subpath"
if [ ! -d "$app_dir" ]; then
    echo "Error: Application directory '$app_dir' does not exist." >&2
    exit 1
fi

cmake_file="$app_dir/CMakeLists.txt"
if [ ! -f "$cmake_file" ]; then
    echo "Error: $cmake_file not found." >&2
    exit 1
fi

proj_name=$(grep -E '^[[:space:]]*project\s*\([^)]+\)' "$cmake_file" | head -1 | sed -E 's/^[[:space:]]*project\s*\(\s*([^)]+)\s*\).*$/\1/')
if [ -z "$proj_name" ]; then
    echo "Warning: Could not find project name in $cmake_file, using '$app_subpath' as name."
    proj_name="$app_subpath"
fi

build_dir="$project_root/build/$proj_name"

if [ $rebuild -eq 1 ]; then
    echo "[cmake]: cleaning $proj_name"
    rm -rf "$build_dir"
fi

BUILD_TEST=
if [ $unit_test -eq 1 ]; then
    BUILD_TEST="ON"
fi

mkdir -p "$build_dir"

uorb_generate_path=${script_dir}/../build/${app_subpath}
uorb_msg_path=${script_dir}/../apps/${app_subpath}/msg/
bash ${script_dir}/../gpm/tools/msg/uorb_msg_generate.sh ${uorb_msg_path} ${uorb_generate_path} ${uorb_generate}
exit_code=$?
if [ ${exit_code} == 0 ]; then
    uorb_including="${uorb_generate_path}/;"
    uorb_enable=y
else
    echo "[uorb] exec failed ${uorb_generate}"
    uorb_including=
    uorb_enable=n
fi

cd "$build_dir"

cmake "$project_root/$app_dir" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DUORB_INCLUDING=${uorb_generate_path} \
    -DBUILD_TEST=${BUILD_TEST}

make -j "$jobs"

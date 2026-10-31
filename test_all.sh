#!/usr/bin/env bash
set -euo pipefail

# Ensure MSYS2 MinGW64 toolchain (cmake, ninja, g++, gtest) is on PATH when run on Windows
if [ -d "/mingw64/bin" ]; then
    export PATH="/mingw64/bin:/usr/bin:${PATH}"
elif [ -d "/c/msys64/mingw64/bin" ]; then
    export PATH="/c/msys64/mingw64/bin:/c/msys64/usr/bin:${PATH}"
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_ROOT="${SCRIPT_DIR}/build_tests"

SUITES=(
    "NOM_STACK/Signals/Sensors/test"
    "NOM_STACK/Services/I2C_COM/test"
    "NOM_STACK/Services/PUMP_IO_COM/test"
    "NOM_STACK/UI/DWIN_protocol/test"
    "NOM_STACK/Sensors/RealTimeClock/test"
    "NOM_STACK/Sensors/PH_sensor/test"
    "NOM_STACK/Sensors/EC_sensor/test"
    "NOM_STACK/Sensors/WL_sensor/test"
    "NOM_STACK/Sensors/WTEMP_sensor/test"
    "NOM_STACK/Application/MAIN_SYS/test"
    "NOM_STACK/Application/WL/test"
    "NOM_STACK/Application/EC/test"
    "NOM_STACK/Application/PH/test"
    "NOM_STACK/Application/LED/test"
    "NOM_STACK/Application/Pipes/test"
    "NOM_STACK/Manager/INIT/test"
    "NOM_STACK/Manager/NormalOp/test"
    "NOM_STACK/Services/Scheduler/test"
)

TOTAL=${#SUITES[@]}
PASSED=0
FAILED=0
FAILED_NAMES=()

echo "=================================================================="
echo " Running AERO_MAIN_SYS GoogleTest Verification (${TOTAL} suites)"
echo "=================================================================="

for REL_DIR in "${SUITES[@]}"; do
    SRC_DIR="${SCRIPT_DIR}/${REL_DIR}"
    SUITE_KEY="$(echo "${REL_DIR}" | tr '/' '_')"
    B_DIR="${BUILD_ROOT}/${SUITE_KEY}"

    echo ""
    echo "------------------------------------------------------------------"
    echo ">>> Suite: ${REL_DIR}"
    echo "------------------------------------------------------------------"

    mkdir -p "${B_DIR}"
    if ! cmake -S "${SRC_DIR}" -B "${B_DIR}" -G Ninja >"${B_DIR}/cmake.log" 2>&1; then
        cat "${B_DIR}/cmake.log"
        FAILED=$((FAILED + 1))
        FAILED_NAMES+=("${REL_DIR}")
        continue
    fi
    if ! cmake --build "${B_DIR}" >"${B_DIR}/build.log" 2>&1; then
        cat "${B_DIR}/build.log"
        FAILED=$((FAILED + 1))
        FAILED_NAMES+=("${REL_DIR}")
        continue
    fi
    if ctest --test-dir "${B_DIR}" --output-on-failure; then
        PASSED=$((PASSED + 1))
    else
        FAILED=$((FAILED + 1))
        FAILED_NAMES+=("${REL_DIR}")
    fi
done

echo ""
echo "=================================================================="
echo " Summary: ${PASSED}/${TOTAL} GoogleTest suites PASSED, ${FAILED} FAILED"
echo "=================================================================="

if [ "${FAILED}" -ne 0 ]; then
    echo "Failed suites:"
    for NAME in "${FAILED_NAMES[@]}"; do
        echo "  - ${NAME}"
    done
    exit 1
fi

# AERO_MAIN_SYS — ESP32 Main Controller Firmware

Main ESP32 controller firmware (`ESP-IDF v4.4`) for the TokaGREEN aeroponic system (`HwTestsFw`), managing:
- **DWIN HMI Touch Panel** (`NOM_STACK/UI/DWIN_protocol`) over UART (`0x5A 0xA5` stream framing, `µS/cm` EC units, crop/stage profiles, manual fill/drain, calibration, and RTC settings).
- **STM32 Pump & Sensor IO Board** (`NOM_STACK/Services/PUMP_IO_COM`) over UART (`0x37..0x3F` 8-byte sensor frames, 5-byte actuator frames, 2.5 s link-health watchdog, and Safe-Off boot gating with automatic NVS state resume once valid STM32 frames arrive).
- **Closed-Loop Control Applications** (`NOM_STACK/Application/{MAIN_SYS,WL,EC,PH,LED,Pipes}`) scheduled deterministically via persistent FreeRTOS tasks (`NOM_STACK/Services/Scheduler`).
- **Host GoogleTest Verification** (`test/` inside every `NOM_STACK` module + `test_support/` ESP-IDF/FreeRTOS host mocks + `./test_all.sh`).

---

## 1. Clone & Open Project

```bash
git clone git@github.com:embedakk/AERO_MAIN_SYS.git
cd AERO_MAIN_SYS
git checkout HwTestsFw
code .
```

---

## 2. Host Unit & Integration Tests (GoogleTest)

Every `NOM_STACK` module with `inc/` and `src/` includes its own `test/` folder (`CMakeLists.txt` + `test_*.cpp` — **18 test suites total**). Tests compile natively on your PC against lightweight ESP-IDF/FreeRTOS/NVS/UART/I2C/GPIO mocks in `test_support/`.

### Prerequisites (MSYS2 MinGW64 on Windows)

Open **MSYS2 MinGW64** shell and install the host C++ toolchain, CMake, Ninja, and GoogleTest if not already installed:

```bash
pacman -S --needed --noconfirm \
    mingw-w64-x86_64-toolchain \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-ninja \
    mingw-w64-x86_64-gtest
```

### Run All 18 GoogleTest Suites (`test_all.sh`)

From **MSYS2 MinGW64** or **Git Bash** in the repository root:

```bash
./test_all.sh
```

Or directly from **Windows PowerShell / CMD**:

```powershell
C:\msys64\usr\bin\bash.exe ./test_all.sh
```

Expected final output:

```text
==================================================================
 Summary: 18/18 GoogleTest suites PASSED, 0 FAILED
==================================================================
```

### Run a Single Module Test Suite

To configure, build, and run one module's test suite with verbose GoogleTest output (example: `Scheduler` or `DWIN_protocol`):

```bash
cmake -S NOM_STACK/Services/Scheduler/test -B build_tests/scheduler -G Ninja
cmake --build build_tests/scheduler
ctest --test-dir build_tests/scheduler --output-on-failure --verbose
```

To clean all host test build artifacts:

```bash
rm -rf build_tests
```

### Covered `NOM_STACK` Test Suites (18/18)

| Layer | Module Path | Test Target |
| :--- | :--- | :--- |
| Signals | `NOM_STACK/Signals/Sensors/test` | `test_sensor_ic_signals` |
| Services | `NOM_STACK/Services/I2C_COM/test` | `test_i2c_com` |
| Services | `NOM_STACK/Services/PUMP_IO_COM/test` | `test_pump_io_com` |
| Services | `NOM_STACK/Services/Scheduler/test` | `test_scheduler` |
| UI | `NOM_STACK/UI/DWIN_protocol/test` | `test_dwin_protocol` |
| Sensors | `NOM_STACK/Sensors/RealTimeClock/test` | `test_real_time_clock` |
| Sensors | `NOM_STACK/Sensors/PH_sensor/test` | `test_ph_sensor` |
| Sensors | `NOM_STACK/Sensors/EC_sensor/test` | `test_ec_sensor` |
| Sensors | `NOM_STACK/Sensors/WL_sensor/test` | `test_wl_sensor` |
| Sensors | `NOM_STACK/Sensors/WTEMP_sensor/test` | `test_wtemp_sensor` |
| Application | `NOM_STACK/Application/MAIN_SYS/test` | `test_main_sys` |
| Application | `NOM_STACK/Application/WL/test` | `test_wl_application` |
| Application | `NOM_STACK/Application/EC/test` | `test_ec_application` |
| Application | `NOM_STACK/Application/PH/test` | `test_ph_application` |
| Application | `NOM_STACK/Application/LED/test` | `test_led_application` |
| Application | `NOM_STACK/Application/Pipes/test` | `test_pipes_application` |
| Manager | `NOM_STACK/Manager/INIT/test` | `test_init_manager` |
| Manager | `NOM_STACK/Manager/NormalOp/test` | `test_normal_op` |

---

## 3. Configure, Build, Flash & Monitor ESP32 Firmware (`ESP-IDF v4.4`)

> **Note:** `NOM_STACK/*/*/test/` and `test_support/` are automatically ignored by ESP-IDF component discovery and only `NOM_STACK` firmware components are compiled into `AERO_MAIN_SYS.bin`.

### Step 1: Activate ESP-IDF v4.4 Environment

Open **ESP-IDF 4.4 PowerShell** (or initialize ESP-IDF v4.4 in an existing PowerShell window):

```powershell
$env:IDF_TOOLS_PATH = "C:\Espressif-4v4"
$env:IDF_PATH = "C:\Espressif-4v4\frameworks\esp-idf-v4.4.3"
. C:\Espressif-4v4\Initialize-Idf.ps1
cd D:\TokaGREEN\repos\AERO_MAIN_SYS
```

### Step 2: Configure & Build Firmware

```powershell
# (First time only or after fullclean) set target chip to ESP32
idf.py set-target esp32

# Optional: open SDK configuration menu
idf.py menuconfig

# Build bootloader, partition table, and AERO_MAIN_SYS.bin
idf.py build
```

### Step 3: Flash & Serial Monitor

Replace `COMx` (for example `COM5`) with your ESP32 USB-UART port:

```powershell
# Flash bootloader, partition table, and application binary
idf.py -p COMx flash

# Open live serial monitor (press Ctrl+] to exit)
idf.py -p COMx monitor

# Build, flash, and monitor in a single command
idf.py -p COMx build flash monitor
```

### Useful Maintenance Commands

```powershell
# Clean ESP-IDF build directory
idf.py fullclean

# Erase entire ESP32 flash (including stored NVS settings/calibration) before re-flashing
idf.py -p COMx erase-flash
```

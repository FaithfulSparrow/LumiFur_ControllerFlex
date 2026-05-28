<p align="center">
<img width="300" alt="LumiFur Controller" src="docs/mps3.png">
</p>
<h1 align="center">
  LumiFur Controller<br>
  
  [![CodeQL Advanced Build with PlatformIO](https://github.com/stef1949/LumiFur_Controller/actions/workflows/codeql.yml/badge.svg?branch=main)](https://github.com/stef1949/LumiFur_Controller/actions/workflows/codeql.yml)
  <img alt="GitHub Release" src="https://img.shields.io/github/v/release/stef1949/lumifur_controller">
  <a href="https://github.com/stef1949/LumiFur_Controller" alt="Activity">
        <img src="https://img.shields.io/github/commit-activity/m/stef1949/LumiFur_Controller" /></a>
  [![Coverage Status](https://coveralls.io/repos/github/stef1949/LumiFur_Controller/badge.svg?branch=main)](https://coveralls.io/github/stef1949/LumiFur_Controller?branch=main)
</h1>
A real-time firmware for animating a **WS2812B addressable LED matrix Protogen mask** with sensor-driven interactions, Bluetooth LE control, and OTA updates, running on the Adafruit MatrixPortal ESP32-S3 as the controller board.

> **Fork note:** This is the `LumiFur_ControllerFlex` fork. The upstream firmware targets HUB75 panels; this fork replaces the display backend with FastLED so the same MatrixPortal can drive a flexible WS2812B / WS2812 matrix over a single data pin. The HUB75 path is preserved behind a build flag, but the default and recommended build is `adafruit_matrixportal_esp32s3_flex`.

## Table of Contents

- ✨ [Features](#features)
- ⚙️ [Hardware](#hardware)
- 🛠️ [Build & Flash](#build--flash)
- 🎛️ [Configuration](#configuration)
- 📡 [BLE Control](#ble-control)
- 🧪 [Testing](#testing)
- 🤖 [GitHub Copilot Integration](#github-copilot-integration)
- 🤝 [Contributing](#contributing)
- 📜 [License](#license)
- 🌐 [Web Firmware Updater](#web-firmware-updater)

## Features
- 20+ animated faces and effects (plasma, flame, fluid, circle eyes, spiral overlays, starfields, scrolling text) authored on a 128×32 design canvas and auto-scaled to whatever physical WS2812 matrix you wire up (default 48×12 = 576 LEDs).
- Sensor-driven reactions: the APDS9960 manages adaptive brightness and proximity-triggered blush/eye bounce, the LIS3DH accelerometer powers shake gestures and PixelDust physics, and an I2S MEMS microphone drives audio-reactive mouth animation. All sensors degrade gracefully if absent.
- Bluetooth Low Energy control via NimBLE with remote expression switching, brightness management, sensor feature toggles, temperature telemetry, and a command channel for log retrieval.
- Wireless firmware delivery through a dedicated OTA characteristic plus runtime build metadata advertisement for companion apps.
- Power-aware runtime with wake-on-motion, sleep dimming, ambient light scaling, and persistent user preferences stored in ESP32 NVS.
- FastLED-backed pipeline (`src/display/FastLEDBackend.cpp`) for WS2812B/WS2812 matrices, plus a preserved HUB75 backend (`src/display/HUB75Backend.cpp`) behind `LF_DISPLAY_BACKEND=1` for users who want the upstream display.
- File-backed monochrome video playback from onboard FATFS storage, including GIF/video conversion into a compact `LFV1` format for `VIEW_VIDEO_PLAYER`.

## Hardware
- **Controller:** Adafruit MatrixPortal ESP32-S3 with built-in APDS9960, LIS3DH, status NeoPixel, and onboard microphone. The MatrixPortal is used here as a general-purpose ESP32-S3 dev board — its HUB75 IDC connector is left empty in the Flex build.
- **Display:** WS2812B (or pin-compatible WS2812) addressable LED matrix, default geometry **48 columns × 12 rows = 576 LEDs**, serpentine row-major wiring (each row reverses direction so the wire snakes back and forth, keeping leads short). Geometry and ordering are configurable via PlatformIO build flags (`LF_FLEX_WIDTH`, `LF_FLEX_HEIGHT`, `LF_FLEX_SERPENTINE`, `LF_FLEX_ROW_MAJOR`, `LF_FLEX_FLIP_X`, `LF_FLEX_FLIP_Y`) — see `src/display/XYMap.h`.
- **Data pin:** GPIO 14 by default (`LF_LED_DATA_PIN`). On the MatrixPortal S3 this is also the HUB75 `OE` pin — leave the HUB75 IDC connector empty when running the Flex firmware.
- **Level shifter (strongly recommended):** [Adafruit #1787 — 74AHCT125 Quad Level-Shifter (3 V → 5 V)](https://www.adafruit.com/product/1787). This is a **unidirectional**, four-channel buffer with push-pull outputs, powered from a single 5 V rail; its inputs are 3.3 V-compatible and its outputs swing rail-to-rail at 5 V. It's also Adafruit's specifically recommended part for NeoPixel / WS2812 level shifting — see their [NeoPixel Überguide → Logic Level page](https://learn.adafruit.com/adafruit-neopixel-uberguide/logic-level). The MatrixPortal S3's GPIOs are 3.3 V, while WS2812B spec calls for a logic-high of ≥ 0.7 × VDD = **3.5 V at 5 V supply** — direct connection often works for short runs of newer LEDs but is marginal. Wiring is shown in [Wiring the WS2812 matrix](#wiring-the-ws2812-matrix) below.
- **Power:** Use an **external 5 V PSU** sized for your real worst-case current (~60 mA × LED count × duty cycle). The firmware caps draw at 5 V / 2 A via `FastLED.setMaxPowerInVoltsAndMilliamps` — that's a brightness-scaling cap, not a hardware limit. Do **not** try to power a full matrix off the MatrixPortal's USB-C rail.
- **Onboard sensors:** APDS9960 (I²C @ default 0x39, SDA=16/SCL=17), LIS3DH (I²C @ 0x19), I²S MEMS mic on A1–A4. All board-specific; the firmware tolerates their absence gracefully on non-MatrixPortal boards.
- **Inputs:** Onboard buttons (BUTTON_UP=GPIO 6, BUTTON_DOWN=GPIO 7 on the MatrixPortal) map to view navigation; alternate hardware can be remapped in `src/hardware/deviceConfig.h`.

### Wiring the WS2812 matrix

The 74AHCT125 is a quad buffer with four independent channels (`1A→1Y`, `2A→2Y`, `3A→3Y`, `4A→4Y`), each with its own active-low output-enable (`1/OE` … `4/OE`). Powering it is simple: a single `Vcc` pin (tie to 5 V) and a single `GND`. There is **no separate logic-side rail** — the AHCT input thresholds already accept 3.3 V signals natively while the part is powered at 5 V, which is exactly why this chip family is recommended for 3.3 V → 5 V NeoPixel shifting.

Use one channel for the WS2812 data line; tie all four `/OE` pins to GND so the buffers are always enabled, and ground the unused `A` inputs so they don't float.

```
MatrixPortal ESP32-S3              Adafruit #1787 (74AHCT125)            WS2812B matrix
─────────────────────              ──────────────────────────            ──────────────
  GPIO 14 (data) ────────────►    1A              1Y ──[330 Ω]──►       DIN
                                                                          ▲
  GND  ─────────────┬──────────►  GND                                     │
                    │                                                     │
                    ├──────────►  1/OE  ┐  ─┐                             │
                    ├──────────►  2/OE  │   │                             │
                    ├──────────►  3/OE  │   ├─ all /OEs tied LOW          │
                    ├──────────►  4/OE  ┘   │  (buffers enabled)          │
                    ├──────────►  2A    ┐   │                             │
                    ├──────────►  3A    │   │  unused channels:           │
                    ├──────────►  4A    ┘  ─┘  inputs grounded, Ys NC     │
                    │                                                     │
                    │                                                     │
PSU +5 V ───────────│──────────►  Vcc  (single supply, 4.5–5.5 V)         │
                    │                                                     │
PSU +5 V ───────────│────────────────────────────────────────────────►   +5 V (LEDs)
PSU GND  ───────────┴────────────────────────────────────────────────►   GND  (LEDs)

   1000 µF / ≥10 V electrolytic across +5 V / GND near the first LED.
```

- Connect **PSU GND ↔ MatrixPortal GND ↔ 74AHCT125 GND ↔ Matrix GND** — a common ground is mandatory or the data line voltage is meaningless.
- The 330 Ω series resistor goes between `1Y` and the first LED's DIN. It damps transmission-line ringing.
- The 74AHCT125 has true push-pull outputs and is comfortable driving WS2812 data over typical wiring (well past a metre). It's a much better choice for this than auto-direction shifters like the TXB0104, which use weak one-shot accelerators and tend to flake out on longer leads. Even so, keep the run between `1Y` and the first LED reasonably short and away from noisy power leads.
- The 1000 µF / ≥10 V electrolytic across the LED supply rail near the first LED smooths inrush at frame transitions — standard WS2812B hookup hygiene.
- Ground the unused inputs (`2A`, `3A`, `4A`). Floating CMOS inputs draw shoot-through current and pick up noise.

## Build & Flash

### Toolchain setup (`uv`)

This fork pins its Python toolchain — including PlatformIO itself — via [`uv`](https://docs.astral.sh/uv/), so the build is reproducible without polluting your global Python. The relevant files are:

- `pyproject.toml` — declares Python ≥ 3.12 and the `dev` group (`platformio>=6.1,<7`, `pip`) and `test` group (`pytest>=8`).
- `.python-version` — pins to **Python 3.12**, which `uv` will install on demand.
- `uv.lock` — fully-resolved lockfile checked into the repo.

Install `uv` once (`curl -LsSf https://astral.sh/uv/install.sh | sh` on macOS/Linux, or `winget install --id=astral-sh.uv` on Windows), then from the repo root:

```sh
# Provisions Python 3.12 + the dev group (PlatformIO) into ./.venv from uv.lock
uv sync --group dev

# Optional: also install the test group (pytest)
uv sync --group dev --group test
```

After `uv sync` you have a working `pio` binary at `.venv/bin/pio`. Three equivalent ways to invoke it:

| | Command form | Notes |
|---|---|---|
| 1 | `uv run pio …` | Doesn't require activating the venv; recommended for one-off commands. |
| 2 | `source .venv/bin/activate && pio …` | Familiar venv pattern; `deactivate` to exit. |
| 3 | `./.venv/bin/pio …` | Explicit path; useful in scripts/CI. |

The examples below use **form 1** (`uv run pio …`) so they work in a fresh shell.

### Build, flash, smoke-test

```sh
git clone https://github.com/FaithfulSparrow/LumiFur_ControllerFlex.git
cd LumiFur_ControllerFlex
uv sync --group dev          # one-time toolchain bootstrap

# Build the Flex / WS2812 firmware (the intended target of this fork)
uv run pio run -e adafruit_matrixportal_esp32s3_flex

# Flash over USB
uv run pio run -e adafruit_matrixportal_esp32s3_flex --target upload

# Optional: open the serial monitor at 115200 baud
uv run pio device monitor -b 115200

# Optional: build and upload the FATFS image used for video assets
uv run pio run -e adafruit_matrixportal_esp32s3_flex -t buildfs
uv run pio run -e adafruit_matrixportal_esp32s3_flex -t uploadfs
```

**Before flashing the full firmware, bring up the matrix with the smoke test** — it cycles solid RGB → walking dot → corner dots → diagonal so you can confirm wiring, GRB color order, and the serpentine XY mapping before any real effects run:

```sh
uv run pio run -e flex_smoke -t upload
```

If the RGB cycle shows the wrong colors, change the color order in `src/display/FastLEDBackend.cpp` (e.g. `RGB`, `BGR`, …). If the walking dot doesn't trace left-to-right row-by-row, flip `LF_FLEX_SERPENTINE` / `LF_FLEX_ROW_MAJOR` / `LF_FLEX_FLIP_X` / `LF_FLEX_FLIP_Y` via build flags in `platformio.ini`.

Additional PlatformIO environments are defined in `platformio.ini`: `dev-flex` (debug build of the Flex env), the original HUB75 environments (`adafruit_matrixportal_esp32s3`, `dev`, etc., kept for users who still want the upstream HUB75 display), and native test environments (`codeql`, `native2`).
The PlatformIO helper scripts used by these environments now live under `tools/platformio/`, and the firmware artifact workflow is documented in `docs/build/FIRMWARE_BUILD.md`.

> **VS Code users**: the official PlatformIO IDE extension can pick up `./.venv` automatically — set `platformio-ide.useBuiltinPIOCore` to `false` and `platformio-ide.customPATH` to the absolute path of `./.venv/bin` (or `.\.venv\Scripts` on Windows), and the extension will reuse the `uv`-managed toolchain instead of installing a private copy.

## Configuration
- **Persistent preferences:** `src/config/userPreferences.h` manages stored defaults for brightness, auto-blink, accelerometer usage, sleep mode, and other controller behaviors.
- **Hardware mapping:** Modify `src/hardware/deviceConfig.h` to adjust button pin assignments. For the Flex backend, matrix dimensions and orientation are controlled by `LF_FLEX_*` build flags in `platformio.ini` (see `src/display/XYMap.h`); the HUB75 pinouts, panel chains, and `VIRTUAL_PANE` simulator are still available under the `adafruit_matrixportal_esp32s3` (HUB75) environments.
- **Effects & assets:** Custom scenes live in `src/effects/`, bitmap assets in `src/assets/bitmaps.h`, and optional font assets in `src/assets/fonts/`; add new animations or alter existing ones there.
- **Video assets:** Put `bad_apple.gif` or another supported source file in `video_sources/` or `data/videos/` for automatic conversion during `buildfs`/`uploadfs`; generated assets are written as lowercase `.lfv` files such as `data/videos/bad_apple.lfv`, using a `64x32` per-panel `cover` layout by default so each panel is filled without distorting aspect ratio.
- **Build metadata:** `platformio.ini` sets `FIRMWARE_VERSION`, device model tags, and compiler flags shared across environments.

## BLE Control
- The controller advertises as `LumiFur_Controller` and exposes a GATT service (`01931c44-3867-7740-9867-c822cb7df308`).
- **Face characteristic:** write a view ID (0…`TOTAL_VIEWS`‑1) to switch expressions; read returns the current value.
- **Config characteristic:** four boolean flags toggle auto-brightness, accelerometer features, sleep mode, and aurora overlays.
- **Brightness characteristic:** write 0–255 to set manual brightness or subscribe for updates when auto-brightness adjusts.
- **Temperature & logs:** subscribe for live temperature readings and request buffered history through the command characteristic (0x01 to stream, 0x02 to clear).
- **OTA characteristic:** supports start/data packets for BLE firmware updates with status acknowledgements; pairing is recommended for production use.
- A JSON metadata characteristic exposes firmware version, git commit, build timestamp, and device ID for companion applications.
- The onboard NeoPixel pulses blue while advertising and turns green when a BLE client connects.

## Testing

[![Coverage Status](https://coveralls.io/repos/github/stef1949/LumiFur_Controller/badge.svg?branch=main)](https://coveralls.io/github/stef1949/LumiFur_Controller?branch=main)

- **80+ Unity tests** covering core functionality: run with `uv run pio test -e native2`
- **Code coverage** tracked via [Coveralls](https://coveralls.io/github/stef1949/LumiFur_Controller) — see [docs/COVERAGE.md](docs/COVERAGE.md) for details
- Run the GoogleTest suite with coverage via `uv run pio test -e codeql`
- Pure-Python tests use `pytest` from the `test` dependency group: `uv run --group test pytest` (configured under `[tool.pytest.ini_options]` in `pyproject.toml`)
- Coverage reports and additional tooling scripts are located under `test/`
- Lightweight smoke tests for the Web Bluetooth firmware updater can run without PlatformIO: `uv run --group test python -m unittest discover docs/firmware-updater/tests`

## Web Firmware Updater
- A browser-based OTA helper lives at `docs/firmware-updater/index.html`. Serve the folder over HTTPS or `http://localhost` (for example, `python -m http.server 8000` from the repo root) because Web Bluetooth is blocked on `file://` origins. Open the page in a supported browser (Chrome or Edge), click **Connect** to choose your LumiFur controller, select a compiled `.bin` firmware file, and press **Upload Firmware** to stream it over the OTA characteristic (`01931c44-3867-7427-96ab-8d7ac0ae09ee`).
- Production builds are published automatically to GitHub Pages at [https://stef1949.github.io/LumiFur_Controller/](https://stef1949.github.io/LumiFur_Controller/); the root URL redirects to the `firmware-updater/` UI.
- Keep the page open during transfer; the device will reboot automatically after the update finishes.

## GitHub Copilot Integration
Developer onboarding guides for GitHub Copilot live in `docs/COPILOT_SETUP.md` and `docs/COPILOT_USAGE.md`, with tailored instructions for embedded patterns, animation workflows, and testing expectations.

## Contributing
Contributions are welcome! Please fork the repository, open an issue or discussion when appropriate, and submit a pull request following the guidelines in `CONTRIBUTING.md`.

## License
This project is licensed under the BSD 3-Clause License. See the `LICENSE` file for full text.

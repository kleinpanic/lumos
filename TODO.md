# TODO: Brightness Manager Enhancements

## 1. Hardware & Platform Support
- **Wayland & DRM fallback**: Detect when X11 isn’t available and use libdrm/direct KMS ioctls for pure‑KMS/Wayland sessions
- **DDC/CI support**: Control external monitors over I²C (HDMI/DP) via libddc or i2c‑dev
- **Ambient‑light sensor integration**: Auto‑adjust or offer “auto” mode using IIO illuminance sensors under `/sys/bus/iio/devices/`
- **Keyboard backlight control**: Unify display and keyboard backlight (e.g. `/sys/class/leds/*kbd_backlight*/brightness`)

## 2. User‑Facing Features
- **Profiles & presets**: Save named presets (e.g. “night”, “reading”, “presentation”) and switch between them
- **Smooth fades & transitions**: Ramp brightness over a configurable duration (e.g. `--fade 0.5s`)
- **Time‑based auto‑dim**: Schedule brightness changes based on sunset/sunrise or cron‑style times
- **Battery‑aware adjustments**: Integrate with UPower/DBus to lower brightness on battery or below a threshold
- **Desktop notifications / OSD**: Use libnotify or DE OSD protocol instead of raw X11 window

## 3. CLI & Scripting Enhancements
- **JSON / machine‑readable output** (`--json`): `{ current: ..., max: ..., percent: ... }`
- **Interactive TUI mode**: Ncurses interface with arrow‑key control
- **Watch mode**: `--watch <interval>` to refresh and display live updates in terminal

## 4. Configuration & Persistence
- **Global/system config**: `/etc/brightness.conf` merged with `~/.config/brightness/config`
- **Plugin API**: Load vendor‑specific modules (e.g. Dell WMI, ThinkPad ACPI) as `.so` plugins

## 5. Development & Packaging Best Practices
- **Unit tests & CI**: Fake sysfs for testing, sanitize with AddressSanitizer, lint with clang‑tidy
- **Doxygen / manpage expansion**: Document library functions and CLI flags, add examples
- **Distribution packaging**: Provide `.deb`/`.rpm` specs or Homebrew formula
- **Home‑built installer**: `make package` for tarball or distro‑native package

## 6. Advanced & Niche Ideas
- **Gamma & color temperature control**: Combine brightness with Redshift‑style color shifts
- **Multi‑zone backlight**: Control per‑key/per‑zone keyboard lighting on gaming laptops
- **Ambient‑sound sync**: Drive brightness animations from microphone input



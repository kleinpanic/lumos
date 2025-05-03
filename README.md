# Lumos

**Lumos** is a command‑line utility to query and set the brightness of both internal backlight panels and external monitors supporting DDC/CI. It offers machine‑readable and human‑readable outputs, step adjustments, direct percentage or raw value setting, on‑screen popups, and persistent device selection.

---

## Features

- **Internal backlight control** via `/sys/class/backlight/*`
- **External DDC/CI control** over `/dev/i2c-*` or `/dev/drm_dp_aux*`
- **Machine‑readable** (`-m`) and **human‑readable** (`-r`) output
- **Step adjust** brightness up/down by 10% (`-a up|down`)
- **Set** brightness by percent or raw value (`-s VALUE`, `-R`)
- **On‑screen popup** via X11 (suppressed with `-n`)
- **Persist default backlight interface** (`-P`)
- **Profiles & presets** skeleton support (`--profile-new`, `--profile-load`)
- **Udev rule installation** (`-U, --setup`) to allow non-root use

---

## Installation

### Prerequisites

Make sure you have these packages installed:

- **GNU C toolchain**: `gcc`, `make`
- **pkg-config**  
- **X11 development headers**: `libx11-dev`
- **I²C‑dev development headers**: `libi2c-dev`
- **Linux kernel headers** (for `/dev/i2c-*` support)
- *(optional)* **DRM development headers**: `libdrm-dev` (for `/dev/drm_dp_aux*` nodes)
- **Build essentials**: `build-essential`

On Debian/Ubuntu:
```bash
sudo apt update
sudo apt install build-essential pkg-config libx11-dev libi2c-dev linux-headers-$(uname -r) libdrm-dev
```

### Build & Install

```bash
git clone https://github.com/yourusername/lumos.git
cd lumos
make
sudo make install
```

This will install:

- BINARY → `/usr/local/bin/lumos`
- HEADER  → `/usr/local/include/brightness.h /usr/local/include/udev_setup.h`
    - This is a comment in the makefile. It no longer does this.
    - This is for future API support through other programs. 
- MANPAGE → `/usr/local/share/man/man1/lumos.1`

---

## Usage

```console
$ lumos --help
Usage: lumos [options]
  -m, --machine-readable       print raw brightness value
  -r, --human-readable        print raw & percent nicely
  -a, --adjust <up|down>      step adjust by 10%
  -s, --set <value>           set brightness (percent)
  -R, --raw                   treat -s value as raw
  -n, --nodisplay             suppress X11 popup
  -d, --device <iface>        internal backlight interface
  -P, --persist-device        save internal interface to config
  -x, --external <connector>  DDC/CI external monitor (e.g. HDMI-1)
  --profile-new <name>        create profile (skeleton)
  --profile-load <name>       load profile (skeleton)
  -h, --help                  this help
```

### Examples

- **Show internal brightness**  
  ```bash
  lumos -r
  # Internal backlight: raw=450, max=1000, percent=45%
  ```

- **Step external HDMI‑1 down**  
  ```bash
  lumos -x HDMI-1 -a down
  ```

- **Set external monitor to 70%**  
  ```bash
  lumos -x HDMI-1 -s 70
  ```

- **Save preferred internal interface**  
  ```bash
  lumos -d intel_backlight -P
  ```

---

## Configuration

Lumos stores your chosen internal backlight interface in:

```
~/.config/lumos/config
```

---

## Development

1. Clone the repo  
2. Edit `src/` and `include/` as needed  
3. `make && sudo make install`  
4. Submit pull requests or open issues!

---

## License

This project is released under the **MIT License**. See [LICENSE](LICENSE) for details.  

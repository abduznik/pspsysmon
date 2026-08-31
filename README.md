# PSPSysMon

A lightweight PSP PRX plugin that renders a real-time **system monitor overlay** on top of any game or the XMB.

## Features

- **FPS counter** — current + rolling average
- **CPU / Bus clock** — live MHz readout
- **RAM** — free memory in KB
- **Battery** — percent, charging state, voltage, low-battery warning
- **Uptime** — system uptime HH:MM:SS
- Semi-transparent overlay, alpha-blended directly into the framebuffer
- Built-in 5x7 bitmap font — zero external assets
- Persistent config saved to `ms0:/PSP/SYSTEM/pspsysmon.cfg`

## Controls

| Combo | Action |
|---|---|
| **L + R + START** | Toggle overlay on/off |

## Installation

1. Copy `pspsysmon.prx` to `ms0:/seplugins/`
2. Add `ms0:/seplugins/pspsysmon.prx 1` to:
   - `ms0:/seplugins/game.txt` (in-game overlay)
   - `ms0:/seplugins/vsh.txt` (XMB overlay)
3. Restart your PSP (or toggle plugins in recovery menu)

## Building

Requires the [pspdev](https://github.com/pspdev/pspdev) toolchain. Easiest path is Docker:

```bash
docker run --rm -v "$PWD:/src" -w "/src" pspdev/pspdev make release
```

Output: `pspsysmon.prx` in the repo root, zip package under `build/`.

## CI/CD

GitHub Actions (`.github/workflows/build.yml`):
- Every push/PR → Docker build (compile gate)
- Tag `v*` or manual dispatch → build + zip + GitHub Release

```bash
# Release v0.10
git tag v0.10 && git push origin v0.10
# or
gh workflow run build.yml -f tag=v0.10
```

## Technical notes

- Kernel module (`PSP_MODULE_KERNEL`) — required for `scePower*` and framebuffer access
- Renders via `sceDisplayGetFrameBuf()` with manual alpha blending
- No libc in hot paths; `strtoul`-style discipline per PSP PRX best practice
- GCC 14 compatible (warnings suppressed; modern pspdev SDK defines `SceLoadCoreExecFileInfo` natively)

## License

MIT

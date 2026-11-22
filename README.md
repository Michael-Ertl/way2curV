# way2curV

Animated space-filling curves for your Wayland background. Requires a compositor
supporting `wlr-layer-shell`, such as Hyprland or niri.

## Demo

[![Animated space-filling curve demo](doc/circle.jpg)](doc/circle.mp4)

[![Disk curve drawn over an image wallpaper via `image_path`](doc/owl.jpg)](doc/owl_demo.mp4)

[![Peano curve with the `polished` animation](doc/peano_demo.jpg)](doc/peano_demo.mp4)

## Install

Dependencies: a C/C++ compiler, CMake 3.20+, Make or Ninja, pkg-config, Wayland,
wayland-protocols, wlr-protocols, and wayland-scanner.

```bash
cmake -S . -B build-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build-release --parallel
cmake --install build-release
```

## Configuration

The config file lives at `~/.config/way2curV/config.conf`. On the first run it is
created automatically by copying the default config that ships with the install,
so you only ever edit that copy.

The file is INI-style: each `[NAME]` section configures one output, where `NAME`
is the monitor name reported by your compositor (e.g. `DP-3`, `eDP-1` — check
with `hyprctl monitors` or `niri msg outputs`). Outputs without a matching
section fall back to `[default]`. Lines starting with `#` are comments and blank
lines are ignored. A section name that appears twice is ignored after the first
occurrence, and an unknown key or invalid value makes the program print an error
and exit at startup.

| Key | Values | Meaning |
| --- | --- | --- |
| `type` | `hilbert`, `moore`, `peano`, `zorder` (alias `z-order`), `disk` (alias `circle`); case-insensitive | Which space-filling curve to draw |
| `depth` | non-negative integer | Recursion depth of the curve |
| `left_margin`, `right_margin`, `top_margin`, `bottom_margin` | pixels | Margins around the drawing area |
| `line_width` | pixels | Thickness of the curve line |
| `background_color` | `0xRRGGBB` hex | Background color (used when `image_path` is empty) |
| `line_color` | `0xRRGGBB` hex | Color of the curve |
| `curve_on` | `true`/`false` (also `1`/`0`, `yes`/`no`, `on`/`off`) | Whether the static curve is drawn; the animation still runs |
| `animation_type` | `none`, `polished`| Moving highlight that travels along the curve |
| `animation_length` | curve points | Length of the animated segment |
| `animation_size` | pixels | Size of the animated element |
| `animation_color` | `0xRRGGBB` hex | Color of the animated element |
| `step_size` | curve points per tick | How far the animation advances each tick |
| `time_step` | milliseconds | Delay between animation ticks |
| `image_path` | file path | Background image drawn instead of a solid color; empty means solid `background_color` |

Example:

```ini
[eDP-1]
type=peano
depth=3
left_margin=0
right_margin=0
top_margin=0
bottom_margin=30
line_width=5
background_color=0x000000
line_color=0x00FF00
curve_on=1
animation_type=pacman
animation_length=3000
animation_size=35
animation_color=0xFFFF00
step_size=1
time_step=5
# empty image_path = solid background_color
image_path=/home/user/Pictures/wallpaper.jpg
```

## Run

```bash
~/.local/bin/way2curV
```

For niri autostart:
```
spawn-at-startup  "way2curV"
```

For Hyprland autostart:

```ini
exec-once = way2curV
```

MIT license — see [LICENSE](LICENSE).


## Wayland internals

Startup (`src/main.cpp`): load `config.conf`, `wl_display_connect`, grab the
`wl_registry`, attach the `registry_listener`, do one roundtrip, then run
`wl_display_dispatch` in a loop.

1. **Registry** (`src/wayland/listener/registry_listener.cpp`): binds
   `wl_compositor` (v4), `wl_shm` (v1) and `zwlr_layer_shell_v1` (v1). Every
   advertised `wl_output` is bound (up to v4) and immediately gets an
   `output_listener`, before its name event arrives. Because compositors may
   advertise globals in any order, a `DisplayState` per output is only created
   once compositor, shm and layer shell are all present — each new global
   re-triggers the check, and it is idempotent. Each `DisplayState` gets a
   `wl_surface` plus a layer surface on the `background` layer, anchored to
   all four edges with size 0×0 (fill the output) and exclusive zone −1 (so
   other panels can't shrink it), then commits. `global_remove` tears all of
   it down when an output is unplugged.
2. **Output name** (`src/wayland/listener/output_listener.cpp`): the `name`
   event (wl_output v4+) stores the monitor name and applies the matching
   `[section]` from the config — an exact monitor-name match wins, `[default]`
   is the fallback.
3. **Layer-surface configure** (`src/wayland/listener/layershell_listener.cpp`):
   `configure` is acked; on a new or changed size a double-buffered shm
   `BufferPair` is (re)created, `curve.width/height` are set, the cached curve
   points and image cache are cleared, and the first frame is rendered. Every
   configure ends by calling `request_frame()`.
4. **Frame loop** (`src/wayland/listener/frame_and_buffer_listeners.cpp`):
   `request_frame` attaches the active buffer, damages the whole surface,
   registers a `wl_callback` frame request and commits. When the compositor
   fires `done`, `handle_frame` advances the animation (`update_curve` renders
   into the back buffer only if `time_step` has elapsed and that buffer has
   been released) and calls `request_frame` again — so `handle_configure`
   kicks off a self-sustaining chain of frame callbacks that drives the
   animation. `buffer.release` only marks a buffer writable again; it never
   triggers a draw.
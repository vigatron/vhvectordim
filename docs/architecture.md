# Architecture

## Purpose

`vhappvectordim` accepts a path to a binary file, verifies that every byte is
either `0` or `1`, arranges the array into an `L × W × H` cuboid (all sides
strictly greater than 1) and displays it in an isometric 3D scene. Cubes with
value `0` are drawn in `color0` (by default almost black, 10%), cubes with
value `1` in `color1` (by default almost white, 90%). The scene background is
set by `colorb` (white by default); the transparency of each cube type is
configurable independently.

## Data flow

```
argv[1]
  │
  ▼
FileValidator::load()        file loading, per-byte validation
  │  std::vector<quint8>
  ▼
CubeLayout::variants()       all L × W × H layouts, the best one first
  │  vector<Dimensions>
  ▼
CubeVisualizer               window, control panel, menus, status bar
  │  data + selected layout + colors + transparency
  ▼
CubeView                     OpenGL scene, camera, mouse interaction
```

The data is read exactly once and passed along the chain. The visualizer never
re-reads the file.

## Modules

| File | Responsibility |
|---|---|
| `src/main.cpp` | argument parsing, `QSurfaceFormat` setup, window startup |
| `src/FileValidator.*` | file reading and per-byte validation |
| `src/CubeLayout.*` | all factorizations of `N` into `L × W × H` with all sides `> 1` |
| `src/gui/CubeView.*` | OpenGL 3.3 Core, cube instancing, colors, transparency, camera, mouse |
| `src/gui/CubeVisualizer.*` | window, menus, control panel, dialogs, status bar |

## Build

Requirements and build commands are documented in [build.md](build.md).

## Running

```bash
./build/vhappvectordim data/test.bin
./build/vhappvectordim data/Qwen3.5-0.8B-bf16.token_embd.weight_nr0s.bin
```

Without an argument the program prints an error and exits with code `1`.
An invalid file or an impossible layout also produces an error message and
return code `1`.

## Quick-run scripts

The scripts live in `scripts/`:

| Script | Purpose |
|---|---|
| `scripts/build.sh` | configure and build the project |
| `scripts/run_example.sh <file>` | run any sample; builds the project first if needed |
| `scripts/run_test.sh` | run `data/test.bin` |
| `scripts/run_qwen.sh` | run the Qwen sample from `data/` |

Examples:

```bash
./scripts/run_test.sh
./scripts/run_qwen.sh
./scripts/run_example.sh data/test.bin
```

If the binary is missing or older than the sources, `run_example.sh` performs
the build automatically.

## Scene controls

| Action | Result |
|---|---|
| Left mouse button + drag | rotate the camera |
| Mouse wheel | zoom in/out |
| `R` or double click | reset the camera to the isometric view |
| `Ctrl+O` | open another file |
| `Ctrl+Q` | quit |

Opening an invalid file shows an error dialog and leaves the current scene
unchanged.

## Layout

`CubeLayout::variants()` returns every triple `L × W × H = N` with all sides
`> 1`, sorted from the most compact one. The first triple is selected by
default; the rest are listed in the right-hand panel. If no variant exists
(for example, `N < 8` or `N` is prime), the program terminates with an error.

## Visualization

`CubeView` uses OpenGL 3.3 Core. A single cube is defined by 36 vertices
(6 faces), while the position and color of each cube are supplied through an
instance buffer. The camera is orthographic with an isometric initial
direction.

Cubes occupy 80% of a cell, so gaps remain visible between them. The defaults
for `color0` and `color1` are almost black (10%) and almost white (90%); both
are changeable from the control panel. The scene background is set by `colorb`
(white by default, `255/255/255`). The transparency of `0` and `1` cubes is
controlled by separate sliders, 50% by default (alpha `0.5`), with a 0–100%
range. Blending is enabled, depth writes are disabled, and instances are
sorted from back to front.

## Control panel

The window is split horizontally: the 3D scene occupies 80% of the width and
the right-hand panel 20%. The panel contains, from top to bottom:

- `wdgVariants` — the list of all layouts formatted as `8x8x16`; selecting a row
  calls `CubeView::setLayout()` and refreshes the scene and the status bar;
- `wdgColor` — three separate color selectors (standard `QColorDialog`) for
  `color0`, `color1` and `colorb` (the 3D scene background);
- `wdgTrans` — two separate 0–100% transparency sliders for `color0` and
  `color1`, both 50% at startup;
- `wdgMisc` — the `Save Screen` button that saves the current 3D scene frame
  as JPEG.

## Limitations

- The file is read into memory in full.
- The array size is limited to `INT_MAX` because of integer dimension types and
  the instance buffer.
- All cubes, including interior ones, are submitted to the GPU; for the current
  samples (8 and 1024 bytes) this is negligible.

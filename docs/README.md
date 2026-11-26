# `vhappvectordim` Documentation

The program loads a binary file whose every byte is either `0` or `1`, arranges
the array into a rectangular cuboid and visualizes it in an isometric 3D scene
rendered with Qt6/OpenGL.

## Quick start

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/vhappvectordim data/test.bin
```

Or use the helper scripts:

```bash
./scripts/run_test.sh
./scripts/run_qwen.sh
./scripts/run_example.sh data/test.bin
```

## Controls

| Action | Result |
|---|---|
| Left mouse button + drag | rotate the camera |
| Mouse wheel | zoom in/out |
| `R` or double click | reset the camera to the isometric view |
| `Ctrl+O` | open another file |
| `Ctrl+Q` | quit |

The right-hand panel lists all layout variants, three color selectors
(`color0`, `color1`, `colorb`), two transparency sliders and the `Save Screen`
button.

## Contents

- [Architecture](architecture.md) — data flow, modules, control panel.
- [Build](build.md) — requirements, CMake and scripts.
- [FileValidator](FileValidator.md) — file loading and per-byte validation.
- [CubeLayout](CubeLayout.md) — factorization into `L × W × H`.
- [CubeView](CubeView.md) — OpenGL 3D scene, camera, transparency.
- [CubeVisualizer](CubeVisualizer.md) — main window and control panel.
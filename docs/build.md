# Build

## Requirements

- CMake 3.16 or newer;
- a compiler with C++17 support;
- Qt 6.4+ with the `Widgets`, `OpenGLWidgets` and `OpenGL` components.

## Building with CMake

From the project root directory:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The executable is produced at `build/vhappvectordim`.

## Quick build

```bash
./scripts/build.sh
```

The script uses the `build` directory by default. The directory can be
overridden with the `BUILD_DIR` environment variable.

## Running

```bash
./build/vhappvectordim data/test.bin
./build/vhappvectordim data/Qwen3.5-0.8B-bf16.token_embd.weight_nr0s.bin
```

Or through the scripts:

```bash
./scripts/run_test.sh
./scripts/run_qwen.sh
./scripts/run_example.sh data/test.bin
```

`scripts/run_example.sh` builds the project first if necessary.

# Raster

## Quick start

> [!WARNING]
> Only tested on linux (ubuntu 20.04) with clang 10, but later releases should be compatible.

### Build the tools

```bash
cd tools
make
make install

```

Tools are installed to `~/.local/bin` by default, but you can specify your own path:
```bash
make install INSTALL_PATH=/some/other/path

```

### WASM and native build

```bash
./compile_assets.sh
./build.sh
```

If all went well, you should have raster and raster.wasm.

```bash
./serve.sh # web at http://localhost:5050
./raster   # native

```

![alt text](https://github.com/azinum/raster/blob/master/images/rendering.png?raw=true)

## Controls

| Key                      | Description                                                                      |
| ------------------------ | -------------------------------------------------------------------------------- |
| WASD                     | Move around (tank controls)                                                      |
| Z                        | Move up                                                                          |
| X                        | Move down                                                                        |
| Q                        | Look up                                                                          |
| E                        | Look down                                                                        |
| R                        | Reset scene                                                                      |
| T                        | Toggle texture mapping                                                           |
| Arrow keys               | Move light                                                                       |
| Spacebar                 | Toggle play/pause                                                                |
| N                        | Decrease time scale                                                              |
| M                        | Increase time scale                                                              |
| K                        | Move light down                                                                  |
| J                        | Move light up                                                                    |
| 1                        | Decrease light strength                                                          |
| 2                        | Increase light strength                                                          |
| 3                        | Decrease light radius                                                            |
| 4                        | Increase light radius                                                            |
| 5                        | Toggle GI debug view (if available, `VOXELGI` is defined)                        |
| 6                        | Toggle dithering                                                                 |
| 7                        | Toggle fog                                                                       |
| 8                        | Toggle depth test                                                                |
| 9                        | Render depth buffer                                                              |
| 0                        | Render normal buffer (if available, only if `NO_NORMAL_BUFFER` is not defined)   |

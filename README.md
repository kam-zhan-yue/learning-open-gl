# learning-open-gl

## Setup (MacOS)

```shell
brew install cmake glfw glew
```

## Setup (Arch Linux)

```shell
sudo pacman cmake xorg-dev libgl1-mesa-dev
```

## Neovim Setup

To fully setup with neovim, I needed to do the following

- Add glad header files
- Add a CMakeLists.txt that adds glad as a library and builds with glfw
- Ensure that the CMakeLists.txt has the following command to output a json file for neovim

```shell
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

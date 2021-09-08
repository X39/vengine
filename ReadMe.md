![GitHub pull requests](https://img.shields.io/github/issues-pr/x39/vengine)
![GitHub pull requests](https://img.shields.io/github/issues/x39/vengine)
![GitHub pull requests](https://img.shields.io/github/commit-activity/m/x39/vengine)

# Dependency & Sources Installing
## Windows
1. Install `vcpkg`
   ```
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.sh
   ./vcpkg integrate install
   ```
2. Install dependencies using `vcpkg`
   1. `vcpkg install EnTT` (add `--triplet x64-windows` if you plan on compiling for x64)
   2. `vcpkg install glm` (add `--triplet x64-windows` if you plan on compiling for x64)
   3. `vcpkg install glfw3` (add `--triplet x64-windows` if you plan on compiling for x64)
3. [Install Vulkan](https://vulkan.lunarg.com/sdk/home#windows)
4. [Install CMake](https://cmake.org/download/#latest)
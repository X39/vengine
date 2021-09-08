![GitHub pull requests](https://img.shields.io/github/issues-pr/x39/vengine)
![GitHub pull requests](https://img.shields.io/github/issues/x39/vengine)
![GitHub pull requests](https://img.shields.io/github/commit-activity/m/x39/vengine)

# Dependencies installation
## Windows
1. Clone the repository using `git clone https://github.com/X39/vengine.git --recursive`
2. Install `vcpkg`
   ```
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.sh
   ./vcpkg integrate install
   ```
3. Install dependencies using `vcpkg`
   1. `vcpkg install EnTT` (add `--triplet x64-windows` if you plan on compiling for x64)
   2. `vcpkg install glm` (add `--triplet x64-windows` if you plan on compiling for x64)
   3. `vcpkg install glfw3` (add `--triplet x64-windows` if you plan on compiling for x64)
4. [Install Vulkan](https://vulkan.lunarg.com/sdk/home#windows)
5. [Install CMake](https://cmake.org/download/#latest)
6. Setup your IDE
   1. **CLion** _(Version 2021.2.1)_
      1. Open the Project Folder (where the `CMakeLists.txt` is located)
      2. In the Menu bar, click `File` > `Settings...` > *Dialog opens* > `Build, Execution, Deployment` > `CMake`
      3. In the `CMake options` field, add `-DCMAKE_TOOLCHAIN_FILE=VCPKGPATH/scripts/buildsystems/vcpkg.cmake`
         where `VCPKGPATH` is the path to your `vcpkg` installation 
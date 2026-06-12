cmake_clean() {
  rm -rf build
  # rm -rf dist
}

cmake_prepare() {
  # Note: $@ forwards argument so you can add additional arguments
  # e.g: cmake_prepare -DCMAKE_BUILD_TYPE=Release to configure the build solution in release mode with gcc
  cmake -S . -B build -DCMAKE_INSTALL_PREFIX=./dist $@
}

cmake_build() {
  # Note: $@ forwards argument so you can add additional arguments
  # e.g: cmake_build --config release to build in release mode with Visual Studio Compiler
  cmake --build build -j $@
}

cmake_install() {
  # Note: $@ forwards argument so you can add additional arguments
  # e.g: cmake_install --config release to build in release mode, then install, with Visual Studio Compiler
  cmake --build build -j --target install $@
}

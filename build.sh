rm -rf build

# Créer build
mkdir build && cd build

# Installer avec Conan (autorise l'installation système)
conan install .. \
    --output-folder=. \
    --build=missing \
    -c tools.system.package_manager:mode=install \
    -c tools.system.package_manager:sudo=True

# Configurer CMake
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release

# Compiler
cmake --build . -j$(nproc)
#!/bin/bash
set -e

# Clone vcpkg if not already present
if [ ! -d "./vcpkg" ]; then
    echo "Cloning vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git
else
    echo "vcpkg directory already exists."
fi

# Bootstrap vcpkg
cd vcpkg
if [ ! -f "./vcpkg" ]; then
    echo "Bootstrapping vcpkg..."
    ./bootstrap-vcpkg.sh
else
    echo "vcpkg is already bootstrapped."
fi

echo "vcpkg installation complete."
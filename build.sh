#!/bin/bash


# this is a helper script for local wasm builds.
# we assume that script is run in the 
# env defined in environment-wasm-build.yml

set -e

# flip this to true to create the environments
if false; then
    micromamba create -f environment-wasm-host.yml --platform=emscripten-wasm32 --yes

else
    echo "Skipping environment creation"
fi

mkdir -p build
pushd build 

export PREFIX=$MAMBA_ROOT_PREFIX/envs/xeus-sqlite-wasm-host
export CMAKE_PREFIX_PATH=$PREFIX
export CMAKE_SYSTEM_PREFIX_PATH=$PREFIX

emcmake cmake \
-DCMAKE_BUILD_TYPE=Release                        \
-DCMAKE_PREFIX_PATH=$PREFIX                       \
-DCMAKE_INSTALL_PREFIX=$PREFIX                    \
-DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ON            \
-Dtabulate_DIR=$PREFIX/lib/cmake/tabulate         \
-DXSQL_BUILD_XSQLITE_EXECUTABLE=OFF               \
-DXSQL_BUILD_SHARED=OFF                           \
-DXSQL_BUILD_STATIC=ON                            \
-DXSQL_USE_SHARED_XEUS=ON                         \
-DXSQL_USE_SHARED_XEUS_SQLITE=OFF                 \
-DXVEGA_STATIC_LIBRARY=$PREFIX/lib/libxvega.a     \
-DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ON            \
..

make -j2 install
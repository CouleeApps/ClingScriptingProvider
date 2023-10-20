ClingScriptingProvider!
--

What do you do when V35 forget to write Python APIs for their new features? Well, why not just use a C++ scripting console instead!

## Fancy Features
* A nearly impossible-to-replicate complication process
* Crashing on most function calls
* Cling library maintained by ... CERN??
* Supports templates!

## Installation
Don't.

## Installation (for those who can't read)
So it turns out you need a patch to Cling for the stdout/stderr redirection, also you have to build shared. 

Something like this (recovered from xonsh history):

    mkdir cling
    cd cling

    git clone -b cling-llvm13 https://github.com/root-project/llvm-project.git src
    git clone https://github.com/root-project/cling.git

    cd cling
    git am '../../ClingScriptingProvider/patches/cling/cling-5829172-Allow setting cling__outs and cling__errs.patch'
    cd ..

    mkdir build
    cd build

    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_BUILD_TOOLS=Off -DLLVM_EXTERNAL_PROJECTS=cling -DLLVM_EXTERNAL_CLING_SOURCE_DIR=../cling -DLLVM_ENABLE_PROJECTS=clang "-DLLVM_TARGETS_TO_BUILD=host;NVPTX" -DCMAKE_INSTALL_PREFIX=../install -DBUILD_SHARED_LIBS=On ../src/llvm

    # This actually fails to build the demo
    # Just run it a bunch until everything else compiles 
    cmake --build .
    cmake --install .

Ok cool now you have cling installed somewhere. Time to point this cursed thing at it:

    git submodule update --init --recursive
    cmake -B build -G Ninja "-DCLING_PATH=../cling/install" "-DBN_INSTALL_DIR=/Applications/Binary Ninja.app/Contents/MacOS"
    cmake --build build
    cmake --install build
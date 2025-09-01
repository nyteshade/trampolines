#!/usr/bin/env sh

if [[ $1 = "clean" ]]; then
  rm -rf build/

  printf "Build artifacts removed!\n"
  
  exit 0
fi

printf "Compiling PowerPC (32-bit)..."
gcc -arch ppc -O2 -c trampoline_ppc.c -o trampoline_ppc.o
gcc -arch ppc -O2 -c trampoline_helpers.c -o trampoline_helpers.ppc.o

if [[ $? -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "Compiling PowerPC (64-bit)..."
gcc -arch ppc64 -O2 -mmacosx-version-min=10.5 \
  -c trampoline_ppc64.c \
  -o trampoline_ppc64.o
  
gcc -arch ppc64 -O2 -mmacosx-version-min=10.5 \
  -c trampoline_helpers.c \
  -o trampoline_helpers.ppc64.o


if [[ $? -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "Compiling Intel (32-bit)..."
gcc -m32 -arch i386 -O2 -mdynamic-no-pic -Wall -Wextra \
  -c trampoline_x86.c \
  -o trampoline_x86.o
  
gcc -m32 -arch i386 -O2 -mdynamic-no-pic -Wall -Wextra \
  -c trampoline_helpers.c \
  -o trampoline_helpers.x86.o

if [[ $? -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "Compiling Intel (64-bit)..."
gcc -m64 -arch x86_64 -O2 -mdynamic-no-pic -Wall -Wextra \
  -c trampoline_x86_64.c \
  -o trampoline_x86_64.o
  
gcc -m64 -arch x86_64 -O2 -mdynamic-no-pic -Wall -Wextra \
  -c trampoline_helpers.c \
  -o trampoline_helpers.x86_64.o


if [[ $? -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "Building ar libs..."
result=0
ar rcs libtrampoline.ppc.a trampoline_ppc.o trampoline_helpers.ppc.o
result=$(( $result + $? ))

ranlib libtrampoline.ppc.a
result=$(( $result + $? ))

ar rcs libtrampoline.ppc64.a trampoline_ppc64.o  trampoline_helpers.ppc64.o
result=$(( $result + $? ))

ranlib libtrampoline.ppc64.a
result=$(( $result + $? ))

ar rcs libtrampoline.x86.a trampoline_x86.o trampoline_helpers.x86.o
result=$(( $result + $? ))

ranlib libtrampoline.x86.a
result=$(( $result + $? ))

ar rcs libtrampoline.x86_64.a trampoline_x86_64.o trampoline_helpers.x86_64.o
result=$(( $result + $? ))

ranlib libtrampoline.x86_64.a
result=$(( $result + $? ))

if [[ $result -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "Building universal binary static libtrampoline..."
lipo -create -output libtrampoline.a \
  libtrampoline.ppc.a \
  libtrampoline.ppc64.a \
  libtrampoline.x86.a \
  libtrampoline.x86_64.a

if [[ $? -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "\x1b[1m"
lipo -info libtrampoline.a
printf "\x1b[22m\n"

# Let's make some dylibs

printf "Creating PowerPC (32-bit) dynamic library..."
result=0
gcc -arch ppc -O2 -fPIC -c trampoline_ppc.c -o trampoline_ppc_dylib.embedded.o
result=$(( $result + $? ))

gcc -arch ppc -O2 -fPIC -c trampoline_helpers.c -o trampoline_helpers_ppc_dylib.embedded.o
result=$(( $result + $? ))

gcc -arch ppc -dynamiclib \
  -install_name @executable_path/../lib/libtrampoline.dylib \
  -compatibility_version 1.0.0 \
  -current_version 1.0.0 \
  -o libtrampoline.embedded.ppc.dylib \
  trampoline_ppc_dylib.embedded.o \
  trampoline_helpers_ppc_dylib.embedded.o
result=$(( $result + $? ))

if [[ $result -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "Creating PowerPC (64-bit) dynamic library..."
result=0
gcc -arch ppc64 -O2 -fPIC \
  -c trampoline_ppc64.c \
  -o trampoline_ppc64_dylib.embedded.o
result=$(( $result + $? ))

gcc -arch ppc64 -O2 -fPIC \
  -c trampoline_helpers.c \
  -o trampoline_helpers_ppc64_dylib.embedded.o
result=$(( $result + $? ))

gcc -arch ppc64 -dynamiclib \
  -install_name @executable_path/../lib/libtrampoline.dylib \
  -compatibility_version 1.0.0 \
  -current_version 1.0.0 \
  -o libtrampoline.embedded.ppc64.dylib \
  trampoline_ppc64_dylib.embedded.o \
  trampoline_helpers_ppc64_dylib.embedded.o
result=$(( $result + $? ))

if [[ $result -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi
  
printf "Creating Intel (32-bit) dynamic library..."
result=0
gcc -arch i386 -O2 -fPIC \
  -mmacosx-version-min=10.4 \
  -c trampoline_x86.c \
  -o trampoline_x86_dylib.embedded.o
result=$(( $result + $? ))

gcc -arch i386 -O2 -fPIC \
  -mmacosx-version-min=10.4 \
  -c trampoline_helpers.c \
  -o trampoline_helpers_x86_dylib.embedded.o
result=$(( $result + $? ))
  
gcc -arch i386 -dynamiclib \
  -install_name @executable_path/../lib/libtrampoline.dylib \
  -compatibility_version 1.0.0 \
  -current_version 1.0.0 \
  -o libtrampoline.embedded.x86.dylib \
  trampoline_x86_dylib.embedded.o \
  trampoline_helpers_x86_dylib.embedded.o
result=$(( $result + $? ))

if [[ $result -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "Creating Intel (64-bit) dynamic library..."
result=0
gcc -arch x86_64 -O2 -fPIC \
  -mmacosx-version-min=10.5 \
  -c trampoline_x86_64.c \
  -o trampoline_x86_64_dylib.embedded.o
result=$(( $result + $? ))

gcc -arch x86_64 -O2 -fPIC \
  -mmacosx-version-min=10.5 \
  -c trampoline_helpers.c \
  -o trampoline_helpers_x86_64_dylib.embedded.o
result=$(( $result + $? ))
  
gcc -arch x86_64 -dynamiclib \
  -install_name @executable_path/../lib/libtrampoline.dylib \
  -compatibility_version 1.0.0 \
  -current_version 1.0.0 \
  -o libtrampoline.embedded.x86_64.dylib \
  trampoline_x86_64_dylib.embedded.o \
  trampoline_helpers_x86_64_dylib.embedded.o
result=$(( $result + $? ))

if [[ $result -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

mkdir -p build/embedded

printf "Building universal binary dynamic libtrampoline..."
lipo -create -output build/embedded/libtrampoline.dylib \
  libtrampoline.embedded.ppc.dylib \
  libtrampoline.embedded.ppc64.dylib \
  libtrampoline.embedded.x86.dylib \
  libtrampoline.embedded.x86_64.dylib
  
if [[ $? -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "\x1b[1m"
lipo -info build/embedded/libtrampoline.dylib
printf "\x1b[22m\n"

# -------------- global dylib -------------------

printf "Creating PowerPC (32-bit) dynamic library..."
result=0
gcc -arch ppc -O2 -fPIC -c trampoline_ppc.c -o trampoline_ppc_dylib.global.o
result=$(( $result + $? ))

gcc -arch ppc -O2 -fPIC -c trampoline_helpers.c -o trampoline_helpers_ppc_dylib.global.o
result=$(( $result + $? ))

gcc -arch ppc -dynamiclib \
  -install_name /usr/lib/libtrampoline.dylib \
  -compatibility_version 1.0.0 \
  -current_version 1.0.0 \
  -o libtrampoline.global.ppc.dylib \
  trampoline_ppc_dylib.global.o \
  trampoline_helpers_ppc_dylib.global.o
result=$(( $result + $? ))

if [[ $result -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "Creating PowerPC (64-bit) dynamic library..."
result=0
gcc -arch ppc64 -O2 -fPIC \
  -c trampoline_ppc64.c \
  -o trampoline_ppc64_dylib.global.o
result=$(( $result + $? ))

gcc -arch ppc64 -O2 -fPIC \
  -c trampoline_helpers.c \
  -o trampoline_helpers_ppc64_dylib.global.o
result=$(( $result + $? ))
  
gcc -arch ppc64 -dynamiclib \
  -install_name /usr/lib/libtrampoline.dylib \
  -compatibility_version 1.0.0 \
  -current_version 1.0.0 \
  -o libtrampoline.global.ppc64.dylib \
  trampoline_ppc64_dylib.global.o \
  trampoline_helpers_ppc64_dylib.global.o
result=$(( $result + $? ))

if [[ $result -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi
  
printf "Creating Intel (32-bit) dynamic library..."
result=0
gcc -arch i386 -O2 -fPIC \
  -mmacosx-version-min=10.4 \
  -c trampoline_x86.c \
  -o trampoline_x86_dylib.global.o
result=$(( $result + $? ))

gcc -arch i386 -O2 -fPIC \
  -mmacosx-version-min=10.4 \
  -c trampoline_helpers.c \
  -o trampoline_helpers_x86_dylib.global.o
result=$(( $result + $? ))
  
gcc -arch i386 -dynamiclib \
  -install_name /usr/lib/libtrampoline.dylib \
  -compatibility_version 1.0.0 \
  -current_version 1.0.0 \
  -o libtrampoline.global.x86.dylib \
  trampoline_x86_dylib.global.o \
  trampoline_helpers_x86_dylib.global.o
result=$(( $result + $? ))

if [[ $result -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "Creating Intel (64-bit) dynamic library..."
result=0
gcc -arch x86_64 -O2 -fPIC \
  -mmacosx-version-min=10.5 \
  -c trampoline_x86_64.c \
  -o trampoline_x86_64_dylib.global.o
result=$(( $result + $? ))

gcc -arch x86_64 -O2 -fPIC \
  -mmacosx-version-min=10.5 \
  -c trampoline_helpers.c \
  -o trampoline_helpers_x86_64_dylib.global.o
result=$(( $result + $? ))
  
gcc -arch x86_64 -dynamiclib \
  -install_name /usr/lib/libtrampoline.dylib \
  -compatibility_version 1.0.0 \
  -current_version 1.0.0 \
  -o libtrampoline.global.x86_64.dylib \
  trampoline_x86_64_dylib.global.o \
  trampoline_helpers_x86_64_dylib.global.o
result=$(( $result + $? ))

if [[ $result -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

mkdir -p build/global

printf "Building universal binary dynamic libtrampoline..."
lipo -create -output build/global/libtrampoline.dylib \
  libtrampoline.global.ppc.dylib \
  libtrampoline.global.ppc64.dylib \
  libtrampoline.global.x86.dylib \
  libtrampoline.global.x86_64.dylib
  
if [[ $? -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "\x1b[1m"
lipo -info build/global/libtrampoline.dylib
printf "\x1b[22m\n"

mkdir -p build/static
mv libtrampoline.*.a libtrampoline.a build/static
mv *.embedded*.dylib build/embedded
mv *.global*.dylib build/global
rm *.o

printf "\n\x1b[1mGlobal dynamic libraries\x1b[22m (./build/global)\n"
ls build/global

printf "\n\x1b[1mEmbedded dynamic libraries\x1b[22m (./build/embedded)\n"
ls build/embedded

printf "\n\x1b[1mStatic libraries\x1b[22m (./build/static)\n"
ls build/static

printf "\n"

# --- Install?

if [[ $1 = "install" ]]; then
  printf "Installing to /usr/lib and /usr/include, you may need to enter your password\n"
  sudo cp build/global/*.dylib /usr/lib
  sudo cp build/static/*.a /usr/lib
  sudo cp *.h /usr/include
  printf "\ndone!\n"
fi
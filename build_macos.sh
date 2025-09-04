#!/usr/bin/env sh

if [[ ! -f trampoline.c ]]; then
  printf "Please execute this from the trampolines root directory\n"
  exit 1
fi

root=$(pwd)

mkdir -p build/static build/global build/embedded

if [[ $1 = "clean" ]]; then
  rm -rf build/

  printf "Build artifacts removed!\n"
  
  exit 0
fi

function staticBuild() {
  local architecture=${1:-arm64}
  local message=${2:-$1}
  local ccflags=${3}
  local result=0
  
  printf "Compiling ${message}..."
  
  gcc -arch ${architecture} -O2 ${ccflags} \
    -c "${root}/trampoline_${architecture}.c" \
    -o "trampoline_${architecture}.o" \
    2>/dev/null >/dev/null
  result=$(( $result + $? ))

  gcc -arch ${architecture} -O2 ${ccflags}\
    -c "${root}/trampoline_helpers.c" \
    -o "trampoline_helpers.${architecture}.o" \
    2>/dev/null >/dev/null
  result=$(( $result + $? ))
    
  if [[ $result -eq 0 ]]; then
    printf "\x1b[32mdone\x1b[39m\n"
  else
    printf "\x1b[31mfailed\x1b[39m\n"
  fi
  
  return $result
}

function buildDylib() {
  local architecture=${1:-arm64}
  local message=${2:-$1}  
  local installPath=${3:-"@executable_path/../lib"}
  local installName=${4:-"libtrampoline"}
  local result=0  
  
  printf "Creating ${message} dynamic library..."

  gcc -arch ${architecture} -O2 -fPIC \
    -c "${root}/trampoline_${architecture}.c" \
    -o trampoline.${architecture}.dylib.o \
    2>/dev/null >/dev/null
  result=$(( $result + $? ))
  
  gcc -arch ${architecture} -O2 -fPIC \
    -c "${root}/trampoline_helpers.c" \
    -o trampoline_helpers.${architecture}.dylib.o \
    2>/dev/null >/dev/null
  result=$(( $result + $? ))
  
  gcc -arch ${architecture} -dynamiclib \
    -install_name "${installPath}/${installName}.dylib" \
    -compatibility_version 1.0.0 \
    -current_version 1.0.0 \
    -o "libtrampoline.${architecture}.dylib" \
    "trampoline.${architecture}.dylib.o" \
    "trampoline_helpers.${architecture}.dylib.o" \
    2>/dev/null >/dev/null
  result=$(( $result + $? ))
  
  if [[ $result -eq 0 ]]; then
    printf "\x1b[32mdone\x1b[39m\n"
  else
    printf "\x1b[31mfailed\x1b[39m\n"
  fi
  
  return ${result}
}

function buildAR() {
  local architecture=${1:-arm64}
  local resultSoFar=${2:-0}
  
  if [[ -f "trampoline_${architecture}.o" ]]; then 
    ar rcs "libtrampoline.${architecture}.a" \
      "trampoline_${architecture}.o" \
      "trampoline_helpers.arm64.o" \
      2>/dev/null >/dev/null
    resultSoFar=$(( $resultSoFar + $? ))
  
    if [[ -f "libtrampoline.${architecture}.a" ]]; then
      ranlib "libtrampoline.${architecture}.a" \
        2>/dev/null >/dev/null
      resultSoFar=$(( $resultSoFar + $? ))
    fi
  fi
  
  return $resultSoFar
}

function buildArchArray() {
  local prefix=${1:-"libtrampoline."}
  local postfix=${2:-".a"}
  local archs=""
  
  if [[ -f "${prefix}arm64${postfix}" ]]; then
    printf "  ...found ARM 64-bit\n"
    archs="${archs} ${prefix}arm64${postfix}"
  fi
  
  if [[ -f "${prefix}x86${postfix}" ]]; then
    printf "  ...found Intel 32-bit\n"
    archs="${archs} ${prefix}x86${postfix}"
  fi
  
  if [[ -f "${prefix}x86_64${postfix}" ]]; then
    printf "  ...found Intel 64-bit\n"
    archs="${archs} ${prefix}x86_64${postfix}"
  fi
  
  if [[ -f "${prefix}ppc${postfix}" ]]; then
    printf "  ...found PowerPC 32-bit\n"
    archs="${archs} ${prefix}ppc${postfix}"
  fi
  
  if [[ -f "${prefix}ppc64${postfix}" ]]; then
    printf "  ...found PowerPC 64-bit\n"
    archs="${archs} ${prefix}ppc64${postfix}"
  fi
  
  printf "${archs}\n"
}

cd ${root}/build/static
printf "Building static libraries...\n"

staticBuild arm64 "ARM (64-bit)" ; result=$(( $result + $? ))
staticBuild ppc "PowerPC (32-bit)" ; result=$(( $result + $? ))
staticBuild ppc64 "PowerPC (64-bit)" "-mmacosx-version-min=10.5" ; result=$(( $result + $? ))
staticBuild i386 "Intel (32-bit)" "-mdynamic-no-pic -Wall -Wextra" ; result=$(( $result + $? ))
staticBuild x86_64 "Intel (64-bit)" "-mdynamic-no-pic -Wall -Wextra" ; result=$(( $result + $? ))

printf "Building ar libs..."
result=0

buildAR arm64 $result; result=$(( $result + $? ))
buildAR ppc $result; result=$(( $result + $? ))
buildAR ppc64 $result; result=$(( $result + $? ))
buildAR x86 $result; result=$(( $result + $? ))
buildAR x86_64 $result; result=$(( $result + $? ))

if [[ $result -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "Building universal binary static libtrampoline...\n"
archs=$(buildArchArray)

printf "  ..."
/usr/bin/lipo -create -output libtrampoline.a ${archs}

if [[ $? -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "\x1b[1m"
/usr/bin/lipo -info libtrampoline.a
printf "\x1b[22m\n"

# Let's make some dylibs

cd ${root}/build/embedded

buildDylib arm64 "ARM (64-bit)"
buildDylib ppc "PowerPC (32-bit)"
buildDylib ppc64 "PowerPC (64-bit)"
buildDylib x86 "Intel (32-bit)"
buildDylib x86_64 "Intel (64-bit)"

archs=$(buildArchArray "libtrampoline." ".dylib")

printf "Building universal binary dynamic libtrampoline..."
/usr/bin/lipo -create \
  -output libtrampoline.dylib \
  ${archs}
  
if [[ $? -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "\x1b[1m"
/usr/bin/lipo -info libtrampoline.dylib
printf "\x1b[22m\n"

# -------------- global dylib -------------------

cd ${root}/build/global

buildDylib arm64 "ARM (64-bit)" /usr/lib
buildDylib ppc "PowerPC (32-bit)" /usr/lib
buildDylib ppc64 "PowerPC (64-bit)" /usr/lib
buildDylib x86 "Intel (32-bit)" /usr/lib 
buildDylib x86_64 "Intel (64-bit)" /usr/lib

archs=$(buildArchArray "libtrampoline." ".dylib")
printf "Building universal binary dynamic libtrampoline..."
lipo -create \
  -output libtrampoline.dylib \
  ${archs}
  
if [[ $? -eq 0 ]]; then
  printf "\x1b[32mdone\x1b[39m\n"
else
  printf "\x1b[31mfailed\x1b[39m\n"
fi

printf "\x1b[1m"
lipo -info libtrampoline.dylib
printf "\x1b[22m\n"

cd ${root}

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
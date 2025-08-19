# **C Trampoline for Object-Oriented Style Methods**

This project provides a robust, cross-platform C implementation of a "trampoline" to enable object-oriented style method calls (e.g., instance-\>method()) without explicitly passing the instance pointer. It achieves this by dynamically generating a small piece of executable machine code for each object instance.

This repository serves as a technical demonstration of low-level programming concepts, including C function pointers, memory management (mmap/VirtualAlloc), platform-specific ABIs (calling conventions), and assembly.

**This is an advanced and platform-specific technique. It is presented for academic and exploratory purposes and may not be suitable for production environments due to its complexity and the security implications of generating executable code at runtime.**

## **Features**

* **Object-Oriented Syntax:** Enables calling functions with an implicit self or this context, like person-\>getName().  
* **Cross-Platform:** Includes a unified build system for:  
  * **macOS:** Universal binaries (x86\_64 and arm64)  
  * **Linux:** x86\_64, x86, arm64, and arm32  
  * **Windows:** x86\_64 and x86  
* **Reusable Library:** Can be built as a universal shared (.dylib/.so) or static (.a) library for easy integration into other projects.  
* **Clean Architecture:** A well-organized Makefile and an "umbrella" include system keep platform-specific code neatly separated.

## **How to Build and Use**

This project uses a Makefile to handle all compilation tasks. Run make help for a full list of commands.

| Target | Description |
| :---- | :---- |
| make or make all | Builds the default executable. On macOS, this creates a universal binary. |
| make shared-lib | Builds a reusable shared library (libtrampoline.dylib on macOS, libtrampoline.so on Linux). |
| make static-lib | Builds a reusable static library (libtrampoline.a) for static linking. |
| make install | Installs the header and libraries to a system or user path. |
| make clean | Removes all compiled binaries, object files, and libraries. |

## **Installation**

You can install the library system-wide (requires root privileges) or locally in your home directory.

### **System-Wide Installation**

This makes the library available to all users. The default location is /usr/local/.

\# 1\. Build and install the library.  
sudo make install

\# 2\. Uninstall the library.  
sudo make uninstall

### **User-Local Installation**

This is useful if you don't have admin rights or want to keep the library isolated.

\# 1\. Install to $HOME/.local  
make install PREFIX=$HOME/.local

\# 2\. Uninstall from $HOME/.local  
make uninstall PREFIX=$HOME/.local

## **Using the Libraries**

No changes are needed in your C code to use the library. You only need to change how you compile and link your program.

### **Linking Against the Static Library (.a)**

The static library bundles the trampoline code directly into your final executable.

1. **Build the static library:**  
   make static-lib

2. **Compile your program and link it:**  
   clang main.c \-L. \-ltrampoline \-o my\_app\_static

   * \-L. tells the linker to look for libraries in the current directory.  
   * \-ltrampoline tells the linker to find and link libtrampoline.a.

### **Linking Against the Shared Library (.dylib / .so)**

The shared library is loaded by your program at runtime.

1. **Build the shared library:**  
   make shared-lib

2. **Compile your program and link it:**  
   clang main.c \-L. \-ltrampoline \-o my\_app\_shared

3. **Run your application:**  
   * **On macOS,** this should work if the .dylib is in the same directory.  
   * **On Linux,** you may need to tell the dynamic linker where to find the .so file:  
     export LD\_LIBRARY\_PATH=.  
     ./my\_app\_shared

## **Project Structure**

.  
├── Makefile                \# The main build script for all platforms.  
├── main.c                  \# An example program demonstrating the Person "class".  
├── trampoline.c            \# Umbrella file that includes the correct implementation.  
├── trampoline.h            \# The public header file for the trampoline library.  
├── trampoline\_arm32.c      \# Implementation for 32-bit ARM (Raspberry Pi).  
├── trampoline\_arm64.c      \# Implementation for 64-bit ARM (Apple Silicon).  
├── trampoline\_x86.c        \# Implementation for 32-bit x86 (Linux/macOS).  
├── trampoline\_x86\_64.c     \# Implementation for 64-bit x86\_64 (Linux/macOS).  
├── trampoline\_x86\_win.c    \# Implementation for 32-bit x86 (Windows).  
└── trampoline\_x86\_64\_win.c \# Implementation for 64-bit x86\_64 (Windows).

## **Acknowledgments**

This code was implemented by **Gemini 2.5 Pro** under the direction of **Brielle Harrison \<nyteshade@gmail.com\>**.

# Applies C++-only compiler flags to C++ sources only (avoids warnings when compiling C files).
Import("env")  # noqa: F821 (provided by PlatformIO/SCons)

env.Append(CXXFLAGS=["-std=gnu++17", "-fno-exceptions", "-fno-rtti"])  # noqa: F821

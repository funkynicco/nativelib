# [WIP] OS Independant Native Library
> This is a native library written in C++ for use on multiple platforms **\[WIP\]**. \
The library project files are generated using [Premake5](https://premake.github.io/download.html), which can generate projects for multiple platforms such as Windows (Visual Studio) and Linux (MAKE).

> Python 3.7+ is required for `publish.py` to run properly. This script copies the final binaries to `lib` folder.

## Some of the features
- Vector container class
- String class
- Shared and Scoped RAII classes (similar to std shared_ptr and unique_ptr)
- Assertions
- JSON interactive objects
- Logger abstraction
- Remote Procedure Calling (RPC) server using Named Pipes (Windows)
- Exceptions with stack trace
- Global allocators to trace memory
- Mathematical vectors (Vector2, Vector3 and Vector4)
project "nativelib"
    language        "C++"
    kind            "StaticLib"
    --characterset    "MBCS"
    cppdialect      "c++17"
    systemversion   "10.0.19041.0"

    defines {
        "_LIB"
    }

    filter "system:windows"
        pchheader "StdAfx.h"
    
    filter {}

    pchsource "src/StdAfx.cpp"

    files {
        "src/**.cpp",
        "src/**.h",
        "src/**.inl",
        "include/NativeLib/**.h",
        "include/NativeLib/**.inl",
        "*.py",
        "*.natvis",
        "*.lua"
    }

    includedirs {
        "src",
        "include",
    }

    if IS_NATIVELIB_SOLUTION then
        filter "system:windows"

            postbuildcommands {
                "py publish.py %{cfg.platform} %{cfg.buildcfg} %{cfg.action}"
            }
    
        filter {}
    end
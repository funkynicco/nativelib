project "nativelib.test"
    dependson       "nativelib"
    kind            "ConsoleApp"
    cppdialect      "c++17"
    systemversion   "10.0.19041.0"

    pchheader "StdAfx.h"
    pchsource "src/StdAfx.cpp"

    files {
        "src/**.cpp",
        "src/**.h",
        "src/**.inl",
        "*.lua"
    }

    includedirs {
        "src",
        "../include"
    }

    links {
        "nativelib"
    }
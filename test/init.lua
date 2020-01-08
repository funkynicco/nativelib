project "nativelib.test"
    dependson "nativelib"
    kind "ConsoleApp"

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
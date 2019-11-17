project "nativelib.test"
    dependson "nativelib"
    kind "ConsoleApp"

    files {
        "src/**.cpp",
        "src/**.h",
        "*.lua"
    }

    includedirs {
        "src",
        "../include"
    }

    links {
        "nativelib"
    }
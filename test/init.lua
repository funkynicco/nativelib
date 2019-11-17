project "nativelib.test"
    dependson "nativelib"
    kind "ConsoleApp"

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
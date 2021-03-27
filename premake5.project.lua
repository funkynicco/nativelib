project "nativelib"

pchheader "StdAfx.h"
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

postbuildcommands {
    "py publish.py %{cfg.platform} %{cfg.buildcfg} %{cfg.action}"
}
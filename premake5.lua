workspace "nativelib"
    targetdir       "build/%{cfg.action}/bin/%{cfg.longname}"
    objdir          "build/%{cfg.action}/obj/%{prj.name}/%{cfg.longname}"

    platforms {
        "Win32",
        "x64",
    }

    filter "platforms:Win32"
        architecture "x86"
        defines {
            "NL_ARCHITECTURE_X86",
            "WIN32"
        }
    
    filter "platforms:x64"
        architecture "x64"
        defines {
            "NL_ARCHITECTURE_X64"
		}
    
    filter {}

    configurations {
        "Debug",
        "Release",
    }
    
    flags {
        "MultiProcessorCompile"
    }

    filter "configurations:Debug"
        symbols "on"

        defines {
            "_DEBUG",
            "_CRT_SECURE_NO_WARNINGS"
        }
    
    filter "configurations:Release"
        optimize        "on"
        staticruntime   "on"

        defines {
            "NDEBUG"
        }
    
        flags {
            "NoIncrementalLink",
            "LinkTimeOptimization",
        }
    
    filter {}

    include "premake5.project.lua"
    
    require "test"
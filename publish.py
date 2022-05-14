import os
import sys
import shutil

platform = sys.argv[1]
buildcfg = sys.argv[2]
compiler = sys.argv[3]

project_name = "nativelib"

def log(msg):
    print("[publish.py] " + msg)

log("Publishing for " + platform + "/" + buildcfg + " (" + compiler + ")")

# os.path.exists
def cp(src, dst):
    if os.path.exists(src):
        shutil.copy2(src, dst)
        return True
    
    log(f"Failed to copy {src} ==> {dst} (source file missing)")
    return False

# Copy libs ...

#D:\Coding\CPP\Game Related\Shelby\Libraries\nativelib\build\vs2022\bin\nativelib\x64\Debug

if platform == "Win32":
    dst_dir = os.path.join("lib", compiler, "x86")
    os.makedirs(dst_dir, exist_ok=True)
    if buildcfg == "Debug":
        cp(os.path.join("build", compiler, "bin", project_name, platform, buildcfg, "nativelib.lib"), os.path.join(dst_dir, "nativelib_d.lib"))
        cp(os.path.join("build", compiler, "bin", project_name, platform, buildcfg, "nativelib.pdb"), os.path.join(dst_dir, "nativelib_d.pdb"))
    elif buildcfg == "Release":
        cp(os.path.join("build", compiler, "bin", project_name, platform, buildcfg, "nativelib.lib"), os.path.join(dst_dir, "nativelib.lib"))
        cp(os.path.join("build", compiler, "bin", project_name, platform, buildcfg, "nativelib.pdb"), os.path.join(dst_dir, "nativelib.pdb"))
    else:
        log("Unknown build configuration: " + buildcfg)
        exit(1)
elif platform == "x64":
    dst_dir = os.path.join("lib", compiler, "x64")
    os.makedirs(dst_dir, exist_ok=True)
    if buildcfg == "Debug":
        cp(os.path.join("build", compiler, "bin", project_name, platform, buildcfg, "nativelib.lib"), os.path.join(dst_dir, "nativelib_d.lib"))
        cp(os.path.join("build", compiler, "bin", project_name, platform, buildcfg, "nativelib.pdb"), os.path.join(dst_dir, "nativelib_d.pdb"))
    elif buildcfg == "Release":
        cp(os.path.join("build", compiler, "bin", project_name, platform, buildcfg, "nativelib.lib"), os.path.join(dst_dir, "nativelib.lib"))
        cp(os.path.join("build", compiler, "bin", project_name, platform, buildcfg, "nativelib.pdb"), os.path.join(dst_dir, "nativelib.pdb"))
    else:
        log("Unknown build configuration: " + buildcfg)
        exit(1)
else:
    log("Unknown platform: " + platform)
    exit(1)

log("Completed")
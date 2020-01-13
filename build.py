import os
import sys
import subprocess

if sys.version_info[0] < 3:
    print("Python version must be 3 or higher.")
    exit(1)

INCLUDE_ROOT = "include"
SOURCE_ROOT = "src"
OBJ_ROOT = "build/emcc/obj"
OUTPUT = "lib/emcc"
APP_NAME = "NativeLib"

if "EMCC" not in os.environ:
    print("Environment variable EMCC is missing!")
    print("Please create it and point to the EMScripten folder containing emcc")
    exit(1)

if not os.path.exists(os.path.join(os.environ["EMCC"], "emcc.bat")):
    print("emcc.bat not found in {}".format(os.environ["EMCC"]))
    exit(1)

emcc_path = os.path.join(os.environ["EMCC"], "emcc.bat")
emar_path = os.path.join(os.environ["EMCC"], "emar.bat")

class SourceFile:
    def __init__(self, src_path, dst_path):
        self.src_path = src_path
        self.dst_path = dst_path

source_files = []

os.makedirs(OBJ_ROOT, exist_ok=True)

for root, dirs, files in os.walk(SOURCE_ROOT):
    for name in files:
        if name.endswith(".cpp"):
            src_path = os.path.join(root, name)
            dst_path = src_path.replace(".cpp", ".o").replace(SOURCE_ROOT, OBJ_ROOT)
            os.makedirs(os.path.dirname(dst_path), exist_ok=True)
            source_files.append(SourceFile(src_path, dst_path))

for source_file in source_files:
    print("Compiling {} ...".format(source_file.src_path))
    ret = subprocess.run([
        emcc_path,
        "-std=c++1z", # C++17
        "-Wno-unused-comparison", # skip the unused warnings such as the result of an expression: x == y in assertion
        #"-w", # DISABLES ALL WARNINGS!!
        "-I{}".format(SOURCE_ROOT),
        "-I{}".format(INCLUDE_ROOT),
        "-c",
        source_file.src_path,
        "-o",
        source_file.dst_path
        ])
    if ret.returncode != 0:
        exit(1)

# Create archive out of app source files
print("Archiving ...")
args = []
args.append(emar_path)
args.append("r")
args.append(os.path.join(OUTPUT, "{}.a".format(APP_NAME)))
for source_file in source_files:
    args.append(source_file.dst_path)
subprocess.run(args)

print("Completed.")
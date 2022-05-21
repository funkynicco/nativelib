import io
import sys
import os
import re

print("Running screening test")

# This script will check the code for screening to catch potential mistakes in including OS-specific files or include path mistakes.

errors = []
scanned_files = 0

# These includes are allowed in exposed headers
# NOTE: These must be lowercase
whitelisted_public_includes = [
    "stdint.h",
    "type_traits",
    "string_view",
    "functional",
    
    # same file
    "cstring", "string.h",
    
    "any",
    "array",
    "cstdio",
    "utility",
    "ostream",
    "errno.h",

    # same file
    "cmath", "math.h",
]

# These includes can only be used inside of private project files not exposed
# NOTE: These must be lowercase
whitelisted_private_includes = [
    "stdafx.h",
    "csignal",
    "shared_mutex"
]

rgx_include = re.compile("#include\\s*(<|\")(.*?)(>|\")")
rgx_using_namespace = re.compile("using\\s+namespace\\s+(.*?);")

def rdir(paths):
    result = []
    for path in paths:
        for dirpath, _, filenames in os.walk(path):
            for filename in filenames:
                result.append(os.path.join(dirpath, filename))
        
    return result

def calculate_line(str, index):
    lines = 1
    for i in range(0, index):
        if str[i] == "\n":
            lines += 1
    
    return lines

def get_included_files(content):
    result = []
    for m in rgx_include.finditer(content):
        result.append(m)
    
    return result

def get_exceptions(content):
    exceptions = []

    rgx_exceptions = re.compile("^\\/\\/!ALLOW_INCLUDE\\s*\"(.*?)\"", re.MULTILINE)
    for m in rgx_exceptions.findall(content):
        exceptions.append(m.lower())

    return exceptions

def is_allowed_include(is_public, include, exceptions):
    if include in exceptions:
        return True

    if include in whitelisted_public_includes:
        return True

    return not is_public and include in whitelisted_private_includes

for filename in rdir(["include", "src"]):
    is_include = filename.startswith("include")
    scanned_files += 1
    with open(filename, "r") as f:
        content = f.read()

        include_exceptions = get_exceptions(content)

        # check includes
        for include in get_included_files(content):
            line = calculate_line(content, include.start())
            include_filename = str(include.groups()[1])
            find = include_filename.find("\\")
            if find != -1:
                errors.append("[{}:{}] Use forward slashes for include '{}'".format(filename, line, include_filename))
                include_filename = include_filename.replace("\\", "/")

            lowercase = include_filename.lower()
            if not lowercase.startswith("nativelib/"):
                if not is_allowed_include(is_include, lowercase, include_exceptions):
                    errors.append("[{}:{}] Inclusion of file not whitelisted: '{}'".format(filename, line, include_filename))
        
        # find "using namespace"
        for m in rgx_using_namespace.finditer(content):
            line = calculate_line(content, m.start())
            errors.append("[{}:{}] Using statements not allowed: {}".format(filename, line, m.group(0)))

for error in errors:
    print(error)

errors_count = len(errors)
if errors_count > 0:
    if errors_count == 1:
        print("{} error must be resolved to pass screening test.".format(errors_count))
    else:
        print("{} errors must be resolved to pass screening test.".format(errors_count))
    
    print("Test failed! {} files scanned.".format(scanned_files))

    sys.exit(1)

print("Test passed! {} files scanned.".format(scanned_files))
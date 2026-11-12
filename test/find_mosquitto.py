#!/usr/bin/env python3
import subprocess
import tempfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]

with tempfile.TemporaryDirectory() as directory:
    source = Path(directory)
    prefix = source / "prefix"
    (prefix / "include").mkdir(parents=True)
    (prefix / "lib").mkdir()
    (prefix / "include" / "mosquitto.h").write_text(
        "#define MOSQUITTO_TEST_VALUE 42\n", encoding="utf-8"
    )
    for name in ("libmosquitto.a", "mosquitto.lib"):
        (prefix / "lib" / name).touch()
    (source / "main.cpp").write_text(
        "#include <mosquitto.h>\n"
        "static_assert(MOSQUITTO_TEST_VALUE == 42);\n"
        "int main() { return 0; }\n",
        encoding="utf-8",
    )
    (source / "CMakeLists.txt").write_text(
        """cmake_minimum_required(VERSION 3.31)
project(find_mosquitto_test LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH "${MODULE_PATH}")
include(FindMosquitto)
if(NOT MOSQUITTO_FOUND)
  message(FATAL_ERROR "Mosquitto fixture not found")
endif()
if(NOT MOSQUITTO_INCLUDE_DIRS STREQUAL "${CMAKE_SOURCE_DIR}/prefix/include")
  message(FATAL_ERROR "Mosquitto include directories not exported")
endif()
add_executable(header_check main.cpp)
target_compile_features(header_check PRIVATE cxx_std_17)
target_include_directories(header_check PRIVATE ${MOSQUITTO_INCLUDE_DIRS})
""",
        encoding="utf-8",
    )
    build = source / "build"
    subprocess.run(
        [
            "cmake",
            "-S", str(source),
            "-B", str(build),
            f"-DMODULE_PATH={root / 'cmake' / 'modules'}",
            f"-DCMAKE_PREFIX_PATH={prefix}",
        ],
        check=True,
    )
    subprocess.run(["cmake", "--build", str(build)], check=True)

print("find_mosquitto: OK")

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy
import os


class RaytracerCConan(ConanFile):
    name = "raytracer-c"
    version = "1.0.0"  # ← Must be specified, used for package identification

    # Basic metadata
    license = "MIT"
    author = "sumitpo"
    url = "https://github.com/sumitpo/raytracer-c"
    description = "A simple raytracer written in C"
    topics = ("raytracer", "graphics", "c")

    # Build settings
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    # Source export (critical!)
    exports_sources = "CMakeLists.txt", "include/*", "src/*", "LICENSE"

    # Generators
    generators = "CMakeDeps", "CMakeToolchain"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def requirements(self):
        self.requires("log4c/1.0.0@local/stable")
        self.requires("libpng/1.6.43")
        self.requires("argtable3/3.2.2")
        self.requires("wavefront-parser/1.0.0@local/stable")
        # Note: cmocka is a testing dependency and should not be a runtime dependency!
        # If your library itself doesn't require cmocka, it should be removed from requirements (see note below)

    def configure(self):
        # Set dependency options (consistent with previous setup)
        self.options["libpng"].system = True
        self.options["argtable3"].system = True

        # self.options["libpng"].shared = True
        # self.options["argtable3"].shared = True

        self.options["wavefront-parser"].build_examples = False
        self.options["wavefront-parser"].build_tests = False
        # cmocka should not appear here (unless your library actually links against it)

    def layout(self):
        cmake_layout(self)

    def generate(self):
        # CMakeDeps and CMakeToolchain are automatically handled by generators
        pass

    def build(self):
        cmake = CMake(self)
        cmake.build()

    def package(self):
        # Install header files and library files
        cmake = CMake(self)
        cmake.install()  # Recommended: implement install() in CMakeLists.txt

        # Copy license file
        copy(self, "LICENSE", src=self.source_folder,
             dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        # Tell consumers how to link your library
        self.cpp_info.libs = ["raytracer-c"]  # Corresponds to the target name in CMake

        # If your library depends on system libraries (e.g., libpng using system version), you might need:
        # self.cpp_info.system_libs = ["png", "z"]
from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy
import os


class RaytracerCConan(ConanFile):
    name = "raytracer-c"
    version = "1.0.0"  # ← 必须指定，用于包标识

    # 基本元数据
    license = "MIT"
    author = "Your Name"
    url = "https://github.com/your/raytracer-c"
    description = "A simple raytracer written in C"
    topics = ("raytracer", "graphics", "c")

    # 构建设置
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    # 源码导出（关键！）
    exports_sources = "CMakeLists.txt", "include/*", "src/*", "LICENSE"

    # 生成器
    generators = "CMakeDeps", "CMakeToolchain"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def requirements(self):
        self.requires("log4c/1.0.0@local/stable")
        self.requires("libpng/1.6.43")
        self.requires("argtable3/3.2.2")
        self.requires("wavefront-parser/1.0.0@local/stable")
        # 注意：cmocka 是测试依赖，不应作为运行时依赖！
        # 如果你的库本身不需要 cmocka，应移出 requirements（见下方说明）

    def configure(self):
        # 设置依赖选项（与之前一致）
        self.options["libpng"].system = True
        self.options["argtable3"].system = True

        # self.options["libpng"].shared = True
        # self.options["argtable3"].shared = True

        self.options["wavefront-parser"].build_examples = False
        self.options["wavefront-parser"].build_tests = False
        # cmocka 不应出现在这里（除非你的库真的链接了它）

    def layout(self):
        cmake_layout(self)

    def generate(self):
        # CMakeDeps 和 CMakeToolchain 已由 generators 自动处理
        pass

    def build(self):
        cmake = CMake(self)
        cmake.build()

    def package(self):
        # 安装头文件和库文件
        cmake = CMake(self)
        cmake.install()  # 推荐：在 CMakeLists.txt 中实现 install()

        # 复制许可证
        copy(self, "LICENSE", src=self.source_folder,
             dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        # 告诉使用者如何链接你的库
        self.cpp_info.libs = ["raytracer-c"]  # 对应 CMake 中的 target 名称

        # 如果你的库依赖系统库（如 libpng 使用系统版），可能需要：
        # self.cpp_info.system_libs = ["png", "z"]

import sys

from pybind11 import get_cmake_dir
# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup
from pathlib import Path


__version__ = "1.0.0"

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

ext_modules = [
    Pybind11Extension("cyphalpypp",
        ["cyphalpypp.cpp"],
        # Example: passing in the version to the compiled code
        define_macrsos = [('VERSION_INFO', __version__)],
        cxx_std=17,
        include_dirs =["../../include/", "../../vendored", "../cytypes"]
        ),
]

setup(
    name="cyphalpypp",
    version=__version__,
    author="Pavel Pletenev",
    author_email="pletenev.pavel@vniizht.ru",
    url="cyphalpypp",
    description="Cyphal/UDP via cpp",
    long_description="",
    ext_modules=ext_modules,
    extras_require={"test": "pytest"},
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.6",
)

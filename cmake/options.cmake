option(examples "build example applicaitons" off)
option(unit_tests "build unit tests" off)

option(p4est_external "force build of p4est" off)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# --- default install directory under build/local
# users can specify like "cmake -B build -DCMAKE_INSTALL_PREFIX=~/mydir"
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  # will not take effect without FORCE
  set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/local" CACHE PATH "Install top-level directory" FORCE)
endif()
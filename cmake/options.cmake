include(CMakeDependentOption)

option(petsc "allow the use the use of PETSc" on)
cmake_dependent_option(petsc_required "fail if PETSc is not found" off "petsc" off)

option(fftw "allow the use the use of FFTW" on)
cmake_dependent_option(fftw_required "fail if FFTW is not found" off "fftw" off)

option(p4est "allow the use the use of p4est" on)
cmake_dependent_option(p4est_required "fail if p4est is not found" off "p4est" off)

option(lapack "allow the use the use of lapack/blas" on)
cmake_dependent_option(lapack_required "fail if lapack/blas is not found" off "lapack" off)

# options for libsc, p4est
set(mpi true)

# --- default install directory under build/local
if(CMAKE_VERSION VERSION_LESS 3.21)
  get_property(_not_top DIRECTORY PROPERTY PARENT_DIRECTORY)
  if(NOT _not_top)
    set(ThunderEgg_IS_TOP_LEVEL true)
  endif()
endif()

option(ThunderEgg_BUILD_TESTING "build tests" ${ThunderEgg_IS_TOP_LEVEL})

# users can specify like "cmake -B build -DCMAKE_INSTALL_PREFIX=~/mydir"
if(ThunderEgg_IS_TOP_LEVEL AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  # will not take effect without FORCE
  set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/local" CACHE PATH "Install top-level directory" FORCE)
endif()

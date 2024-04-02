cmake_minimum_required(VERSION 3.18...3.29)
project(ThunderEgg
        VERSION 1.0.6
        LANGUAGES C CXX)
enable_testing()

# set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

include(cmake/GitSubmodule.cmake)
include(cmake/options.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_CXX_STANDARD 17)

find_package(MPI REQUIRED COMPONENTS C CXX)

set(THUNDEREGG_ENABLED_COMPONENTS "")

if(petsc)

  find_package(PETSc)

  if(petsc_required AND NOT PETSc_FOUND)
    message(FATAL_ERROR "PETSc was not found")
  endif()

  if(PETSc_FOUND)
    set(THUNDEREGG_PETSC_ENABLED TRUE)
    list(APPEND THUNDEREGG_ENABLED_COMPONENTS "PETSC")
  endif()

endif()

if(fftw)

  find_package(FFTW)

  if(fftw_required AND NOT FFTW_FOUND)
    message(FATAL_ERROR "FFTW was not found")
  endif()

  if(FFTW_FOUND)
    set(THUNDEREGG_FFTW_ENABLED TRUE)
    list(APPEND THUNDEREGG_ENABLED_COMPONENTS "FFTW")
  endif()
endif()

if(lapack)

  find_package(LAPACK)
  find_package(BLAS)

  if(lapack_required AND NOT LAPACK_FOUND)
    message(FATAL_ERROR "LAPACK was not found")
  endif()
  if(lapack_required AND NOT BLAS_FOUND)
    message(FATAL_ERROR "BLAS was not found")
  endif()

  if(BLAS_FOUND AND LAPACK_FOUND)
    set(THUNDEREGG_LAPACK_ENABLED TRUE)
    list(APPEND THUNDEREGG_ENABLED_COMPONENTS "LAPACK")
  endif()
endif()


if(p4est)

  if(NOT TARGET P4EST::P4EST)
    if(P4EST_ROOT)
      find_package(P4EST CONFIG REQUIRED NO_DEFAULT_PATH HINTS "${P4EST_ROOT}")
    else()
      git_submodule(${CMAKE_CURRENT_SOURCE_DIR}/p4est)
      add_subdirectory(p4est)
    endif()
  endif()

  if(TARGET P4EST::P4EST)
    set(THUNDEREGG_P4EST_ENABLED TRUE)
    list(APPEND THUNDEREGG_ENABLED_COMPONENTS "P4EST")
  elseif(p4est_required)
    message(FATAL_ERROR "${PROJECT_NAME}: P4EST was requested but not found")
  endif()
endif()

add_subdirectory(src)


if(ThunderEgg_BUILD_TESTING)
  add_subdirectory(test)
endif()

include(cmake/documentation.cmake)

include(CMakePackageConfigHelpers)


configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION lib)

  install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/FindFFTW.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/FindPETSc.cmake
  DESTINATION lib/cmake/Modules)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
  COMPATIBILITY SameMinorVersion)

install(TARGETS ThunderEgg
  EXPORT                        ThunderEggTargets
  RUNTIME DESTINATION           ${CMAKE_INSTALL_BINDIR}
          COMPONENT             ThunderEgg_RunTime
  LIBRARY DESTINATION           ${CMAKE_INSTALL_LIBDIR}
          COMPONENT             ThunderEgg_RunTime
          NAMELINK_COMPONENT    ThunderEgg_Development
  ARCHIVE DESTINATION           ${CMAKE_INSTALL_LIBDIR}
          COMPONENT             ThunderEgg_Development
)

install(EXPORT ${PROJECT_NAME}Targets
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  COMPONENT   ThunderEgg_Development
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  COMPONENT   ThunderEgg_Development
)

export(EXPORT ${PROJECT_NAME}Targets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
)

# -- generate a pkgconfig file
include(cmake/pkgconfig.cmake)

if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  file(GENERATE OUTPUT .gitignore CONTENT "*")
endif()

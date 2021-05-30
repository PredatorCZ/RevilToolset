cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_STANDARD 14)
project(RevilToolset VERSION 1.0)

# vgmstream fix
find_package(PkgConfig REQUIRED)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX})
set(TPD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party)
set(BUILD_SHARED_LIBS ON CACHE BOOL "")
set(PUGI_AS_SHARED ON CACHE BOOL "")
set(BUILD_STATIC_OBJECTS OFF CACHE BOOL "")
add_subdirectory(${TPD_PATH}/RevilLib)

include(targetex)
include(chartype)

#add_subdirectory(spac_conv)
add_subdirectory(re_tex_convert)
add_subdirectory(arc_conv)
add_subdirectory(ddon_sngw)
add_subdirectory(re_to_gltf)

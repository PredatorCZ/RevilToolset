cmake_minimum_required(VERSION 3.12)

project(REAsset2GLTF VERSION 1.0)

build_target(
  NAME
  reasset_to_gltf
  TYPE
  APP
  SOURCES
  reasset_to_gltf
  LINKS
  pugixml
  revil
  INCLUDES
  ${TPD_PATH}/gltf
  AUTHOR
  "Lukas Cone"
  DESCR
  "RE Assets to GLTF"
  START_YEAR
  2021)

install(TARGETS reasset_to_gltf DESTINATION ${CMAKE_INSTALL_PREFIX})
set_precore_sources(
  reasset_to_gltf
  multi_thread
  directory_scanner
  master_printer
  reflector_xml
  reflector
  stat)

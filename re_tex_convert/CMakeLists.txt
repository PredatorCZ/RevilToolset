cmake_minimum_required(VERSION 3.3)

project(RETEXConvert VERSION 1.0)

build_target(
  NAME
  tex_conv_re
  TYPE
  APP
  SOURCES
  tex_convert.cpp
  LINKS
  pugixml
  AUTHOR
  "Lukas Cone"
  DESCR
  "RE TEX Converter"
  START_YEAR
  2020)

install(TARGETS tex_conv_re DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
set_precore_sources(
  tex_conv_re
  multi_thread
  directory_scanner
  master_printer
  reflector_xml
  reflector
  stat)

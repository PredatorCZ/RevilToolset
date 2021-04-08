cmake_minimum_required(VERSION 3.12)

project(DDONSngw VERSION 1.0)

build_target(
  NAME
  ddon_sngw
  TYPE
  APP
  SOURCES
  ddon_sngw.cpp
  LINKS
  pugixml
  INCLUDES
  AUTHOR
  "Lukas Cone"
  DESCR
  "DD:Online SNGW converter"
  START_YEAR
  2021)

install(TARGETS ddon_sngw DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
set_precore_sources(
  ddon_sngw
  multi_thread
  directory_scanner
  master_printer
  reflector_xml
  reflector
  stat)

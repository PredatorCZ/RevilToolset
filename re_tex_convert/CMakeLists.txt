cmake_minimum_required(VERSION 3.3)

project(TEXConvert VERSION 1.0)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
set(TPD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party)
set(TARGETEX_LOCATION ${TPD_PATH}/precore/cmake)
include(${TARGETEX_LOCATION}/targetex.cmake)
include(${TARGETEX_LOCATION}/chartype.cmake)

build_target(
	TYPE APP
	SOURCES
		tex_convert.cpp
        ${TPD_PATH}/precore/datas/MasterPrinter.cpp
		${TPD_PATH}/precore/datas/reflector.cpp
		${TPD_PATH}/precore/datas/reflectorXML.cpp
		${TPD_PATH}/RevilLib/3rd_party/pugixml/src/pugixml.cpp
	INCLUDES
		${TPD_PATH}/precore
		${TPD_PATH}/RevilLib/3rd_party/pugixml/src
	AUTHOR "Lukas Cone"
	DESCR "RE/MTF TEX Converter"
	NAME "TEXConvert"
	START_YEAR 2020
)

install(TARGETS TEXConvert
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

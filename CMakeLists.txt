# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS NOM_STACK/Diagnostics NOM_STACK/Manager NOM_STACK/Sensors NOM_STACK/Services NOM_STACK/Signals NOM_STACK/Application NOM_STACK/UI)
# //NOM_STACK/Signals 
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(AERO_MAIN_SYS)
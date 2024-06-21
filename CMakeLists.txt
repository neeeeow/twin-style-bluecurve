############################################
#                                          #
#  Improvements and feedbacks are welcome  #
#                                          #
#  This file is released under GPL >= 3    #
#                                          #
############################################


##### set project version ########################

include( TDEVersion )
cmake_minimum_required( VERSION ${TDE_CMAKE_MINIMUM_VERSION} )
tde_set_project_version( )


#### general package setup

project( twin-style-bluecurve )


#### include essential cmake modules

include( FindPkgConfig          )
include( CheckFunctionExists    )
include( CheckSymbolExists      )
include( CheckIncludeFile       )
include( CheckLibraryExists     )
include( CheckCSourceCompiles   )
include( CheckCXXSourceCompiles )


#### include our cmake modules

include( TDEMacros )


##### setup install paths

include( TDESetupPaths )
tde_setup_paths( )


##### user-requested modules

option( BUILD_ALL "Build all" ON )
option( BUILD_TRANSLATIONS "Build translations" ${BUILD_ALL} )


##### optional stuff

option( WITH_ALL_OPTIONS "Enable all optional support" OFF                                          )
option( WITH_GCC_VISIBILITY "Enable fvisibility and fvisibility-inlines-hidden" ${WITH_ALL_OPTIONS} )


##### configure checks

include( ConfigureChecks.cmake )


###### global compiler settings

add_definitions( -DHAVE_CONFIG_H )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TQT_CXX_FLAGS}" )
set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined" )
set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined" )


##### directories

add_subdirectory( Bluecurve )
tde_conditional_add_project_translations( BUILD_TRANSLATIONS )


##### write configure files

configure_file( config.h.cmake config.h @ONLY )


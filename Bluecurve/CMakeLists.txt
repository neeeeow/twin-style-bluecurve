tde_import( twin )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${TDE_LIBRARY_DIRS}
)

##### twin_bluecurve (kpart)

tde_add_kpart( twin_bluecurve AUTOMOC

  SOURCES
    bluecurveclient.cpp
  LINK
    tdecore-shared
    tdeui-shared
    tdecorations-shared

  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data

tde_create_translated_desktop(
    SOURCE bluecurve.desktop
    DESTINATION ${DATA_INSTALL_DIR}/twin
)

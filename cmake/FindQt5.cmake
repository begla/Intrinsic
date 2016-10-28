# Locate Qt5 to allow for using its configs
# This module defines
#  Qt5_FOUND, if false, do not try to link to Lua 
#  Qt5_QMAKE_EXECUTABLE
#  Qt5_ROOT_DIR, where to find Qt5
#
# Specifying either Qt5_ROOT_DIR or Qt5_QMAKE_EXECUTABLE is enough
# to find the entire Qt5 install.

IF(NOT Qt5_QMAKE_EXECUTABLE)
  FIND_PROGRAM(Qt5_QMAKE_EXECUTABLE
    NAMES qmake qmake5 qmake-qt5
    PATH_SUFFIXES bin
    PATHS
      ${INTR_QTDIR}
      $ENV{INTR_QTDIR}
      ${QT_SEARCH_PATH}
      $ENV{QTDIR}
      ~/Library/Frameworks
      /Library/Frameworks
      /usr/local
      /usr
      /sw
      /opt/local
      /opt/csw
      /opt
    )
ENDIF()

SET(Qt5_ROOT_DIR NOTFOUND CACHE PATH "")

IF (Qt5_QMAKE_EXECUTABLE)
  EXECUTE_PROCESS(COMMAND ${Qt5_QMAKE_EXECUTABLE} -query QT_INSTALL_PREFIX OUTPUT_VARIABLE INSTALL_PREFIX)
  STRING(STRIP ${INSTALL_PREFIX} Qt5_ROOT_DIR)
  SET(Qt5_ROOT_DIR ${Qt5_ROOT_DIR} CACHE PATH "" FORCE)
ENDIF()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Qt5  DEFAULT_MSG  Qt5_ROOT_DIR)

MARK_AS_ADVANCED(Qt5_QMAKE_EXECUTABLE Qt5_ROOT_DIR)

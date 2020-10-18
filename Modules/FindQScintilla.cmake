# QScintilla_FOUND
# QSCINTILLA_INCLUDE_DIRS
# QSCINTILLA_LIBRARY

if(Qt5Core_FOUND)
  find_path(QSCINTILLA_INCLUDE_INTERNAL
    NAMES Qsci/qsciapis.h
    PATHS ${Qt5Core_INCLUDE_DIRS})

  find_library(QSCINTILLA_LIBRARY
    NAMES
      qscintilla2_qt5
      qt5scintilla2
      qscintilla2-qt5)

  if(QSCINTILLA_INCLUDE_INTERNAL AND QSCINTILLA_LIBRARY)
    set(QScintilla_FOUND TRUE)
    set(QSCINTILLA_INCLUDE_DIRS "${QSCINTILLA_INCLUDE_INTERNAL}/Qsci")
    message(STATUS "QScintilla found: ${QSCINTILLA_LIBRARY}")
  else()
    set(QScintilla_FOUND FALSE)
  endif()
else()
  message(STATUS "Qt5Core not found. Find it before this script")
  set(QScintilla_FOUND FALSE)
endif()

if(NOT QScintilla_FOUND AND QScintilla_FIND_REQUIRED)
  message(FATAL_ERROR "Cannot find QScintilla library")
endif()

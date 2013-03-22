# QScintilla_FOUND
# QSCINTILLA_INCLUDE_DIR
# QSCINTILLA_LIBRARY

if(QT4_FOUND)
  find_path(QSCINTILLA_INCLUDE_INTERNAL Qsci/qsciapi.h)
  find_library(QSCINTILLA_LIBRARY qscintilla2)
  if(EXISTS "{QSCINTILLA_INCLUDE_INTERNAL}" AND EXISTS "{QSCINTILLA_LIBRARY}")
    set(QScintilla_FOUND TRUE)
    set(SCINTILLA_INCLUDE_DIR "${SCINTILLA_INCLUDE_DIR}/Qsci")
    set(SCINTILLA_INCLUDE_DIRS
      "${SCINTILLA_INCLUDE_DIRS}"
      "${QT_QTGUI_INCLUDE_DIR}"
      "${QT_QTCORE_INCLUDE_DIR}")
    message(STATUS "QScintilla found: ${SCINTILLA_LIBRARY}")
  else()
    set(QScintilla_FOUND FALSE)
  endif()
else()
  message(STATUS "Qt4 not found. Find it before this script")
  set(QScintilla_FOUND FALSE)
endif()

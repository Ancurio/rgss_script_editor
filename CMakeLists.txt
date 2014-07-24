cmake_minimum_required(VERSION 2.8)
project(rgss_script_editor CXX C)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Modules")

find_package(Qt4 REQUIRED QtCore QtGui)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

find_package(QScintilla REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  ${RUBY_INCLUDE_DIRS} ${QT_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})

file(GLOB QT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/qt/*.hxx")
qt4_wrap_cpp(QT_MOC_OUTPUT ${QT_HEADERS})

set(EDITOR_SRCS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/main.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/ruby_data.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/qt/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/qt/editor_widget.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/qt/main_window.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/qt/savediscard_dialog.cxx"
	)
set(CLI_SRCS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/ruby_data.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/cli.cxx"
	)

list(APPEND EDITOR_SRCS "${QT_MOC_OUTPUT}")

add_executable("${PROJECT_NAME}" ${EDITOR_SRCS})
add_executable("${PROJECT_NAME}_cli" ${CLI_SRCS})

target_link_libraries("${PROJECT_NAME}"
  ${QT_LIBRARIES}
  ${QSCINTILLA_LIBRARY} ${ZLIB_LIBRARIES})
target_link_libraries("${PROJECT_NAME}_cli"
  ${QT_LIBRARIES}
  ${QSCINTILLA_LIBRARY} ${ZLIB_LIBRARIES})

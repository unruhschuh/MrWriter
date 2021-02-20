cmake_minimum_required(VERSION 3.3.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")

file(READ src/version.h ver)

string(REGEX MATCH "MAJOR_VERSION ([0-9]*)" _ ${ver})
set(ver_major ${CMAKE_MATCH_1})

string(REGEX MATCH "MINOR_VERSION ([0-9]*)" _ ${ver})
set(ver_minor ${CMAKE_MATCH_1})

string(REGEX MATCH "PATCH_VERSION ([0-9]*)" _ ${ver})
set(ver_patch ${CMAKE_MATCH_1})

set(MR_WRITER_VERSION "${ver_major}.${ver_minor}.${ver_patch}")

project(MrWriter VERSION ${MR_WRITER_VERSION})

set (CMAKE_CXX_STANDARD 14)

set(CONAN_BUILD_INFO_FILE ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
if (EXISTS ${CONAN_BUILD_INFO_FILE})
  include(${CONAN_BUILD_INFO_FILE})
  conan_basic_setup(KEEP_RPATHS)
  set(USE_CONAN TRUE)
else()
  set(USE_CONAN FALSE)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

message("                     -= MrWriter ${MR_WRITER_VERSION} =-")
message("0xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx0")
message(",                       ....';;;,'..                       ,")
message(",                 .:dOXWXOk0NMMMMMMWx..;'.                 ,")
message(",             .cOWMMMWXOkOXMMMMMMWx..ckkkkx'               ,")
message(",           c0MMMMWX0kk0NMMMMMMWd..lkkkkkc..cko,           ,")
message(",        .dWMMMWX0kk0XWMMMMMMWo..okkkkx: .lkkkkkx:         ,")
message(",       oWMMMWKkk0XWMMMMMMMNo .okkkkx; .okkkkkkkkkx;       ,")
message(",     'XMMMMXkk0NMMMMMMMMNl .okkkkx; .okkkkkkkkkkkkko      ,")
message(",    :WMMMW0kONMMMMMMMMNc  .',:dx, 'dkkkkkkkkkkkkkx; .'    ,")
message(",   lMMMMWOk0WMMMMMMMMo :xKXKOo. .dkkkkkkkkkkkkkx; 'okk,   ,")
message(",  ;MMMMMXkkWMMMMMMMMO oMMMMMMMMo :kkkkkkkkkkkx, 'dkkkkx   ,")
message(",  NMMMMMNkkWMMMMMMMN.'MMMMMMMMMM' kkkkkkkkkd, 'dkkkkd, :  ,")
message(", lMMMMMMWOk0MMMMMMM; XMMMMMMMMMW..llloxkkd' ,dkkkkd' cXMl ,")
message(", 0MMMMMMMNkkNMMMMMx xMMMMMMMMMK. ,cll:... ;xkkkkd' lNMMM0 ,")
message(", WMMMMMMMMOkKMMMMX ;MMMMMMMMMMXkWMMMMMMK, okkko. lNMMMMMW ,")
message(", WMMMMMMMWkkXMMMW' 0WMMMMMMMMMMMMMMMMMMMM..ko..oNMMMMMMMW ,")
message(", KMMMMMMMXkkWMMMo '...;kWMMMMMMMMMMMMMMMM, ..dWMMMMMMMMMK ,")
message(", oMMMMMMWOkKMMMK 'kkkxl..xMMMMMMMMMMMMMMd .xWMMMMMMMMMMMo ,")
message(", .WMMMMMNkkNMMW. xkkkkkkl :MMMMMMMXko;..'kWMMMMMMMMMMMMW. ,")
message(",  cMMMMMWkkXMMl okkkkkkkk: xKkl;..,cx0NMMMMMMMMMMMMMMMMc  ,")
message(",   dMMMMM0kOWO ;kkkkkxo:,. .;lkKWMMMMMMMMMMMMMMMMMMMMMd   ,")
message(",    oMMMMW0kk..dl:'...:oOXWMMMMMMMMMMMMMMMMMMMMMMMMMMo    ,")
message(",     ;NMMMW0, .':dONMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMN;     ,")
message(",      .kMMMMWNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMk.      ,")
message(",        'OMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMO'        ,")
message(",          .dNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNd.          ,")
message(",             'dXMMMMMMMMMMMMMMMMMMMMMMMMMMXd'             ,")
message(",                .;oOXWMMMMMMMMMMMMMMWXOo;.                ,")
message(",                      .';:cllllc:;'.                      ,")
message("o,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,o")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    myresource.qrc
    src/colorbutton.cpp
    src/commands.cpp
    src/document.cpp
    src/element.cpp
    src/image.cpp
    src/keypresseater.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/mrdoc.cpp
    src/page.cpp
    src/pagesettingsdialog.cpp
    src/qcompressor.cpp
    src/quickmenu.cpp
    src/quickmenu.ui
    src/selection.cpp
    src/settingsdialog.cpp
    src/settingsdialog.ui
    src/stroke.cpp
    src/tabletapplication.cpp
    src/text.cpp
    src/tools.cpp
    src/widget.cpp
    )

set(HEADERS
    src/colorbutton.h
    src/commands.h
    src/document.h
    src/element.h
    src/image.h
    src/keypresseater.h
    src/mainwindow.h
    src/mrdoc.h
    src/page.h
    src/pagesettingsdialog.h
    src/qcompressor.h
    src/quickmenu.h
    src/selection.h
    src/settingsdialog.h
    src/stroke.h
    src/tabletapplication.h
    src/text.h
    src/tictoc.h
    src/tools.h
    src/version.h
    src/widget.h
    )

if (APPLE)
  add_executable(MrWriter MACOSX_BUNDLE ${SOURCES} ${HEADERS})
  set_target_properties(MrWriter PROPERTIES
      BUNDLE True
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist
  )
else()
  add_executable(MrWriter ${SOURCES} ${HEADERS})
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(TARGET MrWriter PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_options(MrWriter PRIVATE -fPIC)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Test REQUIRED)

find_package(ZLIB REQUIRED)

if (WIN32)
  set_target_properties(MrWriter PROPERTIES WIN32_EXECUTABLE true)
endif()

message("build type: ${CMAKE_BUILD_TYPE}")
if (CMAKE_BUILD_TYPE MATCHES Release)
  message("Release build")
  target_compile_definitions(MrWriter PRIVATE QT_NO_DEBUG_OUTPUT)
endif()

if (${USE_CONAN})
  set(LIBS ${CONAN_LIBS})
else()
  set(LIBS Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Xml Qt5::Test Qt5::PrintSupport Qt5::Concurrent Qt5::Test z)
endif()
message(${LIBS})

target_link_libraries(MrWriter ${LIBS})

install(TARGETS MrWriter DESTINATION ${CMAKE_BINARY_DIR}/appdir/usr/bin)

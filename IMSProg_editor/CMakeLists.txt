cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

project(IMSProg_editor LANGUAGES CXX)
# Set the CMAKE_INSTALL_PREFIX to /usr if not specified and /usr/local for macOS
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "The default install prefix on macOS" FORCE)
  # Set Qt5 path for macOS
  	set(CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}/opt/qt@5" CACHE PATH "Path to Qt5 on macOS" FORCE)
  else()
      set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "The default install prefix" FORCE)
  endif()
endif()

# Set the CMAKE_INSTALL_BINDIR to /bin if not specified
if(NOT DEFINED CMAKE_INSTALL_BINDIR)
  set(CMAKE_INSTALL_BINDIR "bin" CACHE PATH "user executables (bin)")
endif()

# Set the CMAKE_INSTALL_DATAROOTDIR to /share if not specified
if(NOT DEFINED CMAKE_INSTALL_DATAROOTDIR)
  set(CMAKE_INSTALL_DATAROOTDIR "share" CACHE PATH "read-only architecture-independent data root (share)")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Qt6 QUIET COMPONENTS Core Widgets)
if (TARGET Qt6::Core AND TARGET Qt6::Widgets)
    set(QT_VERSION_MAJOR 6)
    find_package(Qt6LinguistTools REQUIRED)
else()
    # If QT6 not found, using QT5
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets LinguistTools)
    set(QT_VERSION_MAJOR 5)
endif()

# Translation files
set(TS_FILES
    language/chipEditor_ru_RU.ts
    language/chipEditor_de_DE.ts
    language/chipEditor_es_ES.ts
    language/chipEditor_zh_CN.ts
    language/chipEditor_zh_TW.ts
    language/chipEditor_uk_UA.ts
    language/chipEditor_hu_HU.ts
    language/chipEditor_pt_BR.ts
    language/chipEditor_it_IT.ts
)

qt_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations1 DEPENDS ${QM_FILES})

add_executable(${PROJECT_NAME}
	ezp_chip_editor.h
	ezp_chip_editor.cpp
	ezp_chip_editor.ui
	main.cpp
	delegates.h
	delegates.cpp
	resources.qrc
	${QM_FILES}
	)


target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} Qt::Core)
target_link_libraries(${PROJECT_NAME} Qt::Widgets)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg_editor.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/img/chipEdit64.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pixmaps")

install(FILES ${QM_FILES} DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg_editor.1.gz" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/man/man1")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/io.github.bigbigmdm.imsprog_editor.metainfo.xml" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo")

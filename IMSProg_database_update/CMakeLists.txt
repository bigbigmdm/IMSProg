cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

project(IMSProg_database_update LANGUAGES CXX)

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
#set(OPENSSL_USE_STATIC_LIBS TRUE)
#set(OPENSSL_USE_STATIC_LIBS OFF CACHE BOOL "" FORCE)
#set(OPENSSL_ROOT_DIR "/usr/include")

find_package(Qt5 REQUIRED COMPONENTS Core Widgets LinguistTools)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

# Translation files
set(TS_FILES
    language/chipUpdater_ru_RU.ts
    language/chipUpdater_es_ES.ts
    language/chipUpdater_de_DE.ts
    language/chipUpdater_zh_CN.ts
    language/chipUpdater_zh_TW.ts
    language/chipUpdater_uk_UA.ts
    language/chipUpdater_hu_HU.ts
    language/chipUpdater_pt_BR.ts
    language/chipUpdater_it_IT.ts
)

qt5_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations2 DEPENDS ${QM_FILES})

add_executable(${PROJECT_NAME}
main.cpp
mainwindow.cpp
mainwindow.h
mainwindow.ui
resource.qrc
${QM_FILES}
	)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} Qt5::Network)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg_database_update.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/img/IMSProg_database_update.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pixmaps")

install(FILES ${QM_FILES} DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg_database_update.1.gz" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/man/man1")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/io.github.bigbigmdm.imsprog_database_update.metainfo.xml" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo")

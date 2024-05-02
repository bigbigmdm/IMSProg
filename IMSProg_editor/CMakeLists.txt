cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

project(IMSProg_editor LANGUAGES CXX)

# Set the CMAKE_INSTALL_PREFIX to /usr if not specified
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "The default install prefix" FORCE)
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

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt5Widgets REQUIRED)

add_executable(${PROJECT_NAME}
	ezp_chip_editor.h
	ezp_chip_editor.cpp
	ezp_chip_editor.ui
	main.cpp
	delegates.h
	delegates.cpp
	resources.qrc
	)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg_editor.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/img/chipEdit64.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pixmaps")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipEditor_ru_RU.qm" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipEditor_de_DE.qm" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipEditor_es_ES.qm" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipEditor_zh_CN.qm" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipEditor_uk_UA.qm" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipEditor_hu_HU.qm" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipEditor_pt_BR.qm" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipEditor_it_IT.qm" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg_editor.1.gz" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/man/man1")

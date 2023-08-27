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

install(TARGETS ${PROJECT_NAME} DESTINATION /opt/IMSProg)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg_editor.desktop" DESTINATION /opt/IMSProg)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg_editor.desktop" DESTINATION /usr/share/applications)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipEditor_ru_RU.qm" DESTINATION /opt/IMSProg/language)






# Configure the current application's bundle when requested by the build.
if(APPLE AND CMAKE_MACOSX_BUNDLE)
    string(TOLOWER "${PROJECT_NAME}" bundle_name)
    set(bundle_identifier "io.github.bigbigmdm.${bundle_name}")
    set(bundle_icon "${CMAKE_CURRENT_SOURCE_DIR}/img/icons/apple/${bundle_identifier}.icns")
    set(bundle_database "${CMAKE_CURRENT_LIST_DIR}/../IMSProg_programmer/database/IMSProg.Dat")

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "${bundle_identifier}"
        MACOSX_BUNDLE_ICON_FILE "${bundle_identifier}.icns"
    )

    target_sources(${PROJECT_NAME} PRIVATE "${bundle_icon}" "${bundle_database}")
    set_source_files_properties("${bundle_icon}" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    # Match the ../share/imsprog path used by the applications at runtime.
    set_source_files_properties("${bundle_database}" ${QM_FILES} PROPERTIES
        MACOSX_PACKAGE_LOCATION "share/imsprog"
    )
endif()

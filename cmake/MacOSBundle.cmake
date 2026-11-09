# Add runtime data to the build bundles staged for DMG packaging.
if(APPLE AND CMAKE_MACOSX_BUNDLE)
    set(bundle_database "${CMAKE_CURRENT_LIST_DIR}/../IMSProg_programmer/database/IMSProg.Dat")

    target_sources(${PROJECT_NAME} PRIVATE "${bundle_database}")
    # Match the ../share/imsprog path used by the applications at runtime.
    set_source_files_properties("${bundle_database}" ${QM_FILES} PROPERTIES
        MACOSX_PACKAGE_LOCATION "share/imsprog"
    )
endif()

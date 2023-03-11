include(GNUInstallDirs)

install(TARGETS precompiled infrastructure_obj infrastructure_shared infrastructure_static
    EXPORT InfrastructureLibrary
    ARCHIVE COMPONENT infrastructure_dev
    LIBRARY COMPONENT infrastructure
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/infrastructure
        COMPONENT infrastructure
)

if(UNIX)
    install(CODE "execute_process(COMMAND ldconfig)"
        COMPONENT infrastructure
    )
endif()

install(EXPORT InfrastructureLibrary
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MY_PROJECT_NAME}/cmake
    NAMESPACE ${MY_PACKAGE_NAME}::Infrastructure::
    COMPONENT infrastructure
)

install(FILES "ProjectConfig.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/cmake
    RENAME "${MY_PACKAGE_NAME}Config.cmake"
)

set(CPACK_PACKAGE_VENDOR "Mohammad Rahimi")
set(CPACK_PACKAGE_CONTACT "rahimi.mhmmd@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION "A target management application.")
include(CPack)

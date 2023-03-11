add_library(infrastructure_obj OBJECT
	infrastructure.cpp
)
target_compile_options(infrastructure_obj
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)
target_include_directories(infrastructure_obj INTERFACE
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
set_target_properties(infrastructure_obj PROPERTIES
    PUBLIC_HEADER src/infrastructure/include/infrastructure/infrastructure.h
    POSITION_INDEPENDENT_CODE 1
)
AddPrecompiledHeaders(infrastructure_obj)
BuildInfo(infrastructure_obj)

add_library(infrastructure_shared SHARED)
target_link_libraries(infrastructure_shared infrastructure_obj)
add_library(infrastructure_static STATIC)
target_link_libraries(infrastructure_static infrastructure_obj)

include(Coverage)
CleanCoverage(infrastructure_static)
include(Format)
Format(infrastructure_static .)
include(CppCheck)
AddCppCheck(infrastructure_obj)
include(Doxygen)
Doxygen(infrastructure src/infrastructure)

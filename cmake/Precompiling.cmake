add_library(precompiled INTERFACE)

target_precompile_headers(precompiled INTERFACE
	<cstddef>
	<iostream>
	<string>
	<string_view>
)

macro(AddPrecompiledHeaders target)
	target_link_libraries(${target} PRIVATE precompiled)
endmacro()

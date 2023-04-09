include_guard(GLOBAL)

add_library(precompiled INTERFACE)

target_precompile_headers(precompiled INTERFACE
	<cstddef>
	<iostream>
	<chrono>
	<ranges>
	<memory>
	<atomic>
	<thread>
	<algorithm>
	<random>
	<string>
	<string_view>
	<vector>
)

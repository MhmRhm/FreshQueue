include(Boost)
add_executable(infrastructure_benchmark infrastructure_benchmark.cpp)
target_link_libraries(infrastructure_benchmark PRIVATE infrastructure_static)

target_link_libraries(infrastructure_benchmark PRIVATE precompiled ${Boost_LIBRARIES})

include(Format)
Format(infrastructure_benchmark .)
AddBenchmarks(infrastructure_benchmark)

add_executable(infrastructure_test infrastructure_test.cpp)
target_link_libraries(infrastructure_test PRIVATE infrastructure_static)

target_link_libraries(infrastructure_test PRIVATE precompiled ${Boost_LIBRARIES})

include(Format)
Format(infrastructure_test .)
AddTests(infrastructure_test)
EnableCoverage(infrastructure_obj)

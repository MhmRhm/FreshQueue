include(FetchContent)
set(BOOST_INCLUDE_LIBRARIES lockfree)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
	Boost
	GIT_REPOSITORY https://github.com/boostorg/boost.git
	GIT_TAG boost-1.81.0
)
FetchContent_MakeAvailable(Boost)

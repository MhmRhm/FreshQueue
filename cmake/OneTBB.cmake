include(FetchContent)
FetchContent_Declare(
	TBB
	GIT_REPOSITORY https://github.com/oneapi-src/oneTBB.git
	GIT_TAG v2021.8.0
)
option(TBB_TEST "Enable testing" OFF)
FetchContent_MakeAvailable(TBB)

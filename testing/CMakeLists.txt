enable_testing()

# Fetch GTest from the official GitHub repository
include(FetchContent)
FetchContent_Declare(
  googletest
  PREFIX "${CMAKE_SOURCE_DIR}/extern/gtest"
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Link testing executables
add_executable(${PROJECT_NAME}_tests
    "${CMAKE_CURRENT_SOURCE_DIR}/core/time_test.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/log_test.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fs_test.cpp"
)
target_link_libraries(${PROJECT_NAME}_tests PRIVATE
    ${PROJECT_NAME}_lib
    GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)
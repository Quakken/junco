add_library(${PROJECT_NAME}_lib
    "${CMAKE_CURRENT_SOURCE_DIR}/time.cpp"
)

target_include_directories(${PROJECT_NAME}_lib PUBLIC
    "${CMAKE_SOURCE_DIR}/include/"
)

# Export compile definitions
# Has to be made public because of log definitions
target_compile_definitions(${PROJECT_NAME}_lib PUBLIC
    $<$<CONFIG:Debug>:JC_BUILD_DEBUG>
    $<$<CONFIG:Release>:JC_BUILD_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:JC_BUILD_RELWITHDEBINFO>
)
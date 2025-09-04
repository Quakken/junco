add_library(${PROJECT_NAME}_lib
    "${CMAKE_CURRENT_SOURCE_DIR}/foo.cpp"
)

target_include_directories(${PROJECT_NAME}_lib PUBLIC
    "${CMAKE_SOURCE_DIR}/include/"
)

# Export compile definitions
target_compile_definitions(${PROJECT_NAME}_lib PRIVATE
    $<$<CONFIG:Debug>:JUNCO_BUILD_DEBUG>
    $<$<CONFIG:Release>:JUNCO_BUILD_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:JUNCO_BUILD_RELWITHDEBINFO>
)
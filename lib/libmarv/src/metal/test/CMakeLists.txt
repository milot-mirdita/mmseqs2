add_executable(test_gapless_kernel test_gapless_kernel.cpp)
target_include_directories(test_gapless_kernel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(test_gapless_kernel marv "-framework Metal" "-framework Foundation" "-framework IOKit")

add_test(NAME metal_gapless_kernel COMMAND test_gapless_kernel)

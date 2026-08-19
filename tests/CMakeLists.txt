add_executable(unittest ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/unittest.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/evhttp_router.cpp)
target_include_directories(unittest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    message("skipping code coverage for windows")
    target_link_libraries(unittest PRIVATE evhttp_router_common Maxtest::Maxtest)
else()
    target_compile_options(unittest PRIVATE -fprofile-arcs -ftest-coverage -g -O0)
    target_link_libraries(unittest PRIVATE evhttp_router_common Maxtest::Maxtest gcov "--coverage")
    add_custom_target(
        cover
        DEPENDS unittest)
    add_custom_command(
        TARGET cover POST_BUILD
        COMMAND gcovr -r ${CMAKE_CURRENT_SOURCE_DIR}/.. -e ${CMAKE_CURRENT_SOURCE_DIR})
endif()

maxtest_add_test(unittest basic_test)
maxtest_add_test(unittest method_test)
maxtest_add_test(unittest method_test_cpp)
maxtest_add_test(unittest wildcard_test)
maxtest_add_test(unittest handler_test)
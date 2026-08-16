add_executable(unittest ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(unittest PRIVATE evhttp_router Maxtest::Maxtest)

maxtest_add_test(unittest test_name)
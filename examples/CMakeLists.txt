add_library(example_base INTERFACE)
target_include_directories(example_base INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(example_base INTERFACE evhttp_router)

add_executable(stringservice ${CMAKE_CURRENT_SOURCE_DIR}/example.cpp ${CMAKE_CURRENT_SOURCE_DIR}/stringservice.c)
target_link_libraries(stringservice PRIVATE example_base)

add_executable(logservice ${CMAKE_CURRENT_SOURCE_DIR}/example.cpp ${CMAKE_CURRENT_SOURCE_DIR}/logservice.cpp)
target_link_libraries(logservice PRIVATE example_base)
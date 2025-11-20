add_executable(test_basic test_basic.c)
target_link_libraries(test_basic uci)
add_test(NAME BasicTest COMMAND test_basic)

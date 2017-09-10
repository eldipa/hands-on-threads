project(concurso)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -pedantic -pedantic-errors")

add_executable(threads-c threads-c.c)
add_executable(contador-01 contador-01.cpp)
add_executable(contador-02 contador-02.cpp)
add_executable(contador-03 contador-03.cpp)
add_executable(contador-04 contador-04.cpp)
add_executable(deadlock deadlock.cpp AccountBad.cpp)
add_executable(not-deadlock deadlock-ok.cpp Account.cpp Lock.cpp)

add_executable(random-gen random-gen.cpp)

target_link_libraries(threads-c pthread)
target_link_libraries(contador-02 pthread)
target_link_libraries(contador-03 pthread)
target_link_libraries(contador-04 pthread)
target_link_libraries(deadlock pthread)
target_link_libraries(not-deadlock pthread)

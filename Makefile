all: chklibs f1.1 f2.1 f3.1 f4.1 f5.1 f6.1 f7.1 f8.1 f9.1 f10.1 f11.1 f12.1 f13.1

clean:
	rm -Rf *.o *.a *.so *.exe a.out test_queue

chklibs:
	g++ -std=c++17 -pedantic -Wall -ggdb -o test_queue tests/queue.cpp
	cppcheck --enable=all --language=c++ --std=c++17 --error-exitcode=1 --suppress=missingIncludeSystem --suppress=unusedFunction --inline-suppr libs/*.h libs/*.cpp
	./test_queue

f1.1:
	g++ -std=c++17 -pedantic -Wall -ggdb -o 01_is_prime_sequential.exe 01_is_prime_sequential.cpp

f2.1:
	g++ -std=c++17 -pedantic -Wall -ggdb -o 02_is_prime_parallel_by_composition.exe 02_is_prime_parallel_by_composition.cpp -pthread

f3.1:
	g++ -std=c++17 -pedantic -Wall -ggdb -o 03_is_prime_parallel_by_inheritance.exe 03_is_prime_parallel_by_inheritance.cpp -pthread

f4.1:
	g++ -std=c++17 -pedantic -Wall -ggdb -o 04_sumatoria_with_race_conditions.exe 04_sumatoria_with_race_conditions.cpp -pthread

f5.1:
	g++ -std=c++17 -pedantic -Wall -ggdb -o 05_sumatoria_with_mutex.exe 05_sumatoria_with_mutex.cpp -pthread

f6.1:
	g++ -std=c++17 -pedantic -Wall -ggdb -o 06_sumatoria_with_locks_raii.exe 06_sumatoria_with_locks_raii.cpp -pthread

f7.1:
	g++ -std=c++17 -pedantic -Wall -ggdb -o 07_sumatoria_with_monitor.exe 07_sumatoria_with_monitor.cpp -pthread

f8.1:
	g++ -std=c++17 -pedantic -Wall -ggdb -o 08_monitor_interface_critical_section.exe 08_monitor_interface_critical_section.cpp -pthread

f9.1:
	g++ -std=c++17 -pedantic -Wall -ggdb -o 09_non_blocking_queue.exe 09_non_blocking_queue.cpp -pthread

f10.1:
	g++ -std=c++17 -pedantic -Wall -ggdb -o 10_blocking_queue_with_busy_wait_and_polling.exe 10_blocking_queue_with_busy_wait_and_polling.cpp -pthread

f11.1:
	g++ -std=c++17 -pedantic -Wall -ggdb -o 11_blocking_queue_with_conditional_variables.exe 11_blocking_queue_with_conditional_variables.cpp -pthread

f12.1:
	g++ -std=c++17 -pedantic -Wall -ggdb -o 12_how_to_close_a_queue.exe 12_how_to_close_a_queue.cpp -pthread

f13.1:
	g++ -std=c++17 -pedantic -Wall -ggdb -o 13_fixme.exe 13_fixme.cpp -pthread


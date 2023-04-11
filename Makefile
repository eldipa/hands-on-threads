all: f1.1 f2.1 f3.1 f4.1 f5.1 f6.1 f7.1 f8.1 f9.1 f10.1

clean:
	rm -Rf *.o *.a *.so *.exe a.out

f1.1:
	g++ -std=c++11 -pedantic -Wall -o 01_is_prime_sequential.exe 01_is_prime_sequential.cpp

f2.1:
	g++ -std=c++11 -pedantic -Wall -o 02_is_prime_parallel_by_composition.exe 02_is_prime_parallel_by_composition.cpp -pthread

f3.1:
	g++ -std=c++11 -pedantic -Wall -o 03_is_prime_parallel_by_inheritance.exe 03_is_prime_parallel_by_inheritance.cpp -pthread

f4.1:
	g++ -std=c++11 -pedantic -Wall -o 04_sumatoria_with_race_conditions.exe 04_sumatoria_with_race_conditions.cpp -pthread

f5.1:
	g++ -std=c++11 -pedantic -Wall -o 05_sumatoria_with_mutex.exe 05_sumatoria_with_mutex.cpp -pthread

f6.1:
	g++ -std=c++11 -pedantic -Wall -o 06_sumatoria_with_locks_raii.exe 06_sumatoria_with_locks_raii.cpp -pthread

f7.1:
	g++ -std=c++11 -pedantic -Wall -o 07_sumatoria_with_monitor.exe 07_sumatoria_with_monitor.cpp -pthread

f8.1:
	g++ -std=c++11 -pedantic -Wall -o 08_monitor_interface_critical_section.exe 08_monitor_interface_critical_section.cpp -pthread

f9.1:
	g++ -std=c++11 -pedantic -Wall -o 09_thread_safe_queue_with_busy_wait_and_polling.exe 09_thread_safe_queue_with_busy_wait_and_polling.cpp -pthread

f10.1:
	g++ -std=c++11 -pedantic -Wall -o 10_thread_safe_queue_with_conditional_variables.exe 10_thread_safe_queue_with_conditional_variables.cpp -pthread

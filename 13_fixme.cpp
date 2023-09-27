/*
 * Ejercicio final!
 *
 * Se tienen N alumnos (clase Student) que son objetos activos (Thread).
 *
 * Los alumnos tienen acceso a una lista de asistencia (clase Attendance)
 * en la que deben agregarse si no esta y marcar que están presentes.
 *
 * La clase Attendance ya tiene los métodos implementados
 * (que **no** podes modificar) y la clase Student tienen la lógica.
 *
 * Dado que Attendance esta **compartida** hay una **race condition**
 * y tu objetivo sera arreglar el código.
 *
 * Tenes 2 opciones:
 *
 *  - Podes implementar un Monitor haciendo uso de mutexes y locks para
 *    proteger al objeto Attendance.
 *    Tendrás que descubrir cual/cuales son las **critical sections**
 *    y proteger el acceso. (Nota: la clase Attendance **no** la podes
 *    modificar, por lo que tendras q crear un AttendanceProtected
 *    y modificar a los Student).
 *
 *  - La otra opción es **no** compartir la lista de asistencia y
 *    en cambio hacer que todos los alumnos (Student) compartan
 *    una **única** blocking queue.
 *    El main() pusheara a la queue la lista y cada alumno hara
 *    un pop(), hara lo que tenga que hacer con la lista y
 *    la devolverá haciendo un push().
 *    Como veras, en ningún momento la lista de asistencia es
 *    compartida "simultáneamente" por los threads.
 *
 * Ambas opciones son válidas y equivalentes: ninguna es mejor
 * q la otra (aunque creo que la de Monitor "para este caso" tiene
 * algunos puntos técnicos a favor, pero es debatible).
 *
 *
 * Good luck!!
 *
 * */
#include <iostream>
#include <chrono>
#include <random>
#include <map>
#include <algorithm>

// Misma clase Thread que en 03_is_prime_parallel_by_inheritance.cpp
#include "libs/thread.h"

// Misma clase Queue que en 12_how_to_close_a_queue.cpp
// excepto que es una clase template
#include "libs/queue.h"

namespace {
    const int STUDENTS_CNT = 100;
}

// NO MODIFICAR (no hagas trampa)
class Attendance {
    private:
        std::map<int, bool> list;

    public:
        bool is_student_in_list(int student_id) {
            return list.count(student_id) == 1;
        }

        void add_student_to_list(int student_id) {
            if (is_student_in_list(student_id)) {
                throw std::runtime_error("You cannot add the same student twice");
            }

            list[student_id] = false;
        }

        void mark_attendance_of_student(int student_id) {
            if (not is_student_in_list(student_id)) {
                throw std::runtime_error("The student must be added to the list first");
            }

            list[student_id] = true;
        }

        void print(std::ostream& out) {
            out << "There are " << list.size() << " students in the list\n";

            int present_cnt = std::count_if(list.begin(), list.end(), [](auto const& pair) {
                        return pair.second == true;
            });

            out << "There are " << present_cnt << " present students and ";
            out <<  list.size() - present_cnt << " absent students\n";
        }
};

void sleep_a_little(std::default_random_engine& generator) {
    std::uniform_int_distribution<int> get_random_int(100, 500);

    auto random_int = get_random_int(generator);
    auto milliseconds_to_sleep = std::chrono::milliseconds(random_int);
    std::this_thread::sleep_for(milliseconds_to_sleep); // sleep some "pseudo-random" time
}

class Student : public Thread {
    private:
        int id;
        Attendance& list;

    public:
        explicit Student(int id, Attendance& list) : id(id), list(list) {}

        virtual void run() override {
            std::default_random_engine generator;

            // Simulamos algo de tiempo para q los alumnos quieran
            // firma a la vez pero con algo de randomness
            sleep_a_little(generator);

            if (not list.is_student_in_list(id)) {
                list.add_student_to_list(id);
            }

            list.mark_attendance_of_student(id);

            sleep_a_little(generator);
        }
};


int main(int argc, char *argv[]) {
    Attendance list;

    std::vector<Thread*> students(STUDENTS_CNT);

    for (int i = 0; i < STUDENTS_CNT; ++i) {
        students[i] = new Student(i, list);
        students[i]->start();
    }

    for (int i = 0; i < STUDENTS_CNT; ++i) {
        students[i]->join();
        delete students[i];
    }

    list.print(std::cout);

    return 0;
}

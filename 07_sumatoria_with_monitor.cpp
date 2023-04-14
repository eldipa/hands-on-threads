/*
   [1] Ejemplo de encapsulamiento de un objeto
   compartido y su mutex en un único objeto.

   Este objeto protegido se lo conoce como
   **monitor** (si, lo se, no es un nombre copado,
   de alguna forma quiere decir que el objeto
   monitorea los acceso al objeto compartido).

   El ejemplo debería imprimir por pantalla el
   número 479340.

   Para verificar que efectivamente no hay una
   race condition, correr esto:
     for i in {0..1000}
     do
        ./07_sumatoria_with_monitor.exe
     done | uniq

*/

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <exception>

#define N 10

// Misma clase Thread que en 03_is_prime_parallel_by_inheritance.cpp
#include "libs/thread.h"

class ResultProtected { // aka monitor
    private:

        /* [2] el monitor u objeto protegido tiene
           su mutex y tiene al objeto compartido
           que hay que proteger
        */
        std::mutex m;
        unsigned int result;

    public:
        ResultProtected(unsigned int v): result(v) {}

        /* [3] *** Importante ***
           Cada método "protegido" de un monitor
           debería ser una critical section

           Poner locks por todos lados ***NO** es
           una buena idea. Solo hara que las cosas
           se cuelguen y no funcionen

           En 08_monitor_interface_critical_section.cpp
           lo vamos a ver bien.
        */
        void inc(unsigned int s) {

            /*
              [4] En el ejemplo anterior 06_sumatoria_with_locks_raii.cpp
              implementamos un objeto RAII llamado Lock.

              Acá vamos a usar el provisto por C++, el
              std::unique_lock<std::mutex>.

              C++ provee otros mutexes como:

               - lock_guard: equivale a unique_lock y seria más seguro de usar
                 por q tiene una API publica más reducida.
                 Lo malo es q no lo podes usar con conditional_variables
                 (que las veremos pronto)
               - scoped_lock: te permite hacer lock sobre más de 1 mutex
                 a la vez.
                 99.9% de las veces q tengas q lockear más de 1 mutex es
                 por q tenes un serio problema en el diseño.

              En este caso podríamos usar lock_guard perfectamente
              pero preferí usar unique_lock por q es el mismo que usare
              luego en los ejemplos de conditional_variables.
            */
            std::unique_lock<std::mutex> lck(m);
            result += s;
        }

        unsigned int get_val() {
            std::unique_lock<std::mutex> lck(m);
            return result;
        }

};

class Sum : public Thread {
    private:
        unsigned int *start;
        unsigned int *end;

        /* [5] Una referencia al monitor:
           el objeto compartido y su mutex

           En general los objetos activos (hilos)
           no deberían tener referencias a mutexes
           ni manejarlos sino tener referencias a
           los monitores y que estos coordinen el
           acceso y protejan al recurso compartido
        */
        ResultProtected &result;

    public:
        Sum(unsigned int *start,
                unsigned int *end,
                ResultProtected &result) :
            start(start), end(end),
            result(result) {}

        virtual void run() override {
            unsigned int temporal_sum = 0;
            for (unsigned int *p = start; p < end; ++p) {
                temporal_sum += *p;
            }

            /* [6]  No nos encargamos de proteger
               el recurso compartido (unsigned int
               result) sino que el objeto protegido
               (monitor) de tipo ResultProtected sera
               el responsable de protegerlo.

               Encapsulamos toda la critical section
               CS en un único método del monitor.
            */
            result.inc(temporal_sum);
        }
};

int main() {
    unsigned int nums[N] = { 132131, 1321, 31371,
                             30891, 891, 123891,
                             3171, 30891, 891,
                             123891 };
    ResultProtected result(0);

    std::vector<Thread*> threads;

    for (int i = 0; i < N/2; ++i) {
        threads.push_back(new Sum(
                                    &nums[i*2],
                                    &nums[(i+1)*2],
                                    result
                                ));
    }

    for (int i = 0; i < N/2; ++i) {
        threads[i]->start();
    }

    for (int i = 0; i < N/2; ++i) {
        threads[i]->join();
        delete threads[i];
    }

    std::cout << result.get_val();  // 479340
    std::cout << "\n";

    return 0;
}
/* [7]

   Medita sobre [4] y [6]. La parte realmente complicada
   de trabajar con threads, mutexes y monitores es la descubrir
   las critical sections reales.

   En 08_monitor_interface_critical_section.cpp vamos
   a ver esto con otro ejemplo.
*/


/*
   [1] Ejemplo de RAII encapsulando la toma y
   liberación de un mutex: clase Lock

   El ejemplo debería imprimir por pantalla el
   número 479340.
     for i in {0..1000}
     do
        ./06_sumatoria_with_locks_raii.exe
     done | uniq

*/

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <exception>

#define N 10
#define ROUNDS 1

// Misma clase Thread que en 03_is_prime_parallel_by_inheritance.cpp
#include "libs/thread.h"

/* [2] Encapsulación RAII del recurso
   "mutex tomado"

   Como pueden ver, la memoria no es
   el único recurso que hay que liberar.

   C++11 ya ofrece el mismo objeto std::lock_guard
   pero mostramos esta implementación para
   que quede como ejemplo de como RAII puede
   servirnos para crear construcciones de alto
   nivel.
*/
class Lock {
    private:
        std::mutex &m;

    public:
        /* [3] En el constructor adquirimos el
           recurso:
                lockeamos el mutex
         */
        Lock(std::mutex &m) : m(m) {
            m.lock();
        }

        /* [4] En el destructor liberamos el
           recurso:
                deslockeamos el mutex
         */
        ~Lock() {
            m.unlock();
        }

        /* [5] No tiene sentido copiar locks,
           forzar a que no se pueda.
           y tampoco tiene mucho sentido moverlos
           (aunque es cuestionable)
        */
        Lock(const Lock&) = delete;
        Lock& operator=(const Lock&) = delete;
        Lock(Lock&&) = delete;
        Lock& operator=(Lock&&) = delete;

};

class Sum: public Thread {
    private:
        unsigned int *start;
        unsigned int *end;

        unsigned int &result;
        std::mutex &m;

    public:
        Sum(unsigned int *start,
                unsigned int *end,
                unsigned int &result,
                std::mutex &m) :
            start(start), end(end),
            result(result), m(m) {}

        virtual void run() override {
            unsigned int temporal_sum;
            for (int round = 0; round < ROUNDS; ++round) {
                temporal_sum = 0;
                for (unsigned int *p = start; p < end; ++p) {
                    temporal_sum += *p;
                }
            }

            Lock l(m);              // -+-
            result += temporal_sum; //  | esta es
                                    //  | la CS
                                    //  |
        }   // ---------------------------+-
            /* [6] el mutex es liberado aquí cuando
               la variable "l" es destruida por irse
               de scope.
               Liberación del mutex automática!!
            */
};

int main() {
    unsigned int nums[N] = { 132131, 1321, 31371,
                             30891, 891, 123891,
                             3171, 30891, 891,
                             123891 };
    unsigned int result = 0;
    std::mutex m;
    std::vector<Thread*> threads;

    for (int i = 0; i < N/2; ++i) {
        threads.push_back(new Sum(
                                    &nums[i*2],
                                    &nums[(i+1)*2],
                                    result,
                                    m
                                ));
    }

    for (int i = 0; i < N/2; ++i) {
        threads[i]->start();
    }

    for (int i = 0; i < N/2; ++i) {
        threads[i]->join();
        delete threads[i];
    }

    std::cout << result;     // 479340
    std::cout << "\n";

    return 0;
}
/* [7]

   Challenge: lanza una excepción en el método Sum::run.
   En un experimento lánzala *antes* de tomar el lock, en otra *después*
   de tomar el lock.

   Obviamente que la suma total ya no sera correcta pero, el programa
   se te colgó o no?

   Hace el mismo experimento pero con el código en 05_sumatoria_with_mutex.cpp:
   proba q pasa si se lanza una excepción *antes* de tomar el lock,
   *luego* de tomar el lock (pero *antes* de liberarlo) y que pasa si se lanza
   *luego* de liberarlo.

   En alguno de esos experimentos el programa se te colgara por que habrá threads
   que querrán tomar el lock y no podrán por que otro ya lo tomo y se olvido
   de liberarlo.

   Eso es un **deadlock**

   Ahora enteras por que se hace tanto **énfasis** en usar RAII:
   liberar la memoria **no** es lo único que hay que liberar.

   Has llegado al final del ejercicio, continua
   con el siguiente.
*/

/*
   [1]
   Para evitar que múltiples hilos accedan a un
   recurso compartido (variable/objeto) se usa un
   mecanismo de coordinación llamado Mutex
   (Mutual Exclusion)

   El ejemplo debería imprimir por pantalla el
   número 479340 siempre.

   Para verificar que efectivamente no hay una
   race condition, correr esto:
     for i in {0..10000}
     do
        ./05_sumatoria_with_mutex.exe
     done | uniq

*/

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <exception>

#define N 10
#define ROUNDS 1000000

// Misma clase Thread que en 03_is_prime_parallel_by_inheritance.cpp
#include "libs/thread.h"

class Sum: public Thread {
    private:
        unsigned int *start;
        unsigned int *end;

        unsigned int &result;


        /* [2] Referencia a un mutex:
           no hay que ni copiarlo ni moverlo.

           *** Importante ***
           Si un grupo de hilos va a compartir una
           variable/objeto (y por ende la posibilidad
           de una race condition), los hilos deben
           coordinar entre ellos el acceso a dicha
           variable.
           Para ello deben compartir el mismo
           objeto mutex
        */
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


            /* [3] Tomamos (adquirimos) el mutex.
               Cualquier otro hilo (incluido el
               nuestro) que quiera tomar este mutex
               se bloqueara hasta que nosotros
               llamemos a unlock y lo liberemos.
            */
            m.lock();               // --+-
                                    //   | solo un
            result += temporal_sum; //   | hilo a
                                    //   | la vez:
                                    //   |   CS
            /* [4] Liberamos el mutex para
               que otros hilos lo puedan
               tomar y entrar a la region
               critica (CS).        //   |
            */                      //   |
            m.unlock();             // --+-
        }
};

int main() {
    unsigned int nums[N] = { 132131, 1321, 31371,
                             30891, 891, 123891,
                             3171, 30891, 891,
                             123891 };
    unsigned int result = 0;

    /* [5] Un único mutex; No un mutex por hilo

       [6] Hay otras variantes de mutexes como
       recursive_mutex y timed_mutex que pueden
       resultar "tentadoramente más fáciles y
       convenientes" pero que pueden en realidad
       enmascarar un mal diseño detrás de escena

       No usarlas a menos que no haya otra
       alternativa.
    */
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

   Extra challenges:

    - En [3] proba en mover el `m.lock()` al **principio** del método run.
      Deberías seguir sin una RC **pero** vas a ver q todo funciona más lento.

      Probar en medir los tiempos con `time`. Fíjate el tiempo "real".
      (nota: si no ves mucha diferencia podes probar en aumentar el valor
       de ROUNDS para que se note)

      Cuanto más grande sea la zona cubierta por un lock y cuantos
      más threads **compitan por adquirir el lock**,
      más se van a trabar los threads y menos concurrente va a ser
      el procesamiento (cada vez se parecerá a un sistema secuencial).

      En estos casos se dice que hay **contention** sobre el lock
      o sobre el recurso compartido.

    - En vez de crear un único mutex en [5] y quedarse
      con una referencia en [2], proba en tener un mutex **propio**
      en cada functor (en [2]).

      Te sigue funcionando o volvieron las RCs?

   Has llegado al final del ejercicio, continua
   con el siguiente.
*/

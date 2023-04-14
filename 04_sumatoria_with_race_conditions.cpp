/*
   [1] Ejemplo de una race condition: accesos de
   lecto/escritura a una misma variable u objeto
   por múltiples hilos que terminan dejando a la
   variable/objeto en un estado inconsistente

   Este ejemplo calcula una suma y el resultado final
   debería ser 479340.

   Como siempre se suman los mismos números el
   resultado final 479340 debería ser siempre el
   mismo pero debido a la race condition,
   el resultado puede variar.

   Para tratar de ver el bug (puede ser difícil de
   triggerearlo), correr esto en la consola:
     for i in {0..10000}
     do
        ./04_sumatoria_with_race_conditions.exe;
     done | uniq

   Ese código bash corre el program 10000 veces y se
   queda con los valores únicos (podrás frenarlo antes con Ctrl-C)

   Si no hubiera RC siempre deberías ver el mismo valor (479340)
   una única vez pero veras q no.

   Nota: triggerear la RC es básicamente por azar, probar varias
   veces!

*/

#include <iostream>
#include <vector>
#include <thread>
#include <exception>

#define N 10
#define ROUNDS 1

// Misma clase Thread que en 03_is_prime_parallel_by_inheritance.cpp
#include "libs/thread.h"


class Sum: public Thread {
    private:
        unsigned int *start;
        unsigned int *end;

        unsigned int &result;

    public:
        Sum(unsigned int *start,
                unsigned int *end,
                unsigned int &result) :
            start(start), end(end), result(result) {}

        virtual void run() override {
            /* [2]
             * Sumo un subconjunto de números:
             *
             *        /-- start            /-- end
             *       V                    V
             * - - --+--+--+--+--+--+--+--+--- - -
             *   : ::|nn|mm|nn|nn|nn|nn|mm|::  :
             * - - --+--+--+--+--+--+--+--+--- - -
             * */
            unsigned int temporal_sum;
            for (int round = 0; round < ROUNDS; ++round) {
                /*
                 * Nota: los sumo muchas veces (ROUNDS veces) solo
                 * para poder correr el thread mucho y poder
                 * mostrar fácilmente *race conditions*, *contention*
                 * y otras yerbas.
                 * */
                temporal_sum = 0;
                for (unsigned int *p = start;
                        p < end;
                        ++p) {
                    temporal_sum += *p;
                }
            }

            /* [3] acá esta la race condition:
               múltiples instancias del functor
               Sum corriendo, cada uno, el método
               "run" en threads en paralelo

               Todos **escribiendo** a la variable
               **compartida** "result" con escrituras
               **no-atómicas**.

               Esta línea es la *** critical section ***
               que habría que proteger y evitar
               que múltiples hilos la accedan concurrentemente
            */
            result += temporal_sum;
        }
};

int main() {
    unsigned int nums[N] = { 132131, 1321, 31371,
                             30891, 891, 123891,
                             3171, 30891, 891,
                             123891 };
    unsigned int result = 0;

    std::vector<Thread*> threads;

    for (int i = 0; i < N/2; ++i) {
        /* [4] Nótese como cada hilo tiene acceso a la
           **misma** variable "result" y que cada
           hilo **leera y modificara la misma
           variable**

           Esta variable es un **recurso compartido**
        */
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

    std::cout << result;     // 479340 ??
    std::cout << "\n";

    return 0;
}
/* [5]

   Compila el código con g++ y el flag -fsanitize=thread (thread sanitize)

   g++ -fsanitize=thread -std=c++11 -ggdb -pedantic -Wall -o  \
         04_sumatoria_with_race_conditions_tsan.exe     \
         04_sumatoria_with_race_conditions.cpp          \
         -pthread

   Ahora correrlo con la variable de entorno TSAN_OPTIONS='halt_on_error=1'

     for i in {0..10000}
     do
        TSAN_OPTIONS='halt_on_error=1' ./04_sumatoria_with_race_conditions_tsan.exe;
     done | uniq

   TSAN instrumenta tu binario para detectar race conditions. Hace a tu programa
   *muy* lento y solo detecta la RC cuando esta se produce (o sea q tenes
   q probar el código *muchas* veces).

   No te asustes si no entendes el error de TSAN, lo importante es que lo podes
   usar como heurística para saber si hay RCs o no y con algo de suerte podrás
   ver hasta el donde esta la RC.

   Ante una RC *siempre* busca que objetos son compartidos, y los métodos
   pues son la entrada a las RCs.

   En los siguientes ejercicios vamos a profundizar sobre esto.

   Has llegado al final del ejercicio, continua
   con el siguiente.
*/

/*
   [1] Ejemplo de una race condition: accesos de 
   lecto/escritura a una misma variable u objeto
   por multiples hilos que terminan dejando a la 
   variable/objeto en un estado inconsistente
  
   Compilar con 
     g++ -std=c++11 -pedantic -Wall              \
        -o 04_sumatoria_with_race_conditions.exe \
        04_sumatoria_with_race_conditions.cpp    \
        -pthread
  
   Este ejemplo calcula una suma y el resultado final
   deberia ser 479340.
   Como siempre se suman los mismos numeros el 
   resultado final 479340 deberia ser siempre el 
   mismo pero debido a la race condition, 
   el resultado puede variar.
  
   Para tratar de ver el bug (puede ser dificil de
   triggerearlo), correr esto en la consola:
     for i in {0..1000}
     do
        ./04_sumatoria_with_race_conditions.exe; 
     done | uniq
  
*/

#include <iostream>
#include <vector>
#include <thread>

#define N 10

class Thread {
    private:
        std::thread thread;
 
    public:
        Thread () {}

        void start() {
            thread = std::thread(&Thread::run, this);
        }

        void join() {
            thread.join();
        }

        virtual void run() = 0;
        virtual ~Thread() {}

        Thread(const Thread&) = delete;
        Thread& operator=(const Thread&) = delete;

        Thread(Thread&& other) {
            this->thread = std::move(other.thread);
        }

        Thread& operator=(Thread&& other) {
            this->thread = std::move(other.thread);
            return *this;
        }
};


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

        virtual void run() {
            /* [2]
             * Sumo un subconjunto de numeros:
             *
             *        /-- start            /-- end
             *       V                    V
             * - - --+--+--+--+--+--+--+--+--- - -
             *   : ::|nn|mm|nn|nn|nn|nn|mm|::  :
             * - - --+--+--+--+--+--+--+--+--- - -
             * 
             * */
            unsigned int temporal_sum = 0;
            for (unsigned int *p = start;
                    p < end; 
                    ++p) {
                temporal_sum += *p;
            }

            /* [3] aca esta la race condition:
               multiples instancias del functor 
               Sum corriendo, cada uno, el metodo 
               "run" en threads en paralelo
              
               Todos escribiendo a la variable 
               compartida "result". 
              
               Es esta linea la *** critical section ***
               que habria que proteger y evitar 
               que multiples hilos en paralelo la 
               accedan en simultaneo
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
        /* [4] Notese como cada hilo tiene acceso a la
           **misma** variable "result" y que cada 
           hilo **leera y modificara la misma 
           variable**
           Esto es una race condition
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
   Has llegado al final del ejercicio, continua 
   con el siguiente.
*/

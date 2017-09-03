/*
   [1] Ejemplo de RAII encapsulando la toma y 
   liberacion de un mutex: clase Lock
  
   Compilar con 
        g++ -std=c++11 -pedantic -Wall           \
            -o 06_sumatoria_with_locks_raii.exe  \
            06_sumatoria_with_locks_raii.cpp     \
            -pthread
  
   El ejemplo deberia imprimir por pantalla el 
   numero 479340.
     for i in {0..1000}
     do
        ./06_sumatoria_with_locks_raii.exe
     done | uniq
  
*/

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

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

/* [2] Encapsulacion RAII del recurso 
   "mutex tomado" 
   
   Como pueden ver, la memoria no es
   el unico recurso que hay que liberar.
  
   C++11 ya ofrece el mismo objeto std::lock_guard
   pero mostramos esta implementacion para
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

        virtual void run() {
            unsigned int temporal_sum = 0;
            for (unsigned int *p = start; p < end; ++p) {
                temporal_sum += *p;
            }

            Lock l(m);              // -+- 
            result += temporal_sum; //  | esta es
                                    //  | la CS
                                    //  |
        }   // ---------------------------+- 
            /* [6] el mutex es liberado aqui cuando 
               la variable "l" es destruida por irse
               de scope. 
               Liberacion del mutex automatica!!
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
   Has llegado al final del ejercicio, continua 
   con el siguiente.
*/

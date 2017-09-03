/*
   [1] 
   Para evitar que multiples hilos accedan a un 
   recurso compartido (variable/objeto) se usa un 
   mecanismo de coordinacion llamado Mutex 
   (Mutal Exclusion)
  
   Compilar con 
        g++ -std=c++11 -pedantic -Wall      \
            -o 05_sumatoria_with_mutex.exe  \
            05_sumatoria_with_mutex.cpp     \
            -pthread
  
   El ejemplo deberia imprimir por pantalla el 
   numero 479340 siempre.
  
   Para verificar que efectivamente no hay una 
   race condition, correr esto:
     for i in {0..1000}
     do
        ./05_sumatoria_with_mutex.exe
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

        virtual void run() {
            unsigned int temporal_sum = 0;
            for (unsigned int *p = start; p < end; ++p) {
                temporal_sum += *p;
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

    /* [5] Un unico mutex; No un mutex por hilo
      
       [6] Hay otras variantes de mutexes como 
       recursive_mutex y timed_mutex que pueden 
       resultar "tentadoramente mas faciles y 
       convenientes" pero que pueden en realidad 
       enmascarar un mal diseño detras de escena
      
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
   Has llegado al final del ejercicio, continua 
   con el siguiente.
*/

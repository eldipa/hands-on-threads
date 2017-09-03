/*
   [1] Ejemplo de encapsulamiento de un objeto 
   compartido y su mutex en un unico objeto.
   
   Este objeto protegido se lo conoce como 
   **monitor** (si, lo se, no es un nombre copado,
   de alguna forma quiere decir que el objeto 
   monitorea los acceso al objeto compartido).
  
   Compilar con 
        g++ -std=c++11 -pedantic -Wall        \
            -o 07_sumatoria_with_monitor.exe  \
            07_sumatoria_with_monitor.cpp     \
            -pthread
  
   El ejemplo deberia imprimir por pantalla el 
   numero 479340.
  
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

class Lock {
    private:
        std::mutex &m;

    public:
        Lock(std::mutex &m) : m(m) {
            m.lock();
        }

        ~Lock() {
            m.unlock();
        }

    private:
        Lock(const Lock&) = delete;
        Lock& operator=(const Lock&) = delete;
        Lock(Lock&&) = delete;
        Lock& operator=(Lock&&) = delete;

};

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
           Cada metodo "protegido" de un monitor
           deberia ser una critical section
          
           Poner Locks por todos lados ***NO** es 
           una buena idea. Solo hara que las cosas
           se cuelguen y no funcionen
        */
        void inc(unsigned int s) {
            Lock l(m);
            result += s;
        }

        unsigned int get_val() {
            Lock l(m);
            return result;
        }

};

class Sum : public Thread {
    private:
        unsigned int *start;
        unsigned int *end;
        
        /* [4] Una referencia al monitor: 
           el objeto compartido y su mutex
          
           En general los objetos activos (hilos) 
           no deberian tener referencias a mutexes
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

        virtual void run() {
            unsigned int temporal_sum = 0;
            for (unsigned int *p = start; p < end; ++p) {
                temporal_sum += *p;
            }

            /* [5]  No nos encargamos de proteger
               el recurso compartido (unsigned int
               result) sino que el objeto protegido 
               (monitor) de tipo ResultProtected sera
               el responsable de protegerlo.

               Encapsulamos toda la critical section
               CS en un unico metodo del monitor.
            */ 
            result.inc(temporal_sum); 
        }
        
        // ***Pregunta***: Usando un monitor con todos sus metodos protegidos evitan una race condition siempre?
        // ***Respuesta***: NO, es COMPLETAMENTE FALSO
        //
        // ***Demostracion***:
        //
        // // sea la siguiente funcion que se ejecuta varias veces en paralelo
        // // con result_protected el mismo objeto compartido por los hilos
        // void run() {
        //    int v = result_protected.get_val();
        //    if (v == 0) {
        //      result_protected.inc(1);
        //    }
        // }
        //
        // // si result_protected es inicializado a 0 antes de arrancar, el resultado
        // // esperado al finalizar el programa es que result_protected valga 1
        // // sin embargo no necesariamente !!!
        //
        // // Para verlo veamos a la funcion f en donde pondremos inline el codigo del monitor:
        // void run() {
        //    result_protected.mutex.lock();
        //    int v = result_protected.result;
        //    result_protected.mutex.unlock();
        //
        //    if (v == 0) {
        //      result_protected.mutex.lock();
        //      result_protected.result += 1;
        //      result_protected.mutex.unlock();
        //    }
        // }
        //
        // // se ve que hay dos regiones criticas en vez de una? Al final hay una race condition
        //
        // ***Solucion***: 
        // void run() {
        //    result_protected.inc_only_if_you_are_in_zero(1); 
        // }
        //
        // void ResultProtected::inc_only_if_you_are_in_zero(unsigned int s) {
        //    this->mutex.lock();
        //    if (this->result == 0) {
        //       this->result += s;
        //    }
        //    this->mutex.unlock();
        // }
        //
        // ***Conclusion***: Metodos protegidos MAS una buena interfaz del monitor diseñada para resolver el problema
        // son los que nos evitan las race condition.
        //
        //      *** Se deben encontrar primero las regiones criticas y para ***
        //      *** cada region critica se debe implementar un metodo en el ***
        //      *** monitor.                                                ***
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
/* [6]
   Has llegado al final del ejercicio, continua 
   con el siguiente.
*/


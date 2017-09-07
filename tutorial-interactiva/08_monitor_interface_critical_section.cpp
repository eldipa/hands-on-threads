/*
   [1] Ejemplo de encapsulamiento de un objeto 
   compartido y su mutex en un unico objeto.
   
   Este objeto protegido se lo conoce como 
   **monitor** (si, lo se, no es un nombre copado,
   de alguna forma quiere decir que el objeto 
   monitorea los acceso al objeto compartido).
  
   Compilar con 
 g++ -std=c++11 -pedantic -Wall                  \
   -o 08_monitor_interface_critical_section.exe  \
   08_monitor_interface_critical_section.cpp     \
   -pthread
  
   El ejemplo deberia imprimir por pantalla el 
   numero 1 ya que si bien hay varios numeros
   primos, solo queremos si hay (1) primos o no (0)
  
   Para verificar que efectivamente no hay una 
   race condition, correr esto:
    for i in {0..1000}
    do
       ./08_monitor_interface_critical_section.exe
    done | uniq

   Funciono?
  
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

           En este caso, inc no es la critical
           section
        */
        void inc(unsigned int s) {
            Lock l(m);
            result += s;
        }

        unsigned int get_val() {
            Lock l(m);
            return result;
        }

        /* [5] Nuestras critical condition son
           get_val y inc_if_you_are_zero
          
           Descomentar la siguiente implementacion
           y *borrar* el metodo inc:
           
           Jamas implementen un metodo protegido
           que no represente a una critical section
           Alguien desprevenido podria llegar usar
           a inc pensado que es segura cuando no
           lo es -> esto es *importante*
          */
        /*
        void inc_if_you_are_zero(unsigned int s) {
            Lock l(m);
            if (result == 0) {
                result += s;
            }
        }
        */

};

class AreAnyPrime : public Thread { 
    private:
        unsigned int n;
        ResultProtected &result; 

    public:
        AreAnyPrime(unsigned int n, 
                ResultProtected &result) : 
            n(n), 
            result(result) {}
        
        virtual void run() override {
            /* Si ya encontramos un numero primo, 
             * salimos */
            if (result.get_val() >= 1) 
                return;

            /* Y si no, vemos si nuestro numero es
             * primo o no. */
            for (unsigned int i = 2; i < n; ++i) {
                if (n % i == 0) {
                    return;
                }
            }

            /* Y si el numero es primo, incrementamos
             * el contador.... */

            /* [2] Es este inc correcto? Es nuestra
               critical section?
              
               La respuesta es no: queremos 
               incrementar solo si el contador es
               0.
              
               Si preguntamos con get_val y luego
               incrementamos con inc, no estamos
               haciendo una unica operacion atomica
               sino 2 y esto abre la posibilidad a
               una data race condition.
            */
            result.inc(1);

            /* [6] Borrar la linea "result.inc(1)"
               y reemplazarla por la llamada a
               inc_if_you_are_zero
            */
            /* result.inc_if_you_are_zero(1); */
        }
};


int main() {
    unsigned int nums[N] = { 132131, 132130891, 31371, 
                             132130891, 891, 123891, 
                             132130891, 132130891,
                             132130891 };
    ResultProtected result(0);
 
    std::vector<Thread*> threads;

    for (int i = 0; i < N; ++i) {
        threads.push_back(new AreAnyPrime(
                            nums[i], 
                            result
                        ));
    }
    
    for (int i = 0; i < N; ++i) {
        threads[i]->start();
    }

    for (int i = 0; i < N; ++i) {
        threads[i]->join();
        delete threads[i];
    }
    
    std::cout << result.get_val();  // 1
    std::cout << "\n";

    return 0;
}
/* [7]
  
   Recompilar y volver a probar para verificar que
   la race condition fue removida
  
   Conclusion:
   Metodos protegidos MAS una buena interfaz del 
   monitor diseñada para resolver el problema
   son los que nos evitan las race condition.
  
   Se deben encontrar primero las regiones criticas 
   y para cada region critica se debe implementar
   un metodo en el monitor protegido con un mutex.

   Has llegado al final del ejercicio, y al final
   de este tutorial.

*/


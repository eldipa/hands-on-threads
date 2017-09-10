/*
   [1]
   Ejemplo de como ejecutar una funcion/functor en
   un hilo separado en C++
  
   Esta vez, en vez de usar composicion usaremos 
   herencia.
   Para ello crearemos una objeto Thread que 
   ejecutara un metodo virtual en su  propio hilo 
   definido por las clases hijas que hereden de 
   Thread
  
   Cuando el objeto functor encapsula dentro de él
   el concepto de hilo se dice que el objeto es un
   "objeto activo".
  
   Compilar con 
   g++ -std=c++11 -pedantic -Wall                \
     -o 03_is_prime_parallel_by_inheritance.exe  \
     03_is_prime_parallel_by_inheritance.cpp     \
     -pthread
  
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
            /* [2] Lanzamos el thread que correra 
               siempre la misma funcion (Thread::run)
               
               Como Thread::run es un **metodo** 
               sin parametros y std::thread espera
               a una **funcion** podemos ver a 
               Thread::run como una funcion que 
               recibe como primer argumento al 
               objeto this (tal como en los TDA de C!)
              
               std::thread soporta correr una 
               funcion con argumentos con la llamada:

                  std::thread( funcion, arg1, arg2, ...)
              
               Por lo tanto
                    std::thread( metodo, this )

               es equivalante a correr el metodo 
               sin argumentos en un thread.
              
               Por ser polimorfico el metodo 
               Thread::run de cada objeto ejecutara 
               un codigo particular en el thread.
              
               Objetos distintos podran correr en 
               sus propios threads con esta unica 
               implementacion de Thread mientras
               hereden de Thread y creen sus propias 
               versiones del metodo run.
            */
            thread = std::thread(
                                    &Thread::run, 
                                    this
                                );
        }

        void join() {
            thread.join();
        }

        /* [3] Virtual puro para forzar una
           definicion en las clases hijas
        */
        virtual void run() = 0; 
 

        /* [4] Destructor virtual: sSiempre hacerlo 
           virtual si pensamos en usar herencia.
        */ 
        virtual ~Thread() {} 
                             
                             

        /* [5] No tiene sentido copiar hilos, asi 
           que forzamos a que no se puedan copiar.
        */
        Thread(const Thread&) = delete;
        Thread& operator=(const Thread&) = delete;
        
        /* [6] Pero si tiene sentido que un hilo 
           pueda moverse asi que implementamos su 
           constructor y operador asigancion por 
           movimiento
        */
        Thread(Thread&& other) {
            /*
               [7] Explicitamente decimos: 
                    "move el hilo other.thread 
                    hacia this->thread pero no 
                    copies"

            */
            this->thread = std::move(other.thread);
        }


        Thread& operator=(Thread&& other) {
            this->thread = std::move(other.thread);
            return *this;
        }

};


/* [8] Esto es lo que se conoce como un 
        "objeto activo". 
  
   Un objeto que tiene sus atributos y su logica 
   (que encapsula un algoritmo o una tarea) pero 
   que vive en su propio hilo
*/
class IsPrime : public Thread { 
    private:
        unsigned int n;
        bool &result;

    public:
        IsPrime(unsigned int n, bool &result) : 
            n(n), 
            result(result) {}
        
        virtual void run() override {
            for (unsigned int i = 2; i < n; ++i) {
                if (n % i == 0) {
                    result = false;
                    return;
                }
            }

            result = true;
        }
};



int main() {
    unsigned int nums[N] = { 0, 1, 2, 132130891, 
                            132130891, 4, 13, 
                            132130891, 132130891, 
                            132130871 };
    bool results[N]; 
 
    std::vector<Thread*> threads;

    for (int i = 0; i < N; ++i) {
        /* [9] Aca es donde creamos nuestros objetos */
        Thread *t = new IsPrime(
                            nums[i], 
                            results[i]);

        /* [10] y aca los "activamos" 
           (lanzamos un thread)
        */
        t->start();
        threads.push_back(t);
    }

    /* ************************************** */
    /* Ahora: Todos los hilos estan corriendo */
    /* ************************************** */

    /* [11] Esperamos a que cada hilo termine.
       Cada join bloqueara al hilo llamante (main)
       hasta que el hilo sobre el cual se le hace 
       join (threads[i]) termine
      
       Ademas, por haber usado el heap debemos 
       hacer el correspondiente delete
    */
    for (int i = 0; i < N; ++i) {
        threads[i]->join();
        delete threads[i];
    }

    /* **************************************** */
    /* Ahora: Todos los hilos terminaron y sus  */
    /* recursos limpiados con el join           */
    /* **************************************** */

    for (int i = 0; i < N; ++i) {
        std::cout << results[i] << " ";
    }
    std::cout << "\n";

    return 0;
}

/* [12]
   Has llegado al final del ejercicio, continua 
   con el siguiente.
*/


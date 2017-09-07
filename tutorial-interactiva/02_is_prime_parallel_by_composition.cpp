/*
   [1]
   Ejemplo de como ejecutar una funcion/functor en
   un hilo separado en C++
  
   Se ejecutan varios functors en paralelo en donde 
   el objeto thread tiene una referencia al objeto
   functor (usamos composicion)
  
   Compilar con 
   g++ -std=c++11 -pedantic -Wall               \
     -o 02_is_prime_parallel_by_composition.exe  \
     02_is_prime_parallel_by_composition.cpp     \
     -pthread
  
*/

#include <iostream>
#include <vector>
#include <thread>

#define N 10

class IsPrime { 
    private:
        unsigned int n;
        bool &result;

    public:
        IsPrime(unsigned int n, bool &result) : 
            n(n), 
            result(result) {}
        
        void operator()() {
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
 
    std::vector<std::thread> threads;

    for (int i = 0; i < N; ++i) {
        /* [2] Aca es donde usamos composicion donde 
                "std::thread tiene un IsPrime"
          
           El operator call del functor se ejecutara 
           en el hilo lanzado por el constructor del 
           objeto std::thread
           
           Es por esta razon que tuvimos que 
           sobrecargar el operador call, para que 
           std::thread sepa que que correr.
        */
        threads.push_back(std::thread { 
                            IsPrime(nums[i], 
                                    results[i]) 
                            });
    }

    /* ************************************** */
    /* Ahora: Todos los hilos estan corriendo */
    /* ************************************** */

    /* [3] Esperamos a que cada hilo termine.
       Cada join bloqueara al hilo llamante (main)
       hasta que el hilo sobre el cual se le hace 
       join (threads[i]) termine

       Siempre es necesario hacer un join para
       liberar los recursos. No hacer un join
       implica leaks (solo en muy exoticoss y
       mas que justificados casos se puede 
       precindir de un join).
    */
    for (int i = 0; i < N; ++i) {
        threads[i].join();
    }

    /* ********************************** */
    /* Ahora: Todos los hilos terminaron  */
    /* ********************************** */

    for (int i = 0; i < N; ++i) {
        std::cout << results[i] << " ";
    }
    std::cout << "\n";

    return 0;
}

/* [4]
   Corre el ejecutable con "time":
    time ./02_is_prime_parallel_by_composition.exe
  
   Compara los tiempos con la ejecucion de
   01_is_prime_sequential.exe

   Mejoro el tiempo "real"? y el "user"?

   Has llegado al final del ejercicio, continua 
   con el siguiente.
*/


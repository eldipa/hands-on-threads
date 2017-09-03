/* [1]
   Ejemplo de un functor: la encapsulacion de una 
   funcion o algoritmo en un objeto.
   Para darle una sintaxis copada (y para que sea 
   compatible con la libreria estandar de C++) 
   vamos a sobrecargarle el operador call.

   Compilar con 
      g++ -std=c++11 -pedantic -Wall      \
            -o 01_is_prime_sequential.exe \
            01_is_prime_sequential.cpp
*/


#include <iostream>
#define N 10

/* [2]  Functor: una funcion hecha objeto */
class IsPrime {
    private:
        unsigned int n;
        bool &result;

    public:
        /* [3] Un functor permite desacoplar el 
           pasaje de los paramentros de la 
           ejecucion de la funcion/algoritmo.
          
           En este caso, el functor recibe 2 parametros:
            - n, el numero a determinar is es o no primo
            - result, donde guardar el resultado
        */ 
        IsPrime(unsigned int n, bool &result) : 
            n(n), 
            result(result) {}
        
        /* [4] El algoritmo para saber si un numero 
           es primo o no (version simplificada)
           Notese como el algoritmo no recibe ningun 
           parametro explicito sino que estos fueron 
           pasados por el constructor.
        */ 
        void run() {
            for (unsigned int i = 2; i < n; ++i) {
                if (n % i == 0) {
                    result = false;
                    return;
                }
            }

            result = true;
        }
        
        /* [5]
           Sobrecarga del operator call. Esto 
           permite invocar a IsPrime con la misma 
           sintaxis que se invoca a una funcion. 
          
           Ej:
          
           IsPrime f(n, r); // <- instancio el objeto
           f();             // <- lo llamo como si
                            //    fuera una funcion
          
           Aunque pueda parecer solo syntax sugar,
           la libreria estandar de C++ espera esta
           sintaxis en algunos casos.
        */ 
        void operator()() {
            this->run(); /* [6] 
                            podriamos haber puesto
                            el codigo de 
                            IsPrime::run aqui directamente
                         */
        }
        
};



int main() {
    unsigned int nums[N] = { 0, 1, 2, 132130891, 
                            132130891, 4, 13, 
                            132130891, 132130891, 
                            132130871 };
    bool results[N]; 

    for (int i = 0; i < N; ++i) {
        /* [7] 
           Creamos un functor (function object) 
           con los argumentos de la funcion pero 
           esta no se invoca aqui
        */
        IsPrime is_prime = IsPrime(nums[i], 
                                    results[i]);

        /* [8] 
           Recien aqui se invoca a la funcion 
           "is prime".
          
           Los functors permiten retrasar las 
           llamadas a funciones: el pasaje de 
           argumentos se desacopla de la invocacion 
           del algoritmo.
        */
        is_prime();  // o equivalentemente a 
                     // is_prime.run();
    }


    for (int i = 0; i < N; ++i) {
        std::cout << results[i] << " ";
    }
    std::cout << "\n";

    return 0;
}

/* [9]
   Corre el ejecutable con "time":
    time ./01_is_prime_sequential.exe
  
   Que significan esas mediciones?:
     real 
     user 
     sys

   Lee la pagina de manual con:
     man time

   Has llegado al final del ejercicio, continua 
   con el siguiente.
*/



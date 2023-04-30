/*
   [1]
   Ejemplo de como ejecutar una función/functor en
   un hilo separado en C++

   Se ejecutan varios functors en paralelo en donde
   el objeto thread tiene una referencia al objeto
   functor (usamos composición)
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

    // [4]
    // Notar q estamos usando un *vector* de std::thread
    //
    // Esto esta bien por que los functors IsPrime no
    // tienen punteros a si mismo (this) y por ende
    // no seran afectados cuando el vector se resizee
    // y se mueva.
    //
    // No lo ves? No pasa nada!! En el proximo ejemplo
    // lo vas a ver mejor.
    //
    // Toma nota!
    std::vector<std::thread> threads;

    for (int i = 0; i < N; ++i) {
        /* [2] Acá es donde usamos composición donde
                "std::thread tiene un IsPrime"

           El operator call del functor se ejecutara
           en el hilo lanzado por el constructor del
           objeto std::thread

           std::thread recibe una **función** pero
           en la practica, como las funciones no tienen
           estados, es mucho más **útil** pasarle
           un **functor**

           Es por esta razón que tuvimos que
           sobrecargar el operador call de nuestro functor:
           para que std::thread sepa que correr.
        */
        threads.push_back(std::thread {
                            IsPrime(nums[i],
                                    results[i])
                            });
    }

    /* ************************************** */
    /* Ahora: Todos los hilos están corriendo */
    /* ************************************** */

    /* [3] Esperamos a que cada hilo termine.
       Cada join bloqueara al hilo llamante (main)
       hasta que el hilo sobre el cual se le hace
       join (threads[i]) termine

       Siempre es necesario hacer un join para
       liberar los recursos. No hacer un join
       implica leaks (solo en muy exóticos y
       más que justificados casos se puede
       prescindir de un join).
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

/* [5]
   Corre el ejecutable con "time":
    time ./02_is_prime_parallel_by_composition.exe

   Compara los tiempos con la ejecución de
   01_is_prime_sequential.exe

   Mejoro el tiempo "real"? y el "user"?

   Has llegado al final del ejercicio, continua
   con el siguiente.
*/


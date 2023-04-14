/* [1]

   Antes de arrancar con threads vamos a ver el concepto de
   "functor": la encapsulación de una
   función o algoritmo en un objeto.

   Para darle una sintaxis copada (y para que sea
   compatible con la librería estándar de C++)
   vamos a sobrecargarle el operador call.
*/


#include <iostream>
#define N 10

/* [2]  Functor: una función hecha objeto */
class IsPrime {
    private:
        unsigned int n;
        bool &result;

    public:
        /* [3] Un functor permite desacoplar el
           pasaje de los parámentros de la
           ejecución de la función/algoritmo.

           En este caso, el functor recibe 2 parámetros:
            - n, el número a determinar si es o no primo
            - result, donde guardar el resultado
        */
        IsPrime(unsigned int n, bool &result) :
            n(n),
            result(result) {}

        /* [4] El algoritmo para saber si un número
           es primo o no (version simplificada)

           Nótese como el algoritmo **no** recibe ningún
           parámetro explicito sino que estos fueron
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
           permite llamar a IsPrime con la misma
           sintaxis que se invoca a una función.

           Ej:

           IsPrime f(n, r); // <- instancio el objeto

           f();             // <- lo llamo como si
                            //    fuera una función

           Aunque pueda parecer solo syntax sugar,
           la librería estándar de C++ espera esta
           sintaxis en algunos casos.
        */
        void operator()() {
            this->run(); /* [6]
                            podríamos haber puesto
                            el código de
                            IsPrime::run aquí directamente
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
           con los argumentos de la función pero
           esta no se invoca aquí
        */
        IsPrime is_prime = IsPrime(nums[i],
                                    results[i]);

        /* [8]
           Recién aquí se invoca a la función
           "is prime".

           Los functors permiten retrasar las
           llamadas a funciones: el pasaje de
           argumentos se desacopla de la invocación
           del algoritmo.

           En este caso le pasamos todos los parámetros
           al constructor y ninguno a la llamada.
           En otros casos tal vez quieras pasar algunos
           en el constructor y otros en la llamada.
        */
        is_prime();  // <- equivale a is_prime.run();
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

   Lee la página de manual con:
     man time

   Functors es un tópico ligeramente exótico presente
   en lenguajes como C++ y Java q **no** ven a las funciones/métodos
   como objetos puros.

   El functor encapsula dicha función/método.

   Lenguajes como Python ven a las función/métodos como objetos directamente
   y el concepto de functor es menos explicito.

   Has llegado al final del ejercicio, continua
   con el siguiente.
*/



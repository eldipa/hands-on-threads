/*
   [1]

   Un objeto compartido + mutex no alcanzan:
   uno tiene que diseñar los métodos públicos del objeto
   como las critical sections y protegerlas.

   Solo así tendrás un *monitor*

   Este ejemplo debería imprimir por pantalla el
   número 1 ya que si bien hay varios números
   primos, solo queremos si hay (1) primos o no (0)

   Para verificar que efectivamente no hay una
   race condition, correr esto:
    for i in {0..1000}
    do
       ./08_monitor_interface_critical_section.exe
    done | uniq

   Funcionó??

   ~~~

   El bug esta en [2] y [3].
   El fix lo veras en [4] y [5]

*/

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <exception>

#define N 10

// Misma clase Thread que en 03_is_prime_parallel_by_inheritance.cpp
#include "libs/thread.h"

class ResultProtected { // aka monitor
    private:
        std::mutex m;
        unsigned int result;

    public:
        ResultProtected(unsigned int v): result(v) {}

        /* [3] *** Importante ***
           Cada método "protegido" de un monitor
           debería ser una critical section

           Poner locks por todos lados ***NO** es
           una buena idea. Solo hara que las cosas
           se cuelguen y no funcionen

           Esto ya te lo dije en 07_sumatoria_with_monitor.cpp
           y aquí vas a ver un caso explicito.

           En este caso, inc() **no** es la critical
           section real así que a pesar de protegerla
           con un lock vamos a tener una race condition igual.
        */
        void inc(unsigned int s) {
            std::unique_lock<std::mutex> lck(m);
            result += s;
        }

        unsigned int get_val() {
            std::unique_lock<std::mutex> lck(m);
            return result;
        }

        /* [4] Nuestras critical sections son
           get_val y inc_if_you_are_zero

           Descomentar la siguiente implementación
           y *borrar* el método inc():

           Jamas implementen un método protegido
           que no represente a una critical section
           Alguien desprevenido podría llegar usar
           a inc() pensado que es segura cuando no
           lo es -> esto es *importante*

           Si queres descubrir las critical sections
           siempre pensá "que cosas quiere hacer como
           un todo".

           En nuestro caso queremos incrementar
           solo si el contador esta en 0.
           Queremos "checkear e incrementar" de una
           forma atómica.
          */
        /*
        void inc_if_you_are_zero(unsigned int s) {
            std::unique_lock<std::mutex> lck(m);
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
            /* Si ya encontramos un número primo,
             * salimos */
            if (result.get_val() >= 1)
                return;

            /* Y si no, vemos si nuestro número es
             * primo o no. */
            for (unsigned int i = 2; i < n; ++i) {
                if (n % i == 0) {
                    return;
                }
            }

            /* Y si el número es primo, incrementamos
             * el contador.... */

            /* [2] Es este inc() correcto? Es nuestra
               critical section?

               La respuesta es no: queremos
               incrementar solo si el contador es
               0.

               Si preguntamos con get_val y luego
               incrementamos con inc(), no estamos
               haciendo una única operación atómica
               sino 2 y esto abre la posibilidad a
               una data race condition.
            */
            result.inc(1);

            /* [5] Borrar la línea "result.inc(1)"
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
/* [6]

   Recompilar y volver a probar para verificar que
   la race condition fue removida

   Conclusion:

   Métodos protegidos MÁS una buena interfaz del
   monitor diseñada para resolver el problema
   son los que nos evitan las race condition.

   Se deben encontrar primero las regiones criticas
   y para cada region critica se debe implementar
   un método en el monitor protegido con un mutex.

   Recuerda que una critical section es un código
   que vos queres q se ejecute "atómicamente".

   Has llegado al final del ejercicio, y al final
   de este tutorial.

*/


/* [1]
   Implementación de una queue protegida (thread safe)
   y bloqueante en su version con conditional variables.

   Mientras ejecutas el ejemplo, ejecuta 'top' en otra consola
   y observa el uso de la CPU.

   Si no le pifie en nada deberías que la CPU no se
   te prende fuego!

 **/

#include <mutex>
#include <condition_variable>
#include <queue>

#include <iostream>
#include <thread>
#include <chrono>
#include <random>

/* [2]  Blocking Queue

   Una queue bloqueante debe implementar:
    - un pull (remover de la queue) que se bloquee si la
      queue esta vacía.
    - (opcionalmente) un push (poner en la queue) que
      se bloquee si la queue esta llena.
      (aka BoundedQueue)

   El mecanismo de bloqueo se hara con *condition variables*.

   Salvando algunos detalles de implementación, lenguajes
   como Python y Ruby proveen blocking queues como esta.

   En Golang, hay lo que se llaman "channels" y que son,
   en esencia, equivalentes a una blocking queue.
 **/
class Queue {
    private:
        std::queue<int> q;
	const unsigned int max_size;

        // [3] Observa:
        //  - hay 1 mutex por que la queue *es* un recurso compartido
        //  - hay 2 conditional variables por q el push() va a esperar
        //    q la queue este no-llena y el pop() va a esperar
        //    q la queue este no-vacía.
        //    O sea hay 2 conditional variables por que hay **2 condiciones**
        std::mutex mtx;
        std::condition_variable is_not_full;
        std::condition_variable is_not_empty;

    public:
	Queue(const unsigned int max_size) : max_size(max_size) {}


        // [4]
        //
        // Tanto try_push() como try_pop() siguen siendo no-bloqueantes
        // como en el ejercicio 09_non_blocking_queue.cpp y
        // 10_blocking_queue_with_busy_wait_and_polling.cpp
        //
        // Pero ahora sabemos que habrá threads bloqueados en push()
        // y pop() a la espera de que la queue este no-llena y no-vacía
        // respectivamente.
        //
        // Entonces tenemos la obligación de **notificarles**
        //
        // El try_push() y push() deberán notificar cuando la queue
        // deje de estar no-vacía (por que pushearon un elemento)
        //
        // El try_pop() y pop() deberán notificar cuando la queue
        // deje de estar no-llena (por que retiraron un elemento)
        bool try_push(const int& val) {
            std::unique_lock<std::mutex> lck(mtx);
	    if (q.size() == this->max_size) {
                return false;
	    }

            if (q.empty()) {
                /* [5]
                   La queue esta vacía por lo que este
		   push hara que la queue tenga un
		   elemento y por lo tanto deje de
		   estar vacía.

                   Como puede haber hilos esperando a que
		   la queue no este vacía, despertamos a todos
		   ellos enviándoles una señal con el método
		   notify_all().

                   Esto es: todo hilo que este "esperando" (wait)
                   la *condición* "is_not_empty" se va a despertar.

                   Como este hilo **aun** tiene adquirido
		   el mutex (lock), los otros hilos que
		   se despierten no ejecutaran nada hasta
                   que liberemos nosotros el mutex lo que
		   nos garantiza que no habrá race
		   conditions.

                   Las conditional variables tienen 2 métodos
                   para señalizar:

                    - notify_all
                    - notify_one

                   El primero le avisa a todos los threads, el segundo
                   solo a uno.

                   Es tentador llamar a notify_one por performance
                   pero es super tricky no caer en un deadlock
                   y más aun el OS no suele implementar correctamente
                   la semántica "one".

                   Por lo que recomendamos **siempre** usar notify_all()

                   Ver también [7] que es la otra parte de la
                   ecuación donde se hace el `wait` en el pop()

                   Vas a ver este notify_all() también en push()
                 **/
                is_not_empty.notify_all();
            }

            q.push(val);
            return true;
        }

        bool try_pop(int& val) {
            std::unique_lock<std::mutex> lck(mtx);
            if (q.empty()) {
                return false;
            }

            if (q.size() == this->max_size) {
                /*
                  [6]

                  Al igual que en [5], tanto el try_pop() como el pop()
                  seguro que hara que la queue pase a estar no-llena,
                  así que tienen la obligación de notificarles
                  a quienes estén esperando la condición (que sera push())
                **/
                is_not_full.notify_all();
            }

            val = q.front();
            q.pop();
            return true;
        }

        // Al igual que en 10_blocking_queue_with_busy_wait_and_polling.cpp
        // push() y pop() son bloqueantes pero en vez de una busy wait
        // usaremos conditional variables.
        void push(const int& val) {
            std::unique_lock<std::mutex> lck(mtx);


            /* [7]
               Si la queue esta llena, no podemos
               hacer un push.

               En vez de retornar con un
               código de error esperamos
               a que la queue deje de estar llena
               con el método wait().

               Literalmente este hilo deja de
               ejecutarse a la
               **espera de recibir una señal**.

               **A la espera de que se de una condición**
               (de ahí el nombre "conditional variable")

               Señal que debería llegarnos cuando
               se cumpla la condición y la
               queue no este llena (alguien hizo un pop() o try_pop()).

               Sin embargo como pueden haber
               otros hilos **también** haciendo push,
               es posible que para cuando este hilo en
               particular se despierte la queue vuelva
               a estar llena.

               *Algún otro push() nos gano de mano!!*

               Por eso tenemos un **loop**
               y mientras este llena seguiremos
               haciendo waits.

               Aunque estemos haciendo un loop fijate que esto
               es mucho más eficiente que hacer un mero polling
               como en 10_blocking_queue_with_busy_wait_and_polling.cpp
               ya que al debloquearnos tenemos muchas chances de
               que podamos realizar el push().

               Detalle oscuro:

               Dependiendo de la implementación
               que haga el sistema operativo de las
               conditional variables, algunas implementaciones
               pueden generar "señales espurias" en las que
               un hilo que esta esperando (wait) se despierte
               sin que otro hilo haya hecho una señal (notify)
               real.

               Esta es **otra razón** para tener el **loop**.

               Y que hace exactamente un wait() ?

               Lo primero que hace es liberar el mutex que
               **debe** estar previamente tomado y luego
               pone a dormir el thread.

               Al liberar el mutex le va a permitir a otros
               threads meterse en el pop/push.

               Cuando la señal llegue `wait()` va a retomar
               el mutex:

                - si lo puede retomar (lock()), wait() retorna
                - sino, se queda bloqueado hasta q lo pueda tomar
                  (como pasa con un mutex tradicional)

               Resume:
                 - wait debe siempre llamarse con un lock ya tomado
                 - debe haber siempre un loop para recheckear en caso
                   de un "despertar espurio"

             **/
	    while (q.size() == this->max_size) {
		is_not_full.wait(lck);
	    }

            if (q.empty()) {
                // lo mismo que esta en try_push()
                // le notificamos a quienes estén esperando
                // "is_not_empty"
                is_not_empty.notify_all();
            }

            q.push(val);
        }


        int pop() {
            std::unique_lock<std::mutex> lck(mtx);

            // [8]
            //
            // Al igual que push() espera a que la queue este no-llena,
            // el pop() espera a que este no-vacía.
            //
            // Por las mismas razones hacemos un **loop** y un wait()
            // sobre la conditional variable "is_not_empty"
            while (q.empty()) {
                is_not_empty.wait(lck);
            }

            if (q.size() == this->max_size) {
                // Igual que en try_pop(), le notificamos a quienes
                // estén bloqueados esperando en "is_not_full"
                // que la queue ya no esta llena.
                is_not_full.notify_all();
            }

            const int val = q.front();
            q.pop();

            return val;
        }

    private:
        Queue(const Queue&) = delete;
        Queue& operator=(const Queue&) = delete;

};


namespace {
    const int MAX_NUM  = 30;
    const int PROD_NUM = 10;
    const int CONS_NUM = 10;
    const int QUEUE_MAXSIZE = 10;
}


void sleep_a_little(std::default_random_engine& generator) {
    std::uniform_int_distribution<int> get_random_int(100, 500);

    auto random_int = get_random_int(generator);
    auto milliseconds_to_sleep = std::chrono::milliseconds(random_int);
    std::this_thread::sleep_for(milliseconds_to_sleep); // sleep some "pseudo-random" time
}

void productor_de_numeros(Queue& q) {
    std::default_random_engine generator;

    for (int i = 0; i < MAX_NUM; ++i) {
        sleep_a_little(generator);
        q.push(1);
    }
}

void consumidor_de_numeros(Queue& q, int& resultado_parcial) {
    std::default_random_engine generator;

    int suma = 0;
    int n;
    do {
        n = q.pop();
        suma += n;

        sleep_a_little(generator);
    } while (n != 0);

    resultado_parcial = suma;
}

// Este main es igual q en 09_non_blocking_queue.cpp
// No hay nada nuevo aquí
int main(int argc, char *argv[]) {
    Queue q(QUEUE_MAXSIZE);

    std::vector<std::thread> productores(PROD_NUM);
    std::vector<std::thread> consumidores(CONS_NUM);
    std::vector<int> resultados_parciales(CONS_NUM);

    std::cout << "Lanzando " << CONS_NUM << " consumidores de numeros\n";
    for (int i = 0; i < CONS_NUM; ++i) {
        consumidores[i] = std::thread(&consumidor_de_numeros, std::ref(q), std::ref(resultados_parciales[i]));
    }
    std::cout << "Lanzando " << PROD_NUM << " productores de numeros\n";
    for (int i = 0; i < PROD_NUM; ++i) {
        productores[i] = std::thread(&productor_de_numeros, std::ref(q));
    }

    std::cout << "Esperando a que los " << PROD_NUM << " productores terminen\n\n";
    for (int i = 0; i < PROD_NUM; ++i) {
        productores[i].join();
    }

    std::cout << "Los consumidores deben estar bloqueados en el pop de la queue\n";
    std::cout << "Enviando (push) " << CONS_NUM << " ceros para que cada consumidor lo saque de la queue y finalice.\n\n";
    for (int i = 0; i < CONS_NUM; ++i) {
        q.push(0);
    }

    std::cout << "Esperando a que los " << CONS_NUM << " consumidores terminen\n\n";
    int suma = 0;
    for (int i = 0; i < CONS_NUM; ++i) {
        consumidores[i].join();
        suma += resultados_parciales[i];
    }

    std::cout << "Se lanzaron " << PROD_NUM << " productores que cada uno creo " << MAX_NUM << " 'unos'\n";
    std::cout << "Por lo tanto, la suma total deberia dar " << PROD_NUM * MAX_NUM << " y la suma efectivamente dio " << suma << "\n";
    std::cout << ((PROD_NUM * MAX_NUM == suma)? "OK\n" : "FALLO\n");
    return 0;
}

/* [9]
 * Challenge: modifica esta Queue para q si el max_size es 0 *no* bloquee
 * al hacer un push() y que el try_push() nunca falle.
 *
 * También deberías deshabilitar si max_size es 0 los notify_all en pop()
 * y try_pop().
 *
 * Dicha implementación seria una UnboundedQueue (queue sin limites).
 *
 * Muchas implementación (recuerdo la de Python) ofrecen una única
 * Queue que según el parámetro max_size se comportan como una bounded
 * o unbounded queue.
 *
 * Donde se usan las UnboundedQueue?
 *
 * En ciertas aplicaciones en las q se desea q la aplicación
 * no se bloque a costa de consumir toda la memoria ram de la computadora.
 *
 * Por supuesto q una "queue sin limites" es solo una ilusión: eventualmente
 * si no se hacen pops la queue se llena y revienta la memoria ram.
 *
 * Que lindo es despertase a las 4AM por una alerta en tu celular
 * con un OutOfMemory Error :D
 * */


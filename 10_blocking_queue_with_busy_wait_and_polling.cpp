/* [1]
   Implementación de una queue protegida (thread safe)
   y bloqueante en su version busy-wait / polling
   (**ineficiente** en términos de CPU)

   No solo los métodos push y pop usan un mutex
   para evitar race conditions sino que el push
   dejara de poner elementos en la queue si esta esta llena
   y el pull no retornara hasta que no haya algo en la queue
   para retirar.

   El push y pop se **bloquean** cuando la queue esta llena / vacía
   respectivamente.

   Te suena a algo? Es igual que lo que sucede con los sockets
   donde el send y recv se bloquean si no se puede enviar más datos
   o no se recibió dato alguno.

   **Misma idea**

   En esta implementación se hara uso de busy-waits y polling
   algo que es muy ineficiente en términos de CPU pero en ciertas
   aplicaciones es la única solución.

   Mientras ejecutas el ejemplo, ejecuta 'top' en otra consola
   y observa el uso de la CPU.

   A cuanto se dispara al CPU de tu máquina?
 **/

#include <mutex>
#include <queue>

#include <iostream>
#include <thread>
#include <chrono>
#include <random>

/* [2]  Blocking Queue.

   Se implementan los métodos push() y pop()
   que ponen y retiran elementos de la queue.

   Si la queue esta llena, push() se bloquea;
   si la queue esta vacía pop() se bloquea.

   El mecanismo de bloqueo se hara con loops: busy-waits y
   polling.
 **/
class Queue {
    private:
        std::queue<int> q;
	const unsigned int max_size;

        std::mutex mtx;

    public:
	Queue(const unsigned int max_size) : max_size(max_size) {}
        /*
         * [3]
         *
         * Un Blocking Queue puede también ofrecer try_push() y try_pop()
         * no-bloqueantes.
         *
         * Esto permite que un thread (digamos el consumidor) haga
         * try_pop() y no se bloque si esta vacía y al mismo tiempo
         * otro thread (un productor) llame a push() y se bloquee
         * se la queue esta llena.
         */
        bool try_push(const int& val) {
            std::unique_lock<std::mutex> lck(mtx);
	    if (q.size() == this->max_size) {
                return false;
	    }

            q.push(val);
            return true;
        }

        bool try_pop(int& val) {
            std::unique_lock<std::mutex> lck(mtx);
            if (q.empty()) {
                return false;
            }

            val = q.front();
            q.pop();
            return true;
        }

        void push(const int& val) {
            mtx.lock();

	    /* [4]
	       Busy wait: esperamos que la queue este no-llena
	       para poder guardar el elemento en la queue.

	       La forma de esperar es con un loop que virtualmente
	       no hace nada.

	       Obviamente debemos deslockear y re-lockead el mutex
	       para que otros hilos tengan la oportunidad de hacer
	       un pull.

	       Es muy ineficiente en términos de CPU pero en ciertas
	       aplicaciones es la única solución.
	    */
	    while (q.size() >= this->max_size) {
		mtx.unlock();
		// Entre el unlock() y el lock()
                // otros hilos podrán tomar el mutex aquí

		/*
		 * [6]
		   Dormir 10 milisegundos es suficiente?
		   Tal vez es demasiado y dormimos de más (ineficiente)
		   Tal vez es poco y loopeamos de más (ineficiente)
		 * */
		// std::this_thread::sleep_for(std::chrono::milliseconds(10));
		mtx.lock();
	    }

            q.push(val);
	    mtx.unlock();
        }


        int pop() {
	    mtx.lock();

	   /* [5]

	      Al igual que en [4], esperamos hasta que la queue este
	      no-vacía

	      En este caso en vez de hacer un busy-wait haremos una
	      variante:

	      La alternativa es poner un sleep entre el unlock y
	      el lock. Esto se lo conoce como Polling. Reduce el uso
	      de CPU pero sigue siendo ineficiente ya que no es fácil
	      predecir cuanto tiempo se debe dormir (el parámetro del
	      sleep)

	    **/
            while (q.empty()) {
		mtx.unlock();
		/*
		 * [7]
		   Dormir 10 milisegundos es suficiente?
		   Tal vez es demasiado y dormimos de más (ineficiente)
		   Tal vez es poco y loopeamos de más (ineficiente)
		 * */
		// std::this_thread::sleep_for(std::chrono::milliseconds(10));
		mtx.lock();
            }

            const int val = q.front();
            q.pop();

	    mtx.unlock();
            return val;
        }

    private:
        Queue(const Queue&) = delete;
        Queue& operator=(const Queue&) = delete;
};


namespace {
    const int MAX_NUM  = 30;
    const int PROD_NUM = 20;
    const int CONS_NUM = 10;
    const int QUEUE_MAXSIZE = 10;
}


void sleep_a_little(std::default_random_engine& generator) {
    std::uniform_int_distribution<int> get_random_int(100, 500);

    auto random_int = get_random_int(generator);
    auto milliseconds_to_sleep = std::chrono::milliseconds(random_int);
    std::this_thread::sleep_for(milliseconds_to_sleep); // sleep some "pseudo-random" time
}

/* [8]
 *
 * Ahora el productor llama a push() y si la queue esta llena
 * el bloqueo se hace dentro de push().
 *
 * El productor no debe hacer ningún loop de reintento
 * (a diferencia de lo que pasaba con try_push())
 * */
void productor_de_numeros(Queue& q) {
    std::default_random_engine generator;

    for (int i = 0; i < MAX_NUM; ++i) {
        sleep_a_little(generator);
        q.push(1);
    }
}

/* [9]
 *
 * Ahora el consumidor llama a pop() y si la queue esta vacía
 * el bloqueo se hace dentro de pop().
 *
 * El consumidor no debe hacer ningún loop de reintento
 * (a diferencia de lo que pasaba con try_pop())
 * */
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


// Este main es igual al de 09_non_blocking_queue.cpp
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

/*
 * [10]

   Viste como se dispara la CPU?

   Un busy-wait es un loop que corre tan rápido como puede para
   ver si una condición esta dada o no.

   Se usa en aplicaciones **muy** especificas por que como veras,
   *te quema la CPU!*

   Anda a [6] y [7] y descomenta el "sleep". Compila y volvé
   a correr el código.

   Proba valores de 10 milliseconds, de 1 millisecond
   y otro de 1000 milliseconds.

   Que te dice `top`? Ya no te quema la CPU?
   Y el programa en total, cuanto tarda en cada caso?
   (medilo con `time`)

   Los busy-waits con sleep "martillan" menos a la CPU
   (less hammering) pero introducen un delay/latencia
   que es difícil de tunear.

   El polling se usa solo cuando no tenes otra forma
   de coordinación (imagínate que estas esperando un mail
   muy importante y estas apretando F5 una y otra vez
   refrescando el inbox: no te queda otra que polling)

   En el siguiente ejemplo veremos **la** herramienta
   para evitar polling: las *conditional variables*
*/


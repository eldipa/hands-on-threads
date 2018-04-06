/* [1]
   Implementacion de una cola protegida (thread safe)
   version busy-wait y polling (**ineficiente** en terminos de CPU)

   No solo los metodos push y pop usan un mutex
   para evitar race conditions sino que el push
   dejara de poner elementos en la cola si esta esta llena
   y el pull no retornara hasta que no haya algo en la cola
   para retirar.

   Se hara uso de busy-waits y polling algo que es muy ineficiente
   en terminos de CPU pero en ciertas aplicaciones es la
   unica solucion.

   Compilar con
      g++ -std=c++11 -pedantic -Wall
            -o 09_thread_safe_queue_with_busy_wait_and_polling.exe
            09_thread_safe_queue_with_busy_wait_and_polling.cpp

   Mientras ejecutas el ejemplo, ejecuta 'top' en otra consola
   y observa el uso de la CPU.
 **/

#include <mutex>
#include <queue>

#include <iostream>
#include <thread>
#include <chrono>
#include <random>

/* [2]  Thread Safe Queue: en este caso, es una cola de enteros.

   Una cola thread safe debe implementar:
    - un pull (remover de la cola) que se bloquee si la
      cola esta vacia.
    - (opcionalmente) un push (poner en la cola) que
      se bloquee si la cola esta llena.

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

        void push(const int& val) {
            mtx.lock();

	    /* [4]
	       Busy wait: esperamos que la cola este no-llena
	       para poder guardar el elemento en la cola.

	       La forma de esperar es con un loop que virtualmente
	       no hace nada.

	       Obviamente debemos deslockear y re-lockead el mutex
	       para que otros hilos tengan la oportunidad de hacer
	       un pull.

	       Es muy ineficiente en terminos de CPU pero en ciertas
	       aplicaciones es la unica solucion.

		** NO SE PERMITE NI BUSY WAITS NI POLLINGS EN NINGUNO **
		** DE LOS TRABAJOS PRACTICOS			      **
	    */
	    while (q.size() >= this->max_size) {
		mtx.unlock();
		// otros hilos podran tomar el mutex aqui
		mtx.lock();
	    }

            q.push(val);
	    mtx.unlock();
        }


        int pop() {
	    mtx.lock();

	   /* [5]

	      Al igual que en [4], esperamos hasta que la cola este
	      no-vacia

	      En este caso en vez de hacer un busy-wait haremos una
	      variante:

	      La alternativa es poner un sleep entre el unlock y
	      el lock. Esto se lo conoce como Polling. Reduce el uso
	      de CPU pero sigue siendo ineficiente ya que no es facil
	      predecir cuanto tiempo se debe dormir (el parametro del
	      sleep)

		** NO SE PERMITE NI BUSY WAITS NI POLLINGS EN NINGUNO **
		** DE LOS TRABAJOS PRACTICOS			      **

	    **/
            while (q.empty()) {
		mtx.unlock();
		/*
		 * [6]
		   Dormir 10 milisegundos es suficiente?
		   Tal vez es demasiado y dormimos de mas (ineficiente)
		   Tal vez es poco y loopeamos de mas (ineficiente)
		 * */
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
 * Para probar la Queue vamos a tener muchos
 * "productores" de numeros
 * que seran pusheados en la cola.
 * */
void productor_de_numeros(Queue& q) {
    std::default_random_engine generator;

    for (int i = 0; i < MAX_NUM; ++i) {
        sleep_a_little(generator);
        q.push(1);
    }
}

/* [9]
 * Por el otro lado vamos a tener muchos
 * consumidores que leen de la cola
 * hasta que lean el numero 0 para luego finalizar.
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

int main(int argc, char *argv[]) {
    Queue q(QUEUE_MAXSIZE);

    std::vector<std::thread> productores(PROD_NUM);
    std::vector<std::thread> consumidores(CONS_NUM);
    std::vector<int> resultados_parciales(CONS_NUM);

    /*
     * [7]
     * Lanzamos los productores y consumidores,
     * cada uno en su hilo.
     *  - Los productores iran poniendo un 1
     *    en la cola cada cierto tiempo.
     *  - Los consumidores iran sacando un 1
     *    e iran sumandolos en resultados_parciales
     **/
    std::cout << "Lanzando " << CONS_NUM << " consumidores de numeros\n";
    for (int i = 0; i < CONS_NUM; ++i) {
        consumidores[i] = std::thread(&consumidor_de_numeros, std::ref(q), std::ref(resultados_parciales[i]));
    }
    std::cout << "Lanzando " << PROD_NUM << " productores de numeros\n";
    for (int i = 0; i < PROD_NUM; ++i) {
        productores[i] = std::thread(&productor_de_numeros, std::ref(q));
    }

    /* [10]
     * Esperamos a que todos los productores terminen
     * */
    std::cout << "Esperando a que los " << PROD_NUM << " productores terminen\n\n";
    for (int i = 0; i < PROD_NUM; ++i) {
        productores[i].join();
    }

    /*
     * [11]
     * Los consumidores estan bloqueados en la cola,
     * tratando de hacer un pop.
     * Como le decimos que ya no va a ver ningun
     * elemento mas y que deben terminar?
     *
     * En este ejemplo usamos un valor 'dummy' que
     * cada consumidor entendera que representa el fin de
     * la cola (una especie de EOF).
     *
     * NOTA: No es la unica solucion y de hecho no es
     * necesariamente la mas elegante, pero funciona.
     **/
    std::cout << "Los consumidores deben estar bloqueados en el pop de la cola\n";
    std::cout << "Enviando (push) " << CONS_NUM << " ceros para que cada consumidor lo saque de la cola y finalice.\n\n";
    for (int i = 0; i < CONS_NUM; ++i) {
        q.push(0);
    }

    /* [12]
     * Esperamos a que todos los consumidores terminen
     * */
    std::cout << "Esperando a que los " << CONS_NUM << " consumidores terminen\n\n";
    int suma = 0;
    for (int i = 0; i < CONS_NUM; ++i) {
        consumidores[i].join();
        suma += resultados_parciales[i];
    }

    std::cout << "Se lanzaron " << PROD_NUM << " productores que cada uno creo " << MAX_NUM << " 'unos'\n";
    std::cout << "Por lo tanto, la suma tota deberia dar " << PROD_NUM * MAX_NUM << " y la suma efectivamente dio " << suma << "\n";
    std::cout << ((PROD_NUM * MAX_NUM == suma)? "OK\n" : "FALLO\n");
    return 0;
}

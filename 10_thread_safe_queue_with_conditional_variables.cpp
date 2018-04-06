/* [1]
   Implementacion de una cola protegida (thread safe).
   No solo los metodos push y pop usan un mutex
   para evitar race conditions sino que ademas
   hacen uso de una condition variable para permitir
   que el metodo pop se bloquee cuando la cola este vacia
   y se despierte cuando haya algo que sacar (cola no vacia).

   Compilar con
      g++ -std=c++11 -pedantic -Wall
            -o 10_thread_safe_queue_with_conditional_variables.exe
	    10_thread_safe_queue_with_conditional_variables.cpp

   Mientras ejecutas el ejemplo, ejecuta 'top' en otra consola
   y observa el uso de la CPU.

   Se deja al lector las siguientes posibles mejoras:
    - hacer que Queue sea movible (implementar el
      constructor y el operador asignacion por movimiento)
    - implementar una version template generica Queue<T>
    - implementar una especializacion total Queue<void*>
      y luego una especializacion parcial Queue<T*> con
      el fin de evitar el code bloat por parte del compilador.
 **/

#include <mutex>
#include <condition_variable>
#include <queue>

#include <iostream>
#include <thread>
#include <chrono>
#include <random>

/* [2]  Blocking Queue: en este caso, una cola de enteros.

   Una cola bloqueante debe implementar:
    - un pull (remover de la cola) que se bloquee si la
      cola esta vacia.
    - (opcionalmente) un push (poner en la cola) que
      se bloquee si la cola esta llena.

   El mecanismo de bloqueo se hara condition variables.
 **/
class Queue {
    private:
        std::queue<int> q;
	const unsigned int max_size;

        std::mutex mtx;
        std::condition_variable is_not_full;
        std::condition_variable is_not_empty;

    public:
	Queue(const unsigned int max_size) : max_size(max_size) {}

        void push(const int& val) {
            // [3] mtx.lock() pero en modo RAII
            std::unique_lock<std::mutex> lck(mtx);

            if (q.empty()) {
                /* [4]
                   La cola esta vacia por lo que este
		   push hara que la cola tenga un
		   elemento y por lo tanto deje de
		   estar vacia.

                   Como puede haber hilos esperando a que
		   la cola no este vacia, despertamos a todos
		   ellos enviandoles una señal con el metodo
		   notify_all().

                   Como este hilo aun tiene adquirido
		   el mutex (lock), los otros hilos que
		   se despierten no ejecutaran nada hasta
                   que liberemos nosotros el mutex lo que
		   nos garantiza que no habra race
		   conditions.
                 **/
                is_not_empty.notify_all();
            }

	    while (q.size() >= this->max_size) {
		/*
		   Si la cola esta llena, esperar.
		   Esto evita que una cola crezca demasiado
		   y consumamos memoria demas.

	           Ver las implicaciones de un notify_all en [6]
		*/
		std::cout << " cola llena, el push se bloquea\n";
		is_not_full.wait(lck);
	    }

            q.push(val);
        } // <--- lck se destruye y automaticamente
	  // libera el mutex con mtx.unlock()


        int pop() {
	    // [5] mtx.lock() pero en modo RAII
            std::unique_lock<std::mutex> lck(mtx);

            while (q.empty()) {
                /* [6]
		   Si la cola esta vacia, no podemos
		   hacer un pop.
                   En vez de retornar con un
		   codigo de error esperamos
                   a que la cola deje de estar vacia
		   con el metodo wait().

                   Literalmente este hilo deja de
		   ejecutarse a la espera de
                   recibir una señal.

		   Señal que deberia llegarnos cuando la
		   cola no este vacia (alguien hizo un push).

		   Sin embargo como pueden haber
                   otros hilos tambien haciendo pop,
		   es posible que para cuando este hilo en
		   particular se despierte la cola vuelva
                   a estar vacia: por eso tenemos un loop
		   y mientras este vacia seguiremos
		   haciendo waits.

		   Ademas, dependiendo de la implementacion
		   que haga el sistema operativo de las
		   conditional variables, algunas implementaciones
		   pueden generar "señales espurias" en las que
		   un hilo que esta esperando (wait) se despierte
		   sin que otro hilo haya hecho una señal (notify)
		   real.
		   Esta es otra razon para tener el loop.

                   Por supuesto, el metodo wait()
		   automaticamente libera
                   el mutex asociado de otro modo ningun
		   otro hilo podria hacer el push.

		   Cuando este hilo se despierte y se
                   retorne de wait(), el mutex es automaticamente
		   re-obtenido (lock) de forma transparente
		   al desarrollador.

                   Colorario: el metodo wait debe llamarse
		   solo si el mutex ya fue adquirido
                   sino el comportamiento es indefinido
                 **/
		std::cout << " cola vacia, el pull se bloquea\n";
                is_not_empty.wait(lck);
            }

            const int val = q.front();
            q.pop();

	    /*
	      La cola seguro ya no esta llena, avisarle a todos
	      los hilos que lo estan esperando.

	      Ver las implicaciones de un notify_all en [4]
	    **/
	    is_not_full.notify_all();

            return val;
        } // <--- lck se destruye y automaticamente
	  // libera el mutex con mtx.unlock()

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

    std::cout << "Los consumidores deben estar bloqueados en el pop de la cola\n";
    std::cout << "Enviando (push) " << CONS_NUM << " ceros para que cada consumidor lo saque de la cola y finalice.\n\n";
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
    std::cout << "Por lo tanto, la suma tota deberia dar " << PROD_NUM * MAX_NUM << " y la suma efectivamente dio " << suma << "\n";
    std::cout << ((PROD_NUM * MAX_NUM == suma)? "OK\n" : "FALLO\n");
    return 0;
}

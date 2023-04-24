
#include <mutex>
#include <condition_variable>
#include <queue>

#include <iostream>
#include <thread>
#include <chrono>
#include <random>

struct ClosedQueue : public std::runtime_error {
    ClosedQueue() : std::runtime_error("The queue is closed") {}
};

class Queue {
    private:
        std::queue<int> q;
	const unsigned int max_size;

        bool closed;

        std::mutex mtx;
        std::condition_variable is_not_full;
        std::condition_variable is_not_empty;

    public:
	Queue(const unsigned int max_size) : max_size(max_size), closed(false) {}


        bool try_push(const int& val) {
            std::unique_lock<std::mutex> lck(mtx);

            /*
             * Si la queue esta cerrada no se aceptan más elementos
             * y try_push() y push() deben fallar.
             *
             * En esta implementación se lanza una excepción si la queue
             * esta cerrada;
             * en otras implementaciones se retorna via algún mecanismo
             * un bool o tienen un método is_closed()
             *
             * Python por ejemplo implementa ambos cosa pero realmente
             * is_closed() te puede llevar a una race condition.
             *
             * Por ejemplo, podrías pensar que esto es correcto:
             *
             *      while (not blocking_queue.is_closed()) {
             *          blocking_queue.try_push(...);
             *      }
             *
             * Pero ese código **no** evitara q hagas un push() (o un pop())
             * sobre una queue que esta cerrada.
             *
             * Pudiste ver la RC?
             * */
            if (closed) {
                throw ClosedQueue();
            }

	    if (q.size() == this->max_size) {
                return false;
	    }

            if (q.empty()) {
                is_not_empty.notify_all();
            }

            q.push(val);
            return true;
        }

        bool try_pop(int& val) {
            std::unique_lock<std::mutex> lck(mtx);

            /*
             * Tentador pero **no**!!
             *
             * Tanto try_pop() como pop() deben fallar si la queue
             * esta cerrada **y** esta vacía.
             *
             * Si la queue esta cerrada eso **no** implica que no
             * queden elementos **aun** en la queue y para
             * la perspectiva de cualquier thread consumidor
             * **aun** hay trabajo por hacer.
             * */
            //if (closed) {
            //    throw ClosedQueue();
            //}

            if (q.empty()) {
                if (closed) {
                    throw ClosedQueue();
                }
                return false;
            }

            if (q.size() == this->max_size) {
                is_not_full.notify_all();
            }

            val = q.front();
            q.pop();
            return true;
        }

        void push(const int& val) {
            std::unique_lock<std::mutex> lck(mtx);

            if (closed) {
                throw ClosedQueue();
            }

	    while (q.size() == this->max_size) {
		is_not_full.wait(lck);
	    }

            if (q.empty()) {
                is_not_empty.notify_all();
            }

            q.push(val);
        }


        int pop() {
            std::unique_lock<std::mutex> lck(mtx);

            while (q.empty()) {
                if (closed) {
                    throw ClosedQueue();
                }
                is_not_empty.wait(lck);
            }

            if (q.size() == this->max_size) {
                is_not_full.notify_all();
            }

            const int val = q.front();
            q.pop();

            return val;
        }

        void close() {
            std::unique_lock<std::mutex> lck(mtx);

            /*
             * Cerrar dos veces una queue no es ningún problema
             * realmente (al final es cambiar un booleano).
             *
             * Pero **lógicamente esta mal**: o hay algo mal programado
             * o hay algún bug que hizo q se llame a close() más de
             * una vez.
             *
             * Es como querer cerrar un file o hacer un free dos veces.
             *
             * Consejo: si sabes que hay un estado invalido, checkearlo
             * y fallar rápido. Hara que descubras bugs mucho más rápido.
             * */
            if (closed) {
                throw std::runtime_error("The queue is already closed.");
            }
            closed = true;

            /* Probablemente estoy mintiendo aquí ya q no se si la queue
             * esta vacio o no realmente **pero** en el caso de que lo
             * este puede que algún thread este bloqueado en el pop().
             *
             * Si es así este notify_all() lo va a despertar y ese thread
             * tendrá la oportunidad de ver q la queue esta cerrada y
             * q no debe continuar con el pop()
             *
             * */
            is_not_empty.notify_all();
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
        /*
         * Debería poner un try-catch en caso q la queue este cerrada?
         *
         * En el caso de los productores (quienes hagan push)
         * "no" debería ser necesario.
         *
         * Si la queue esta cerrada y se hace un push, se va a
         * lanzar una excepción. Eso no cambia.
         *
         * Pero en un buen diseño las queues deberían cerrarse
         * **solo** si los productores terminaron.
         *
         * Cuando hay una sola queue para N productores
         * (como es este caso) el cierre de la queue la debe hacer
         * alguien de afuera (en este ejemplo lo hace el main())
         *
         * Cuando la queue es una por cada productor, es el mismo
         * productor cuando termina quien cierra **su** queue.
         *
         * Por eso, 99.99% de las veces un productor debería
         * asumir que la queue (o "su" queue) esta abierta
         * siempre y sino, esta OK q se lance una excepción
         * por que realmente es algo *excepcional*.
         * */
        q.push(1);
    }
}

void consumidor_de_numeros(Queue& q, int& resultado_parcial) {
    std::default_random_engine generator;

    int suma = 0;
    int n;
    do {
        // Al contrario de un productor, el consumidor no tiene
        // ni idea de cuando habrá o no más elementos.
        //
        // Recordad que una queue vacía no significa que no
        // habrá "nunca más" nada ahí.
        //
        // Solo al atrapar la excepción podemos saber que la
        // queue realmente *nunca más* tendrá elementos por
        // que la queue esta **cerrada y vacía**.
        try {
            n = q.pop();
        } catch (const ClosedQueue&) {
            break;
        }
        suma += n;

        sleep_a_little(generator);
    } while (true);

    resultado_parcial = suma;
}

// Este main es igual q en 09_non_blocking_queue.cpp excepto
// que ahora en vez de pushear valores dummy para hacer que los
// consumidores finalicen, hacemos un close de la queue explicito.
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
    std::cout << "Cerrando la queue\n\n";
    q.close();

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
 * Challenge:
 *
 * Cuando una queue es cerrada, los consumidores pueden (y deben)
 * seguir sacando elementos hasta que la queue esta vacía.
 *
 * Así uno puede asegurarse q todos los elementos que fueron pusheados
 * en algún momento van a ser procesados eventualmente.
 *
 * 99.99% es lo que necesitas pero.... Habrá situaciones excepcionales
 * en las que vas a querer "frenar" todo lo más rápido posible.
 *
 * Entonces, cerrar la queue no te va a alcanzar, vas a querer
 * cerrar la queue y *vaciarla* forzadamente cosa que los consumidores
 * terminen lo antes posible (a costa de dejar elementos sin procesar).
 *
 * Tu misión: modificar close() para que reciba un parámetro opcional
 * llamado drain, cuyo default es false.
 *
 * Si drain es true, close() debe cerrar la queue y luego vaciarla
 * completamente.
 *
 *
 * (Challenge)^2:
 *
 * En ciertas ocasiones hay N productores q pushean datos a una queue y
 * no esta claro cuando la queue hay q cerrarla.
 *
 * No hay ningún otro thread q detecte el fin de los productores y pueda
 * llamar a close() (como lo esta haciendo main() en este ejercicio).
 *
 * En ese caso se podría modificar la queue para que tenga un contador
 *  - la queue lo inicializa con la cantidad de productores N
 *  - cada llamada a close() decrementa en 1 ese contador y si es cero hace realmente
 *  el close.
 *
 * Así, N productores llaman a close() y solo el último realmente cerrara la queue.
 *
 * Notar q este esquema es solo valido cuando N es fijo, fijate
 * si ves cual seria el issue si permitís q N sea dinámico.
 * */


/* [1]
 *
   Hasta ahora has visto como proteger un objeto compartido
   con mutex, construyéndole una capa de protección llamada monitor.

   Cuando tengas múltiple threads leyendo y escribiendo sobre
   un mismo objeto compartido sin un claro orden, construir
   un monitor es tu mejor opción.

   Sin embargo habrá ocasiones en que los threads no acceden
   "aleatoriamente" a un objeto.

   Habrá ocasiones en que un thread (digamos Alice) lee y escribe
   sobre el objeto compartido, **luego** deja de hacerlo y **luego**
   lo lee/escribe un segundo thread (digamos Bob).

   Aunque los threads Alice y Bob tienen acceso via un puntero/referencia
   al mismo objeto (y por eso esta compartido),
   no esta "realmente" siendo compartido *a la vez*.

   En estos caso podemos optar por darle el **ownership** del
   objeto a Alice, que el thread haga lo q necesite y cuando
   ya no necesite de él, **se lo pase** al thread Bob.

   En este escenario ese objeto ya **no** estaría compartido
   ya que en un momento es accesible *únicamente* por Alice
   y al momento siguiente *únicamente* por Bob.

   El único instante donde habría un problema es en el **pasaje**.

   Necesitamos un mecanismo thread-safe que nos permita
   **pasar** objetos de un thread a otro.

   Necesitamos un **thread-safe queue**.

   En este ejemplo veremos una thread-safe queue **no-bloqueante**.

   A los threads como Alice que pasan los objetos (los pushean
   en la queue) los llamaremos "productores"; a los threads
   como Bob que reciben los objetos (los que hacen pop())
   los llamaremos "consumidores".

   Conserva esas dos palabritas cerca de tu corazón, aparecen
   en todas las literaturas.

   ~~~

   Mientras ejecutas el ejemplo, ejecuta 'top' en otra consola
   y observa el uso de la CPU.

   Se prende fuego no? (ya veremos como fixear eso)

 **/

#include <mutex>
#include <condition_variable>
#include <queue>

#include <iostream>
#include <thread>
#include <chrono>
#include <random>

/* [2]  NonBlocking Queue

   Se implementan los métodos try_push() y try_pop()
   que ponen y retiran elementos de la queue.

   Ambos métodos protegen al recurso (Queue es un monitor)
   y por lo tanto es thread safe.

   Los métodos son "try_" por que pueden fallar:
   - try_push puede no poner un elemento si la queue esta llena
   - try_pop puede no poner un elemento si la queue esta vacía.

   En caso de falla, los métodos *no* reintentan *ni esperan*:
   son *no bloqueantes*.

   Estos métodos se usan cuando el thread puede seguir trabajando
   y no quiere bloquearse si la queue esta llena/vacía.

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
         * Intentamos pushear un elemento. Si la queue esta llena
         * retornamos que fallamos.
         *
         * Una queue que impone un limite en la cantidad de elementos
         * se la llama BoundedQueue. Aquellas que no, se las llaman
         * UnboundedQueue.
         */
        bool try_push(const int& val) {
            std::unique_lock<std::mutex> lck(mtx);
	    if (q.size() == this->max_size) {
                return false;
	    }

            q.push(val);
            return true;
        }

        /*
         * [4]
         *
           Retornamos el valor pop'eado por referencia así dejamos
           el retorno para el booleano al igual que en el push()
         */
        bool try_pop(int& val) {
            std::unique_lock<std::mutex> lck(mtx);
            if (q.empty()) {
                return false;
            }

            val = q.front();
            q.pop();
            return true;
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


// Esto esta solo para simular tiempos aleatorios de trabajo en
// los productores y consumidores
void sleep_a_little(std::default_random_engine& generator) {
    std::uniform_int_distribution<int> get_random_int(100, 500);

    auto random_int = get_random_int(generator);
    auto milliseconds_to_sleep = std::chrono::milliseconds(random_int);
    std::this_thread::sleep_for(milliseconds_to_sleep); // sleep some "pseudo-random" time
}



/* [6]
 * Para probar la Queue vamos a tener muchos
 * "productores" de números
 * que serán pusheados en la queue.
 *
 * Notar que try_push() puede fallar así que el productor
 * es el responsable de reintentar.
 * */
void productor_de_numeros(Queue& q) {
    std::default_random_engine generator;

    bool ok = false;
    for (int i = 0; i < MAX_NUM; ++i) {
        ok = false;
        sleep_a_little(generator);

        while (not ok)
            ok = q.try_push(1);
    }
}

/* [7]
 * Por el otro lado vamos a tener muchos
 * consumidores que leen de la queue
 * hasta que lean el número 0 para luego finalizar.
 *
 * Notar que try_pop() puede fallar así que el consumidor
 * es el responsable de reintentar.
 * */
void consumidor_de_numeros(Queue& q, int& resultado_parcial) {
    std::default_random_engine generator;

    bool ok = false;
    int suma = 0;
    int n;
    do {
        ok = false;
        while (not ok)
            ok = q.try_pop(n);

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
     * [5]
     * Lanzamos los productores y consumidores,
     * cada uno en su hilo.
     *  - Los productores irán poniendo un 1
     *    en la queue cada cierto tiempo.
     *  - Los consumidores irán sacando un 1
     *    e irán sumándolos en resultados_parciales
     **/

    std::cout << "Lanzando " << CONS_NUM << " consumidores de numeros\n";
    for (int i = 0; i < CONS_NUM; ++i) {
        consumidores[i] = std::thread(&consumidor_de_numeros, std::ref(q), std::ref(resultados_parciales[i]));
    }
    std::cout << "Lanzando " << PROD_NUM << " productores de numeros\n";
    for (int i = 0; i < PROD_NUM; ++i) {
        productores[i] = std::thread(&productor_de_numeros, std::ref(q));
    }

    /* [8]
     * Esperamos a que todos los productores terminen
     * */
    std::cout << "Esperando a que los " << PROD_NUM << " productores terminen\n\n";
    for (int i = 0; i < PROD_NUM; ++i) {
        productores[i].join();
    }

    /*
     * [9]
     * Los consumidores están bloqueados en la queue,
     * tratando de hacer un pop.
     * Como le decimos que ya no va a ver ningún
     * elemento más y que deben terminar?
     *
     * En este ejemplo usamos un valor 'dummy' que
     * cada consumidor entenderá que representa el fin de
     * la queue (una especie de EOF).
     *
     * Es responsabilidad de quien quiere cerrar la queue enviar
     * un dummy por cada consumidor.
     *
     * Es responsabilidad de cada consumidor que al sacar un dummy
     * deje de sacar elementos y finalize.
     *
     * Así, si pusheamos N dummys, se cerraran N consumidores.
     *
     * NOTA: No es la única solución y de hecho no es
     * necesariamente la más elegante, pero funciona.
     * En un ejercicio siguiente te mostrare una alternativa.
     **/
    std::cout << "Los consumidores deben estar bloqueados en el pop de la queue\n";
    std::cout << "Enviando (push) " << CONS_NUM << " ceros para que cada consumidor lo saque de la queue y finalice.\n\n";
    for (int i = 0; i < CONS_NUM; ++i) {
        while (!q.try_push(0)) {}
    }

    /* [10]
     * Esperamos a que todos los consumidores terminen
     * */
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

/* [11]
 * Challenge:
 *
 * Agrega 2 nuevos métodos thread safe y no-bloqueantes:
 *
 *  - int push_some(const int* values, const int cnt)
 *  - int pop_some(int* values, int& cnt)
 *
 * push_some() pushea de forma atómica hasta cnt elementos en la queue.
 * Menos elementos pueden ser pusheados si se llega al limite.
 * Retorna la cantidad de elementos que fueron efectivamente pusheados.
 *
 * pop_some() retira de forma atómica hasta cnt elementos de la queue.
 * Menos elementos pueden ser retirados si la queue queda vacía.
 * Retorna la cantidad de elementos que fueron efectivamente retirados.
 *
 * Notas una semejanza entre push_some()/pop_some() de la queue
 * y el recvsome()/sendsome() del socket?
 *
 * */


/*
   [1]
   Ejemplo de como ejecutar una función/functor en
   un hilo separado en C++

   Esta vez, en vez de usar composición usaremos
   **herencia**.

   Para ello crearemos una objeto Thread que
   ejecutara un método virtual en su  propio hilo
   definido por las clases hijas que hereden de
   Thread

   Cuando el objeto functor encapsula dentro de él
   el concepto de hilo se dice que el objeto es un
   "objeto activo".


   Threads por herencia es la forma de usar threads
   en lenguajes como Java.

   Otros, como Python, son iguales a C++ y permiten
   las dos opciones (composición y herencia).

   Golang en cambio tiene go-rutinas que son
   "threads ligeros" manejados por el runtime de Golang.
*/

#include <iostream>
#include <vector>
#include <thread>
#include <exception>

#define N 10

class Thread {
    private:
        std::thread thread;

    public:
        Thread () {}

        void start() {
            /* [2] Lanzamos el thread que correrá
               siempre la misma función (Thread::main)

               Como Thread::main es un **método**
               sin parámetros y std::thread espera
               a una **función** podemos ver a
               Thread::main como una función que
               recibe como primer argumento al
               objeto this (tal como en los TDA de C!)

               std::thread soporta correr una
               función con argumentos con la llamada:

                  std::thread( funcion, arg1, arg2, ...)

               Por lo tanto
                    std::thread( metodo, this )

               es equivalente a correr el método
               sin argumentos en un thread.

               Como Thread::main llama a Thread::run
               y Thread::run es un método **polimórfico**,
               cada objeto ejecutara
               un código particular en el thread.

               Objetos distintos podrán correr en
               sus propios threads con esta única
               implementación de Thread mientras
               hereden de Thread y creen sus propias
               versiones del método run.
            */
            thread = std::thread(
                                    &Thread::main,
                                    this
                                );
        }

        // [3]
        //
        // Este método es el que correrá en su propio thread.
        //
        // Obviamente el que hacer esta dentro de Thread::run
        // que es polimórfico
        //
        // La idea de Thread::main es que me permite poner
        // un try-catch y atrapar cualquier excepción que
        // se lance en Thread::run.
        //
        // Si una excepción se escapa de la función que esta
        // corriendo en un thread, el program termina con un abort
        //
        // Not nice.
        //
        // Quienes implementen Thread::run deberían atrapar
        // las excepciones ellos. El try-catch de Thread::main
        // es solo como último recurso.
        void main() {
            try {
                this->run();
            } catch(const std::exception &err) {
                // Nota: por simplicidad estoy haciendo unos prints.
                // Código productivo debería *loggear* el error, no solo
                // prints.
                std::cerr << "Unexpected exception: " << err.what() << "\n";
            } catch(...) {
                std::cerr << "Unexpected exception: <unknown>\n";
            }
        }

        void join() {
            thread.join();
        }

        /* [4] Virtual puro para forzar una
           definición en las clases hijas.

           Sera responsabilidad de ellas implementar lo que quieran
           que corran en un thread aquí.
        */
        virtual void run() = 0;


        /* [5] Destructor virtual: siempre hacerlo
           virtual si pensamos en usar herencia.
        */
        virtual ~Thread() {}



        /* [6] No tiene sentido copiar hilos, así
           que forzamos a que no se puedan copiar.
        */
        Thread(const Thread&) = delete;
        Thread& operator=(const Thread&) = delete;

        /* [7] Y aunque tiene sentido, vamos a ver
           que es un peligro permitir mover un thread
           así que también vamos a prohibir el move.

           Lo vas a entender cuando veas [10]
        */
        Thread(Thread&& other) = delete;
        Thread& operator=(Thread&& other) = delete;

};


/* [8] Un objeto q encapsula a un thread se lo conoce como un
       "objeto activo".

   Un objeto que tiene sus atributos y su lógica
   (que encapsula un algoritmo o una tarea) pero
   que vive en su propio hilo
*/
class IsPrime : public Thread {
    private:
        unsigned int n;
        bool &result;

    public:
        IsPrime(unsigned int n, bool &result) :
            n(n),
            result(result) {}

        /* [9] El contenido de este método sera el que se ejecute
           en el thread

           Nota: las keywords virtual y override en una clase hija
           no son necesarias pero **ayudan** a que quede explicita
           la intención: estamos **sobrescribiendo** un método virtual
           heredado.
        */
        virtual void run() override {
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

    std::vector<Thread*> threads;

    for (int i = 0; i < N; ++i) {
        /* [10] Acá es donde creamos nuestros objetos

           Por que usamos el heap?

           La vas a flipar:

           Cuando hagas Thread::start() vas a estar pasándole
           a std::thread un puntero this (un puntero al objeto IsPrime)

           No te olvides q esto no es más que una **dirección**
           de la memoria donde IsPrime esta "en ese momento"

           Si guardas el objeto IsPrime en el stack de main
           y luego lo moves a otro lado, tu objeto estará ahora
           en **otro lugar de la memoria** y esa **dirección** q le pasaste
           a std::thread ya **no** sera la dirección "actual"
           de IsPrime..

           En otras palabras std::thread estará usando un puntero/dirección
           con contenido indefinido.

           Vos podrías ser cauteloso y **no** mover a IsPrime nunca
           pero... que pasa si guardas IsPrime en un container
           como std::vector y este te lo mueve?

           >> Game over <<

           Esto se lo conoce como "pointer instability" y aunque hay
           algunos containers que garantizan estabilidad, es tricky.

           Tenes varias alternativas:

            - usas containers q provean pointer stability como std::list
            - usas containers pre-allocados y te aseguras q no sean
              redimensionados (como std::vector cuando le seteas capacity)
            - o usas el heap y asi tus objetos no seran movidos *aun*
              si el container se mueve/redimensiona

           La ultima opcion es la menos eficiente pero es **mucho**
           mas segura.

           Por eso usamos el heap: para que quede en un solo lugar
           y cualquier puntero a IsPrime se mantenga válido.
         * */
        Thread *t = new IsPrime(
                            nums[i],
                            results[i]);
        threads.push_back(t);

        /* [11] y acá "activamos" a los "objetos activos"
           (lanzamos el thread)
        */
        t->start();
    }

    /* ************************************** */
    /* Ahora: Todos los hilos están corriendo */
    /* ************************************** */

    /* [12] Esperamos a que cada hilo termine.
       Cada join bloqueara al hilo llamante (main)
       hasta que el hilo sobre el cual se le hace
       join (threads[i]) termine

       Ademas, por haber usado el heap debemos
       hacer el correspondiente delete
    */
    for (int i = 0; i < N; ++i) {
        threads[i]->join();
        delete threads[i];
    }

    /* **************************************** */
    /* Ahora: Todos los hilos terminaron y sus  */
    /* recursos limpiados con el join           */
    /* **************************************** */

    for (int i = 0; i < N; ++i) {
        std::cout << results[i] << " ";
    }
    std::cout << "\n";

    return 0;
}

/* [13]

   Challenge: reemplaza el std::vector<Thread*> por std::vector<IsPrime>
   con un capacity inicial de N

   O sea:

    std::vector<IsPrime> threads(N);

   Vas a tener q ajustar la creacion de los threads obviamente
   para q no sea en el heap sino *directamente* dentro del container:

    threads.emplace_back(nums[i], results[i]);

   Que va a pasar? Deberia funcionar sin problema.

   Al haber reservado capacidad para N threads vas a poder
   agregar hasta N threads sin reallocs.

   Y como los threads los creaste directamente dentro
   del container con emplace_back, no va a ver moves

   Asi, ese puntero "this" q esta en Thread::start
   sigue apuntando a un objeto valido.

   [14]

   Ahora lo vamos a romper.

   Cambia el capacity a 1:

    std::vector<IsPrime> threads(1);


   Que va a pasar? El emplace_back no va a encontrar espacio
   y va a redimensionar (realloc) el vector.

   Si tenes suerte, el vector va a tener espacio sin necesidad
   de moverse pero sino, el vector se **movera** a otra
   parte de la memoria con mas espacio.

   Y entonces, ese puntero "this" en Thread::start, va a estar
   apuntando "al viejo" objeto, no al que se movio.

   Tal vez se te va a romper, tal vez no :D

   [15]

   Ahora lo vamos a romper, pero de verdad.

   Cambia el capacity a N como era antes pero ahora no uses
   emplace_back sino crea el thread afuera y luego movelo
   al container con un std::move + push_back:

    IsPrime t(nums[i], results[i]);
    threads.push_back(std::move(t));

   Que va a pasar? Deberias tener crash garantizado.

   [16]
   Bonus question: por q en el challenge anterior usamos
   std::vector<IsPrime> y no std::vector<Thread> ? Por q
   no se puede instanciar std::vector<Thread> ?

   [17]
   Has llegado al final del ejercicio, continua
   con el siguiente.
*/


# Concurrencia


## Historia de la concurrencia

Los primeros problemas de concurrencia datan finales de 1800, donde se originaron algunos términos, como *semáforo*, para resolver problemas como el tránsito de múltiples trenes sobre las mismas vías, o transmitir multiples transmisiones telegráficas en un set de cables.

El primer paper académico sobre el tema fue publicado en 1965 por Edsger Dijkstra, y trataba sobre la exclusión mutua de N procesos.
Este problema trata sobre N procesos corriendo una aplicación, de forma concurrente, y tal de que solo uno de ellos por vez pueda entrar a una zona de código conocida como *sección crítica*.


### Primeras computadoras

En los inicios de la computación, las computadoras eran rentadas por tiempo fijo y un único usuario. Esto implicaba derroche de recursos cada vez que un usuario dejaba sin utilizar la CPU. Para combatir esto, se automatizó la carga de *trabajos*, por lo que un usuario podía encolar su programa en tarjetas perforadas / cinta, y un programa *monitor* en la CPU (similar a un SO) los cargaba y ejecutaba uno a uno automaticamente.

Al tiempo apareció un nuevo cuello de botella: cada vez que había que leer desde un dispositivo, se perdían muchos ciclos de computo esperando a que termine la lectura. Este desperdicio de ciclos fue sobrellevado con varias técnicas.
Una eran las entradas y salidas con *interrupciones*, de forma que una vez que termina una operación de E/S, se emite una interrupción al sistema.
Otra técnica era la *multiprogramación*, mediante la cuál varios programas pueden residir en memoria y se reparten el tiempo de CPU.

El uso de interrupciones trae los primeros problemas de concurrencia: las interrupciones pueden ocurrir aleatoriamente, si el sistema operativo está alterando una estructura de datos y ocurre una interrupción en el medio, si el handler de interrupciones utiliza esa estructura, puede llegar a estar en un estado inválido.

A medida que la tecnología fue mejorando, cada vez más aplicaciones entraban en memoria. Mientras que en los sistemas primitivos solo ocurría cambio de *contexto* cuando ocurría una E/S, posteriormente se desarrollaron sistemas *preventivos*, en los que el cambio de contexto se realizaba varias veces por segundo, evitando, por ejemplo, que el sistema se quede bloqueado ante un entrada muy lenta como es la del teclado.

Las CPUs siguieron incrementando su velocidad, pero cuando la frecuencia de la CPU empezó a toparse con límites físicos, se encontró la necesidad de trabajar con *múltiples núcleos*. Esto permite que cada núcleo ejecute un proceso en paralelo con los demás, incrementando más la necesidad de incorporar la concurrencia a la programación.


## Concurrencia vs paralelismo

*Concurrencia* es descomponer una o más tareas en subtareas más chicas de forma que la ejecución de las mismas pueda solaparse. *Paralelismo* es ejecutar dos o más tareas en simultaneo.
Concurrencia no necesariamente implica paralelismo. Se utiliza concurrencia en computadoras de un sólo núcleo para que las aplicaciones sean más usables.


### Ejemplo de concurrencia

Supongamos que tengo varios exámenes finales para resolver y practicar. Un alumno puede hacer todos por su cuenta, o dividir la tarea con un amigo, resolviendo cada uno un final por su cuenta. Como resultado, se acorta el tiempo total de la tarea, ya que se resuelven más problemas en simultaneo.

En este caso, la tarea de resolver finales se divide en subtareas (cada examen es una subtarea), con alto grado de paralelismo, ya que cada subtarea es independiente del resto y se pueden ejecutar en simultaneo sin dificultades.

Ahora, supongamos que se le acaba la tinta a uno de los alumnos, por lo que su compañero decide compartir la lapicera para seguir con la tarea.
Para no pelearse por quién usa la lapicera, coordinan que, mientras no estén usándola, dejarla disponible en la mesa para que cualquiera pueda agarrarla.

En este nuevo escenario, disminuye el paralelismo, ya que los alumnos no van a poder escribir al mismo tiempo (por falta de recursos), pero aumenta la concurrencia, porque las tareas ahora son más atómicas: pensar un ejercicio, tomar la lapicera, escribirlo, etc.


## Cómo se implementa

La concurrencia en un sistema operativo viene en 2 sabores: **Procesos** e **Hilos**

### Procesos

Un proceso es una instancia en ejecución de un programa. Posee su propio:

* Espacio de memoria
* Variables globales
* File descriptors
* Handlers de interrupciones
* Hilos

### Hilos

Todos los procesos tienen al menos un hilo, conocido como *hilo principal*. Los hilos comparten todo lo mencionado anteriormente excepto que cada uno tiene su propio:

* *Program Counter*
* Registros de uso general
* Stack
* Estado de ejecución (corriendo, esperando I/O, durmiendo, etc)

Pero *¿Qué es un hilo entonces?*

Un hilo (de ahora en adelante *thread*), es una función que se ejecuta concurrentemente.

Los threads tienen distintas implementaciones según el lenguaje. Por ejemplo, en C, la función que crea un thread recibe un puntero a función y un puntero a lo que sería el argumento genérico de esa función.
En Java, por otro lado, nos permite crear una clase que implemente un método *run()*, y un objeto *Thread* recibe un objeto que implemente este *run()* para ser llamado y ejecutado concurrentemente.
En C++ ofrece ambas formas de iniciar un thread (con diferencias sutiles).

La finalización, por su parte, es muy similar en todos los lenguajes. Un hilo, que generalmente es el que creó el thread, puede llamar a la función *join()* sobre un thread en ejecución. Esto hace que el hilo que llamó a *join()* se quede bloqueado hasta que el thread termine de ejecutar su función correspondiente.

#### Threads Posix en C

En los sistemas POSIX, existe una implementación de threads conocida como *pthreads*.
Esta implementación provee una función llamada *pthread_create*, que tiene como argumentos:

* Un puntero a una estructura thread, donde se escribe los datos del hilo creado.
* Una estructura con atributos para crear el hilo.
* Un puntero a función que recibe un único puntero void como parámetro.
* Un puntero void para pasarle información al thread.

**Ver src/threads-c.c**

#### Threads en la biblioteca STL de C++11

En el standard *C++11* se agregó threads a la biblioteca. La nueva clase *std::thread* puede ser usada de dos formas posibles: con funciones, de forma similar a pthread_create, o con un objeto *functor*, es decir, de una clase donde el operador paréntesis está sobreescrito.

**Ver src/Thread.cpp, agregar más ejemplos**

## Contador de palabras concurrente

### Contador secuencial

Vamos a hacer una aplicación que lea las ocurrencias de un caracter determinado en una lista de archivos pasados por parámetro.

En nuestra primera iteración, el programa será secuencial. La implementación, sencilla, se puede ver en *contador-01.cpp*

Para realizar benchmarks de la aplicación, utilizamos el comando *time*. Se puede ver que el resultado viene en 3 filas:

~~~
real    0m0.000s
user    0m0.000s
sys     0m0.000s
~~~

* *real* es el tiempo de reloj, como si lo cronometraramos.
* *user* es el tiempo de CPU consumido (por instrucciones de usuario). Si la aplicación tiene múltiples hilos, se los suma. El tiempo en que la aplicación está bloqueada o con sleep no se computa.
* *sys* es el tiempo de CPU consumido en instrucciones del kernel.

Para probar esta aplicación se recomienda generar un archivo considerablemente grande con la aplicación *random-gen* cuya fuente se encuentra en la misma carpeta.


### Contadores independientes en múltiples hilos

Vamos a optimizar la aplicación anterior ejecutando una instancia de contador de caracteres en un hilo aparte para cada archivo pasado por parámetro.

En esta iteración implementamos el contador de caracteres como un objeto *functor*, que va a correr sobre un hilo aparte. Este objeto *CharCounter* tiene como atributo un contador que se utilizará después de que el hilo haya terminado su ejecución. La implementación se puede ver en *contador-02.cpp*.

Notar que, si los archivos a procesar son muy grandes, se nota una baja considerable en el tiempo en que tarda en procesar todos los archivos.
Notar también que, a medida que se van agregando más archivos, la mejora del tiempo va achicándose.
El número óptimo de hilos suele ser la cantidad de CPUs virtuales que tenga la máquina en la que esté corriendo. Esto se puede usando el comando *lscpu*.


### Contador compartido, primer intento

Ahora modificamos la clase *CharCounter* para que, en vez de tener un contador propio, tenga un contador compartido con los otros contadores (por si, por ejemplo, queremos informar al usuario sobre el progreso de la aplicación).

La implementación en *contador-03.cpp* ahora tiene un contador en el hilo main, y los objetos CharCounter se construyen con una referencia al mismo

¿Qué pasa cuando lo ejecuto con grandes volúmenes de datos?

Se puede ver que los caracteres detectados "son menos". Lo que está ocurriendo realmente es lo que se conoce como **race condition**: los contadores son accedidos por más de un hilo a la vez, uno lee un valor, lo modifica, pero antes que logre guardarlo, otro hilo también lo modifica y lo guarda.
El primer hilo finalmente guarda *su* modificación, sin enterarse de los cambios ingresados por el segundo hilo, corrompiéndose la información.

Este tipo de errores sucede aleatoriamente, y no siempre es tan evidente el origen del mismo. Por esta razón debuggear una aplicación concurrente es muy complicado y no puede hacerse con un debugger como tradicionalmente hacíamos.

Para solucionar este problema es necesario identificar cuál es la *critical section* (sección crítica) del programa, es decir, la sección de código que no soporta correctamente la concurrencia. En nuestro ejemplo, la sección crítica es el acceso y actualización del contador, por lo que sincronizar el acceso al contador debería reparar la aplicación.


### Contador compartido, final

En este ejemplo final, *contador-04.cpp*, se agrega un mecanismo de sincronización, llamado **Mutex** para acceder al recurso compartido.

El resultado vuelve a ser el correcto, sin embargo, si se vuelve a medir el performance de la aplicación frente a la implementación 2, se puede apreciar que el tiempo real y el tiempo del sistema aumenta. Esto se debe a que las llamadas a *lock()* y *unlock()* del mutex requieren llamadas al sistema para bloquear y desbloquear otros hilos.

Con este ejemplo se puede apreciar que mayor concurrencia no siempre equivale a mayor rendimiento, y que a mayor paralelismo, menos problemas de sincronización hay

## Deadlocks

**Agregar**
Ver **deadlock.cpp** y **deadlock-ok**, por qué es que una implementación de account falla y la otra no.


## Monitores

**Definir monitores** (wrapper de una clase, de forma tal que quede thread-safe...)


# Bibliografía

* Michael L. Scott: *Programming Language Pragmatics* (Cap 12)

* https://blog.heroku.com/archives/2013/2/24/concurrency_is_not_parallelism
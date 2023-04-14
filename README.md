# Tutoriales interactivos de programación multithreading

En este *hands-on* veras como tener programas multithreading,
los problemas que trae (race conditions, deadlocks) y las herramientas
para resolverlos (mutexes, locks, conditional variables y queues)

Como bonus veras functors y caso de aplicación de RAII en C++

Lee el código fuente de los archivos en orden,
comenzando por `01_is_prime_sequential.cpp`

En cada tutorial encontraras comentarios para guiarte en los ejercicios.
Debes seguirlos en orden comenzando por `[1]`, `[2]`, ...

Este es un tutorial *interactivo* y depende de vos hacerlo; contiene
información muy *valiosa* para entender como los threads funcionan
así como también los principales errores y técnicas para solucionarlos.

Otras primitivas de sincronización (read-write locks, semáforos,
reentrant locks) no están incluidas en este *hands-on* así como tampoco
performance (cache invalidation, false sharing) ni problemas clásicos
de concurrencia ni queues más avanzadas.

Eso quedaran para *hands-on*s futuros.

## Como compilar / correr los tests?

Solo tenes que correr `make`

## Licencia

GPL v2

## Puedo usar este código en el Trabajo Práctico?

Si, pero tenes que decir de donde lo sacaste y respetar la licencia.

En `libs/` tendrás la clase `Thread` y la clase `Queue<T>` para que
uses.

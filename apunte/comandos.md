# Comandos

## Compilar

Para compilar es necesario *cmake* y gcc compatible con C++11

Utilizar CMake para generar el makefile necesario para compilar las aplicaciones. En un entorno unix se puede hacer, desde la carpeta src:

~~~{.bash}
# deberias estar posicionado en el mismo lugar que este .md
cd src
mkdir build
cd build
cmake ..
make
~~~

## Aplicaciones

* *threads-c*: Ejemplo de threads POSIX en C

* *contador-01*: Contador de letras secuencial

* *contador-02*: Contador de letras multihilo

* *contador-03*: Contador de letras, con race-condition

* *contador-04*: Contador de letras sincronizado

* *deadlock*: Ejemplo de deadlock

* *not-deadlock*: Solución al problema de deadlock anterior

* *random-gen*: generador de texto aleatorio


### Ejemplo de ejecución

~~~{.bash}
# deberias estar posicionado en la carpeta build
./random-gen 1200400
time ./contador-01 random.txt
~~~

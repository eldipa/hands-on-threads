#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

struct printer_param_t {
	int times;
	char* str;
};

void* printString(void* printerParam) {
    struct printer_param_t *param = (struct printer_param_t *) printerParam;
    for (int i = 0; i < param->times; ++i) {
        printf("[%d] str: %s\n", i, param->str);
        sleep(1);
    }
}

int main (int argc, char** argv) {
    struct printer_param_t thParams, mainParams;
    // No abusen de los TDAs
    thParams.times = 5;
    thParams.str = "Hola posix threads, soy un hilo";
    mainParams.times = 3;
    mainParams.str = "Hola posix threads, soy main";
    
    printf("Creando un thread\n");
    pthread_t printThread;
    // Imprimo desde un hilo
    pthread_create(&printThread, 0, printString, &thParams);

    // Tambien imprimo desde main
    printString(&mainParams);
    
    printf("Haciendo el join\n");
    // Espero a que printThread haya terminado
    pthread_join(printThread, 0);
    printf("Joined!\n");
}

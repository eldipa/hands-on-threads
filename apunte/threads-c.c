#include <pthread.h>
#include <stdio.h>

struct printer_param_t {
	int times;
	char* str;
};

void* printString(void* printerParam) {
    struct printer_param_t *param = (struct printer_param_t *) printerParam;
    for (int i = 0; i < param->times; ++i) {
        printf("[%d] str: %s\n", i, param->str);
    }
}

int main (int argc, char** argv) {
    struct printer_param_t printParams;
    // No abusen de los TDAs
    printParams.times = 5;
    printParams.str = "Hola posix threads";
    printf("Creando un thread\n");
    pthread_t printThread;
    pthread_create(&printThread, 0, printString, &printParams);
    printf("Haciendo el join\n");
    pthread_join(printThread, 0);
    printf("Joined!\n");
}

#include <cstdio>
#include <vector>

int countChar(char* filename, char myChar);

int main (int argc, char** argv) {
	std::vector<int> counters(argc);
	for (int i = 1; i < argc; ++i){
		counters[i] = countChar(argv[i], 'a');
	}
	for (int i = 1; i < argc; ++i){
		printf("%s tiene %d letras 'a'\n", argv[i], counters[i]);
	}
}

int countChar(char* filename, char myChar) {
	FILE* fd = fopen(filename, "r");
	int counter = 0;
	char readChar;
	while (fread(&readChar, 1, 1, fd)) {
		if (readChar == myChar) {
			counter++;
		}
	}
        fclose(fd);
	return counter;
}

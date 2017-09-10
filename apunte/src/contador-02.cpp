#include <cstdio>
#include <vector>
#include <string>
#include <thread>

class CharCounter {
public:
	CharCounter(const char* filename, char countChar);
	void operator()();
	void printResult() const;
	int getResult() const;
private:
	std::string filename;
	char countChar;
	int result;
};

/**
 * Demo2: Se instancian N counters, como objeto thread lo copia
 * le paso una referencia a los resultados para que escriba afuera del thread
 */

int main (int argc, char** argv) {
	std::vector<CharCounter> counters;
	std::vector<std::thread*> threads;
	
	counters.reserve(argc);
	for (int i = 0; i < argc-1; ++i){
		counters.push_back(CharCounter(argv[i+1], 'a'));
	}
	
	for (unsigned int i = 0; i < (counters.size()); ++i){
		std::thread *t = new std::thread(std::ref(counters[i]));
		threads.push_back(t);
	}
	printf("Llamando a join\n");
	for (unsigned int i = 0; i < (counters.size()); ++i){
		threads[i]->join();
		delete threads[i];
	}
	int finalResult = 0;
	for (unsigned int i = 0; i < (counters.size()); ++i){
		counters[i].printResult();
		finalResult += counters[i].getResult();
	}
	printf("El resultado final es %d. \n", finalResult);
}


/**
 * Función a correr en paralelo
 */

void CharCounter::operator() () {
	printf("Busco '%c' sobre %s\n", this->countChar, this->filename.c_str());
	FILE* fd = fopen(this->filename.c_str(), "r");
	char readChar;
	while (fread(&readChar, 1, 1, fd)){
		if (readChar == this->countChar) {
			this->result++;
		}
	}
        fclose(fd);
}

CharCounter::CharCounter(const char* filename, char countChar) :
	filename(filename), countChar(countChar), result(0){}

void CharCounter::printResult() const {
	printf("%s tiene %d letras '%c'\n", this->filename.c_str(), this->result, this->countChar);
}

int CharCounter::getResult() const {
	return this->result;
}

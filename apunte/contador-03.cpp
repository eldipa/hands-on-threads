#include <cstdio>
#include <vector>
#include <string>
#include <thread>

class CharCounter {
public:
	CharCounter(const char* filename, char countChar, int& result);
	void operator()();
	void printResult() const;
private:
	std::string filename;
	char countChar;
	int& result;
};


/**
 * Demo3: Se instancian N counters, esta vez se utiliza un único
 * contador para los resultados, pero al no estar protegido
 * falla el conteo
 */


int main (int argc, char** argv) {
	std::vector<CharCounter> counters;
	int finalResult = 0;
	std::vector<std::thread*> threads;
	
	for (int i = 1; i < argc; ++i){
		counters.push_back(CharCounter(argv[i], 'a', finalResult));
	}
	
	for (unsigned int i = 0; i < (counters.size()); ++i){
		threads.push_back(new std::thread(counters[i]));
	}
	printf("Llamando a join\n");
	for (unsigned int i = 0; i < (counters.size()); ++i){
		threads[i]->join();
		delete threads[i];
	}
	printf("Hay %d letras 'a'", finalResult);
}

void CharCounter::operator() () {
	printf("Busco '%c' sobre %s\n", this->countChar, this->filename.c_str());
	FILE* fd = fopen(this->filename.c_str(), "r");
	char readChar;
	while (fread(&readChar, 1, 1, fd)){
		if (readChar == this->countChar) {
			this->result++;
		}
	}
}

CharCounter::CharCounter(const char* filename, char countChar, int& result) :
	filename(filename), countChar(countChar), result(result){}

void CharCounter::printResult() const {
	printf("%s tiene %d letras '%c'\n", this->filename.c_str(), this->countChar, this->result);
}

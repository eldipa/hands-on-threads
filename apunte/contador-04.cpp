#include <cstdio>
#include <vector>
#include <string>
#include <thread>
#include <mutex>

class CharCounter {
public:
	CharCounter(const char* filename, char countChar,
				int& result, std::mutex& mutex);
	void operator()();
	void printResult() const;
private:
	std::string filename;
	char countChar;
	int& result;
	std::mutex& mutex;
};

int main (int argc, char** argv) {
	std::vector<CharCounter> counters;
	int finalResult = 0;
	std::mutex mutex;
	std::vector<std::thread*> threads;
	
	for (int i = 1; i < argc; ++i){
		counters.push_back(CharCounter(argv[i], 'a', finalResult, mutex));
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
			mutex.lock();
			this->result++;
			mutex.unlock();
		}
	}
}

CharCounter::CharCounter(const char* filename, char countChar,
						 int& result, std::mutex& mutex) :
	filename(filename), countChar(countChar),
	result(result), mutex(mutex){}

void CharCounter::printResult() const {
	printf("%s tiene %d letras '%c'\n", this->filename.c_str(), this->countChar, this->result);
}

#include <fstream>
#include <iostream>
#include <random>
#include <string>

#define OUTPUT_FILE_BUFFER_SIZE 1024

static const std::string validChars = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEF \n";

char getRandChar();

int main (int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Uso: ./random-gen <output-size>" << std::endl;
        return 1;
    }
    int outputSize = std::stoi(argv[1]);

    std::ofstream output ("random.txt", std::ofstream::binary);

    char outputBuffer[OUTPUT_FILE_BUFFER_SIZE];
    int i = 0;
    for (; i < outputSize; ++i) {
        int charPos = i % OUTPUT_FILE_BUFFER_SIZE;
        if (charPos == 0 && i != 0) {
            output.write(outputBuffer, OUTPUT_FILE_BUFFER_SIZE);
        }
        outputBuffer[charPos] = getRandChar();
    }
    output.write(outputBuffer, i % OUTPUT_FILE_BUFFER_SIZE);
    output.close();
    return 0;
}

char getRandChar() {
    static std::default_random_engine generator;
    static std::uniform_int_distribution<int> distribution(0, validChars.size() - 1);
    int randPos = distribution(generator);
    return validChars[randPos];
}
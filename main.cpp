#include <iostream>
#include <fstream>


#include "CSVParser.h"


int main() {
    std::ifstream file("test.csv");

    CSVParser<std::string, int, double> parser(file, 0, ',', '"');

    try {
        for (const auto &rs: parser) {
            std::cout << rs << std::endl;
        }
    } catch (const CSVParseException &e) {
        std::cerr << "Error at line " << e.getLine() << ", column " << e.getColumn() << ": " << e.what() << std::endl;
    }

    return 0;
}


#include <iostream>
#include <string>

#include "Compile.h"
#include "defs.h"

int main(int argc, const char** argv) { 
    LINIT();

    if(argc < 4) {
        std::cout << "Failed to parse arguments:\n\t-c fileName outputName: compile binary from one file\n\t-o fileName outputFile: compile linker ready object file\n";
        return 1;
    }

    LTRACE("START");
    if(strcmp(argv[1], "-c") == 0) {
        std::string fileName = argv[2];
        std::string outputName = argv[3];
        
        compileFile(fileName, outputName);
    }
    else if(strcmp(argv[1], "-o") == 0) {

    }
    LTRACE("STOPPED");

    return 0;
}
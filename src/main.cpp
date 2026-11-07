#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include "Interpreter.h"

using namespace std;

int main(int argc, char* argv[]) {
    if(argc < 4) { // program needs the input argument
        cerr << "usage: ./aps1 <input> <default_registers> <default_memory>" << endl;
        return -1;
    }

    string_view path = argv[1];
    ifstream f(path.data());
    if(f.is_open()) {
        // parse each line into an instructions array
        vector<string> instructions;

        string line;
        while(getline(f, line)) {
            instructions.push_back(line);
        }

        f.close();

        // set cout's output to output file
        ofstream output("output.txt");

        streambuf* console_buffer = cout.rdbuf();
        cout.rdbuf(output.rdbuf());

        // begin emulation
        Interpreter interpreter;
        if(!interpreter.load_registers(argv[2])) {
            cout.rdbuf(console_buffer);
            return -1;
        }

        if(!interpreter.load_memory(argv[3])) {
            cout.rdbuf(console_buffer);
            return -1;
        }

        interpreter.push_instructions(instructions);
        interpreter.run();

        // return cout's output to console
        cout.rdbuf(console_buffer);
    }
    else {
        cerr << "invalid input path: " << path << endl;
        return -1;
    }

    return 0;
}

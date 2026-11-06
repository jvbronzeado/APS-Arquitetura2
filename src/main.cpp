#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <charconv>

#include "types.h"
#include "Emulator.h"

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
        vector<u32> instructions;

        string line;
        while(getline(f, line)) {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            if(line.size() != 23 || line.find_first_not_of("01") != string::npos) {
                cerr << "invalid line " << line << endl;
                continue;
            }
            int ins = stoi(line, nullptr, 2);
            instructions.push_back(ins);
        }

        f.close();

        // set cout's output to output file
        ofstream output("output.txt");

        streambuf* console_buffer = cout.rdbuf();
        cout.rdbuf(output.rdbuf());

        // begin emulation        
        Emulator emu(instructions);
        if(!emu.load_registers(argv[2])) {
            cout.rdbuf(console_buffer);
            return -1;
        }

        if(!emu.load_memory(argv[3])) {
            cout.rdbuf(console_buffer);
            return -1;
        }

        emu.init();
        while(emu.cycle()) {}

        // return cout's output to console
        cout.rdbuf(console_buffer);
    }
    else {
        cerr << "invalid input path: " << path << endl;
        return -1;
    }

    return 0;
}

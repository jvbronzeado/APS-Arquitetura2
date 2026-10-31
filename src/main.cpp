#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <charconv>

#include "types.h"
#include "Emulator.h"

using namespace std;

int main(int argc, char* argv[]) {
    if(argc < 2) { // program needs the input argument
        cerr << "usage: ./aps1 <input> [A=value] [B=value]" << endl;
        return -1;
    }

    u32 a_param = ~0;
    u32 b_param = 1;
    
    const vector<string_view> args(argv, argv + argc); // parse args to string
    for(int i = 2; i < argc; i++) {
        string_view s = args[i];
        if(s.size() <= 2)
            continue;
        if(s[1] != '=')
            continue;

        string_view numstr = s.substr(2);

        u32 value = 0;
        auto [ptr, ec] = from_chars(numstr.data(), numstr.data() + numstr.size(), value);
        if(ec == errc::invalid_argument) {
            cerr << "error: not a valid number (" << s << ")" << endl;
            continue;
        }
        else if(ec == errc::result_out_of_range) {
            cerr << "error: number too large or small (" << s << ")" << endl;
            continue;
        }

        if(s[0] == 'A')
            a_param = value;
        else if(s[0] == 'B')
            b_param = value;
        else
            cerr << "error: unknown parameter (" << s << ")" << endl;
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

            if(line.size() != 6) {
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
        Emulator emu(instructions, a_param, b_param);
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

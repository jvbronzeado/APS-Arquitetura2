#include "Interpreter.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <bitset>
#include <iomanip>

#include "instructions/ILOAD.h"
#include "instructions/DUP.h"
#include "instructions/BIPUSH.h"

namespace {
const string table_border = "+--------+----------------------------------+----------------------------------+";

void print_table_header(const string& first_column) {
    cout << table_border << '\n'
         << "| " << left << setw(6) << first_column
         << " | Before                           | After                            |\n"
         << table_border << '\n';
}

void print_table_row(const string& label, const string& before, const string& after) {
    cout << "| " << left << setw(6) << label
         << " | " << left << setw(32) << before
         << " | " << left << setw(32) << after << " |\n";
}

string register_bits(u32 value, bool is_byte) {
    return is_byte ? bitset<8>(value).to_string() : bitset<32>(value).to_string();
}

void print_state_change(const Mic1::State& before, const Mic1::State& after) {
    const char* register_names[] = {"MAR", "MDR", "PC", "MBR", "SP", "LV", "CPP", "TOS", "OPC", "H"};
    constexpr size_t mbr_index = 3;

    print_table_header("State");
    for (size_t i = 0; i < before.registers.size(); ++i) {
        bool is_byte = i == mbr_index;
        print_table_row(register_names[i],
                        register_bits(before.registers[i], is_byte),
                        register_bits(after.registers[i], is_byte));
    }
    cout << table_border << "\nMemory\n";

    print_table_header("Address");
    for (size_t i = 0; i < before.memory.size(); ++i) {
        print_table_row(to_string(i),
                        bitset<32>(before.memory[i]).to_string(),
                        bitset<32>(after.memory[i]).to_string());
    }
    cout << table_border << "\n\n";
}
}

Interpreter::Interpreter() {
    m_processor = new Mic1();
}

Interpreter::~Interpreter() {
    delete m_processor;
}

bool Interpreter::load_registers(const string& path) {
    return m_processor->load_registers(path);
}

bool Interpreter::load_memory(const string& path) {
    return m_processor->load_memory(path);
}

void Interpreter::push_instructions(const vector<string>& instructions) {
    for(const string& s : instructions) {
        m_pending.push(s);
    }
}

void Interpreter::run() {
    m_processor->init();
    
    while(!m_pending.empty()) {
        string line = m_pending.front();
        m_pending.pop();

        // tokenize
        stringstream stream(line);

        vector<string> tokens;
        string t;
        while(stream >> t) {
            tokens.push_back(t); 
        }

        if(tokens.size() == 0)
            continue;

        // create instruction from tokens
        optional<unique_ptr<IJVMInstruction>> instruction = handle_tokens(tokens);
        if(!instruction.has_value()) {
            cerr << "failed to execute line: " << line << endl;
            return;
        }

        // execute
        execute_microinstructions(*instruction.value());
    }   
}

optional<unique_ptr<IJVMInstruction>> Interpreter::handle_tokens(const vector<string>& tokens) {
    const string& mnemonic = tokens[0];
    if(mnemonic == "ILOAD") {
        if(tokens.size() < 2) {
            cerr << "syntax error: missing address variable in ILOAD" << endl;
            return nullopt;
        }
        
        if(tokens.size() > 2) {
            cerr << "syntax error: unknown extra variables for ILOAD" << endl;
            return nullopt;
        }

        string address = tokens[1];
        if(!std::all_of(address.begin(), address.end(), [](unsigned char c) {
            return std::isdigit(c);
        })) {
            cerr << address << " not a number" << endl;
            return nullopt;
        }
        
        u32 num = stoul(address);
        return make_unique<ILOAD>(num);
    }
    else if(mnemonic == "DUP") {
        if(tokens.size() > 1) {
            cerr << "unknown extra variable for DUP" << endl;
            return nullopt;
        }

        return make_unique<DUP>();
    }
    else if(mnemonic == "BIPUSH") {
        if(tokens.size() < 2) {
            cerr << "missing byte variable in BIPUSH" << endl;
            return nullopt;
        }
        
        if(tokens.size() > 2) {
            cerr << "unknown extra variables for BIPUSH" << endl;
            return nullopt;
        }        

        string address = tokens[1];
        if(address.find_first_not_of("01") != string::npos) {
            cerr << address << " not a number" << endl;
            return nullopt;
        }

        if(address.size() != 8) {
            cerr << address << " doesn't have 8 bits" << endl;
            return nullopt;
        }
        
        u8 num = stoul(address, nullptr, 2);
        return make_unique<BIPUSH>(num);
    }
    else {
        cerr << "unknown mnemonic " << mnemonic << endl;
    }
    return nullopt;
}

void Interpreter::execute_microinstructions(IJVMInstruction& instruction)  {
    const Mic1::State before = m_processor->state();

    m_processor->load_instructions(instruction.translate());
    while (m_processor->cycle()) {}

    const Mic1::State after = m_processor->state();

    cout << "Instruction: " << instruction.get_name() << '\n';
    print_state_change(before, after);
}

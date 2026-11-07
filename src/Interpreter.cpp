#include "Interpreter.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <bitset>

#include "instructions/ILOAD.h"
#include "instructions/DUP.h"
#include "instructions/BIPUSH.h"

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
    cout << "Executing " << instruction.get_name() << " instruction" << endl;

    vector<u32> microinstructions = instruction.translate();

    for(u32 x : microinstructions) {
        cout << bitset<23>(x) << '\n';
    }

    cout << endl;
    
    m_processor->load_instructions(microinstructions);
    while(m_processor->cycle()){}

    cout << endl;
}

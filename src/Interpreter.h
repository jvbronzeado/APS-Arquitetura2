#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <vector>
#include <queue>
#include <string>
#include <optional>
#include <memory>

#include "instructions/IJVMInstruction.h"
#include "Microprocessor.h"

using namespace std;

class Interpreter {
public:
    Interpreter();
    ~Interpreter();

    bool load_registers(const string& path);
    bool load_memory(const string& path);
    
    void push_instructions(const vector<string>& instructions);

    void run();
private:
    optional<unique_ptr<IJVMInstruction>> handle_tokens(const vector<string>& tokens);
    void execute_microinstructions(IJVMInstruction& instruction);

    Mic1* m_processor;
    queue<string> m_pending;
};

#endif

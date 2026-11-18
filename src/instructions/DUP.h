#ifndef DUP_H_
#define DUP_H_

#include <vector>

#include "../types.h"
#include "IJVMInstruction.h"

using namespace std;

/*
DUP: duplicates the value at the top of the stack.
Increments SP and stores a copy of TOS in the new position
*/
class DUP : public IJVMInstruction {
public:
    DUP() {};

    string get_name() override {
        return "DUP";        
    }

    vector<u32> translate() override {
        return {        
            0b00110101000001001000100, // MAR = SP = SP+1;
            0b00110100000000010100111 // MDR = TOS; wr
        };
    };

    vector<string> translation_log() override {
        return {"MAR = SP = SP+1", "MDR = TOS; wr"};
    }
private:
};

#endif

#ifndef BIPUSH_H_
#define BIPUSH_H_

#include <vector>
#include <bitset>

#include "../types.h"
#include "IJVMInstruction.h"

using namespace std;

/*
BIPUSH byte: places the given byte at the top of the stack.
Increments SP, stores the value in memory, and updates TOS
*/
class BIPUSH : public IJVMInstruction {
public:
    BIPUSH(u8 byte) : m_byte(byte) {};

    string get_name() override {
        return "BIPUSH " + std::bitset<8>(m_byte).to_string();
    }
    
    vector<u32> translate() override {
        return {        
        0b00110101000001001000100, // SP = MAR = SP+1;
        ((u32)(m_byte) << 15) | 0b000000000110000, // fetch
        0b00011000001000010100111 // MDR = TOS = H; wr
        };
    };

    vector<string> translation_log() override {
        return {"SP = MAR = SP+1", "fetch", "MDR = TOS = H; wr"};
    }
private:
    u8 m_byte;
};

#endif

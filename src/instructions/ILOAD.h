#ifndef ILOAD_H_
#define ILOAD_H_

#include <vector>
#include <string>

#include "../types.h"
#include "IJVMInstruction.h"

using namespace std;

class ILOAD : public IJVMInstruction {
public:
    ILOAD(u32 address) : m_address(address) {};

    string get_name() override {
        return "ILOAD";
    }
        
    vector<u32> translate() override {
        vector<u32> out;
        out.push_back(0b00010100100000000000101); // H = LV

        for(u32 i = 0; i < m_address; i++) {
            out.push_back(0b00111001100000000000000); // H = H + 1
        }

        out.push_back(0b00111000000000001010000); // MAR = H; rd
        out.push_back(0b00110101000001001100100); // MAR = SP = SP+1; wr
        out.push_back(0b00110100001000000000000); // TOS = MDR

        return out;
    };
private:
    u32 m_address;
};

#endif

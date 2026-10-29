#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <stdint.h>
#include <vector>

#include "types.h"

using namespace std;

class Emulator {
public:
    Emulator(const vector<u32>& instructions, u32 a, u32 b)
        : m_instructions(instructions), m_a(a), m_b(b){
        
    }

    void init();
    bool cycle();
private:
    struct ULAInput {
        u8 a, b;
        u8 ena, enb;
        u8 inva;

        u8 f0, f1;
        u8 in_carry;
    };
    struct ULAOutput {
        u8 sum;
        u8 out_carry;
    };
    
    ULAOutput process_ula(ULAInput input);

    pair<u32, u32> execute_instruction();
    
    vector<u32> m_instructions;
    u32 m_a;
    u32 m_b;
    u32 m_s;
    u32 m_co;
    
    u32 m_cycle = 0;
    u32 m_pc = 0;
    u32 m_ir = 0;
};

#endif

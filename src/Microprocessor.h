#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <stdint.h>
#include <vector>
#include <string>
#include <array>

#include "types.h"

using namespace std;

class Mic1 {
public:
    struct State {
        std::array<u32, 10> registers;
        std::array<u32, 16> memory;
    };
    Mic1() {}
    
    Mic1(const vector<u32>& instructions) {
        load_instructions(instructions);
    }

    void init();
    void load_instructions(const vector<u32>& instructions);
    bool cycle();
    State state() const;
    bool load_registers(const std::string& path); // load default registers
    bool load_memory(const std::string& path); // load init memory
private:

    struct ULAInput {
        u8 a, b;
        u8 ena, enb;
        u8 inva;

        u8 f0, f1;
        u8 in_carry;

        u8 sll8, sra1;
    };
    struct ULAOutput {
        u8 sum;
        u8 out_carry;
    };
    
    ULAOutput process_ula(ULAInput input);

    void process_decoder(u8 mask); // handles B bus register selection
    void process_selector(u16 mask); // handles write to selected registers

    void execute_instruction();
    
    vector<u32> m_instructions;
    u32 m_memory[16];
    
    u32 m_a;
    u32 m_b;
    u32 m_s;
    u32 m_sd;
    u32 m_co;

    u32 m_n;
    u32 m_z;
    
    u32 m_cycle = 0;
    u32 m_ir = 0;

    struct Registers {
        u32 h; // auxiliar registror utilized on ULA input
        u32 opc; // temporaly stores a value from PC
        u32 tos; // stores a copy of the value at the top of the stack
        u32 cpp; // points to constants
        u32 lv; // points to the base of local variables
        u32 sp; // points to the top of the stack
        u32 pc; // points to the next instruction of the program
        u32 mdr; // stores the read data in memory or the data that will be written in it
        u32 mar; // stores the memory address that will be read or written

        u8 mbr; // stores a byte. in this implementation, fetch puts in it the BIPUSH argument
    };
    
    Registers m_regs{};
    Registers m_defaultregs{};
};

#endif

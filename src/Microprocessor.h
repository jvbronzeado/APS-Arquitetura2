#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <stdint.h>
#include <vector>
#include <string>

#include "types.h"

using namespace std;

class Mic1 {
public:
    Mic1() {}
    
    Mic1(const vector<u32>& instructions) {
        load_instructions(instructions);
    }

    void init();
    void load_instructions(const vector<u32>& instructions);
    bool cycle();
    bool load_registers(const std::string& path); // load default registers
    bool load_memory(const std::string& path); // load init memory
private:
    void print_registers() const;
    void print_memory() const;

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
        u32 h;
        u32 opc;
        u32 tos;
        u32 cpp;
        u32 lv;
        u32 sp;
        u32 pc;
        u32 mdr;
        u32 mar;

        u8 mbr;
    };

    Registers m_regs{};
    Registers m_defaultregs{};
};

#endif

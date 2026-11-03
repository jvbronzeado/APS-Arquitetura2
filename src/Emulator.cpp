#include "Emulator.h"
#include <iostream>
#include <bitset>
#include "types.h"

void Emulator::init() {
    cout << "b = " << std::bitset<32>(m_b) << endl;    
    cout << "a = " << std::bitset<32>(m_a) << endl;
    cout << endl;
    cout << "Start of program" << endl;

    m_cycle = 0;
    m_regs.pc = 0;
    m_ir = 0;
}

bool Emulator::cycle() {
    m_cycle++;
    
    cout << "========================================" << endl;
    cout << "Cycle " << m_cycle << endl;
    cout << endl;
    cout << "PC = " << m_regs.pc + 1 << endl;

    if(m_regs.pc >= m_instructions.size()) {
        cout << "> Line is empty, EOP" << endl;
        return false;
    }

    m_ir = m_instructions[m_regs.pc++];

    cout << "IR " << std::bitset<8>(m_ir) << endl;

    if((m_ir & (0b11000000)) == 0b11000000) {
        cout << "> Error, invalid control signals." << endl;
        return true;
    }

    auto vars = execute_instruction();
    
    cout << "b " << std::bitset<32>(vars.second) << endl;
    cout << "a " << std::bitset<32>(vars.first) << endl;
    cout << "s " << std::bitset<32>(m_s) << endl;
    cout << "sd " << std::bitset<32>(m_sd) << endl;
    cout << "N " << (u32)m_n << endl;
    cout << "Z " << (u32)m_z << endl;
    cout << "co " << (u32)m_co << endl;

    return true;
}

Emulator::ULAOutput Emulator::process_ula(ULAInput input) {
    ULAOutput result;
    
    u8 ca = (input.a & input.ena) ^ input.inva;
    u8 cb = (input.b & input.enb);

    // Logic Unit
    u8 AB = ca & cb;
    u8 ApB = ca | cb;
    u8 nB = (~cb) & 1;

    // decoder
    u8 d0 = (~input.f0) & (~input.f1);
    u8 d1 = (~input.f0) & (input.f1);
    u8 d2 = (input.f0) & (~input.f1);
    u8 d3 = (input.f0) & (input.f1);
    
    // complete Adder
    u8 xor0 = ca ^ cb;
    u8 adder_result = input.in_carry ^ xor0;

    u8 carry_and0 = ca & cb & d3;
    u8 carry_and1 = xor0 & input.in_carry & d3;

    result.out_carry = carry_and0 | carry_and1;
    result.sum = (AB & d0) | (ApB & d1) | (nB & d2) | (adder_result & d3);

    return result;
}

pair<u32, u32> Emulator::execute_instruction() {
    ULAInput input = {
        .a = 0,
        .b = 0,
        .ena = (u8)((m_ir >> 3) & 1), // read ENA bit
        .enb = (u8)((m_ir >> 2) & 1), // read ENB bit
        .inva = (u8)((m_ir >> 1) & 1), // read INVA bit
        .f0 = (u8)((m_ir >> 5) & 1), // read F0 bit
        .f1 = (u8)((m_ir >> 4) & 1), // read F1 bit
        .in_carry = (u8)(m_ir & 1), // read INC bit
        .sll8 = (u8)((m_ir >> 7) & 1), // read SLL8 bit
        .sra1 = (u8)((m_ir >> 6) & 1)
    };

    m_s = 0;

    // go through all bits, from right to left, with the INC of the last iteration
    for(int i = 0; i < 32; i++) {
        input.a = (m_a >> i) & 1;
        input.b = (m_b >> i) & 1;

        ULAOutput output = process_ula(input);
        m_s |= ((u32)(output.sum) << i);

        input.in_carry = output.out_carry;
        m_co = output.out_carry;
    }

    m_sd = m_s;

    // calculate Negative and Zero sinals
    m_n = (m_s >> 31) & 1;
    m_z = (m_s == 0);

    // process sll8 and sra1
    if(input.sll8 && input.sra1) {
        cerr << "INVALID CONTROL SIGNALS" << endl;
    }
    else if(input.sll8) {
        m_sd = (m_s << 8);
    }
    else if(input.sra1) {
        m_sd = (m_s >> 1) | (m_s & (1 << 31));
    }

    // returns A with ENA and INVA effects, and B with ENB effects
    u32 a_out = input.ena ? m_a : 0u;
    if (input.inva)
        a_out = ~a_out;

    u32 b_out = input.enb ? m_b : 0u;

    return {a_out, b_out};
}

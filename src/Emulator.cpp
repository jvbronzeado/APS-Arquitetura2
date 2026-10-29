#include "Emulator.h"
#include <iostream>
#include <bitset>

void Emulator::init() {
    cout << "b = " << std::bitset<32>(m_b) << endl;    
    cout << "a = " << std::bitset<32>(m_a) << endl;
    cout << endl;
    cout << "Start of program" << endl;

    m_cycle = 0;
    m_pc = 0;
    m_ir = 0;
}

bool Emulator::cycle() {
    m_cycle++;
    
    cout << "========================================" << endl;
    cout << "Cycle " << m_cycle << endl;
    cout << endl;
    cout << "PC = " << m_pc + 1 << endl;

    if(m_pc >= m_instructions.size()) {
        cout << "> Line is empty, EOP" << endl;
        return false;
    }

    m_ir = m_instructions[m_pc];

    auto vars = execute_instruction();
    
    cout << "IR " << std::bitset<6>(m_ir) << endl;
    cout << "b " << std::bitset<32>(vars.second) << endl;
    cout << "a " << std::bitset<32>(vars.first) << endl;
    cout << "s " << std::bitset<32>(m_s) << endl;
    cout << "co " << m_co << endl;

    m_pc++;
    return true;
}

Emulator::ULAOutput Emulator::process_ula(ULAInput input) {
    ULAOutput result;
    
    uint8_t ca = (input.a & input.ena) ^ input.inva;
    uint8_t cb = (input.b & input.enb);

    // Logic Unit
    uint8_t AB = ca & cb;
    uint8_t ApB = ca | cb;
    uint8_t nB = ~input.b;

    // decoder
    uint8_t d0 = (~input.f0) & (~input.f1);
    uint8_t d1 = (~input.f0) & (input.f1);
    uint8_t d2 = (~input.f0) & (input.f1);
    uint8_t d3 = (input.f0) & (input.f1);
    
    // complete Adder
    uint8_t xor0 = ca ^ cb;
    uint8_t adder_result = input.in_carry ^ xor0;

    uint8_t carry_and0 = ca & cb & d3;
    uint8_t carry_and1 = xor0 & input.in_carry & d3;

    result.out_carry = carry_and0 | carry_and1;
    result.sum = (AB & d0) | (ApB & d1) | (nB & d2) | (adder_result & d3);

    return result;
}

pair<uint32_t, uint32_t> Emulator::execute_instruction() {
    ULAInput input = {
        .a = 0,
        .b = 0,
        .ena = (uint8_t)((m_ir >> 3) & 1), // read ENA bit
        .enb = (uint8_t)((m_ir >> 2) & 1), // read ENB bit
        .inva = (uint8_t)((m_ir >> 1) & 1), // read INVA bit
        .f0 = (uint8_t)((m_ir >> 5) & 1), // read F0 bit
        .f1 = (uint8_t)((m_ir >> 4) & 1), // read F1 bit
        .in_carry = (uint8_t)(m_ir & 1), // read INC bit
    };

    m_s = 0;

    // go through all bits, from right to left, with the INC of the last iteration
    for(int i = 0; i < 32; i++) {
        input.a = (m_a >> i) & 1;
        input.b = (m_b >> i) & 1;

        ULAOutput output = process_ula(input);
        m_s |= (output.sum << i);

        input.in_carry = output.out_carry;
        m_co = output.out_carry;
    }

    // returns A with ENA and INVA effects, and B with ENB effects 
    return {
        (input.inva ? ~m_a : m_a) * input.ena,
        m_b * input.enb,
    };
}

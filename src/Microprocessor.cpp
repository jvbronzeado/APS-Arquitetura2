#include "Microprocessor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "types.h"

void Mic1::init() {
    m_cycle = 0;
    m_ir = 0;
}

void Mic1::load_instructions(const vector<u32>& instructions) {
    m_instructions = instructions;
    m_cycle = 0;
}

bool Mic1::cycle() {
    if(m_cycle >= m_instructions.size()) {
        return false;
    }

    m_ir = m_instructions[m_cycle++];

    u32 b_selector = m_ir & 0xF;
    u32 memory_mask = (m_ir >> 4) & 0x3;

    if((m_ir & (3u << 21)) == (3u << 21)) {
        cerr << "Error, invalid control signals.\n";
    }
    else if(b_selector >= 9) {
        cerr << "Error, invalid B bus selector.\n";
    }
    else if(memory_mask == 3 && (m_ir & 0x7FFF) != 0b110000) {
        cerr << "Error, invalid memory signals.\n";
    }
    else {
        execute_instruction();
    }

    return true;
}

Mic1::State Mic1::state() const {
    State snapshot{{m_regs.mar, m_regs.mdr, m_regs.pc, m_regs.mbr,
                    m_regs.sp, m_regs.lv, m_regs.cpp, m_regs.tos,
                    m_regs.opc, m_regs.h}, {}};
    for (size_t i = 0; i < snapshot.memory.size(); ++i)
        snapshot.memory[i] = m_memory[i];
    return snapshot;
}

bool Mic1::load_registers(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        cerr << "invalid register path: " << path << endl;
        return false;
    }
    std::string line;
    while (std::getline(input, line)) {
        std::istringstream stream(line);
        std::string name, equals, bits;
        if (!(stream >> name))
            continue;

        if (!(stream >> equals >> bits) || equals != "=" ||
            bits.size() != (name == "mbr" ? 8u : 32u) ||
            bits.find_first_not_of("01") != std::string::npos) {
            cerr << "invalid register line: " << line << endl;
            return false;
        }

        u32 value = (u32)(std::stoul(bits, nullptr, 2));
        if (name == "mar") m_defaultregs.mar = value;
        else if (name == "mdr") m_defaultregs.mdr = value;
        else if (name == "pc") m_defaultregs.pc = value;
        else if (name == "mbr") m_defaultregs.mbr = value;
        else if (name == "sp") m_defaultregs.sp = value;
        else if (name == "lv") m_defaultregs.lv = value;
        else if (name == "cpp") m_defaultregs.cpp = value;
        else if (name == "tos") m_defaultregs.tos = value;
        else if (name == "opc") m_defaultregs.opc = value;
        else if (name == "h") m_defaultregs.h = value;
        else {
            cerr << "unknown register: " << name << endl;
            return false;
        }
    }

    m_regs = m_defaultregs;
    return true;
}

bool Mic1::load_memory(const string& path) {
    ifstream file(path);
    if(!file.is_open()) {
        cerr << "invalid memory path: " << path << endl;
        return false;
    }

    string line;
    for(int i = 0; i < 16; i++) {
        if(!getline(file, line)) {
            cerr << "missing memory at line " << i << " (" << path << ")" << endl; 
            return false;
        }

        if(line.find_first_not_of("01") != std::string::npos) {
            cerr << "invalid memory line " << i << ": " << line << endl;
            return false;
        }

        u32 value = (u32)(stoul(line, nullptr, 2));
        m_memory[i] = value;
    }

    file.close();
    return true;
}

void Mic1::process_decoder(u8 mask) {
    mask &= 0xF;

    switch(mask) {
    case 0: m_b = m_regs.mdr; break;
    case 1: m_b = m_regs.pc; break;
    case 2: m_b = (u32)((int32_t)((int8_t)m_regs.mbr)); break;
    case 3: m_b = (u32)m_regs.mbr; break;
    case 4: m_b = m_regs.sp; break;
    case 5: m_b = m_regs.lv; break;
    case 6: m_b = m_regs.cpp; break;
    case 7: m_b = m_regs.tos; break;
    case 8: m_b = m_regs.opc; break;
    default:
        break;
    }
}

void Mic1::process_selector(u16 mask) {
    for(int i = 0; i < 9; i++) {
        u8 bit = (mask >> i) & 1;
        if(bit == 0)
            continue;
        switch(i) {
        case 0: m_regs.mar = m_sd; break;
        case 1: m_regs.mdr = m_sd; break;
        case 2: m_regs.pc = m_sd; break;
        case 3: m_regs.sp = m_sd; break;
        case 4: m_regs.lv = m_sd; break;
        case 5: m_regs.cpp = m_sd; break;
        case 6: m_regs.tos = m_sd; break;
        case 7: m_regs.opc = m_sd; break;
        case 8: m_regs.h = m_sd; break;
        }
    }
}

Mic1::ULAOutput Mic1::process_ula(ULAInput input) {
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

void Mic1::execute_instruction() {
    u8 ulac = (m_ir >> 15);
    u8 mem = (m_ir >> 4) & 3;
    if(mem == 3 && (m_ir & 0x7FFF) == 0b110000) {
        // fetch instruction
        m_regs.mbr = ulac;
        m_regs.h = ulac;
        return;
    }
    
    ULAInput input = {
        .a = 0,
        .b = 0,
        .ena = (u8)((ulac >> 3) & 1), // read ENA bit
        .enb = (u8)((ulac >> 2) & 1), // read ENB bit
        .inva = (u8)((ulac >> 1) & 1), // read INVA bit
        .f0 = (u8)((ulac >> 5) & 1), // read F0 bit
        .f1 = (u8)((ulac >> 4) & 1), // read F1 bit
        .in_carry = (u8)((ulac >> 0) & 1), // read INC bit
        .sll8 = (u8)((ulac >> 7) & 1), // read SLL8 bit
        .sra1 = (u8)((ulac >> 6) & 1) // read SRA1 bit
    };

    m_a = m_regs.h;
    process_decoder(m_ir & 0xF);

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
    if(input.sll8) {
        m_sd = (m_s << 8);
    }
    else if(input.sra1) {
        m_sd = (m_s >> 1) | (m_s & (1 << 31));
    }

    // apply results from C bus
    process_selector((m_ir >> 6) & 0x1FF);

    // apply results to memory
    bool read = (mem & 1) == 1;
    bool write = ((mem >> 1) & 1) == 1;

    if(write) {
        m_memory[m_regs.mar] = m_regs.mdr;
    }

    if(read) {
        m_regs.mdr = m_memory[m_regs.mar];
    }
}

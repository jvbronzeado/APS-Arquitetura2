#include "Emulator.h"
#include <iostream>
#include <bitset>
#include <fstream>
#include <sstream>
#include "types.h"

const char* b_names[] = {"mdr", "pc", "mbr", "mbru", "sp", "lv", "cpp", "tos", "opc"};
const char* c_names[] = {"mar", "mdr", "pc", "sp", "lv", "cpp", "tos", "opc", "h"};

void Emulator::init() {
    for (u32 instruction : m_instructions)
        cout << std::bitset<21>(instruction) << endl;
    cout << "\n=====================================================\n> Initial register states" << endl;
    print_registers();
    cout << "\n=====================================================" << endl;
    cout << "Start of program" << endl;

    m_cycle = 0;
    m_ir = 0;
}

bool Emulator::cycle() {
    m_cycle++;
    
    cout << "=====================================================" << endl;
    cout << "Cycle " << m_cycle << endl;

    if(m_cycle > m_instructions.size()) {
        cout << "No more lines, EOP." << endl;
        return false;
    }

    m_ir = m_instructions[m_cycle-1];

    u32 b_selector = m_ir & 0xF;
    u32 c_mask = (m_ir >> 4) & 0x1FF;
    cout << "ir = " << std::bitset<8>(m_ir >> 13) << ' '
         << std::bitset<9>(c_mask) << ' ' << std::bitset<4>(b_selector) << "\n\n";
    cout << "b_bus = " << (b_selector < 9 ? b_names[b_selector] : "invalid") << endl;

    cout << "c_bus = ";
    bool first = true;
    for (int i = 8; i >= 0; --i) if (c_mask & (1u << i)) {
        if (!first) cout << ", ";
        cout << c_names[i];
        first = false;
    }
    if (first) {
        cout << "none";
    }

    cout << "\n\n> Registers before instruction" << endl;
    print_registers();

    if((m_ir & (3u << 19)) == (3u << 19)) {
        cout << "> Error, invalid control signals." << endl;
    }
    else if(b_selector >= 9) {
        cout << "> Error, invalid B bus selector." << endl;
    }
    else {
        execute_instruction();
    }

    cout << "\n> Registers after instruction" << endl;
    print_registers();

    //m_regs = m_defaultregs;
    return true;
}

void Emulator::print_registers() const {
    cout << "mar = " << std::bitset<32>(m_regs.mar) << '\n'
         << "mdr = " << std::bitset<32>(m_regs.mdr) << '\n'
         << "pc = " << std::bitset<32>(m_regs.pc) << '\n'
         << "mbr = " << std::bitset<8>(m_regs.mbr) << '\n'
         << "sp = " << std::bitset<32>(m_regs.sp) << '\n'
         << "lv = " << std::bitset<32>(m_regs.lv) << '\n'
         << "cpp = " << std::bitset<32>(m_regs.cpp) << '\n'
         << "tos = " << std::bitset<32>(m_regs.tos) << '\n'
         << "opc = " << std::bitset<32>(m_regs.opc) << '\n'
         << "h = " << std::bitset<32>(m_regs.h) << endl;
}

bool Emulator::load_registers(const std::string& path) {
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
            cerr << "invalid register line: " << line << endl; return false;
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

void Emulator::process_decoder(u8 mask) {
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

void Emulator::process_selector(u16 mask) {
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

void Emulator::execute_instruction() {
    ULAInput input = {
        .a = 0,
        .b = 0,
        .ena = (u8)((m_ir >> 16) & 1), // read ENA bit
        .enb = (u8)((m_ir >> 15) & 1), // read ENB bit
        .inva = (u8)((m_ir >> 14) & 1), // read INVA bit
        .f0 = (u8)((m_ir >> 18) & 1), // read F0 bit
        .f1 = (u8)((m_ir >> 17) & 1), // read F1 bit
        .in_carry = (u8)((m_ir >> 13) & 1), // read INC bit
        .sll8 = (u8)((m_ir >> 20) & 1), // read SLL8 bit
        .sra1 = (u8)((m_ir >> 19) & 1) // read SRA1 bit
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
    process_selector((m_ir >> 4) & 0x1FF);
}

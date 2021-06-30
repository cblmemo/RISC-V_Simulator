//
// Created by Rainy Memory on 2021/6/28.
//

#ifndef RISC_V_SIMULATOR_PARSER_H
#define RISC_V_SIMULATOR_PARSER_H

#include <string>

typedef unsigned int uint;

unsigned int hex_to_uint(const std::string &str) {
    unsigned int ret = 0;
    for (char c:str) {
        ret <<= 4;
        if (c >= '0' && c <= '9')ret += c - '0';
        else ret += c - 'A' + 10;
    }
    return ret;
}

unsigned char hex_to_uc(char c) {
    return c >= '0' && c <= '9' ? c - '0' : c - 'A' + 10;
}

enum CommandFormat {
    R, I, S, B, U, J, None
};

enum CommandType {
    //format: [funct7](7bit)+[funct3](3bit)+[opcode](7bit)
    LUI = 0b0110111,
    AUIPC = 0b0010111,
    JAL = 0b1101111,
    JALR = 0b1100111,
    BEQ = 0b1100011 + (0b000 << 7),
    BNE = 0b1100011 + (0b001 << 7),
    BLT = 0b1100011 + (0b100 << 7),
    BGE = 0b1100011 + (0b101 << 7),
    BLTU = 0b1100011 + (0b110 << 7),
    BGEU = 0b1100011 + (0b111 << 7),
    LB = 0b0000011 + (0b000 << 7),
    LH = 0b0000011 + (0b001 << 7),
    LW = 0b0000011 + (0b010 << 7),
    LBU = 0b0000011 + (0b100 << 7),
    LHU = 0b0000011 + (0b101 << 7),
    SB = 0b0100011 + (0b000 << 7),
    SH = 0b0100011 + (0b001 << 7),
    SW = 0b0100011 + (0b010 << 7),
    ADDI = 0b0010011 + (0b000 << 7),
    SLTI = 0b0010011 + (0b010 << 7),
    SLTIU = 0b0010011 + (0b011 << 7),
    XORI = 0b0010011 + (0b100 << 7),
    ORI = 0b0010011 + (0b110 << 7),
    ANDI = 0b0010011 + (0b111 << 7),
    SLLI = 0b0010011 + (0b001 << 7),
    SRLI = 0b0010011 + (0b101 << 7) + (0b0000000 << 10),
    SRAI = 0b0010011 + (0b101 << 7) + (0b0100000 << 10),
    ADD = 0b0110011 + (0b000 << 7) + (0b0000000 << 10),
    SUB = 0b0110011 + (0b000 << 7) + (0b0100000 << 10),
    SLL = 0b0110011 + (0b001 << 7),
    SLT = 0b0110011 + (0b010 << 7),
    SLTU = 0b0110011 + (0b011 << 7),
    XOR = 0b0110011 + (0b100 << 7),
    SRL = 0b0110011 + (0b101 << 7) + (0b0000000 << 10),
    SRA = 0b0110011 + (0b101 << 7) + (0b0100000 << 10),
    OR = 0b0110011 + (0b110 << 7),
    AND = 0b0110011 + (0b111 << 7)
};

CommandFormat get_format(CommandType type) {
    switch (type) {
        case LUI:
        case AUIPC:
            return CommandFormat::U;
        case JAL:
            return CommandFormat::J;
        case JALR:
        case LB:
        case LH:
        case LW:
        case LBU:
        case LHU:
        case ADDI:
        case SLTI:
        case SLTIU:
        case XORI:
        case ORI:
        case ANDI:
        case SLLI:
        case SRLI:
        case SRAI:
            return CommandFormat::I;
        case BEQ:
        case BNE:
        case BLT:
        case BGE:
        case BLTU:
        case BGEU:
            return CommandFormat::B;
        case SB:
        case SH:
        case SW:
            return CommandFormat::S;
        case ADD:
        case SUB:
        case SLL:
        case SLT:
        case SLTU:
        case XOR:
        case SRL:
        case SRA:
        case OR:
        case AND:
            return CommandFormat::R;
        default:
            std::cerr << "[Error]function [get_format] wrong with a undefined CommandType." << std::endl;
            return CommandFormat::None;
    }
}

uint get_immediate(uint cmd, CommandFormat type) {
    uint ret = 0, temp;
    switch (type) {
        case R:
            break;
        case I:
            ret = (cmd & (uint) 0xfff00000) >> 20;
            if ((cmd & (uint) 0x80000000) == 0x80000000)ret |= 0xfffff000;
            break;
        case S:
            ret = (cmd & (uint) 0xfe000000) >> 20;
            temp = (cmd & (uint) 0x00000f80) >> 7;
            ret |= temp;
            if ((cmd & (uint) 0x80000000) == 0x80000000)ret |= 0xfffff000;
            break;
        case B:
            ret = (cmd & (uint) 0x7e000000) >> 20;
            temp = (cmd & (uint) 0x00000f00) >> 7;
            ret |= temp;
            temp = (cmd & (uint) 0x00000080) << 4;
            ret |= temp;
            if ((cmd & (uint) 0x80000000) == 0x80000000)ret |= 0xfffff000;
            break;
        case U:
            ret = cmd & (uint) 0xfffff000;
            break;
        case J:
            ret = (cmd & (uint) 0x7fe00000) >> 20;
            temp = (cmd & (uint) 0x000ff000);
            ret |= temp;
            temp = (cmd & (uint) 0x00100000) >> 9;
            ret |= temp;
            if ((cmd & (uint) 0x80000000) == 0x80000000)ret |= 0xfff00000;
            break;
        default:
            break;
    }
    return ret;
}

uint get_opcode(uint cmd) {
    return cmd & (uint) 0x0000007f;
}

uint get_rs1(uint cmd) {
    return (cmd & (uint) 0x000f8000) >> 15;
}

uint get_rs2(uint cmd) {
    return (cmd & (uint) 0x01f00000) >> 20;
}

uint get_rd(uint cmd) {
    return (cmd & (uint) 0x00000f80) >> 7;
}

uint get_funct3(uint cmd) {
    return (cmd & (uint) 0x00007000) >> 12;
}

uint get_funct7(uint cmd) {
    return (cmd & (uint) 0xfe000000) >> 25;
}

uint get_shamt(uint cmd) {
    return (cmd & (uint) 0x01f00000) >> 20;
}

#endif //RISC_V_SIMULATOR_PARSER_H

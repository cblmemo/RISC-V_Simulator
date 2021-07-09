//
// Created by Rainy Memory on 2021/7/1.
//

#pragma GCC optimize ("Ofast")

#include <iostream>
#include <bitset>

using uint = unsigned int;
using uc = unsigned char;

#pragma region parser

uint hex_to_uint(const std::string &str) {
    unsigned int ret = 0;
    for (char c:str) {
        ret <<= 4;
        if (c >= '0' && c <= '9')ret += c - '0';
        else ret += c - 'A' + 10;
    }
    return ret;
}

uc hex_to_uc(char c) {
    return c >= '0' && c <= '9' ? c - '0' : c - 'A' + 10;
}

enum CommandFormat {
    R, I, S, B, U, J, None
};

enum CommandType {
    //format: [funct7](7bit) + [funct3](3bit) + [opcode](7bit)
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
            std::cerr << "[Error]function [get_format()] wrong with a undefined CommandType." << std::endl;
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
            std::cerr << "[Error]function [get_immediate()] wrong with a undefined CommandType." << std::endl;
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

#pragma endregion

class CPU {
private:
    using uint = unsigned int;
    using uc = unsigned char;
    
    class Memory {
    private:
        static constexpr uint MEMORY_SIZE = 0x20000;
        
        uc memory[MEMORY_SIZE] = {0};
    
    public:
        void initialize(std::istream &is) {
            uint address;
            std::string address_str;
            char temp;
            while (is >> temp) {
                if (temp == '@') {
                    is >> address_str;
                    address = hex_to_uint(address_str);
                }
                else if (temp == '#')break;
                else {
                    memory[address] |= hex_to_uc(temp) << 4;
                    is >> temp;
                    memory[address++] |= hex_to_uc(temp);
                }
            }
        }
        
        uint getCommand(uint address) {
            return (uint) memory[address] + ((uint) memory[address + 1] << 8) + ((uint) memory[address + 2] << 16) + ((uint) memory[address + 3] << 24);
        }
        
        uint readByte(uint addr) {
            return (uint) memory[addr];
        }
        
        uint readHalfWord(uint addr) {
            return (uint) memory[addr] | (uint) memory[addr + 1] << 8;
        }
        
        uint readWord(uint addr) {
            return (uint) memory[addr] | (uint) memory[addr + 1] << 8 | (uint) memory[addr + 2] << 16 | (uint) memory[addr + 3] << 24;
        }
        
        void writeByte(uint addr, uint val) {
            memory[addr] = (uc) val;
        }
        
        void writeHalfWord(uint addr, uint val) {
            memory[addr] = (uc) val;
            memory[addr + 1] = (uc) (val >> 8);
        }
        
        void writeWord(uint addr, uint val) {
            memory[addr] = (uc) val;
            memory[addr + 1] = (uc) (val >> 8);
            memory[addr + 2] = (uc) (val >> 16);
            memory[addr + 3] = (uc) (val >> 24);
        }
    };
    
    class Register {
    private:
        using uint = unsigned int;
        
        uint reg[32] = {0};
    
    public:
        uint &operator[](uint addr) {
            return reg[addr];
        }
        
        const uint &operator[](uint addr) const {
            return reg[addr];
        }
    };
    
    Register reg;
    Memory mem;
    bool stop = false;
    uint pc = 0;
    
    void InstructionFetch() {
        //read command to register
        f2dReg.cmd = mem.getCommand(pc);
        std::cerr << pc << "\t\t" << std::hex << f2dReg.cmd << "\t\t" << std::dec;
        if (f2dReg.cmd == 0x0ff00513)stop = true;
        pc += 4;
    }
    
    struct IF2IDReg {
        uint cmd;
    } f2dReg {};
    
    void InstructionDecode() { // && Register Fetch
        uint code = get_opcode(f2dReg.cmd);
        switch (code) {
            case 0b0110111:
            case 0b0010111:
            case 0b1101111:
            case 0b1100111:
                d2eReg.type = (CommandType) code;
                break;
            default:
                code |= get_funct3(f2dReg.cmd) << 7;
                uint temp;
                switch (code) {
                    case SRLI:
                        temp = code | (get_funct7(f2dReg.cmd) << 10);
                        d2eReg.type = temp == code ? SRLI : SRAI;
                        break;
                    case ADD:
                        temp = code | (get_funct7(f2dReg.cmd) << 10);
                        d2eReg.type = temp == code ? ADD : SUB;
                        break;
                    case SRL:
                        temp = code | (get_funct7(f2dReg.cmd) << 10);
                        d2eReg.type = temp == code ? SRL : SRA;
                        break;
                    default:
                        d2eReg.type = (CommandType) code;
                        break;
                }
                break;
        }
        CommandFormat format = get_format(d2eReg.type);
        d2eReg.immediate = get_immediate(f2dReg.cmd, format);
        uint rs1 = get_rs1(f2dReg.cmd);
        uint rs2 = get_rs2(f2dReg.cmd);
        d2eReg.rd = get_rd(f2dReg.cmd);
        //Register Fetch
        switch (d2eReg.type) {
            case LUI:
            case AUIPC:
            case JAL:
                //do nothing
                break;
            case SLLI:
            case SRLI:
            case SRAI:
                d2eReg.shamt = get_shamt(f2dReg.cmd);
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
                d2eReg.rs1val = reg[rs1];
                break;
            case BEQ:
            case BNE:
            case BLT:
            case BGE:
            case BLTU:
            case BGEU:
            case SB:
            case SH:
            case SW:
            case ADD:
            case SUB:
            case SLL:
            case SRL:
            case SRA:
            case SLT:
            case SLTU:
            case XOR:
            case OR:
            case AND:
                d2eReg.rs1val = reg[rs1];
                d2eReg.rs2val = reg[rs2];
                break;
            default:
                std::cerr << "[Error]function [InstructionDecode()] wrong with a undefined CommandType." << std::endl;
                break;
        }
        std::cerr << std::bitset<sizeof(uint) * 2>(d2eReg.type) << "\t\t" << d2eReg.immediate << "\t\t\t" << d2eReg.rd << "\t\t"
                  << rs1 << "\t\t" << rs2 << "\t\t" << d2eReg.rs1val << "\t\t" << d2eReg.rs2val << std::endl;
    }
    
    struct ID2EXReg {
        CommandType type;
        uint immediate;
        uint rs1val;
        uint rs2val;
        uint rd;
        uint shamt;
    } d2eReg {};
    
    void Execute() {
        //calculate
        switch (d2eReg.type) {
            case LB:
            case LH:
            case LW:
            case LBU:
            case LHU:
            case SB:
            case SH:
            case SW:
                //do nothing
                break;
            case LUI:
                e2mReg.val = d2eReg.immediate;
                break;
            case AUIPC:
                pc += d2eReg.immediate - 4;
                e2mReg.val = d2eReg.immediate;
                break;
            case JAL:
                e2mReg.val = pc;
                pc += d2eReg.immediate - 4;
                break;
            case JALR:
                e2mReg.val = pc;
                pc = (d2eReg.rs1val + d2eReg.immediate) & -1;
                break;
            case BEQ:
                if ((int) d2eReg.rs1val == (int) d2eReg.rs2val)
                    pc += d2eReg.immediate - 4;
                break;
            case BNE:
                if ((int) d2eReg.rs1val != (int) d2eReg.rs2val)
                    pc += d2eReg.immediate - 4;
                break;
            case BLT:
                if ((int) d2eReg.rs1val < (int) d2eReg.rs2val)
                    pc += d2eReg.immediate - 4;
                break;
            case BGE:
                if ((int) d2eReg.rs1val >= (int) d2eReg.rs2val)
                    pc += d2eReg.immediate - 4;
                break;
            case BLTU:
                if (d2eReg.rs1val < d2eReg.rs2val)
                    pc += d2eReg.immediate - 4;
                break;
            case BGEU:
                if (d2eReg.rs1val >= d2eReg.rs2val)
                    pc += d2eReg.immediate - 4;
                break;
            case ADDI:
                e2mReg.val = d2eReg.rs1val + d2eReg.immediate;
                break;
            case SLTI:
                e2mReg.val = (int) d2eReg.rs1val < (int) d2eReg.immediate ? 1 : 0;
                break;
            case SLTIU:
                e2mReg.val = d2eReg.rs1val < d2eReg.immediate ? 1 : 0;
                break;
            case XORI:
                e2mReg.val = d2eReg.rs1val ^ d2eReg.immediate;
                break;
            case ORI:
                e2mReg.val = d2eReg.rs1val | d2eReg.immediate;
                break;
            case ANDI:
                e2mReg.val = d2eReg.rs1val & d2eReg.immediate;
                break;
            case SLLI:
                e2mReg.val = d2eReg.rs1val << d2eReg.shamt;
                break;
            case SRLI:
                e2mReg.val = d2eReg.rs1val >> d2eReg.shamt;
                break;
            case SRAI:
                e2mReg.val = d2eReg.rs1val >> d2eReg.shamt;
                if ((d2eReg.rs1val & 0x80000000) == 0x80000000)
                    e2mReg.val |= (0xffffffff << (32 - d2eReg.shamt));
                break;
            case ADD:
                e2mReg.val = d2eReg.rs1val + d2eReg.rs2val;
                break;
            case SUB:
                e2mReg.val = d2eReg.rs1val - d2eReg.rs2val;
                break;
            case SLL:
                e2mReg.val = d2eReg.rs1val << (d2eReg.rs2val & 0x0000001f);
                break;
            case SRL:
                e2mReg.val = d2eReg.rs1val >> (d2eReg.rs2val & 0x0000001f);
                break;
            case SRA:
                e2mReg.val = d2eReg.rs1val >> (d2eReg.rs2val & 0x0000001f);
                if ((d2eReg.rs1val & 0x80000000) == 0x80000000)
                    e2mReg.val |= (0xffffffff << (32 - (d2eReg.rs2val & 0x0000001f)));
                break;
            case SLT:
                e2mReg.val = (int) d2eReg.rs1val < (int) d2eReg.rs2val ? 1 : 0;
                break;
            case SLTU:
                e2mReg.val = d2eReg.rs1val < d2eReg.rs2val ? 1 : 0;
                break;
            case XOR:
                e2mReg.val = d2eReg.rs1val ^ d2eReg.rs2val;
                break;
            case OR:
                e2mReg.val = d2eReg.rs1val | d2eReg.rs2val;
                break;
            case AND:
                e2mReg.val = d2eReg.rs1val & d2eReg.rs2val;
                break;
            default:
                std::cerr << "[Error]function [Execute()] wrong with a undefined CommandType." << std::endl;
                break;
        }
        e2mReg.type = d2eReg.type;
        e2mReg.rs1val = d2eReg.rs1val;
        e2mReg.rs2val = d2eReg.rs2val;
        e2mReg.rd = d2eReg.rd;
        e2mReg.immediate = d2eReg.immediate;
    }
    
    struct EX2MEMReg {
        CommandType type;
        uint immediate;
        uint val;
        uint rs1val;
        uint rs2val;
        uint rd;
    } e2mReg {};
    
    void MemoryAccess() {
        //read from and write to memory
        m2wReg.needWB = true;
        switch (e2mReg.type) {
            case LUI:
            case AUIPC:
            case JAL:
            case JALR:
            case ADDI:
            case SLTI:
            case SLTIU:
            case XORI:
            case ORI:
            case ANDI:
            case SLLI:
            case SRLI:
            case SRAI:
            case ADD:
            case SUB:
            case SLL:
            case SRL:
            case SRA:
            case SLT:
            case SLTU:
            case XOR:
            case OR:
            case AND:
                m2wReg.val = e2mReg.val;
                break;
            case BEQ:
            case BNE:
            case BLT:
            case BGE:
            case BLTU:
            case BGEU:
                m2wReg.needWB = false;
                break;
            case LB:
                m2wReg.val = mem.readByte(e2mReg.rs1val + e2mReg.immediate);
                if ((m2wReg.val & 0x00000080) == 0x00000080)m2wReg.val |= 0xffffff00;
                break;
            case LH:
                m2wReg.val = mem.readHalfWord(e2mReg.rs1val + e2mReg.immediate);
                if ((m2wReg.val & 0x00008000) == 0x00008000)m2wReg.val |= 0xffff0000;
                break;
            case LW:
                m2wReg.val = mem.readWord(e2mReg.rs1val + e2mReg.immediate);
                break;
            case LBU:
                m2wReg.val = mem.readByte(e2mReg.rs1val + e2mReg.immediate);
                break;
            case LHU:
                m2wReg.val = mem.readHalfWord(e2mReg.rs1val + e2mReg.immediate);
                break;
            case SB:
                m2wReg.needWB = false;
                mem.writeByte(e2mReg.rs1val + e2mReg.immediate, e2mReg.rs2val);
                break;
            case SH:
                m2wReg.needWB = false;
                mem.writeHalfWord(e2mReg.rs1val + e2mReg.immediate, e2mReg.rs2val);
                break;
            case SW:
                m2wReg.needWB = false;
                mem.writeWord(e2mReg.rs1val + e2mReg.immediate, e2mReg.rs2val);
                break;
            default:
                std::cerr << "[Error]function [MemoryAccess()] wrong with a undefined CommandType." << std::endl;
                break;
        }
        m2wReg.rd = e2mReg.rd;
    }
    
    struct MEM2WBReg {
        uint rd;
        uint val;
        bool needWB;
    } m2wReg {};
    
    void WriteBack() {
        //write to register
        if (m2wReg.needWB && m2wReg.rd != 0)reg[m2wReg.rd] = m2wReg.val;
    }

public:
    void runSequenceExecute() {
        std::ios::sync_with_stdio(false);
        mem.initialize(std::cin);
        std::cerr << "pc\t\tinst\t\ttype\t\timm\t\t\trd\t\trs1\t\trs2\t\trs1val\t\trs2val" << std::endl;
        while (true) {
            InstructionFetch();
            if (stop)break;
            InstructionDecode();
            Execute();
            MemoryAccess();
            WriteBack();
        }
        std::cout << (((uint) reg[10]) & 0x000000ff) << std::endl;
    }
};

int main() {
    CPU m1;
    m1.runSequenceExecute();
    return 0;
}
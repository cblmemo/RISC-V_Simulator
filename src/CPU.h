//
// Created by Rainy Memory on 2021/6/28.
//

#ifndef RISC_V_SIMULATOR_CPU_H
#define RISC_V_SIMULATOR_CPU_H

#include "Register.h"
#include "Memory.h"

class CPU {
private:
    using uint = unsigned int;
    
    static constexpr uint codes[9] = {
            0b0110111,
            0b0010111,
            0b1101111,
            0b1100111,
            0b1100011,
            0b0000011,
            0b0100011,
            0b0010011,
            0b0110011,
    };
    
    Register reg;
    Memory mem;
    bool stop = false;
    uint pc = 0;
    
    void InstructionFetch() {
        //read command to register
        fReg.cmd = mem.getCommand(pc);
        if (fReg.cmd == 0x0ff00513)stop = true;
        pc += 4;
    }
    
    struct IF2IDReg {
        uint cmd;
    } fReg {};
    
    void InstructionDecode() { // && Register Fetch
        uint code = get_opcode(fReg.cmd);
        int index;
        for (int i = 0; i < 9; i++)
            if (code == codes[i])index = i;
        switch (index) {
            case 0:
            case 1:
            case 2:
            case 3:
                dReg.type = (CommandType) code;
                break;
            default:
                code |= get_funct3(fReg.cmd) << 7;
                uint temp;
                switch (code) {
                    case SRLI:
                        temp = code | (get_funct7(fReg.cmd) << 10);
                        dReg.type = temp == code ? SRLI : SRAI;
                        break;
                    case ADD:
                        temp = code | (get_funct7(fReg.cmd) << 10);
                        dReg.type = temp == code ? ADD : SUB;
                        break;
                    case SRL:
                        temp = code | (get_funct7(fReg.cmd) << 10);
                        dReg.type = temp == code ? SRL : SRA;
                        break;
                    default:
                        dReg.type = (CommandType) code;
                        break;
                }
                break;
        }
        CommandFormat format = get_format(dReg.type);
        dReg.immediate = get_immediate(fReg.cmd, format);
        uint rs1 = get_rs1(fReg.cmd);
        uint rs2 = get_rs2(fReg.cmd);
        dReg.rd = get_rd(fReg.cmd);
        //Register Fetch
        switch (dReg.type) {
            case LUI:
            case AUIPC:
            case JAL:
                //do nothing
                break;
            case SLLI:
            case SRLI:
            case SRAI:
                dReg.shamt = get_shamt(fReg.cmd);
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
                dReg.rs1val = reg[rs1];
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
                dReg.rs1val = reg[rs1];
                dReg.rs2val = reg[rs2];
                break;
            default:
                std::cerr << "[Error]function [InstructionDecode()] wrong with a undefined CommandType." << std::endl;
                break;
        }
    }
    
    struct ID2EXReg {
        CommandType type;
        uint immediate;
        uint rs1val;
        uint rs2val;
        uint rd;
        uint shamt;
    } dReg {};
    
    void Execute() {
        //calculate
        switch (dReg.type) {
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
                eReg.val = dReg.immediate;
                break;
            case AUIPC:
                pc += dReg.immediate - 4;
                eReg.val = dReg.immediate;
                break;
            case JAL:
                eReg.val = pc;
                pc += dReg.immediate - 4;
                break;
            case JALR:
                eReg.val = pc;
                pc = (dReg.rs1val + dReg.immediate) & -1;
                break;
            case BEQ:
                if ((int) dReg.rs1val == (int) dReg.rs2val)
                    pc += dReg.immediate - 4;
                break;
            case BNE:
                if ((int) dReg.rs1val != (int) dReg.rs2val)
                    pc += dReg.immediate - 4;
                break;
            case BLT:
                if ((int) dReg.rs1val < (int) dReg.rs2val)
                    pc += dReg.immediate - 4;
                break;
            case BGE:
                if ((int) dReg.rs1val >= (int) dReg.rs2val)
                    pc += dReg.immediate - 4;
                break;
            case BLTU:
                if (dReg.rs1val < dReg.rs2val)
                    pc += dReg.immediate - 4;
                break;
            case BGEU:
                if (dReg.rs1val >= dReg.rs2val)
                    pc += dReg.immediate - 4;
                break;
            case ADDI:
                eReg.val = dReg.rs1val + dReg.immediate;
                break;
            case SLTI:
                eReg.val = (int) dReg.rs1val < (int) dReg.immediate ? 1 : 0;
                break;
            case SLTIU:
                eReg.val = dReg.rs1val < dReg.immediate ? 1 : 0;
                break;
            case XORI:
                eReg.val = dReg.rs1val ^ dReg.immediate;
                break;
            case ORI:
                eReg.val = dReg.rs1val | dReg.immediate;
                break;
            case ANDI:
                eReg.val = dReg.rs1val & dReg.immediate;
                break;
            case SLLI:
                eReg.val = dReg.rs1val << dReg.shamt;
                break;
            case SRLI:
                eReg.val = dReg.rs1val >> dReg.shamt;
                break;
            case SRAI:
                eReg.val = dReg.rs1val >> dReg.shamt;
                if ((dReg.rs1val & 0x80000000) == 0x80000000)
                    eReg.val |= (0xffffffff << (32 - dReg.shamt));
                break;
            case ADD:
                eReg.val = dReg.rs1val + dReg.rs2val;
                break;
            case SUB:
                eReg.val = dReg.rs1val - dReg.rs2val;
                break;
            case SLL:
                eReg.val = dReg.rs1val << (dReg.rs2val & 0x0000001f);
                break;
            case SRL:
                eReg.val = dReg.rs1val >> (dReg.rs2val & 0x0000001f);
                break;
            case SRA:
                eReg.val = dReg.rs1val >> (dReg.rs2val & 0x0000001f);
                if ((dReg.rs1val & 0x80000000) == 0x80000000)
                    eReg.val |= (0xffffffff << (32 - (dReg.rs2val & 0x0000001f)));
                break;
            case SLT:
                eReg.val = (int) dReg.rs1val < (int) dReg.rs2val ? 1 : 0;
                break;
            case SLTU:
                eReg.val = dReg.rs1val < dReg.rs2val ? 1 : 0;
                break;
            case XOR:
                eReg.val = dReg.rs1val ^ dReg.rs2val;
                break;
            case OR:
                eReg.val = dReg.rs1val | dReg.rs2val;
                break;
            case AND:
                eReg.val = dReg.rs1val & dReg.rs2val;
                break;
            default:
                std::cerr << "[Error]function [Execute()] wrong with a undefined CommandType." << std::endl;
                break;
        }
        eReg.type = dReg.type;
        eReg.rs1val = dReg.rs1val;
        eReg.rs2val = dReg.rs2val;
        eReg.rd = dReg.rd;
        eReg.immediate = dReg.immediate;
    }
    
    struct EX2MEMReg {
        CommandType type;
        uint immediate;
        uint val;
        uint rs1val;
        uint rs2val;
        uint rd;
    } eReg {};
    
    void MemoryAccess() {
        //read from and write to memory
        switch (eReg.type) {
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
                mReg.val = eReg.val;
                break;
            case BEQ:
            case BNE:
            case BLT:
            case BGE:
            case BLTU:
            case BGEU:
                //do nothing
                break;
            case LB:
                mReg.val = mem.readByte(eReg.rs1val + eReg.immediate);
                if ((mReg.val & 0x00000080) == 0x00000080)mReg.val |= 0xffffff00;
                break;
            case LH:
                mReg.val = mem.readHalfWord(eReg.rs1val + eReg.immediate);
                if ((mReg.val & 0x00008000) == 0x00008000)mReg.val |= 0xffff0000;
                break;
            case LW:
                mReg.val = mem.readWord(eReg.rs1val + eReg.immediate);
                break;
            case LBU:
                mReg.val = mem.readByte(eReg.rs1val + eReg.immediate);
                break;
            case LHU:
                mReg.val = mem.readHalfWord(eReg.rs1val + eReg.immediate);
                break;
            case SB:
                mem.writeByte(eReg.rs1val + eReg.immediate, eReg.rs2val);
                break;
            case SH:
                mem.writeHalfWord(eReg.rs1val + eReg.immediate, eReg.rs2val);
                break;
            case SW:
                mem.writeWord(eReg.rs1val + eReg.immediate, eReg.rs2val);
                break;
            default:
                std::cerr << "[Error]function [MemoryAccess()] wrong with a undefined CommandType." << std::endl;
                break;
        }
        mReg.type = eReg.type;
        mReg.rd = eReg.rd;
    }
    
    struct MEM2WBReg {
        CommandType type;
        uint rd;
        uint val;
    } mReg {};
    
    void WriteBack() {
        //write to register
        switch (mReg.type) {
            case LUI:
            case AUIPC:
            case JAL:
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
                reg[mReg.rd] = mReg.val;
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
                //do nothing
                break;
            default:
                std::cerr << "[Error]function [WriteBack()] wrong with a undefined CommandType." << std::endl;
                break;
        }
    }

public:
    void runSequenceExecute() {
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

#endif //RISC_V_SIMULATOR_CPU_H

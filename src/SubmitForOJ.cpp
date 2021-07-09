#pragma GCC optimize ("Ofast")
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <bitset>

class CPU {
private:
    static void dPri(const auto &arg) {
#ifdef debug
        std::cerr << arg << std::endl;
#endif
    }
    
    static void dPrintSingle(const auto &arg) {
#ifdef debug
        std::cerr << arg;
#endif
    }
    
    static void dPri(const auto &... argList) {
#ifdef debug
        (dPrintSingle(argList), ...);
        std::cerr << std::endl;
#endif
    }
    
    using uint = unsigned int;
    using uc = unsigned char;

#pragma region parser
    
    static uint hex_to_uint(const std::string &str) {
        unsigned int ret = 0;
        for (char c:str) {
            ret <<= 4;
            if (c >= '0' && c <= '9')ret += c - '0';
            else ret += c - 'A' + 10;
        }
        return ret;
    }
    
    static uc hex_to_uc(char c) {
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
    
    static CommandFormat get_format(CommandType type) {
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
    
    static uint get_immediate(uint cmd, CommandFormat type) {
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
    
    static uint get_opcode(uint cmd) {
        return cmd & (uint) 0x0000007f;
    }
    
    static uint get_rs1(uint cmd) {
        return (cmd & (uint) 0x000f8000) >> 15;
    }
    
    static uint get_rs2(uint cmd) {
        return (cmd & (uint) 0x01f00000) >> 20;
    }
    
    static uint get_rd(uint cmd) {
        return (cmd & (uint) 0x00000f80) >> 7;
    }
    
    static uint get_funct3(uint cmd) {
        return (cmd & (uint) 0x00007000) >> 12;
    }
    
    static uint get_funct7(uint cmd) {
        return (cmd & (uint) 0xfe000000) >> 25;
    }
    
    static uint get_shamt(uint cmd) {
        return (cmd & (uint) 0x01f00000) >> 20;
    }

#pragma endregion
    
    constexpr static uint BRANCH_PREDICT_SIZE = 8191;
    constexpr static uint BRANCH_TARGET_BUFFER_SIZE = 64;
    
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
        
        uint getInstruction(uint address) {
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
    
    class SaturatingCounter {
    private:
        uc nowState = 0b01;
    
    public:
        void update(bool taken) {
            if (taken) {
                switch (nowState) {
                    case 0b11:
                        break;
                    default:
                        nowState += 0b01;
                }
            }
            else {
                switch (nowState) {
                    case 0b00:
                        break;
                    default:
                        nowState -= 0b01;
                }
            }
        }
        
        [[nodiscard]] bool predict() const {
            return (nowState & 0b10) == 0b10;
        }
    } branchPredict[BRANCH_PREDICT_SIZE];
    
    uint predictSuccess = 0;
    uint predictFailure = 0;
    
    class BranchTargetBuffer {
    private:
        struct node {
            node *prev = nullptr;
            node *next = nullptr;
            uint originalAddr {};
            uint targetAddr {};
        } *head, *tail;
        
        uint size = 0;
    
    public:
        BranchTargetBuffer() {
            head = new node();
            tail = new node();
            head->next = tail, tail->prev = head;
        }
        
        uint query(uint orgAddr) {
            for (node *i = head->next; i != tail; i = i->next)
                if (i->originalAddr == orgAddr)return i->targetAddr;
            return (uint) -1;
        }
        
        void push(uint orgAddr, uint tarAddr) {
            if (size == BRANCH_TARGET_BUFFER_SIZE) {
                node *temp = tail->prev;
                tail->prev = temp->prev, temp->prev->next = tail;
                delete temp;
            }
            else size++;
            auto *temp = new node {head, head->next, orgAddr, tarAddr};
            head->next = head->next->prev = temp;
        }
        
        ~BranchTargetBuffer() {
            node *temp = head;
            while (head != nullptr) {
                head = head->next;
                delete temp;
                temp = head;
            }
        }
    } buffer;
    
    static uint hashPC(uint pc) {
        return pc % BRANCH_PREDICT_SIZE;
    }
    
    Register reg;
    Memory mem;
    bool stop = false;
    uint pc = 0;
    int pause[5] = {0, 1, 2, 3, 4};// IF ID EX MEM WB
    
    static bool isJumpType(CommandType t) {
        static constexpr uint targetSize = 2;
        static CommandType target[targetSize] {JAL, JALR};
        return std::any_of(target, target + targetSize, [t](CommandType i) { return i == t; });
    }
    
    static bool isBranchType(CommandType t) {
        static constexpr uint targetSize = 6;
        static CommandType target[targetSize] {BEQ, BNE, BLT, BGE, BLTU, BGEU};
        return std::any_of(target, target + targetSize, [t](CommandType i) { return i == t; });
    }
    
    static bool isLoadType(CommandType t) {
        static constexpr uint targetSize = 5;
        static CommandType target[targetSize] {LB, LH, LW, LBU, LHU};
        return std::any_of(target, target + targetSize, [t](CommandType i) { return i == t; });
    }
    
    static bool isBranchInst(uint inst) {
        uint opcode = get_opcode(inst);
        return opcode == 0b1100011;
    }
    
    void setStop() {
        stop = true;
    }
    
    void InstructionFetch() {
        //read command to register
        if (stop)return;
        uint index = 0;
        if (pause[index] > 0) {
            pause[index]--;
            return;
        }
        else if (pause[index] < 0) {
            pause[index + 1] = pause[index];
            pause[index]++;
            return;
        }
        f2dReg.instruction = mem.getInstruction(pc);
        dPri("[debug] pc last fetched is ", pc);
        
        f2dReg.orgAddr = pc;
        if (isBranchInst(f2dReg.instruction)) {
            if (branchPredict[hashPC(pc)].predict()) {
                uint tarAddr = buffer.query(pc);
                if (tarAddr != (uint) -1)f2dReg.tarAddr = pc = tarAddr, dPri("[debug] Predict Branch from ", f2dReg.orgAddr, " to ", tarAddr);
                else pc += 4, f2dReg.tarAddr = pc;
            }
            else pc += 4, f2dReg.tarAddr = pc;
        }
        else pc += 4;
    }
    
    struct IF2IDReg {
        uint instruction {};
        uint tarAddr;
        uint orgAddr;
    } f2dReg {};
    
    void InstructionDecode() { // && Register Fetch
        if (stop)return;
        uint index = 1;
        if (pause[index] > 0) {
            pause[index]--;
            return;
        }
        else if (pause[index] < 0) {
            pause[index + 1] = pause[index];
            pause[index]++;
            return;
        }
        d2eReg.instruction = f2dReg.instruction;
        d2eReg.tarAddr = f2dReg.tarAddr;
        d2eReg.orgAddr = f2dReg.orgAddr;
        dPri("[debug] Decode instruction: ", std::hex, f2dReg.instruction, std::dec);
        uint code = get_opcode(f2dReg.instruction);
        switch (code) {
            case 0b0110111:
            case 0b0010111:
            case 0b1101111:
            case 0b1100111:
                d2eReg.type = (CommandType) code;
                break;
            default:
                code |= get_funct3(f2dReg.instruction) << 7;
                uint temp;
                switch (code) {
                    case SRLI:
                        temp = code | (get_funct7(f2dReg.instruction) << 10);
                        d2eReg.type = temp == code ? SRLI : SRAI;
                        break;
                    case ADD:
                        temp = code | (get_funct7(f2dReg.instruction) << 10);
                        d2eReg.type = temp == code ? ADD : SUB;
                        break;
                    case SRL:
                        temp = code | (get_funct7(f2dReg.instruction) << 10);
                        d2eReg.type = temp == code ? SRL : SRA;
                        break;
                    default:
                        d2eReg.type = (CommandType) code;
                        break;
                }
                break;
        }
        CommandFormat format = get_format(d2eReg.type);
        d2eReg.immediate = get_immediate(f2dReg.instruction, format);
        uint rs1 = get_rs1(f2dReg.instruction);
        uint rs2 = get_rs2(f2dReg.instruction);
        d2eReg.rd = get_rd(f2dReg.instruction);
        d2eReg.shamt = get_shamt(f2dReg.instruction);
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
                d2eReg.shamt = get_shamt(f2dReg.instruction);
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
                if (rs1 == 0)d2eReg.rs1val = 0;
                else {
                    if (e2mReg.needWB && !e2mReg.invalid && rs1 == e2mReg.rd) {
                        if (isLoadType(e2mReg.type)) {
                            dPri("[debug] Register Fetch crashed with a load instruction");
                            pause[0] = 1;
                            pause[2] = -1;
                            return;
                        }
                        else d2eReg.rs1val = e2mReg.val;
                    }
                    else {
                        if (m2wReg.needWB && !m2wReg.invalid && rs1 == m2wReg.rd)d2eReg.rs1val = m2wReg.val;
                        else d2eReg.rs1val = reg[rs1];
                    }
                }
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
                if (rs1 == 0)d2eReg.rs1val = 0;
                else {
                    if (e2mReg.needWB && !e2mReg.invalid && rs1 == e2mReg.rd) {
                        if (isLoadType(e2mReg.type)) {
                            dPri("[debug] Register Fetch crashed with a load instruction");
                            pause[0] = 1;
                            pause[2] = -1;
                            return;
                        }
                        else d2eReg.rs1val = e2mReg.val;
                    }
                    else {
                        if (m2wReg.needWB && !m2wReg.invalid && rs1 == m2wReg.rd)d2eReg.rs1val = m2wReg.val;
                        else d2eReg.rs1val = reg[rs1];
                    }
                }
                if (rs2 == 0)d2eReg.rs2val = 0;
                else {
                    if (e2mReg.needWB && !e2mReg.invalid && rs2 == e2mReg.rd) {
                        if (isLoadType(e2mReg.type)) {
                            dPri("[debug] Register Fetch crashed with a load instruction");
                            pause[0] = 1;
                            pause[2] = -1;
                            return;
                        }
                        else d2eReg.rs2val = e2mReg.val;
                    }
                    else {
                        if (m2wReg.needWB && !m2wReg.invalid && rs2 == m2wReg.rd)d2eReg.rs2val = m2wReg.val;
                        else d2eReg.rs2val = reg[rs2];
                    }
                }
                break;
            default:
                std::cerr << "[Error]function [InstructionDecode()] wrong with a undefined CommandType." << std::endl;
                break;
        }
        dPri("[debug] type: ", std::bitset<sizeof(uint) * 2>(d2eReg.type), " imm: ", d2eReg.immediate, " rd: ", d2eReg.rd,
             " rs1: ", rs1, " rs2: ", rs2, " rs1val: ", d2eReg.rs1val, " rs2val: ", d2eReg.rs2val);
        if (isJumpType(d2eReg.type)) {
            uint org = pc;
            if (d2eReg.type == JAL) {
                pc = d2eReg.orgAddr + d2eReg.immediate;
                dPri("[debug] JAL jump from ", org, " to ", pc);
            }
            else {
                pc = (d2eReg.rs1val + d2eReg.immediate) & -1;
                dPri("[debug] JALR jump from ", org, " to ", pc);
            }
        }
    }
    
    struct ID2EXReg {
        uint instruction {};
        CommandType type {};
        uint immediate {};
        uint rs1val {};
        uint rs2val {};
        uint rd {};
        uint shamt {};
        uint tarAddr {};
        uint orgAddr;
    } d2eReg {};
    
    void Execute() {//calculate
        if (stop)return;
        e2mReg.invalid = false;
        uint index = 2;
        if (pause[index] > 0) {
            pause[index]--;
            e2mReg.needWB = false;
            e2mReg.invalid = true;
            return;
        }
        else if (pause[index] < 0) {
            pause[index + 1] = pause[index];
            pause[index]++;
            e2mReg.needWB = false;
            e2mReg.invalid = true;
            return;
        }
        e2mReg.needWB = true;
        e2mReg.instruction = d2eReg.instruction;
        uint actAddr;
        bool branchResult = false;
        switch (d2eReg.type) {
            case LB:
            case LH:
            case LW:
            case LBU:
            case LHU:
                //do nothing
                break;
            case SB:
            case SH:
            case SW:
                e2mReg.needWB = false;
                break;
            case LUI:
            case AUIPC:
                e2mReg.val = d2eReg.immediate;
                break;
            case JAL:
            case JALR:
                e2mReg.val = d2eReg.orgAddr + 4;
                break;
            case BEQ:
                branchResult = ((int) d2eReg.rs1val == (int) d2eReg.rs2val);
                break;
            case BNE:
                branchResult = ((int) d2eReg.rs1val != (int) d2eReg.rs2val);
                break;
            case BLT:
                branchResult = ((int) d2eReg.rs1val < (int) d2eReg.rs2val);
                break;
            case BGE:
                branchResult = ((int) d2eReg.rs1val >= (int) d2eReg.rs2val);
                break;
            case BLTU:
                branchResult = (d2eReg.rs1val < d2eReg.rs2val);
                break;
            case BGEU:
                branchResult = (d2eReg.rs1val >= d2eReg.rs2val);
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
//        if (isJumpType(d2eReg.type)) {
//            if (actAddr != d2eReg.tarAddr) {
//                pc = actAddr;
//                pause[1] = -1;
//            }
//            branchPredict[hashPC(d2eReg.orgAddr)].update(true);
//            if (d2eReg.type == JAL)buffer.push(d2eReg.orgAddr, actAddr);
//            e2mReg.val = d2eReg.orgAddr + 4;
//        }
        if (isBranchType(d2eReg.type)) {
            if (branchResult) {
                branchPredict[hashPC(d2eReg.orgAddr)].update(true);
                buffer.push(d2eReg.orgAddr, actAddr = d2eReg.orgAddr + d2eReg.immediate);
            }
            else {
                branchPredict[hashPC(d2eReg.orgAddr)].update(false);
                actAddr = d2eReg.orgAddr + 4;
            }
            if (actAddr == d2eReg.tarAddr) {
                predictSuccess++;
            }
            else {
                predictFailure++;
                dPri("[debug] pc [", pc - 4, "] is wrong, roll back to ", actAddr);
                pc = actAddr;
                pause[1] = -1;
            }
            e2mReg.needWB = false;
        }
        e2mReg.type = d2eReg.type;
        e2mReg.rs1val = d2eReg.rs1val;
        e2mReg.rs2val = d2eReg.rs2val;
        e2mReg.rd = d2eReg.rd;
        e2mReg.immediate = d2eReg.immediate;
    }
    
    struct EX2MEMReg {
        bool invalid = false;
        uint instruction {};
        CommandType type {};
        uint immediate {};
        uint val {};
        uint rs1val {};
        uint rs2val {};
        uint rd {};
        bool needWB {};
    } e2mReg {};
    
    void MemoryAccess() {
        //read from and write to memory
        if (stop)return;
        m2wReg.invalid = false;
        uint index = 3;
        if (pause[index] > 0) {
            pause[index]--;
            m2wReg.needWB = false;
            m2wReg.invalid = true;
            return;
        }
        else if (pause[index] < 0) {
            pause[index + 1] = pause[index];
            pause[index]++;
            m2wReg.needWB = false;
            m2wReg.invalid = true;
            return;
        }
        if (e2mReg.instruction == 0x0ff00513)return setStop();
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
        bool invalid = false;
        uint rd {};
        uint val {};
        bool needWB {};
    } m2wReg {};
    
    void WriteBack() {
        //write to register
        if (stop)return;
        uint index = 4;
        if (pause[index] > 0) {
            pause[index]--;
            return;
        }
        else if (pause[index] < 0) {
            pause[index]++;
            return;
        }
        if (m2wReg.needWB && m2wReg.rd != 0)
            reg[m2wReg.rd] = m2wReg.val;
    }

public:
    void runSequenceExecute() {
        mem.initialize(std::cin);
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
    
    void runFiveStagePipeline() {
        mem.initialize(std::cin);
        while (!stop) {
            WriteBack();
            MemoryAccess();
            Execute();
            InstructionDecode();
            InstructionFetch();
        }
        std::cout << (((uint) reg[10]) & 0x000000ff) << std::endl;
    }
    
    void printSuccessRate() const {
        if (predictSuccess + predictFailure == 0) {
            std::cout << "No prediction has made." << std::endl;
            return;
        }
        double rate = (double) (predictSuccess * 100) / (double) (predictSuccess + predictFailure);
        std::cout << "Success Rate is " << predictSuccess << " / " << (predictSuccess + predictFailure) << " = "
                  << std::setiosflags(std::ios::fixed) << std::setprecision(5) << rate << "%." << std::endl;
    }
};

int main() {
    CPU cpu;
    cpu.runFiveStagePipeline();
    return 0;
}
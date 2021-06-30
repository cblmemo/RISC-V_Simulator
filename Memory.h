//
// Created by Rainy Memory on 2021/6/28.
//

#ifndef RISC_V_SIMULATOR_MEMORY_H
#define RISC_V_SIMULATOR_MEMORY_H

#include <iostream>
#include "parser.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

class Memory {
private:
    using uc = unsigned char;
    using uint = unsigned int;
    
    enum coefficient {
        MEMORY_SIZE = 0x200000
    };
    
    uc memory[MEMORY_SIZE] = {0};
    
    void initialize() {
        uint address;
        string address_str;
        char temp;
        while (cin >> temp) {
            if (temp == '@') {
                cin >> address_str;
                address = hex_to_uint(address_str);
            }
            else if (temp == '#')break;
            else {
                memory[address] |= hex_to_uc(temp) << 4;
                cin >> temp;
                memory[address++] |= hex_to_uc(temp);
            }
        }
    }

public:
    Memory() {
        initialize();
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

#endif //RISC_V_SIMULATOR_MEMORY_H

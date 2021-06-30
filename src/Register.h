//
// Created by Rainy Memory on 2021/6/29.
//

#ifndef RISC_V_SIMULATOR_REGISTER_H
#define RISC_V_SIMULATOR_REGISTER_H

class Register {
private:
    using uint = unsigned int;
    
    uint reg[32] = {0};

public:
    uint &operator[](uint addr) {
        if (addr == 0)reg[0] = 0;
        return reg[addr];
    }
    
    const uint &operator[](uint addr) const {
        return reg[addr];
    }
};

#endif //RISC_V_SIMULATOR_REGISTER_H

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
    uint &operator[](uint address) {
        if (address == 0)reg[0] = 0;
        return reg[address];
    }
    
    const uint &operator[](uint address) const {
        return reg[address];
    }
};

#endif //RISC_V_SIMULATOR_REGISTER_H

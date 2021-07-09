#include "CPU.h"

int main() {
    CPU cpu;
    cpu.runFiveStagePipeline();
    cpu.printSuccessRate();
    return 0;
}

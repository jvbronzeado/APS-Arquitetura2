#ifndef IJVMINSTRUCTION_H_
#define IJVMINSTRUCTION_H_

#include <vector>
#include <string>

#include "../types.h"

using namespace std;

class IJVMInstruction {
public:
    virtual ~IJVMInstruction() = default;
    
    virtual string get_name() {return "UNKNOWN";};
    virtual vector<u32> translate() {return {};};
private:
};

#endif

#include <iostream>
#include <cmath>
#include "alloc.h"

using namespace std;

#pragma pack(push, 1)

int main(int argc, char** argv) {
    uint32_t* var1 = (uint32_t*)alloc(sizeof(uint32_t));
    uint32_t* var2 = (uint32_t*)alloc(sizeof(int));
    uint32_t* var3 = (uint32_t*)alloc(sizeof(short int));

    *var1 = 1;
    *var2 = 2;
    *var3 = *var1 + *var2;
    cout << *var3 << endl;

    dump(0, 64);

    del((void*)var3);
    del((void*)var2);
    del((void*)var1);
    
    dump(0, 64);

    uint32_t* var4 = (uint32_t*)alloc(sizeof(int));
    *var4 = 255;
    //cout << *var4 << endl;

    dump(0, 64);
    del((void*)var4);
    dump(0, 64);
}
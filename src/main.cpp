#include <iostream>
#include <cmath>
#include "alloc.h"

using namespace std;

int main(int argc, char** argv) {
    uint32_t* var0 = (uint32_t*)alloc(sizeof(uint32_t));
    uint32_t* var1 = (uint32_t*)alloc(sizeof(uint32_t));

    cout << var0 << endl << var1;

    dump(0, DEFAULT_SIZE);

    del(var0);
    del(var1);

    dump(0, DEFAULT_SIZE);

    uint16_t* var2 = (uint16_t*)alloc(sizeof(uint16_t));
    dump(0, DEFAULT_SIZE);
}
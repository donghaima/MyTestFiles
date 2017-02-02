#include <bitset>
#include "common.hh"

#include <stdlib.h>

int main( int argc, char** argv )
{
    int sz    = atoi(argv[1]);
    int popc  = atoi(argv[2])/100.0 * sz;
    int loops = atoi(argv[3]);
    int opcode = atoi(argv[4]);

    if( sz != BITSETSZ )
    {
        cerr << "hey, you have to use a bitset size of " << BITSETSZ << endl;
        exit(1);
    }
    
    bitset<BITSETSZ> bv1, bv2;
    
    for( int i=0; i<popc; ++i )
    {
        bv1[ rand() % sz ] = 1;
        bv2[ rand() % sz ] = 1;
    }
    
    bitset<BITSETSZ> res;
    for( int i=0; i<loops; ++i )
    {
        switch( opcode )
        {
        case OP_AND:
            res = bv1 & bv2;
            break;
        case OP_OR:
            res = bv1 | bv2;
            break;
        case OP_XOR:
            res = bv1 ^ bv2;
            break;
        }
    }
    return 0;
}



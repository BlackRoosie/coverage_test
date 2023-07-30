#include <iostream>
#include <bitset>
#include <cstdio>

const int ROUNDS = 4096;    //2^12

using namespace std;

void calculate_set(unsigned char* input, int len, int* range)
{
    unsigned char *output = new unsigned char[len];
    int resno;      //result number

    input[len-2] &= 0xf0;
    input[len-1] &= 0x00;
    // compare with 0x00 0x00 0x00 ({0}^12)

    for(int i = 0; i < ROUNDS; i++){
        if(i < 256)
            input[len-1] = char(i);
        else{
            input[len-1] = char(i%256);    //0x00 -> 0xff
            input[len-2] &= 0xf0;           //f0 ->ff
            input[len-2] ^= char(i/256);
        }

        //algorytm enckrypcji

        output[len-2] &= 0x0f; 
        resno = ((int)output[len-2] * 256 + (int)input[len-1]);

        if(resno < 2572)
            range[0]++;
        else if(resno < 2584)
            range[1]++;
        else if(resno < 2594)
            range[2]++;
        else if(resno < 2606)
            range[3]++;
        else 
            range[4]++;
    }

    delete[] output;
}

int main(){
    unsigned char plaintext[10] = {'a', 'b', 's', 't', 'r', 'a', 'k', 'c', 'j', 'a'};
    int ptlen = sizeof(plaintext);
    int range[5] = {0, 0, 0, 0, 0};

    calculate_set(plaintext, ptlen, range);


    


    return 0;
}
#include <iostream>
#include <bitset>
#include <cstdio>

const int ROUNDS = 4096;    //2^12

using namespace std;

void calculate_set(unsigned char* input, int len, int* range)
{
    unsigned char *output = new unsigned char[len];
    int resno;      //result number

    // for(int i = 0; i < len; i++)
    //     output[i] = input[i];
    input[len-2] &= 0xf0;
    input[len-1] &= 0x00;
    // compare with 0x00 0x00 0x00 ({0}^12)

    for(int i = 0; i < ROUNDS; i++){
        if(i < 256)
            input[len-1] = char(i);
        else{
            input[len-1] = char(i-256);    //0x00 -> 0xff
            input[len-2] = char(240+i/256);    //to jest ŹLE!!!
        }

        cout<<i<<'\t'<<int(input[len-2])<<'\t'<<int(input[len-1])<<endl;

        //algorytm enckrypcji

        if(i < 256)
            resno = ((int)output[len-1]);
        else
            resno = ((int)output[len-2] * 256 + (int)output[len-1]);


        if(i==300)
            break;
        

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
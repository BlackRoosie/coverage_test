#include <iostream>
#include <bitset>
#include <cstdio>
#include <math.h>

const int REPEATROUNDS = 4096;    //2^12
const int SETSIZE = 4096;
const int BYTES = 8;

//sould be already in algortihm
const int KEYBYTES = 16;
const int NONCEBYTES = 12;

using namespace std;

void randomBytes(unsigned char* bytes, int n) {

    int temp;
	for (int i = 0; i < n; i++)
	{
		temp = rand();
		bytes[i] = temp & 255;
	}
}

void calculate_set(unsigned char* input, int len, int* range)
{
    unsigned char *output = new unsigned char[len];
    unsigned char key[KEYBYTES];
    unsigned char nonce[NONCEBYTES];
    int resno;      //result number

    input[len-2] &= 0xf0;
    input[len-1] &= 0x00;
    // compare with 0x00 0x00 0x00 ({0}^12)

    for(int i = 0; i < SETSIZE; i++){
        if(i < 256)
            input[len-1] = char(i);
        else{
            input[len-1] = char(i%256);    //0x00 -> 0xff
            input[len-2] &= 0xf0;           //f0 ->ff
            input[len-2] ^= char(i/256);
        }

        randomBytes(key, KEYBYTES);
        randomBytes(nonce, NONCEBYTES);

        //algorytm enckrypcji

        output[len-2] &= 0x0f; 
        resno = ((int)output[len-2] * 256 + (int)output[len-1]);

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

float coverage_test(){

    uint8_t input[BYTES];
    int range[5] = {0, 0, 0, 0, 0};
    float expected[5] = {0.199176f, 0.204681f, 0.197862f, 0.203232f, 0.195049f};
    float probabilities[REPEATROUNDS] = {0};
    float pValue = 0.0;

    for(int i = 0; i < REPEATROUNDS; i++){
        randomBytes(input, BYTES);
        calculate_set(input, BYTES, range);
        
        for(int j = 0; j < 5; j++){
            probabilities[i] += powf(range[j]/SETSIZE - expected[j], 2) / expected[j];
            range[j] = 0;
        }

        pValue += probabilities[i];
    }

    return pValue /= REPEATROUNDS;
}

// int main(){
//     unsigned char plaintext[8] = {'c', 'o', 'v', 'e', 'r', 'a', 'g', 'e'};
//     int ptlen = sizeof(plaintext);
//     // int range[5] = {0, 0, 0, 0, 0};

//     // calculate_set(plaintext, ptlen, range);


    


//     return 0;
// }
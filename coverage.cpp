/*
download https://vcpkg.io/en/getting-started
and install boost-math using it

compile using command:
g++ coverage.cpp -o coverage -I <path to boost>"
e.g.
g++ coverage.cpp -o coverage -I "C:/dev/vcpkg/installed/x64-windows/include"
*/

#include <iostream>
#include <bitset>
#include <cstdio>
#include <math.h>
#include <boost/math/distributions/chi_squared.hpp>

const int REPEATROUNDS = 4096;    //2^12
const int SETSIZE = 4096;
const int BYTES = 8;
const int DF = 4;       //degrees of freedom
int expected[5] = { 816, 839, 810, 832, 799 };

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

void calculate_set(unsigned char* key, unsigned char* input, int len, int* range)
{
    unsigned char *output = new unsigned char[len];
    // unsigned char key[KEYBYTES];
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

        // randomBytes(key, KEYBYTES);
        randomBytes(nonce, NONCEBYTES);

        //encryption algorithm e.g. encryption(key, nonce, ad, adlen, input, msglen, output, tag);

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

    //for this set pValue = 0,27623 
    // range[0] = 787;
    // range[1] = 861;
    // range[2] = 772;
    // range[3] = 841;
    // range[4] = 835;

    delete[] output;
}

float coverage_test(unsigned char* key, unsigned char* ad, int adlen, unsigned char* tag) {

    uint8_t input[BYTES];
    int range[5] = {0, 0, 0, 0, 0};
    float probabilities[REPEATROUNDS] = {0};
    float pValue, chi = 0.0f;

    for(int i = 0; i < SETSIZE; i++){
        randomBytes(input, BYTES);
        calculate_set(key, input, BYTES, range);
        
        for(int j = 0; j < 5; j++){
            probabilities[i] += powf(range[j] - expected[j], 2) / expected[j];
            range[j] = 0;
        }

        chi += probabilities[i];
    }

    chi /= SETSIZE;
    pValue = boost::math::cdf(complement(boost::math::chi_squared_distribution<float>(DF), chi));

    return pValue;
}

// int main(){
//     unsigned char plaintext[8] = {'c', 'o', 'v', 'e', 'r', 'a', 'g', 'e'};
//     int ptlen = sizeof(plaintext);
//     // int range[5] = {0, 0, 0, 0, 0};

//     // calculate_set(plaintext, ptlen, range);

//     unsigned char key[KEYBYTES];
// 	randomBytes(key, KEYBYTES);

//     unsigned char ad[8] = { 'E', 'L', 'E', 'P', 'H', 'A', 'N', 'T' };
// 	int adlen = sizeof(ad);

//     unsigned char tagEncryption[8] = { 0 };

//     float pValue = coverage_test(key, ad, adlen, tagEncryption);
//     cout << fixed << setprecision(5) << "final P value: " << pValue << endl;
    


//     return 0;
// }
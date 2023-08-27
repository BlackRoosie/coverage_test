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

using namespace std;

// #include "tiny_jambu.cpp"

const int REPEATROUNDS = 4096;    //2^12
const int SETSIZE = 4096;
const int BYTES = 8;        //input bytes
const int DF = 4;       //degrees of freedom
int expected[5] = { 816, 839, 810, 832, 799 };

//sould be already in algortihm
const int KEYBYTES = 16;
const int NONCEBYTES = 12;

#define P640 640
#define P1024 1024

void update_state(unsigned char* state, const unsigned char* key, const unsigned int number_of_steps) {
    unsigned char t1, t2, t3, t4, feedback;
    for (int i = 0; i < (number_of_steps >> 3); i++) {
        t1 = (state[5] >> 7) | (state[6] << 1);
        t2 = (state[8] >> 6) | (state[9] << 2);
        t3 = (state[10] >> 5) | (state[11] << 3);
        t4 = (state[11] >> 3) | (state[12] << 5);
        feedback = state[0] ^ t1 ^ (~(t2 & t3)) ^ t4 ^ key[i & 15];
        state[0] = state[1]; state[1] = state[2]; state[2] = state[3]; state[3] = state[4];
        state[4] = state[5]; state[5] = state[6]; state[6] = state[7]; state[7] = state[8];
        state[8] = state[9]; state[9] = state[10]; state[10] = state[11]; state[11] = state[12];
        state[12] = state[13]; state[13] = state[14]; state[14] = state[15]; state[15] = feedback;
    }
}

//12 bytes nonce, 16 bytes key
void initialize(unsigned char* state, const unsigned char* key, const unsigned char* nonce) {
    unsigned int i;
    for (i = 0; i < 16; i++) state[i] = 0;
    update_state(state, key, P1024);
    for (i = 0; i < 3; i++) {
        state[4] ^= 16; //0b0010000 FrameBitsNonce = 1
        update_state(state, key, P640);
        state[15] ^= nonce[4 * i];
        state[14] ^= nonce[4 * i + 1];
        state[13] ^= nonce[4 * i + 2];
        state[12] ^= nonce[4 * i + 3];
    }
}

void process_ad(unsigned char* state, const unsigned char* key, const unsigned char* ad, unsigned long long adlen) {
    unsigned long long i;
    unsigned int j;
    for (i = 0; i < (adlen >> 2); i++) {
        state[4] ^= 48; //0b0110000 FrameBitsAD = 3
        update_state(state, key, P640);
        state[12] ^= ad[4 * i];
        state[13] ^= ad[4 * i + 1];
        state[14] ^= ad[4 * i + 2];
        state[15] ^= ad[4 * i + 3];
    }
    if (adlen & 3 > 0) {
        state[4] ^= 48; //0b0110000 FrameBitsAD = 3
        update_state(state, key, P640);
        for (j = 0; j < (adlen & 3); j++)
            state[12 + j] ^= ad[(i << 2) + j];
        state[4] ^= adlen & 3;
    }
}

void encrypt_text(unsigned char *c, unsigned char* state, const unsigned char* key, const unsigned char* m, unsigned long long mlen) {
    unsigned long long i;
    unsigned int j;

    for (i = 0; i < (mlen >> 2); i++) {
        state[4] ^= 80; //0b0110000 FrameBitsPC = 5
        update_state(state, key, P1024);
        state[12] ^= m[4 * i];
        state[13] ^= m[4 * i + 1];
        state[14] ^= m[4 * i + 2];
        state[15] ^= m[4 * i + 3];

		c[4 * i]     = state[8]  ^ m[4 * i];
        c[4 * i + 1] = state[9]  ^ m[4 * i + 1];
        c[4 * i + 2] = state[10] ^ m[4 * i + 2];
        c[4 * i + 3] = state[11] ^ m[4 * i + 3];
    }

    if ((mlen & 3) > 0) {
        state[4] ^= 80; //0b1010000 FrameBitsPC = 5
        update_state(state, key, P1024);

        for (j = 0; j < (mlen & 3); j++) {
            state[12 + j] ^= m[(i << 2) + j];
            c[(i << 2) + j] = state[8 + j] ^ m[(i << 2) + j];
        }
        state[4] ^= mlen & 3;
    }
}

void decrypt_text(unsigned char *m, unsigned char* state, const unsigned char* key, const unsigned char* c, unsigned long long *mlen) {
    unsigned long long i;
    unsigned int j;

    for (i = 0; i < (*mlen >> 2); i++) {
        state[4] ^= 80; //0b0110000 FrameBitsPC = 5
        update_state(state, key, P1024);
		m[4 * i]     = state[8]  ^ c[4 * i];
        m[4 * i + 1] = state[9]  ^ c[4 * i + 1];
        m[4 * i + 2] = state[10] ^ c[4 * i + 2];
        m[4 * i + 3] = state[11] ^ c[4 * i + 3];

        state[12] ^= m[4 * i];
        state[13] ^= m[4 * i + 1];
        state[14] ^= m[4 * i + 2];
        state[15] ^= m[4 * i + 3];
    }

    if ((*mlen & 3) > 0) {
        state[4] ^= 80; //0b1010000 FrameBitsPC = 5
        update_state(state, key, P1024);

        for (j = 0; j < (*mlen & 3); j++) {
            m[(i << 2) + j] = state[8 + j] ^ c[(i << 2) + j];
            state[12 + j] ^= m[(i << 2) + j];
        }
        state[4] ^= *mlen & 3;
    }
}

void finalize_encryption(unsigned char *c, unsigned char* state, const unsigned char* key, unsigned long long mlen, unsigned long long *clen) {
    unsigned char mac[8];
    unsigned int j;

    state[4] ^= 112; //0b1110000 FrameBitsFin = 7
    update_state(state, key, P1024);
    mac[0] = state[8];
    mac[1] = state[9];
    mac[2] = state[10];
    mac[3] = state[11];

    state[4] ^= 112; //0b1110000 FrameBitsFin = 7
    update_state(state, key, P640);
    mac[4] = state[8];
    mac[5] = state[9];
    mac[6] = state[10];
    mac[7] = state[11];

    *clen = mlen + 8;
    for (j = 0; j < 8; j++) c[mlen+j] = mac[j];
}

int finalize_decryption(unsigned char *m, unsigned char* state, const unsigned char* key, const unsigned char* c, unsigned long long clen, unsigned long long *mlen) {
    unsigned char mac[8];
    unsigned int j, check = 0;

    state[4] ^= 112; //0b1110000 FrameBitsFin = 7
    update_state(state, key, P1024);
    mac[0] = state[8];
    mac[1] = state[9];
    mac[2] = state[10];
    mac[3] = state[11];

    state[4] ^= 112; //0b1110000 FrameBitsFin = 7
    update_state(state, key, P640);
    mac[4] = state[8];
    mac[5] = state[9];
    mac[6] = state[10];
    mac[7] = state[11];

    for (j = 0; j < 8; j++) { check |= (mac[j] ^ c[clen - 8 + j]); }
    if (check == 0) return 0;
    else return -1;
}

int crypto_aead_encrypt_tinyjambu(
	unsigned char *c, unsigned long long *clen,
	const unsigned char *m,unsigned long long mlen,
	const unsigned char *ad,unsigned long long adlen,
	const unsigned char *nsec,
	const unsigned char *npub,
	const unsigned char *k
	) {
    unsigned char state[16];

    initialize(state, k, npub);
	process_ad(state, k, ad, adlen);
	encrypt_text(c, state, k, m, mlen);
	finalize_encryption(c, state, k, mlen, clen);

	return 0;
}

int crypto_aead_decrypt_tinyjambu(
	unsigned char *m,unsigned long long *mlen,
	const unsigned char *c,unsigned long long clen,
	const unsigned char *ad,unsigned long long adlen,
	const unsigned char *nsec,
	const unsigned char *npub,
	const unsigned char *k
	) {
    unsigned char state[16], mac[8];
    unsigned long long i;
    unsigned int j, check = 0;

    *mlen = clen - 8;
    initialize(state, k, npub);
	process_ad(state, k, ad, adlen);
    decrypt_text(m, state, k, c, mlen);
    return finalize_decryption(m, state, k, c, clen, mlen);
}






void randomBytes(unsigned char* bytes, int n) {

	int temp;
	for (int i = 0; i < n; i++)
	{
		temp = rand();
		bytes[i] = temp & 255;
	}
}

// void calculate_set(unsigned char* input, int len, int* range, unsigned char* ad, int adlen, unsigned char* tag)
int calculate_set(unsigned char* input, int len, unsigned char* ad, int adlen, unsigned char* tag)
{
    unsigned char *output = new unsigned char[len];
    unsigned char key[KEYBYTES];
    unsigned char nonce[NONCEBYTES];
    bool valuesCounter[SETSIZE] = {false};  //to counting number of different values
    int outNumber;      //output as a number
    int differentValues = 0;

    randomBytes(key, KEYBYTES);

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

        randomBytes(nonce, NONCEBYTES);

        unsigned long long clen;
        //encryption algorithm e.g. encryption(key, nonce, ad, adlen, input, msglen, output, tag);
        crypto_aead_encrypt_tinyjambu(output, &clen, input, len, ad, adlen, NULL, nonce, key);

        output[len-2] &= 0x0f; 
        outNumber = ((int)output[len-2] * 256 + (int)output[len-1]);

        if(valuesCounter[outNumber + 1] == false){
            valuesCounter[outNumber + 1] = true;
            differentValues++;
        }
            
    }

    //for this set pValue = 0,27623 
    // range[0] = 787;
    // range[1] = 861;
    // range[2] = 772;
    // range[3] = 841;
    // range[4] = 835;

    delete[] output;

    return differentValues;
}

float coverage_test(unsigned char* ad, int adlen, unsigned char* tag) {
    cout<<"coverage"<<endl;
    uint8_t input[BYTES];
    int range[5] = {0, 0, 0, 0, 0};
    // float probabilities[REPEATROUNDS] = {0};
    int value;
    float pValue, chi = 0.0f;

    // unsigned long long len = 8;

    for(int i = 0; i < REPEATROUNDS; i++){
        randomBytes(input, BYTES);
        value = calculate_set(input, BYTES, ad, adlen, tag);
        cout<<i<<endl;

        if(value < 2573)
            range[0]++;
        else if(value < 2585)
            range[1]++;
        else if(value < 2595)
            range[2]++;
        else if(value < 2607)
            range[3]++;
        else 
            range[4]++;
        
    }

    // range[0] = 787;
    // range[1] = 861;
    // range[2] = 772;
    // range[3] = 841;
    // range[4] = 835;

    for(int j = 0; j < 5; j++){
        chi += powf(range[j] - expected[j], 2) / expected[j];
        range[j] = 0;
    }

    pValue = boost::math::cdf(complement(boost::math::chi_squared_distribution<float>(DF), chi));

    return pValue;
}

int main(){
    unsigned char plaintext[8] = {'c', 'o', 'v', 'e', 'r', 'a', 'g', 'e'};
    int ptlen = sizeof(plaintext);

    unsigned char ad[8] = { 'E', 'L', 'E', 'P', 'H', 'A', 'N', 'T' };
	int adlen = sizeof(ad);

    unsigned char tagEncryption[8] = { 0 };

    float pValue = coverage_test(ad, adlen, tagEncryption);
    cout << fixed << setprecision(5) << "final P value: " << pValue << endl;


    // unsigned char output[BYTES];
    // unsigned char key[KEYBYTES];
	// unsigned char nonce[NONCEBYTES];

	// randomBytes(key, KEYBYTES);
	// randomBytes(nonce, NONCEBYTES);


    // unsigned long long clen;
    //     //encryption algorithm e.g. encryption(key, nonce, ad, adlen, input, msglen, output, tag);
    //     cout<<"HALO"<<endl;
    // crypto_aead_encrypt_tinyjambu(output, &clen, plaintext, ptlen, ad, adlen, NULL, nonce, key);
    // cout<<"plaintext"<<'\t'<<"output"<<endl;
    // for(int i = 0; i < ptlen; i ++)
    //     {
    //         cout<<plaintext[i]<<'\t'<<output[i]<<endl;
    //     }
    


    return 0;
}
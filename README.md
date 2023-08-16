To run this test you need to download https://vcpkg.io/en/getting-started
and install boost-math using it

then compile using command:
g++ coverage.cpp -o coverage -I <path to boost>"
e.g.
g++ coverage.cpp -o coverage -I "C:/dev/vcpkg/installed/x64-windows/include"

REMEMBER
1) variables KEYBYTES and NONCEBYTES should be already initialize in your algorithms (or under the diferent name, still size of the key and nonce you should have earlier)
2) in line 62 there is a comment "encryption algorithm" -> that's the place where you should call your encryption function with 'input' as plaintext and 'output' as ciphertext

have fun ;)

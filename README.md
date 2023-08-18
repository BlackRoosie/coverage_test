To run this test you need to download https://vcpkg.io/en/getting-started
and install boost-math using it

then compile using this command:
g++ coverage.cpp -o coverage -I <path to boost>"
e.g.
g++ coverage.cpp -o coverage -I "C:/dev/vcpkg/installed/x64-windows/include"

REMEMBER
1) variables KEYBYTES and NONCEBYTES should be already initialised in your algorithms (or under a different name, still the size of the key and nonce you should have earlier)
2) in line 62 there is a comment "encryption algorithm" -> that's the place where you should call your encryption function with 'input' as plaintext and 'output' as ciphertext
3) remember to change size of the input in variable BYTES
4) main() is only to present an example working of this (remember to comment example set of ranges during real tests)

have fun ;)

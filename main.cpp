#include <iostream>
#include <iomanip>
#include "obfuscator.hpp"

int main() {
    // 1. basic obfuscation (no password)
    auto s1 = SECURE_STR("Hello Polymorphic World!");
    std::cout << "[Test 1] Decrypted: " << s1.decrypt() << std::endl;

    // 2. encryption with password
    // inside the binary the string is encrypted using the password "my_secret_pass"
    auto s2 = SECURE_STR_K("Top Secret Database Password", "my_secret_pass");
    
    // correct password
    std::cout << "[Test 2] Correct Pass: " << s2.decrypt("my_secret_pass") << std::endl;
    
    // wrong password (should yield garbage)
    std::cout << "[Test 2] Wrong Pass:   " << s2.decrypt("wrong_pass") << std::endl;

    // 3. different lines, different pipelines
    auto s3 = SECURE_STR("String A");
    auto s4 = SECURE_STR("String A");
    
    std::cout << "[Test 3] Metadata A: " << (int)s3.meta.layer_count << " layers, Seed: 0x" << std::hex << s3.meta.key_seed << std::endl;
    std::cout << "[Test 3] Metadata B: " << (int)s4.meta.layer_count << " layers, Seed: 0x" << std::hex << s4.meta.key_seed << std::endl;

    if (s3.meta.key_seed != s4.meta.key_seed) {
        std::cout << "[Test 3] SUCCESS: Strings on different lines generated different seeds/pipelines." << std::endl;
    }

    return 0;
}

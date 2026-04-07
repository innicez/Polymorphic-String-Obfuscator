import random
import sys

# Pre-generate key-based transformations for C++ code

def generate_cpp_array(name, data, seed):
    # This simulates the logic in obfuscator.hpp but for pre-generation
    # Only XOR and ROL for now
    
    transformed = bytearray(data.encode('utf-8'))
    
    # Simple XOR
    for i in range(len(transformed)):
        transformed[i] ^= ((seed >> (i % 4)) ^ (i * 13)) & 0xFF
        
    cpp_data = ", ".join([f"0x{b:02x}" for b in transformed])
    
    template = f"""
// --- Pre-Generated String: {name} ---
// Seed: 0x{seed:08x}
static const char {name}_data[] = {{ {cpp_data} }};
#define DECRYPT_{name}() obfuscator::ProtectedString<{len(transformed)}>({name}_data, 0x{seed:08x}).decrypt()
"""
    return template

def main():
    if len(sys.argv) < 2:
        print("Usage: python obfuscate.py <string_to_obfuscate>")
        return

    input_str = sys.argv[1]
    seed = random.getrandbits(32)
    
    print(generate_cpp_array("MY_SECURE_STRING", input_str, seed))

if __name__ == "__main__":
    main()

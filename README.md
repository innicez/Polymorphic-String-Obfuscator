# 🛡️ Polymorphic String Obfuscator (C++17)

A header-only, highly advanced polymorphic string obfuscation system for modern C++ (C++17 and newer). Designed to protect sensitive strings like API keys, hardcoded passwords, and internal logic from static analysis, string dumping, and automated signatures.

## ✨ Key Features

- **🚀 Header-Only**: Just drop `obfuscator.hpp` into your project.
- **🔄 Polymorphic Pipelines**: Each string is protected by a unique, randomized sequence of 3–5 transformation layers (XOR, ROL, Shuffle, etc.).
- **⚡ Compile-Time Encryption**: Plaintext strings are encrypted at compile-time and never touch the binary's data section.
- **🧩 Key Splitting**: Reconstructs decryption keys at runtime to defeat simple static analysis.
- **🔑 Password-Locking**: Supports mandatory runtime passwords via `SECURE_STR_K`. If the password is wrong, the output is garbage.
- **🌀 Stateful XOR**: Implements autocorrelated XOR where each byte depends on the previous one.
- **🧪 Dummy Ops**: Intersperces decryption logic with harmless "junk code" to confuse decompilers like IDA Pro and Ghidra.

## 🛠️ Requirements

- **C++17** or newer.
- Supports GCC (MinGW-W64), Clang, and MSVC.
- No external dependencies.

## 🚀 Quick Start

### 1. Basic Usage
Use the `SECURE_STR` macro to hide any string literal.

```cpp
#include "obfuscator.hpp"

void init() {
    auto welcome = SECURE_STR("Welcome to the Secure App!");
    std::cout << welcome.decrypt() << std::endl;
}
```

### 2. Password Protected
Use `SECURE_STR_K` to link decryption to a runtime key.

```cpp
// Encrypted at compile time with link to "my_secret_pass"
auto api_key = SECURE_STR_K("SECRET-DATA-12345", "password123");

// Only decrypts correctly if "password123" is provided at runtime
std::string key = api_key.decrypt("password123");
```

## 🔍 How it Works

The system utilizes C++17 template metaprogramming and `constexpr` evaluation to:
1. Generate a unique **Seed** for each string based on `__TIME__` and `__LINE__`.
2. Deterministically select a **Transformation Pipeline** (e.g., XOR -> ROL -> Shuffle).
3. Apply the pipeline in reverse during runtime only when `.decrypt()` is explicitly called.

## 🕵️ From an Attacker's Perspective (IDA Pro)

When an analyst opens your binary in **IDA Pro** or **Ghidra**, they will see **nothing** usable. Your plaintext strings are completely gone, replaced by high-entropy stack assignments and polymorphic metadata.

### 🛑 Static String Table
Standard tools like `strings.exe` or IDA's **Strings Window** will yield nothing but garbage for your protected content.

### 🧪 Decompiled Code (`main`)
Instead of seeing `"My Secret API Key"`, the analyst will see an opaque block of hex constants and a complex decryption call:

```cpp
// Before Obfuscation:
// std::cout << "My Secret API Key" << std::endl;

// After Obfuscation (Decompiled IDA View):
v25 = -1463868572; 
v26 = -16082;
v28 = 0xEE6ACC21570D005AuLL; // Opaque encrypted blob
v30 = 0x44BD1BF74C0884FFLL; 
v31 = 0x5010101; 

// Decryption happens only in memory (Stack-based)
obfuscator::ProtectedString<24ull>::decrypt(v33, &v25, 0); 
v4 = std::operator<<<char>(std::cout, v33);
```

### 🎯 Why this is Effective
- **No Global Literals**: Data is scattered on the stack.
- **Polymorphic Pipelines**: Every string has a different decryption logic, preventing automated "fix-up" scripts.
- **Anti-Pattern Matching**: The addition of **junk code** and **stateful dependencies** prevents standard signature detection.

## 🐍 Python Helper

A bonus [obfuscate.py](obfuscate.py) script is included to help you pre-generate encrypted arrays if you prefer manual integration for specific legacy workflows.

```bash
python obfuscate.py "My Hidden String"
```

## ⚖️ License & Ethics

This project is intended for **intellectual property protection** and **legitimate software security**. Use of this tool for creating malware, ransomware, or any malicious purposes is strictly prohibited and a violation of the ethics guidelines.

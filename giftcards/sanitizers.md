
# Security Development with Sanitizers 🛡️
> We wanted to give you a quick guide on effectively using sanitizers in security development while ensuring real-world exploit reliability.

## Overview

When developing exploits, remember that real-world targets run without sanitizers. While sanitizers are excellent development tools, your final exploit must work against an unsanitized binary because:

- Production systems run without sanitizers due to performance concerns
- Many real-world vulnerabilities might not be detected by sanitizers
- Some exploitation techniques that work under sanitizers may fail on standard binaries

## Development Workflow

### 1️⃣ Initial Development
Use sanitizers to identify potential vulnerabilities:

```bash
# ASAN for memory issues
gcc -fsanitize=address -g giftcardreader.c -o giftcardreader.asan

# UBSAN for undefined behavior
gcc -fsanitize=undefined -g giftcardreader.c -o giftcardreader.ubsan
```

### 2️⃣ Testing with Sanitizers

```bash
# Memory issue detection
./giftcardreader.asan your_test.gft

# Undefined behavior detection
./giftcardreader.ubsan your_test.gft
```

### 3️⃣ Final Verification

```bash
# Compile without sanitizers
gcc -g giftcardreader.c -o giftcardreader

# Test your exploit
./giftcardreader your_exploit.gft
```

## Sanitizer Usage

### 🔍 ASAN Detects
- Buffer overflows
- Use-after-free
- Memory leaks
- Stack-use-after-return
- Stack-use-after-scope

### 🔍 UBSAN Detects
- Integer overflow
- Null pointer dereferences
- Division by zero
- Invalid shift operations

## Common Pitfalls

### ⚠️ Watch Out For
1. **Sanitizer Dependence**
   - Don't rely solely on sanitizer output
   - Some exploitable issues might not be caught
   - Some non-exploitable issues might be flagged

2. **Binary Differences**
   - Memory layout varies between sanitized/unsanitized binaries
   - Some crashes might only occur with sanitizers

3. **Edge Cases**
   - Sanitizer-specific behavior
   - Environmental dependencies

## Best Practices

### ✅ Do
- Use sanitizers for development and debugging
- Keep both sanitized and unsanitized binaries
- Document discovered vulnerabilities
- Understand the underlying issues
- Test thoroughly against unsanitized binaries

### ❌ Don't
- Rely only on sanitizer feedback
- Skip testing on unsanitized binaries
- Assume sanitizer crashes equal exploitability
- Submit without unsanitized binary verification

## Quick Reference

### 🔧 Development Commands
```bash
# Development Build
gcc -fsanitize=address -g giftcardreader.c -o giftcardreader.asan
ASAN_OPTIONS=halt_on_error=1 ./giftcardreader.asan your_exploit.gft

# Final Testing
gcc -g giftcardreader.c -o giftcardreader
./giftcardreader your_exploit.gft
```

## 📝 Note
Success in this course requires demonstrating vulnerabilities in the original, unsanitized binary. Use sanitizers for development, but verify without them.
The gradescope autograder will always look at the original compiled binary for the various test cases.




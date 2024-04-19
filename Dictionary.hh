#ifndef DICTIONARY_HH
#define DICTIONARY_HH

#include <vector>

typedef unsigned char byte;

struct CodeString {
  unsigned prefixIndex;
  unsigned first;                 // First CodeString using this CodeString as prefix:
  unsigned nextLeft, nextRight;   // Next CodeStrings using the same prefixIndex as this one:
  byte k;

  CodeString(byte newByte = 0, unsigned pI = ~0U);
};

class Dictionary {
  private:
    std::vector<CodeString> table;
    unsigned codeStart, newCodeStringIndex;
    std::vector<byte> decodedString;

    unsigned add(CodeString& c);
    void fillDecodedString(unsigned code);

  public:
    Dictionary(unsigned maxBits, unsigned codeStart);

    bool searchCodeString(CodeString& c);
    void decode(unsigned oldCode, unsigned code, std::vector<byte>& outStream, const byte* byteMap);
    unsigned size() const;
    void reset();
    bool isFull() const;
};

#endif // DICTIONARY_HH

#include "Dictionary.hh"

CodeString::CodeString(byte newByte, unsigned pI) : prefixIndex(pI), first(~0U), nextLeft(~0U), nextRight(~0U), k(newByte) { }

Dictionary::Dictionary(unsigned maxBits, unsigned codeStart) : table(1 << maxBits), codeStart(codeStart), newCodeStringIndex(codeStart) {
  for (unsigned i = 0; i < 255; ++i) {
    table[i].k = i;
  }
}

// Returns ~0U if c didn't already exist, else the index to the
// existing CodeString:
unsigned Dictionary::add(CodeString& c) {
  if (c.prefixIndex == ~0U) return c.k;

  unsigned index = table[c.prefixIndex].first;
  
  if (index == ~0U)
    table[c.prefixIndex].first = newCodeStringIndex;
  else {
    while (true) {
      if (c.k == table[index].k) return index;
      
      if (c.k < table[index].k) {
        const unsigned next = table[index].nextLeft;
        
        if (next == ~0U) {
          table[index].nextLeft = newCodeStringIndex;
          break;
        }

        index = next;
      } else {
        const unsigned next = table[index].nextRight;
        
        if (next == ~0U) {
          table[index].nextRight = newCodeStringIndex;
          break;
        }

        index = next;
      }
    }
  }

  table[newCodeStringIndex++] = c;

  return ~0U;
}

void Dictionary::fillDecodedString(unsigned code) {
  decodedString.clear();
  while (code != ~0U) {
    const CodeString& cs = table[code];
    decodedString.push_back(cs.k);
    code = cs.prefixIndex;
  }
}

bool Dictionary::searchCodeString(CodeString& c) {
  unsigned index = add(c);

  if (index != ~0U) {
    c.prefixIndex = index;
    return true;
  }

  return false;
}

void Dictionary::decode(unsigned oldCode, unsigned code, std::vector<byte>& outStream, const byte* byteMap) {
  const bool exists = code < newCodeStringIndex;

  if (exists) fillDecodedString(code);
  else fillDecodedString(oldCode);

  for (std::size_t i = decodedString.size(); i > 0;)
    outStream.push_back(byteMap[decodedString[--i]]);
  
  if (!exists) outStream.push_back(byteMap[decodedString.back()]);

  table[newCodeStringIndex].prefixIndex = oldCode;
  table[newCodeStringIndex++].k = decodedString.back();
}

unsigned Dictionary::size() const {
  return newCodeStringIndex;
}

void Dictionary::reset() {
  newCodeStringIndex = codeStart;
  for (unsigned i = 0; i < codeStart; ++i)
    table[i] = CodeString(i);
}

bool Dictionary::isFull() const {
  return newCodeStringIndex == table.size();
}

#include <iostream>
#include <cassert>
#include <ctime>

#include "LZW.hh"
#include "StreamIO.hh"
#include "Dictionary.hh"

LZW::LZW(unsigned maxBits1, unsigned maxBits2, bool verbose, bool printTime): verbose(verbose), printTime(printTime), maxBits1(maxBits1), maxBits2(maxBits2) { }

void LZW::encode(const std::vector<unsigned char>& input, std::vector<unsigned char>& encoded) {
    if (maxBits2 < maxBits1) maxBits2 = maxBits1;

    unsigned bestBits = 0;
    size_t bestSize = 0;
    clock_t iTime = 0;

    // Inicialize o número total de bits codificados e o número total de símbolos
    size_t totalBits = 0;
    size_t numSymbols = 0;

    for (unsigned maxBits = maxBits1; maxBits <= maxBits2; ++maxBits) {
        if (printTime) iTime = std::clock();

        // Calcula o número total de bits antes de codificar
        totalBits = encoded.size() * 8;

        compress(input, encoded, maxBits);

        // Calcula o número total de bits após a codificação
        totalBits = encoded.size() * 8 - totalBits;

        // Calcula o número total de símbolos
        numSymbols = input.size();

        // Calcula o comprimento médio dos bits por símbolo
        double avgBitsPerSymbol = static_cast<double>(totalBits) / numSymbols;

        if (maxBits == maxBits1 || encoded.size() <= bestSize) {
            bestBits = maxBits;
            bestSize = encoded.size();
        }

        if (verbose) {
            std::cout << "LZW: maxBits=";
            std::cout.width(2);
            std::cout << maxBits << ", size=";
            std::cout.width(10);
            std::cout << encoded.size();
            if (printTime)
                std::cout << "  (" << int(100.0*(std::clock()-iTime)/CLOCKS_PER_SEC+.5)/100.0 << " seconds)"; std::cout << std::endl;
        }

        if (encoded.size() < (1U << maxBits)) break;
    }

    if(bestBits != maxBits2)
        compress(input, encoded, bestBits);
}

void LZW::decode(const std::vector<byte>& encoded, std::vector<byte>& decoded) {
    decoded.clear();

    byte byteMap[256];
    const unsigned maxBits = encoded[0];
    const unsigned byteMapSize = (encoded[1] == 0 ? 256 : encoded[1]);
    const unsigned eoiCode = byteMapSize;
    const unsigned codeStart = byteMapSize+1;
    const unsigned minBits = requiredBits(codeStart);

    if (byteMapSize < 256)
        for (unsigned i = 0; i < byteMapSize; ++i)
            byteMap[i] = encoded[i+2];
    else
        for (unsigned i = 0; i < 256; ++i)
            byteMap[i] = byte(i);

    Dictionary dictionary(maxBits, codeStart);
    StreamReader reader(encoded, encoded[1]==0 ? 2 : 2+byteMapSize);

    while (true) {
        dictionary.reset();
        unsigned currentBits = minBits;
        unsigned nextBitIncLimit = (1 << minBits) - 2;

        unsigned code = reader.read(currentBits);
        if (code == eoiCode) return;
        decoded.push_back(byteMap[code]);
        unsigned oldCode = code;

        while (true) {
            code = reader.read(currentBits);
            if (code == eoiCode) return;

            dictionary.decode(oldCode, code, decoded, byteMap);
            if (dictionary.size() == nextBitIncLimit) {
                if(currentBits == maxBits)
                    break;
                else
                    ++currentBits;
                
                nextBitIncLimit = (1 << currentBits) - 2;
            }

            oldCode = code;
        }
    }
}

void LZW::compress(const std::vector<byte>& input, std::vector<byte>& encoded, unsigned maxBits) {
    assert(maxBits < 32);
        const size_t size = input.size();

        encoded.clear();
        encoded.reserve(size + size / 8);

        byte byteMap[256];
        const unsigned byteMapSize = calculateByteMap(input, byteMap);

        const bool mapped = byteMapSize < 256;
        const unsigned eoiCode = byteMapSize;
        const unsigned codeStart = byteMapSize + 1;

        const unsigned minBits = requiredBits(codeStart);

        std::cout << "MIN BITS " << minBits << std::endl;
        std::cout << "MAX BITS " << maxBits << std::endl;
        std::cout << "CODE START " << codeStart << std::endl;

        if (maxBits < minBits) maxBits = minBits;

        encoded.push_back(byte(maxBits));
        encoded.push_back(byte(byteMapSize));

        if (mapped) {
            for (unsigned i = 0; i < 256; ++i) {
                if (byteMap[i] < 255) {
                    encoded.push_back(byte(i));
                }
            }
        }

        Dictionary dictionary(maxBits, codeStart);
        StreamWriter writer(encoded);
        CodeString currentString;
        unsigned currentBits = minBits;
        unsigned nextBitIncLimit = (1 << minBits) - 1;

        std::cout << "CURRENT BITS " << currentBits << std::endl;
        std::cout << "NEXT BIT " << nextBitIncLimit << std::endl;

        for (size_t i = 0; i < size; ++i) {
            currentString.k = byteMap[input[i]];

            if (!dictionary.searchCodeString(currentString)) {
                writer.write(currentString.prefixIndex, currentBits);
                currentString.prefixIndex = currentString.k;

                if (dictionary.size() == nextBitIncLimit) {
                    if (currentBits == maxBits) {
                        currentBits = minBits;
                        dictionary.reset();
                    } else
                        ++currentBits;
                    
                    nextBitIncLimit = (1 << currentBits) - 1;
                }
            }
        }

        writer.write(currentString.prefixIndex, currentBits);
        
        if (dictionary.size() == nextBitIncLimit-1) ++currentBits;
        
        writer.write(eoiCode, currentBits);

        // Calculando o comprimento médio dos bits por símbolo
        double avgBitsPerSymbol = static_cast<double>(encoded.size() * 8) / size;
        std::cout << "Average bits per symbol: " << avgBitsPerSymbol << std::endl;
}

// Maps bytes in the input to the lowest possible values:
unsigned LZW::calculateByteMap(const std::vector<byte>& input, byte* byteMap) {
    const size_t size = input.size();
    bool usedByte[256];

    for (unsigned i = 0; i < 256; ++i) {
        usedByte[i] = false;
        byteMap[i] = 255;
    }

    for (size_t i = 0; i < size; ++i) usedByte[input[i]] = true;

    unsigned byteMapCount = 0;
    for (unsigned i = 0; i < 256; ++i) {
        if (usedByte[i]) byteMap[i] = byteMapCount++;
    }

    return byteMapCount;
}

// Calculates the minimum amount of bits required to store the specified value:
unsigned LZW::requiredBits(unsigned value) {
    unsigned bits = 1;
        
    while((value >>= 1) > 0) ++bits;
    
    return bits;

    // return 8;
}

#ifndef _LZW_DOT_H
#define _LZW_DOT_H

#include <string>
#include <unordered_map>
#include <cmath>

namespace lzw {

    bool isDictionaryFull(unsigned int next_code, unsigned int max_code) {
        return next_code > max_code;
    }

    void resetDictionary(std::unordered_map<std::string, unsigned int> &codes) {
        codes.clear();

        for (unsigned int i = 0; i < 256; i++)
            codes[std::string(1, i)] = i;
    }

    void resetDictionary(std::unordered_map<unsigned int, std::string>& strings) {
        strings.clear();

        for (unsigned int i = 0; i < 256; i++)
            strings[i] = std::string(1, i);
    }

    unsigned int bitsNeeded(unsigned int code) {
        return static_cast<unsigned int>(std::ceil(std::log2(code + 1)));
    }

    template<class INPUT, class OUTPUT> int compress(INPUT &input, OUTPUT &output, const unsigned int max_code) {
        int flagEstrategiaDicionario = 0;
        
        input_symbol_stream<INPUT> in(input);
        output_code_stream<OUTPUT> out(output, max_code);

        std::unordered_map<std::string, unsigned int> codes((max_code * 11) / 10);

        resetDictionary(codes);

        unsigned int next_code = 258;
        std::string current_string;
        char c;

        unsigned int total_bits = 0;

        while (in >> c) {
            current_string = current_string + c;

            if (codes.find(current_string) == codes.end()) {
                if (isDictionaryFull(next_code, max_code)) {
                    if (flagEstrategiaDicionario == 0) {
                        resetDictionary(codes);
                        next_code = 258;
                    }
                } else {
                    codes[current_string] = next_code++;
                }

                current_string.erase(current_string.size() - 1);

                out << codes[current_string];

                unsigned int bits_used = out.get_code_size_bits();

                total_bits += bits_used;

                current_string = c;
            }
        }

        if (current_string.size()) {
            out << codes[current_string];

            unsigned int bits_used = out.get_code_size_bits();

            total_bits += bits_used;
        }

        std::streampos size = in.get_size();

        std::cout << "SIZE IN " << size << std::endl;

        return total_bits;
    }

    template<class INPUT, class OUTPUT> void decompress(INPUT &input, OUTPUT &output, const unsigned int max_code) {
        int flagEstrategiaDicionario = 0;

        input_code_stream<INPUT> in(input, max_code);
        output_symbol_stream<OUTPUT> out(output);

        std::unordered_map<unsigned int, std::string> strings((max_code * 11) / 10);

        resetDictionary(strings);
        
        std::string previous_string;
        
        unsigned int code;
        unsigned int next_code = 258;

        while (in >> code) {
            if (strings.find(code) == strings.end()) {
                strings[code] = previous_string + previous_string[0];
            }
            
            if (code == SEPARATOR_FILE) {
                break;
            }

            out << strings[code];
            
            if (previous_string.size()) {
                if (isDictionaryFull(next_code, max_code)) {
                    if (flagEstrategiaDicionario == 0) {
                        resetDictionary(strings);
                        next_code = 258;
                    }
                } else {
                    strings[next_code++] = previous_string + strings[code][0];
                }
            }

            previous_string = strings[code];
        }
    }

};

#endif // _LZW_DOT_H

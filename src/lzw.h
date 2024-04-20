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

    template<class INPUT, class OUTPUT> void compress(INPUT &input, OUTPUT &output, const unsigned int max_code) {
        int flagEstrategiaDicionario = 0;

        input_symbol_stream<INPUT> in(input);
        output_code_stream<OUTPUT> out(output, max_code);

        std::unordered_map<std::string, unsigned int> codes((max_code * 11) / 10);

        resetDictionary(codes);
        std::cout << "RESET DICTIONARY " << codes.size() << std::endl;

        unsigned int next_code = 257;
        std::string current_string;
        char c;

        while (in >> c) {
            current_string = current_string + c;

            if (codes.find(current_string) == codes.end()) {
                if (isDictionaryFull(next_code, max_code)) {
                    if (flagEstrategiaDicionario == 0) {
                        resetDictionary(codes);
                        next_code = 257;

                        std::cout << "RESET DICTIONARY " << codes.size() << std::endl;
                        std::cout << "RESET DICTIONARY NEXT CODE " << next_code << std::endl;
                    }
                } else {
                    codes[current_string] = next_code++;
                }

                current_string.erase(current_string.size() - 1);

                // std::cout << "CURRENT STRING " << current_string << std::endl;
                // std::cout << "CODES[CURRENT STRING] " << codes[current_string] << std::endl;

                out << codes[current_string];

                // std::cout << "Código para " << current_string << " requer " << static_cast<int>(std::ceil(std::log2(next_code - 1))) << " bits." << std::endl;
                // std::cout << std::endl;

                current_string = c;
            }
        }

        if (current_string.size())
            out << codes[current_string];
    }

    template<class INPUT, class OUTPUT> void decompress(INPUT &input, OUTPUT &output, const unsigned int max_code) {
        int flagEstrategiaDicionario = 0;

        input_code_stream<INPUT> in(input, max_code);
        output_symbol_stream<OUTPUT> out(output);

        std::unordered_map<unsigned int, std::string> strings((max_code * 11) / 10);

        resetDictionary(strings);
        
        std::string previous_string;
        
        unsigned int code;
        unsigned int next_code = 257;

        while (in >> code) {
            if (strings.find(code) == strings.end()) {
                strings[code] = previous_string + previous_string[0];

                std::cout << "PREVIOUS STRING + PREVIOUS STRING[0] " << previous_string + previous_string[0] << std::endl;
            }
            
            out << strings[code];
            
            if (previous_string.size()) {
                if (isDictionaryFull(next_code, max_code)) {
                    if (flagEstrategiaDicionario == 0) {
                        resetDictionary(strings);
                        next_code = 257;
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

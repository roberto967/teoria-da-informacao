#ifndef _LZW_DOT_H
#define _LZW_DOT_H

#include <chrono>
#include <cmath>
#include <string>
#include <unordered_map>
#include <utility>

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

    void saveAverageLengths(const std::vector<long double> &average_lengths,
                            const std::string &filename) {
      std::ofstream file(filename);
      if (file.is_open()) { 
        for (const auto &length : average_lengths) {
          file << length << "\n"; 
        }
        file.close();
        std::cout << "Array de comprimentos medios salvo com sucesso em '"
                  << filename << "'." << std::endl;
      } else {
        std::cerr << "Nao foi possivel abrir o arquivo para escrita."
                  << std::endl;
      }
    }

    template <class INPUT, class OUTPUT>
    std::pair<int, long double> compress(INPUT &input, OUTPUT &output,
                                         const unsigned int max_code,
                                         int flagEstrategiaDicionario) {

      input_symbol_stream<INPUT> in(input);
      output_code_stream<OUTPUT> out(output, max_code);

      std::unordered_map<std::string, unsigned int> codes((max_code * 11) / 10);

      resetDictionary(codes);

      unsigned int next_code = 258;
      std::string current_string;
      char c;

      unsigned int total_bits = 0;

      // Início do cálculo do tempo
      auto start = std::chrono::high_resolution_clock::now();

      std::vector<long double>
          average_lengths; // Array para armazenar os comprimentos médios
      unsigned int total_symbols =
          0; // Contador para o número total de símbolos processados

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

          out << codes[current_string];
          total_symbols++;

          // Calcular o comprimento do código atual e armazená-lo
          unsigned int bits_used = out.get_code_size_bits();
          total_bits += bits_used;
          average_lengths.push_back(static_cast<long double>(total_bits) /
                                    total_symbols);

          current_string.clear();
          current_string = c;
        } else {
          // Atualizar o total de símbolos para símbolos repetidos
          total_symbols++;
        }
      }

      // Fim do cálculo do tempo
      auto finish = std::chrono::high_resolution_clock::now();

      long double duration =
          std::chrono::duration_cast<std::chrono::milliseconds>(finish - start)
              .count();

      if (current_string.size()) {
        out << codes[current_string];

        unsigned int bits_used = out.get_code_size_bits();

        total_bits += bits_used;
        average_lengths.push_back(static_cast<long double>(total_bits) /
                                  total_symbols);
      }

      std::streampos size = in.get_size();

      std::cout << "SIZE IN " << size << std::endl;

      saveAverageLengths(average_lengths, "comprimentosMedios.txt");

      return std::make_pair(total_bits, duration);
    }

    template<class INPUT, class OUTPUT> long double decompress(INPUT &input, OUTPUT &output, const unsigned int max_code, int flagEstrategiaDicionario) {

        input_code_stream<INPUT> in(input, max_code);
        output_symbol_stream<OUTPUT> out(output);

        std::unordered_map<unsigned int, std::string> strings((max_code * 11) / 10);

        resetDictionary(strings);
        
        std::string previous_string;
        
        unsigned int code;
        unsigned int next_code = 258;

        //inicio do calculo do tempo
        auto start = std::chrono::high_resolution_clock::now();

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

        // fim do calculo do tempo
        auto finish = std::chrono::high_resolution_clock::now();

        long double duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

        return duration;
    }

};

#endif // _LZW_DOT_H

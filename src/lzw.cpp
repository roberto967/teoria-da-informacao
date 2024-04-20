#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>

#include "lzw_streambase.h"
#include "lzw-d.h"
#include "lzw.h"

#define _ITERATOR_DEBUG_LEVEL 0

namespace fs = std::filesystem;

enum TamanhoDicionario {
  K_4   = 4096,
  K_32  = 32768,
  K_256 = 262144,
  M_2   = 2097152
};

std::vector<char> lerArquivoBinario(const fs::path &arquivo) {
  std::ifstream arquivoStream(arquivo, std::ios::binary);
  std::vector<char> conteudo;

  if (arquivoStream.is_open()) {
    // Move o ponteiro do arquivo para o final para determinar o tamanho do arquivo
    arquivoStream.seekg(0, std::ios::end);
    std::streamsize tamanhoArquivo = arquivoStream.tellg();
    arquivoStream.seekg(0, std::ios::beg);

    // Redimensiona o vetor de conteúdo para o tamanho do arquivo
    conteudo.resize(static_cast<size_t>(tamanhoArquivo));

    // Lê o conteúdo do arquivo e armazena no vetor
    arquivoStream.read(conteudo.data(), tamanhoArquivo);
    
    arquivoStream.close();
  } else {
    std::cerr << "Não foi possível abrir o arquivo: " << arquivo << std::endl;
  }

  return conteudo;
}

void usage() {
  std::cerr << 
    "Usage:\n"
    "lzw [-max max_code] -c input output #compress file input to file output\n"
    "lzw [-max max_code] -c - output     #compress stdin to file otuput\n"
    "lzw [-max max_code] -c input        #compress file input to stdout\n"
    "lzw [-max max_code] -c              #compress stdin to stdout\n"
    "lzw [-max max_code] -d input output #decompress file input to file output\n"
    "lzw [-max max_code] -d - output     #decompress stdin to file otuput\n"
    "lzw [-max max_code] -d input        #decompress file input to stdout\n"
    "lzw [-max max_code] -d              #decompress stdin to stdout\n";
  exit(1);
}

int main(int argc, char *argv[]) {
  TamanhoDicionario tamanhoPadraoDicionario = M_2;

  int max_code = tamanhoPadraoDicionario;

  if (argc >= 2 && !strcmp("-max", argv[1])) {
    if (sscanf(argv[2], "%d", &max_code) != 1) usage();
    
    argc -= 2;
    argv += 2;
  }
  
  if (argc < 2) usage();
  
  bool compress;
      
  if (std::string("-c") == argv[1]) compress = true;
  else if (std::string("-d") == argv[1]) compress = false;
  else usage();
  
  std::istream *in = &std::cin;
  std::ostream *out = &std::cout;
  
  bool delete_instream = false;
  bool delete_ostream = false;
  
  std::cout << "ARGC " << argc << std::endl;

  if (argc == 3) {
    in = new std::ifstream(argv[2]);
    delete_instream = true;
  }

  std::vector<std::vector<char>> conteudoArquivosBinarios;

  if (argc == 4) {
    out = new std::ofstream(argv[3]);
    
    delete_ostream = true;

    if (std::string("-") != argv[2]) {
      fs::path pasta = argv[2];
      
      if (fs::exists(pasta) && fs::is_directory(pasta)) {
        for (const auto &arquivo : fs::directory_iterator(pasta)) {
          if (fs::is_regular_file(arquivo.path())) {
            conteudoArquivosBinarios.push_back(lerArquivoBinario(arquivo.path()));
          }
        }
      }

      in = new std::ifstream(argv[2]);

      delete_instream = true;
    }
  }

  if (compress) {
    lzw::compress(*in, *out, max_code, conteudoArquivosBinarios);
  } else {
    lzw::decompress(*in, *out, max_code);
  }
  
  if (delete_instream) delete in;
  if (delete_ostream) delete out;

  return 0;
}

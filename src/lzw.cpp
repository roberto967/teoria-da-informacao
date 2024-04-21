#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "lzw-d.h"
#include "lzw.h"
#include "lzw_streambase.h"

#define _ITERATOR_DEBUG_LEVEL 0

namespace fs = std::filesystem;

enum TamanhoDicionario {
  K_4 = 4096,
  K_32 = 32768,
  K_256 = 262144,
  M_2 = 2097152
};

std::vector<char> lerArquivoBinario(const fs::path &arquivo) {
  std::ifstream arquivoStream(arquivo, std::ios::binary);
  std::vector<char> conteudo;

  if (arquivoStream.is_open()) {
    // Move o ponteiro do arquivo para o final para determinar o tamanho do
    // arquivo
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
  std::cerr
      << "Usage:\n"
         "lzw [-max max_code] -c input output #compress file input to file "
         "output\n"
         "lzw [-max max_code] -c - output     #compress stdin to file otuput\n"
         "lzw [-max max_code] -c input        #compress file input to stdout\n"
         "lzw [-max max_code] -c              #compress stdin to stdout\n"
         "lzw [-max max_code] -d input output #decompress file input to file "
         "output\n"
         "lzw [-max max_code] -d - output     #decompress stdin to file "
         "otuput\n"
         "lzw [-max max_code] -d input        #decompress file input to "
         "stdout\n"
         "lzw [-max max_code] -d              #decompress stdin to stdout\n";
  exit(1);
}

// João
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace std;

void listarArquivos(string nomePasta) {
  cout << "-------------------" << endl;
  cout << "Arquivos disponiveis:\n" << endl;
  for (const auto &arquivo : fs::directory_iterator(nomePasta)) {
    if (fs::is_regular_file(arquivo.path())) {
      ostringstream file_info;
      file_info << left << setw(30) << arquivo.path().filename().string();

      file_info << " | Tamanho: " << fixed << setprecision(1) << setw(5)
                << static_cast<double>(fs::file_size(arquivo.path())) / 1e+6
                << " MB";
      cout << file_info.str() << endl;
    }
  }
  cout << "-------------------" << endl;
}

void alterarTamanhoDicionario() {
  cout << "Escolha o tamanho do dicionario:" << endl;
  cout << "1 - 4KB" << endl;
  cout << "2 - 32KB" << endl;
  cout << "3 - 256KB" << endl;
  cout << "4 - 2MB" << endl;
  cout << "-------------------" << endl;
  cout << "Opcao: ";
}

string tamDicToString(const int &tamDic) {
  switch (tamDic) {
  case K_4:
    return "4KB";
  case K_32:
    return "32KB";
  case K_256:
    return "256KB";
  case M_2:
    return "2MB";
  default:
    return "Tamanho invalido";
  }
}

void menu(const int &tamAtualDic) {
  cout << "!!! O tamanho atual do dicionario eh : "
       << tamDicToString(tamAtualDic) << " !!!" << endl;
  cout << "Escolha uma opcao:" << endl;
  cout << "1 - Comprimir arquivos" << endl;
  cout << "2 - Descomprimir arquivos" << endl;
  cout << "3 - Listar arquivos" << endl;
  cout << "4 - Alterar tamanho do dicionario" << endl;
  cout << "5 - Alterar pasta" << endl;
  cout << "0 - Sair" << endl;
  cout << "-------------------" << endl;
  cout << "Opcao: ";
}

vector<string> getFiles(const string &nomePasta) {
  vector<string> arquivos;
  for (const auto &arquivo : fs::directory_iterator(nomePasta)) {
    if (fs::is_regular_file(arquivo.path())) {
      arquivos.push_back(nomePasta + '/' + arquivo.path().filename().string());
    }
  }
  return arquivos;
}

using namespace std;

vector<string> outNameFiles(const vector<string> &arquivos,
                            const bool &compress) {
  vector<string> outNames;

  string saida = compress ? "_compress" : "_decompress";
  for (const auto &arquivo : arquivos) {
    string nomeDoArquivo = filesystem::path(arquivo).filename().string();
    string outName = nomeDoArquivo + saida;
    outNames.push_back(outName);
  }

  return outNames;
}

void comprimirDescomprimir(const string &nomePasta, const bool &compress,
                           const int &max_code) {
  vector<string> arquivos = getFiles(nomePasta);
  vector<string> outNames = outNameFiles(arquivos, compress);

  string pastaDestino = compress ? "comprimidos" : "descomprimidos";
  fs::create_directory(pastaDestino);

  for (int i = 0; i < arquivos.size(); i++) {
    string nomeArquivoOrigem = arquivos[i];
    string nomeArquivoDestino = outNames[i];

    istream *in = new ifstream(nomeArquivoOrigem, ios::binary);
    ostream *out =
        new ofstream(pastaDestino + '/' + nomeArquivoDestino, ios::binary);

    if (compress) {
      lzw::compress(*in, *out, max_code);
    } else {
      lzw::decompress(*in, *out, max_code);
    }

    delete in;
    delete out;
  }
}

int main(int argc, char *argv[]) {

  int opcao, opcaoTamDic;
  string nomePasta;

  int max_code = M_2;

  cout << "Digite o nome da pasta que contem os arquivos: ";
  cin >> nomePasta;

  vector<string> arquivos = getFiles(nomePasta);
  listarArquivos(nomePasta);

  do {
    menu(max_code);
    cin >> opcao;
    cin.ignore();

    switch (opcao) {
    case 1: {
      comprimirDescomprimir(nomePasta, true, max_code);
      break;
    }
    case 2: {
      comprimirDescomprimir(nomePasta, false, max_code);
      break;
    }
    case 3: {
      listarArquivos(nomePasta);
      break;
    }
    case 4: {
      alterarTamanhoDicionario();
      cin >> opcaoTamDic;
      switch (opcaoTamDic) {
      case 1:
        max_code = K_4;
        cout << "Tamanho do dicionario alterado para 4KB!!!" << endl;
        break;
      case 2:
        max_code = K_32;
        cout << "Tamanho do dicionario alterado para 32KB!!!" << endl;
        break;
      case 3:
        max_code = K_256;
        cout << "Tamanho do dicionario alterado para 256KB!!!" << endl;
        break;
      case 4:
        max_code = M_2;
        cout << "Tamanho do dicionario alterado para 2MB!!!" << endl;
        break;
      default:
        cout << "Opcao invalida" << endl;
        break;
      }
      break;
    }
    case 5: {
      cout << "Digite o nome da pasta que contem os arquivos: ";
      cin >> nomePasta;
      arquivos = getFiles(nomePasta);
      listarArquivos(nomePasta);
      break;
    }
    case 0:
      cout << "Saindo..." << endl;
      break;
    default:
      cout << "Opcao invalida" << endl;
      break;
    }
  } while (opcao != 0);

  // bool compress =
  //     false;

  // std::string nomeArquivoOrigem = "c_dick";

  // std::string nomeArquivoDestino = "out_dick";

  // std::istream *in = new std::ifstream(nomeArquivoOrigem, std::ios::binary);
  // std::ostream *out = new std::ofstream(nomeArquivoDestino,
  // std::ios::binary);

  // std::vector<std::vector<char>> conteudoArquivosBinarios;

  // if (compress) {
  //   lzw::compress(*in, *out, max_code);
  // } else {
  //   lzw::decompress(*in, *out, max_code);
  // }

  // delete in;
  // delete out;

  return 0;
}
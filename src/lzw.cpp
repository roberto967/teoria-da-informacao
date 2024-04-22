#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>

#include "lzw-d.h"
#include "lzw.h"
#include "lzw_streambase.h"

#include <unordered_set>

#define _ITERATOR_DEBUG_LEVEL 0

namespace fs = std::filesystem;

enum TamanhoDicionario {
  K_4 = 4096,
  K_32 = 32768,
  K_256 = 262144,
  M_2 = 2097152
};

enum EstrategiaDicionario { RESETA = 0, MANTER = 1 };

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

void menu(const int &tamAtualDic, const int &flagEstrategiaDicionario) {
  cout << "!!! O tamanho atual do dicionario eh : "
       << tamDicToString(tamAtualDic) << " !!!" << endl;
  cout << "!!! A estrategia atual do dicionario eh : "
       << (flagEstrategiaDicionario == 0 ? "RESETA" : "MANTEM") << " !!!"
       << endl;
  cout << "Escolha uma opcao:" << endl;
  cout << "1 - Comprimir arquivos" << endl;
  cout << "2 - Descomprimir arquivos" << endl;
  cout << "3 - Listar arquivos" << endl;
  cout << "4 - Alterar tamanho do dicionario" << endl;
  cout << "5 - Alterar pasta" << endl;
  cout << "6 - Alterar estrategia do dicionario" << endl;
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

int contarSimbolosNoArquivo(const string &nomeArquivo) {
  ifstream arquivo(nomeArquivo, ios::binary);
  int simbolos = 0;

  char c;
  while (arquivo.get(c)) {
    simbolos++;
  }

  return simbolos;
}

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
                           const int &max_code, const int &estrategiaAtual) {
  vector<string> arquivos = getFiles(nomePasta);
  vector<string> outNames = outNameFiles(arquivos, compress);

  string pastaDestino = compress ? "comprimidos" : "descomprimidos";
  fs::create_directory(pastaDestino);

  double comprimento_medio_total = 0.0;
  long double tempoTotal = 0.0;

  for (int i = 0; i < arquivos.size(); i++) {
    string nomeArquivoOrigem = arquivos[i];
    string nomeArquivoDestino = outNames[i];

    istream *in = new ifstream(nomeArquivoOrigem, ios::binary);
    ostream *out =
        new ofstream(pastaDestino + '/' + nomeArquivoDestino, ios::binary);

    if (compress) {
      // L = N / M
      // L - comprimento médio da codificação em bits por símbolo
      // N - número total de bits após a compressão
      // M - número total de símbolos no arquivo original

      pair<int, long double> result =
          lzw::compress(*in, *out, max_code, estrategiaAtual);

      int total_bits = result.first;
      long double tempoDeCompressao = result.second;

      double comprimento_medio =
          static_cast<double>(total_bits) / fs::file_size(nomeArquivoOrigem);
      std::cout << "L = " << std::setprecision(4) << comprimento_medio << " "
                << nomeArquivoOrigem << std::endl;

      cout << "Tempo de compressao do arquivo " << nomeArquivoOrigem << ": "
           << tempoDeCompressao << "ms" << endl;

      comprimento_medio_total += comprimento_medio;
      tempoTotal += tempoDeCompressao;
    } else {
      long double tempoDeDescomp =
          lzw::decompress(*in, *out, max_code, estrategiaAtual);

      cout << "Tempo de descompressao do arquivo " << nomeArquivoOrigem << ": "
           << tempoDeDescomp << "ms" << endl;
      tempoTotal += tempoDeDescomp;
    }

    delete in;
    delete out;
  }

  std::cout << std::endl;
  std::cout << "--------------------------------------------" << std::endl;

  cout << "(comprimento médio total da compressão)" << std::endl;
  std::cout << "L = " << std::setprecision(4) << comprimento_medio_total
            << std::endl;
	cout << "Tempo total de " << (compress ? "compressao" : "descompressao") << ": " << tempoTotal << "ms" << endl;
  std::cout << "--------------------------------------------" << std::endl;
  std::cout << std::endl;
}

int main(int argc, char *argv[]) {
  int opcao, opcaoTamDic;
  string nomePasta;

  int max_code = M_2;
  int estrategiaAtual = MANTER;

  cout << "Digite o nome da pasta que contem os arquivos: ";
  cin >> nomePasta;

  vector<string> arquivos = getFiles(nomePasta);
  listarArquivos(nomePasta);

  do {
    menu(max_code, estrategiaAtual);
    cin >> opcao;
    cin.ignore();

    switch (opcao) {
    case 1: {
      comprimirDescomprimir(nomePasta, true, max_code, estrategiaAtual);
      break;
    }
    case 2: {
      comprimirDescomprimir(nomePasta, false, max_code, estrategiaAtual);
      break;
    }
    case 3: {
      cout << "!!! Pasta Atual " << nomePasta << " !!!" << endl;
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
    case 6: {
      cout << "Escolha a estrategia do dicionario:" << endl;
      cout << "0 - RESETA" << endl;
      cout << "1 - MANTER" << endl;
      cout << "-------------------" << endl;
      cout << "Opcao: ";
      cin >> estrategiaAtual;
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

  return 0;
}

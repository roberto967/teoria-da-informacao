#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "LZW.hh" // Inclui o cabeçalho com as definições das funções

// Função para exibir a mensagem de uso da CLI
void showUsage() {
  std::cout << "Uso: lzw_cli [-c | -d] arquivo_entrada arquivo_saida" << std::endl;
  std::cout << "  -c: Comprimir o arquivo de entrada" << std::endl;
  std::cout << "  -d: Descomprimir o arquivo de entrada" << std::endl;
}

int main(int argc, char *argv[]) {
  // Verifica se o número de argumentos está correto
  if (argc != 4) {
    showUsage();
    return 1;
  }

  // Verifica a operação a ser realizada (compressão ou descompressão)
  std::string operation = argv[1];
  if (operation != "-c" && operation != "-d") {
    std::cerr << "Operação inválida. Use -c para compressão ou -d para descompressão." << std::endl;
    showUsage();
    return 1;
  }

  // Nome do arquivo de entrada e saída
  std::string inputFileName = argv[2];
  std::string outputFileName = argv[3];

  // Vetor para armazenar os dados do arquivo de entrada
  std::vector<unsigned char> inputData;

  // Verifica se o arquivo de entrada pode ser aberto
  std::ifstream inputFile(inputFileName, std::ios::binary);
  if (!inputFile) {
    std::cerr << "Erro ao abrir o arquivo de entrada: " << inputFileName << std::endl;
    return 1;
  }

  // Lê os dados do arquivo de entrada para um vetor de bytes
  inputData.assign((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());

  // Fecha o arquivo de entrada
  inputFile.close();

  // Vetor para armazenar os dados do arquivo de saída
  std::vector<unsigned char> outputData;

  // Objeto LZWCodec
  // LZWCodec lzw;
  LZW lzw(16, 0, true, true);

  // Comprime ou descomprime os dados, conforme especificado pela operação
  if (operation == "-c") {
    // Codifica os dados lidos do arquivo de entrada
    lzw.encode(inputData, outputData);
  } else {
    // Decodifica os dados lidos do arquivo de entrada
    lzw.decode(inputData, outputData);
  }

  // Abre o arquivo de saída em modo binário
  std::ofstream outputFile(outputFileName, std::ios::binary);
  if (!outputFile) {
    std::cerr << "Erro ao abrir o arquivo de saída: " << outputFileName << std::endl;
    return 1;
  }

  // Escreve os dados comprimidos ou descomprimidos no arquivo de saída
  outputFile.write(reinterpret_cast<const char*>(&outputData[0]), outputData.size());

  // Fecha o arquivo de saída
  outputFile.close();

  std::cout << "Operação concluída com sucesso. Arquivo de saída: " << outputFileName << std::endl;

  return 0;
}

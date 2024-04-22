import matplotlib.pyplot as plt

# Função para ler os dados do arquivo e extrair os valores do comprimento médio


def extract_average_lengths(filename):
    with open(filename, 'r') as file:
        lines = file.readlines()
        average_lengths = []
        for line in lines:
            if line.strip():  # Ignorar linhas em branco
                average_lengths.append(float(line.strip()))
    return average_lengths


# Arquivo de saída gerado pela função compress em C++
filename = 'comprimentosMedios.txt'

average_lengths = extract_average_lengths(filename)

symbols = list(range(1, len(average_lengths) + 1))

# Plota a curva do comprimento médio ao longo do processo
plt.plot(symbols, average_lengths)
plt.title('Curva do Comprimento Médio ao Longo do Processo de Compressão')
plt.xlabel('Número de Símbolos Processados')
plt.ylabel('Comprimento Médio')
plt.grid(True)
plt.show()

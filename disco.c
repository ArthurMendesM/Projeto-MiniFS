#include <stdio.h>
#include <string.h>
#include "disco.h"

Superbloco superbloco;
uint8_t bitmap[MAX_BLOCOS];

// Lê um bloco específico de 512 bytes diretamente do arquivo binário
int lerBloco(int indiceBloco, void *buffer) {
    if (indiceBloco < 0 || indiceBloco >= MAX_BLOCOS) {
        return -1;
    }

    FILE *arquivo = fopen(NOME_ARQUIVO_DISCO, "rb");
    if (arquivo == NULL) {
        return -1;
    }

    // Pula para a posição exata do bloco no arquivo e lê os 512 bytes
    fseek(arquivo, indiceBloco * TAMANHO_BLOCO, SEEK_SET);
    size_t lidos = fread(buffer, TAMANHO_BLOCO, 1, arquivo);
    fclose(arquivo);

    return (lidos == 1) ? 0 : -1;
}

// Escreve um bloco de 512 bytes diretamente na posição correspondente do arquivo
int escreverBloco(int indiceBloco, const void *buffer) {
    if (indiceBloco < 0 || indiceBloco >= MAX_BLOCOS) {
        return -1;
    }

    FILE *arquivo = fopen(NOME_ARQUIVO_DISCO, "rb+");
    if (arquivo == NULL) {
        return -1;
    }

    // Pula para a posição exata do bloco no arquivo e grava os 512 bytes
    fseek(arquivo, indiceBloco * TAMANHO_BLOCO, SEEK_SET);
    size_t escritos = fwrite(buffer, TAMANHO_BLOCO, 1, arquivo);
    fclose(arquivo);

    return (escritos == 1) ? 0 : -1;
}

// Grava o Superbloco e o Bitmap atuais da memória RAM para o arquivo disco.bin
int salvarEstadoDisco() {
    uint8_t bufferBloco[TAMANHO_BLOCO];

    // Prepara um bloco zerado, copia o Superbloco para dentro dele e salva no Bloco 0
    memset(bufferBloco, 0, TAMANHO_BLOCO);
    memcpy(bufferBloco, &superbloco, sizeof(Superbloco));
    if (escreverBloco(BLOCO_SUPERBLOCO, bufferBloco) != 0) {
        return -1;
    }

    // Salva o vetor do Bitmap dividido nos 4 blocos reservados (Blocos 1 a 4)
    for (int i = 0; i < QTD_BLOCOS_BITMAP; i++) {
        int deslocamento = i * TAMANHO_BLOCO;
        if (escreverBloco(BLOCO_INICIO_BITMAP + i, &bitmap[deslocamento]) != 0) {
            return -1;
        }
    }

    return 0;
}

// Lê o Superbloco e o Bitmap do arquivo disco.bin para a memória RAM
int carregarDisco() {
    uint8_t bufferBloco[TAMANHO_BLOCO];

    // Lê o Bloco 0 e extrai a struct do Superbloco
    if (lerBloco(BLOCO_SUPERBLOCO, bufferBloco) != 0) {
        return -1;
    }
    memcpy(&superbloco, bufferBloco, sizeof(Superbloco));

    // Verifica se o arquivo realmente possui a assinatura do nosso sistema
    if (strcmp(superbloco.assinatura, ASSINATURA_FS) != 0) {
        return -1;
    }

    // Carrega os 4 blocos do Bitmap do disco para o vetor em memória
    for (int i = 0; i < QTD_BLOCOS_BITMAP; i++) {
        int deslocamento = i * TAMANHO_BLOCO;
        if (lerBloco(BLOCO_INICIO_BITMAP + i, &bitmap[deslocamento]) != 0) {
            return -1;
        }
    }

    return 0;
}

// Cria o arquivo físico zerado e inicializa o Superbloco e o Bitmap
int formatarDisco(uint32_t totalBlocos) {
    if (totalBlocos <= BLOCO_INICIO_DADOS || totalBlocos > MAX_BLOCOS) {
        return -1;
    }

    // Cria um novo arquivo binário do zero
    FILE *arquivo = fopen(NOME_ARQUIVO_DISCO, "wb+");
    if (arquivo == NULL) {
        return -1;
    }

    // Preenche todo o disco virtual com zeros para reservar o espaço físico
    uint8_t blocoZero[TAMANHO_BLOCO];
    memset(blocoZero, 0, TAMANHO_BLOCO);
    for (uint32_t i = 0; i < totalBlocos; i++) {
        fwrite(blocoZero, TAMANHO_BLOCO, 1, arquivo);
    }
    fclose(arquivo);

    // Configura os dados iniciais do Superbloco
    memset(&superbloco, 0, sizeof(Superbloco));
    strcpy(superbloco.assinatura, ASSINATURA_FS);
    superbloco.tamanhoBloco = TAMANHO_BLOCO;
    superbloco.totalBlocos = totalBlocos;
    superbloco.blocosLivres = totalBlocos - BLOCO_INICIO_DADOS;

    // Inicializa o Bitmap: marca os blocos do sistema como ocupados (1) e os de dados como livres (0)
    memset(bitmap, 0, MAX_BLOCOS);
    for (int i = 0; i < BLOCO_INICIO_DADOS; i++) {
        bitmap[i] = 1;
    }

    // Persiste o Superbloco e o Bitmap recém-criados no disco
    return salvarEstadoDisco();
}

// Busca o primeiro bloco livre no Bitmap, marca como ocupado e retorna seu índice
int alocarBloco() {
    if (superbloco.blocosLivres == 0) {
        return -1; // Disco cheio
    }

    for (uint32_t i = BLOCO_INICIO_DADOS; i < superbloco.totalBlocos; i++) {
        if (bitmap[i] == 0) {
            bitmap[i] = 1;
            superbloco.blocosLivres--;

            // Limpa o bloco no disco antes de entregá-lo para uso
            uint8_t blocoZero[TAMANHO_BLOCO];
            memset(blocoZero, 0, TAMANHO_BLOCO);
            escreverBloco(i, blocoZero);

            // Atualiza o Bitmap e o Superbloco no arquivo físico
            salvarEstadoDisco();
            return i;
        }
    }

    return -1;
}

// Marca um bloco específico como livre novamente no Bitmap
int liberarBloco(int indiceBloco) {
    // Impede a liberação de blocos reservados ao sistema ou fora do limite
    if (indiceBloco < BLOCO_INICIO_DADOS || indiceBloco >= (int)superbloco.totalBlocos) {
        return -1;
    }

    if (bitmap[indiceBloco] == 1) {
        bitmap[indiceBloco] = 0;
        superbloco.blocosLivres++;
        salvarEstadoDisco();
        return 0;
    }

    return -1;
}

// Exibe o status de ocupação do disco (útil para o comando 'df')
void exibirInfoDisco() {
    uint32_t blocosUsados = superbloco.totalBlocos - superbloco.blocosLivres;
    printf("Sistema: %s | Bloco: %u bytes | Total: %u blocos | Usados: %u | Livres: %u\n",
           superbloco.assinatura,
           superbloco.tamanhoBloco,
           superbloco.totalBlocos,
           blocosUsados,
           superbloco.blocosLivres);
}
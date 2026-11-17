#include <stdio.h>
#include <string.h>
#include "arquivos.h"

// Grava uma string no arquivo virtual, alocando os blocos necessários (comando write)
int escreverArquivo(const char *nome, const char *conteudo) {
    if (nome == NULL || conteudo == NULL) {
        printf("Erro: parametros invalidos para escrita.\n");
        return -1;
    }

    int posicao = buscarArquivo(nome);
    if (posicao == -1) {
        printf("Erro: arquivo '%s' nao encontrado.\n", nome);
        return -1;
    }

    size_t tamanho = strlen(conteudo);
    if (tamanho > TAMANHO_MAX_ARQUIVO) {
        printf("Erro: arquivo excede o limite maximo suportado (%d bytes).\n",
               TAMANHO_MAX_ARQUIVO);
        return -1;
    }

    Inode *inode = &tabelaInodes[posicao];

    // Calcula quantos blocos de 512 bytes são necessários (arredondando para cima)
    int blocosNecessarios = (tamanho == 0)
                                ? 0
                                : (int)((tamanho + TAMANHO_BLOCO - 1) / TAMANHO_BLOCO);

    // Verifica se há blocos livres suficientes (considerando os blocos que o próprio arquivo já ocupa)
    uint32_t blocosDisponiveis = superbloco.blocosLivres + (uint32_t)inode->quantidadeBlocos;
    if ((uint32_t)blocosNecessarios > blocosDisponiveis) {
        printf("Erro: disco cheio. Necessarios %d bloco(s), disponiveis %u.\n",
               blocosNecessarios, blocosDisponiveis);
        return -1;
    }

    // Se o arquivo já possuía blocos alocados anteriormente, libera-os antes da reescrita
    for (int i = 0; i < inode->quantidadeBlocos; i++) {
        liberarBloco(inode->blocos[i]);
    }
    inode->quantidadeBlocos = 0;
    inode->tamanho = 0;
    memset(inode->blocos, 0, sizeof(inode->blocos));

    // Aloca os novos blocos no Bitmap
    int novosBlocos[MAX_BLOCOS_ARQUIVO];
    for (int i = 0; i < blocosNecessarios; i++) {
        int bloco = alocarBloco();
        if (bloco == -1) {
            // Desfaz alocações parciais em caso de falha inesperada
            for (int j = 0; j < i; j++) {
                liberarBloco(novosBlocos[j]);
            }
            salvarInodes();
            printf("Erro: falha ao alocar blocos no disco (disco cheio).\n");
            return -1;
        }
        novosBlocos[i] = bloco;
    }

    // Fatia o conteúdo em pedaços de 512 bytes e grava em cada bloco físico
    for (int i = 0; i < blocosNecessarios; i++) {
        uint8_t bufferBloco[TAMANHO_BLOCO];
        memset(bufferBloco, 0, TAMANHO_BLOCO);

        size_t deslocamento = (size_t)i * TAMANHO_BLOCO;
        size_t bytesRestantes = tamanho - deslocamento;
        size_t bytesCopiar = (bytesRestantes > TAMANHO_BLOCO) ? TAMANHO_BLOCO : bytesRestantes;

        memcpy(bufferBloco, conteudo + deslocamento, bytesCopiar);

        if (escreverBloco(novosBlocos[i], bufferBloco) != 0) {
            for (int j = 0; j < blocosNecessarios; j++) {
                liberarBloco(novosBlocos[j]);
            }
            salvarInodes();
            printf("Erro: falha fisica ao escrever no bloco %d.\n", novosBlocos[i]);
            return -1;
        }
    }

    // Atualiza os metadados no Inode e persiste no disco
    inode->tamanho = (int)tamanho;
    inode->quantidadeBlocos = blocosNecessarios;
    for (int i = 0; i < blocosNecessarios; i++) {
        inode->blocos[i] = novosBlocos[i];
    }

    if (salvarInodes() != 0) {
        printf("Erro: falha ao persistir metadados do arquivo '%s'.\n", nome);
        return -1;
    }

    printf("Arquivo '%s' gravado com sucesso (%zu bytes em %d bloco(s)).\n",
           nome, tamanho, blocosNecessarios);
    return 0;
}

// Lê todos os blocos vinculados ao Inode e exibe o conteúdo (comando read)
int lerArquivo(const char *nome) {
    if (nome == NULL) {
        printf("Erro: nome de arquivo invalido.\n");
        return -1;
    }

    int posicao = buscarArquivo(nome);
    if (posicao == -1) {
        printf("Erro: arquivo '%s' nao encontrado.\n", nome);
        return -1;
    }

    Inode *inode = &tabelaInodes[posicao];

    if (inode->tamanho == 0 || inode->quantidadeBlocos == 0) {
        printf("(arquivo vazio)\n");
        return 0;
    }

    char conteudo[TAMANHO_MAX_ARQUIVO + 1];
    memset(conteudo, 0, sizeof(conteudo));

    int bytesRestantes = inode->tamanho;
    for (int i = 0; i < inode->quantidadeBlocos; i++) {
        int bloco = inode->blocos[i];

        if (bloco < BLOCO_INICIO_DADOS || bloco >= (int)superbloco.totalBlocos) {
            printf("Erro: indice de bloco corrompido (%d) no arquivo '%s'.\n", bloco, nome);
            return -1;
        }

        uint8_t bufferBloco[TAMANHO_BLOCO];
        if (lerBloco(bloco, bufferBloco) != 0) {
            printf("Erro: falha ao ler o bloco %d do disco.\n", bloco);
            return -1;
        }

        int bytesCopiar = (bytesRestantes > TAMANHO_BLOCO) ? TAMANHO_BLOCO : bytesRestantes;
        memcpy(conteudo + (i * TAMANHO_BLOCO), bufferBloco, (size_t)bytesCopiar);
        bytesRestantes -= bytesCopiar;
    }

    conteudo[inode->tamanho] = '\0';
    printf("%s\n", conteudo);
    return 0;
}

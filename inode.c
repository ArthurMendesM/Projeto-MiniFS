
#include <stdio.h>
#include <string.h>
#include "inode.h"

// Tabela de metadados mantida na memoria RAM
Inode tabelaInodes[MAX_ARQUIVOS];

// Inicializa a tabela de inodes
int inicializarInodes() {
    memset(tabelaInodes, 0, sizeof(tabelaInodes));

    return salvarInodes();
}

// Salva a tabela de inodes nos blocos 5 a 12
int salvarInodes() {
    uint8_t buffer[QTD_BLOCOS_INODES * TAMANHO_BLOCO];

    // Zera o buffer antes de copiar os metadados
    memset(buffer, 0, sizeof(buffer));

    // Copia a tabela de inodes para o buffer
    memcpy(buffer, tabelaInodes, sizeof(tabelaInodes));

    // Divide o buffer e grava nos blocos reservados
    for (int i = 0; i < QTD_BLOCOS_INODES; i++) {
        if (escreverBloco(
                BLOCO_INICIO_INODES + i,
                &buffer[i * TAMANHO_BLOCO]) != 0) {
            return -1;
        }
    }

    return 0;
}

// Carrega os metadados do disco para a memoria RAM
int carregarInodes() {
    uint8_t buffer[QTD_BLOCOS_INODES * TAMANHO_BLOCO];

    // Le os blocos reservados aos inodes
    for (int i = 0; i < QTD_BLOCOS_INODES; i++) {
        if (lerBloco(
                BLOCO_INICIO_INODES + i,
                &buffer[i * TAMANHO_BLOCO]) != 0) {
            return -1;
        }
    }

    // Recupera a tabela de inodes
    memcpy(tabelaInodes, buffer, sizeof(tabelaInodes));

    // Verifica a integridade basica dos registros
    for (int i = 0; i < MAX_ARQUIVOS; i++) {
        if (tabelaInodes[i].usado != 0 &&
            tabelaInodes[i].usado != 1) {
            return -1;
        }

        if (tabelaInodes[i].usado == 1) {
            if (tabelaInodes[i].tamanho < 0 ||
                tabelaInodes[i].quantidadeBlocos < 0 ||
                tabelaInodes[i].quantidadeBlocos >
                    MAX_BLOCOS_ARQUIVO) {
                return -1;
            }

            if (memchr(tabelaInodes[i].nome, '\0',
                       TAMANHO_NOME) == NULL) {
                return -1;
            }
        }
    }

    return 0;
}

// Procura um arquivo pelo nome
// Retorna a posicao do inode ou -1 se nao encontrar
int buscarArquivo(const char *nome) {
    if (nome == NULL) {
        return -1;
    }

    for (int i = 0; i < MAX_ARQUIVOS; i++) {
        if (tabelaInodes[i].usado == 1 &&
            strcmp(tabelaInodes[i].nome, nome) == 0) {
            return i;
        }
    }

    return -1;
}

// Cria um novo arquivo
int criarArquivo(const char *nome) {
    if (nome == NULL || nome[0] == '\0' ||
        strlen(nome) >= TAMANHO_NOME) {
        printf("Erro: nome invalido ou muito grande.\n");
        return -1;
    }

    // Verifica se ja existe um arquivo com esse nome
    if (buscarArquivo(nome) != -1) {
        printf("Erro: arquivo ja existe.\n");
        return -1;
    }

    // Procura um inode disponivel
    int posicao = -1;

    for (int i = 0; i < MAX_ARQUIVOS; i++) {
        if (tabelaInodes[i].usado == 0) {
            posicao = i;
            break;
        }
    }

    if (posicao == -1) {
        printf("Erro: limite de arquivos atingido.\n");
        return -1;
    }

    // Encontra o maior identificador existente
    int maiorId = 0;

    for (int i = 0; i < MAX_ARQUIVOS; i++) {
        if (tabelaInodes[i].usado == 1 &&
            tabelaInodes[i].id > maiorId) {
            maiorId = tabelaInodes[i].id;
        }
    }

    // Inicializa o novo inode
    Inode novoInode;

    memset(&novoInode, 0, sizeof(Inode));

    novoInode.usado = 1;
    novoInode.id = maiorId + 1;

    strcpy(novoInode.nome, nome);

    novoInode.tamanho = 0;
    novoInode.quantidadeBlocos = 0;

    // Registra o inode na tabela
    tabelaInodes[posicao] = novoInode;

    // Persiste os metadados
    if (salvarInodes() != 0) {
        memset(&tabelaInodes[posicao], 0, sizeof(Inode));
        return -1;
    }

    printf("Arquivo '%s' criado com sucesso. ID: %d\n",
           nome, novoInode.id);

    return 0;
}


int removerArquivo(const char *nome) {
    int posicao = buscarArquivo(nome);

    if (posicao == -1) {
        printf("Erro: arquivo nao encontrado.\n");
        return -1;
    }

    Inode *inode = &tabelaInodes[posicao];

    // Guarda o nome antes de apagar os metadados
    char nomeArquivo[TAMANHO_NOME];
    strcpy(nomeArquivo, inode->nome);

    // Verifica as referencias aos blocos
    for (int i = 0; i < inode->quantidadeBlocos; i++) {
        int bloco = inode->blocos[i];

        if (bloco < BLOCO_INICIO_DADOS ||
            bloco >= (int)superbloco.totalBlocos ||
            bitmap[bloco] != 1) {
            printf("Erro: referencia de bloco invalida.\n");
            return -1;
        }
    }

    // Libera os blocos de dados
    for (int i = 0; i < inode->quantidadeBlocos; i++) {
        if (liberarBloco(inode->blocos[i]) != 0) {
            printf("Erro ao liberar bloco.\n");
            return -1;
        }
    }

    // Remove os metadados
    memset(inode, 0, sizeof(Inode));

    if (salvarInodes() != 0) {
        return -1;
    }

    printf("Arquivo '%s' removido com sucesso.\n",
           nomeArquivo);

    return 0;
}

// Exibe as informacoes de um arquivo (stat)
int consultarArquivo(const char *nome) {
    int posicao = buscarArquivo(nome);

    if (posicao == -1) {
        printf("Erro: arquivo nao encontrado.\n");
        return -1;
    }

    Inode *inode = &tabelaInodes[posicao];

    printf("\n----- Informacoes do arquivo -----\n");

    printf("Nome: %s\n", inode->nome);
    printf("ID: %d\n", inode->id);
    printf("Tamanho: %d bytes\n", inode->tamanho);
    printf("Quantidade de blocos: %d\n",
           inode->quantidadeBlocos);

    printf("Blocos utilizados: ");

    if (inode->quantidadeBlocos == 0) {
        printf("Nenhum");
    } else {
        for (int i = 0; i < inode->quantidadeBlocos; i++) {
            printf("%d", inode->blocos[i]);

            if (i < inode->quantidadeBlocos - 1) {
                printf(", ");
            }
        }
    }

    printf("\n----------------------------------\n");

    return 0;
}

// Lista todos os arquivos ativos no sistema (comando ls)
void listarArquivos() {
    int encontrados = 0;

    printf("\n----- Arquivos no MiniFS -----\n");
    for (int i = 0; i < MAX_ARQUIVOS; i++) {
        if (tabelaInodes[i].usado == 1) {
            printf("ID: %d | Nome: %-20s | Tamanho: %d bytes | Blocos: %d\n",
                   tabelaInodes[i].id,
                   tabelaInodes[i].nome,
                   tabelaInodes[i].tamanho,
                   tabelaInodes[i].quantidadeBlocos);
            encontrados++;
        }
    }

    if (encontrados == 0) {
        printf("Nenhum arquivo encontrado.\n");
    }
    printf("------------------------------\n");
}

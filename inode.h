#ifndef INODE_H
#define INODE_H

#include "disco.h"

#define MAX_ARQUIVOS 32
#define MAX_BLOCOS_ARQUIVO 10
#define TAMANHO_NOME 32

typedef struct {
    int usado;
    int id;
    char nome[TAMANHO_NOME];
    int tamanho;
    int quantidadeBlocos;
    int blocos[MAX_BLOCOS_ARQUIVO];
} Inode;

extern Inode tabelaInodes[MAX_ARQUIVOS];

int inicializarInodes();
int salvarInodes();
int carregarInodes();

int buscarArquivo(const char *nome);
int criarArquivo(const char *nome);
int removerArquivo(const char *nome);
int consultarArquivo(const char *nome);

#endif
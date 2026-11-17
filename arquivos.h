#ifndef ARQUIVOS_H
#define ARQUIVOS_H

#include "disco.h"
#include "inode.h"

// Limite máximo por arquivo: 10 blocos * 512 bytes = 5120 bytes
#define TAMANHO_MAX_ARQUIVO (MAX_BLOCOS_ARQUIVO * TAMANHO_BLOCO)

// Operações de leitura e escrita de dados (Estudante 3)
int escreverArquivo(const char *nome, const char *conteudo);
int lerArquivo(const char *nome);

#endif // ARQUIVOS_H

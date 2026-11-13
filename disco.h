#ifndef DISCO_H
#define DISCO_H

#include <stdint.h>
#include <stdbool.h>

// Configurações físicas do disco virtual
#define NOME_ARQUIVO_DISCO "disco.bin"
#define TAMANHO_BLOCO 512
#define MAX_BLOCOS 2048
#define ASSINATURA_FS "MINIFS"

// Divisão fixa dos blocos iniciais do disco (Layout)
#define BLOCO_SUPERBLOCO 0
#define BLOCO_INICIO_BITMAP 1
#define QTD_BLOCOS_BITMAP 4     // 4 blocos de 512 bytes = 2048 bytes (cobre os 2048 blocos)
#define BLOCO_INICIO_INODES 5
#define QTD_BLOCOS_INODES 8     // 8 blocos reservados para o Integrante 2 salvar os metadados
#define BLOCO_INICIO_DADOS 13   // Blocos de dados dos arquivos começam no índice 13

// Estrutura do Superbloco (guardada no Bloco 0)
typedef struct {
    char assinatura[8];
    uint32_t tamanhoBloco;
    uint32_t totalBlocos;
    uint32_t blocosLivres;
} Superbloco;

// Variáveis globais mantidas em memória após carregar ou formatar o disco
extern Superbloco superbloco;
extern uint8_t bitmap[MAX_BLOCOS];

// Funções de controle do disco e persistência
int formatarDisco(uint32_t totalBlocos);
int carregarDisco();
int salvarEstadoDisco();

// Funções de gerenciamento de blocos (Bitmap)
int alocarBloco();
int liberarBloco(int indiceBloco);

// Funções de leitura e escrita física de blocos (I/O de baixo nível)
int lerBloco(int indiceBloco, void *buffer);
int escreverBloco(int indiceBloco, const void *buffer);

// Função auxiliar para o comando 'df'
void exibirInfoDisco();

#endif // DISCO_H
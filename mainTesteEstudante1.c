#include <stdio.h>
#include <string.h>
#include "disco.h"

int main() {
    printf("TESTE DA INFRAESTRUTURA DO DISCO (ESTUDANTE 1)\n\n");

    // 1. Teste de Formatacao e Superbloco
    printf("[1] Formatando o disco virtual com 2048 blocos...\n");
    if (formatarDisco(2048) == 0) {
        printf("Disco formatado com sucesso!\n");
        exibirInfoDisco();
    } else {
        printf("Erro ao formatar o disco.\n");
        return 1;
    }

    // 2. Teste de Alocacao de Blocos no Bitmap
    printf("\n[2] Alocando 3 blocos de dados no Bitmap...\n");
    int b1 = alocarBloco();
    int b2 = alocarBloco();
    int b3 = alocarBloco();
    printf("Blocos alocados: %d, %d e %d (0 a 12 sao reservados ao sistema)\n", b1, b2, b3);
    exibirInfoDisco();

    // 3. Teste de Liberacao e Reutilizacao de Blocos
    printf("\n[3] Liberando o bloco %d e alocando um novo em seguida...\n", b2);
    liberarBloco(b2);
    exibirInfoDisco();

    int bNovo = alocarBloco();
    printf("Novo bloco alocado: %d (Comprova a reutilizacao do bloco %d)\n", bNovo, b2);
    exibirInfoDisco();

    // 4. Teste de Escrita e Leitura Fisica em um Bloco
    printf("\n[4] Testando gravacao bruta de 512 bytes no bloco %d...\n", b1);
    char dadosEscrita[TAMANHO_BLOCO];
    memset(dadosEscrita, 0, TAMANHO_BLOCO);
    strcpy(dadosEscrita, "Teste de gravacao fisica direta no bloco do MiniFS!");

    escreverBloco(b1, dadosEscrita);
    printf("Texto gravado no disco.bin no bloco %d.\n", b1);

    // 5. Teste de Persistencia (Simulando reinicio do programa)
    printf("\n[5] Simulando fechamento do programa (zerando variaveis da RAM)...\n");
    memset(&superbloco, 0, sizeof(Superbloco));
    memset(bitmap, 0, sizeof(bitmap));
    printf("RAM zerada -> Blocos livres na RAM agora: %u\n", superbloco.blocosLivres);

    printf("Recarregando estado a partir do arquivo fisico disco.bin...\n");
    if (carregarDisco() == 0) {
        printf("Disco recuperado com sucesso!\n");
        exibirInfoDisco();

        char dadosLeitura[TAMANHO_BLOCO];
        lerBloco(b1, dadosLeitura);
        printf("Conteudo lido do bloco %d apos recarregar: \"%s\"\n", b1, dadosLeitura);
    } else {
        printf("Falha ao recuperar o disco.\n");
    }

    // 6. Teste de Limite (Tratamento de erro de Disco Cheio)
    printf("\n[6] Testando protecao contra Disco Cheio...\n");
    int alocadosExtras = 0;
    while (alocarBloco() != -1) {
        alocadosExtras++;
    }
    printf("Foram alocados mais %d blocos ate esgotar o disco.\n", alocadosExtras);
    exibirInfoDisco();

    int tentativaExtra = alocarBloco();
    printf("Tentativa de alocar com disco cheio retornou: %d (Bloqueio OK)\n", tentativaExtra);

    return 0;
}
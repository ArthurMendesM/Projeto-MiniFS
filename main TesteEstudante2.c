
#include <stdio.h>
#include "disco.h"
#include "inode.h"

int main() {
    printf("====================================\n");
    printf("     TESTES DO SISTEMA DE INODES\n");
    printf("====================================\n\n");

    // 1. Formatar o disco
    printf("TESTE 1 - Formatacao do disco\n");

    if (formatarDisco(MAX_BLOCOS) != 0) {
        printf("Erro ao formatar o disco.\n");
        return 1;
    }

    if (inicializarInodes() != 0) {
        printf("Erro ao inicializar os Inodes.\n");
        return 1;
    }

    printf("Disco formatado e Inodes inicializados.\n\n");


    // 2. Criar um arquivo
    printf("TESTE 2 - Criacao de arquivo\n");

    if (criarArquivo("teste.txt") == 0) {
        printf("Teste de criacao: APROVADO\n\n");
    } else {
        printf("Teste de criacao: FALHOU\n");
        return 1;
    }


    // 3. Tentar criar um arquivo duplicado
    printf("TESTE 3 - Bloqueio de nomes duplicados\n");

    if (criarArquivo("teste.txt") != 0) {
        printf("Teste de duplicidade: APROVADO\n\n");
    } else {
        printf("Teste de duplicidade: FALHOU\n\n");
    }


    // 4. Alocar blocos e associá-los ao Inode
    printf("TESTE 4 - Associacao de blocos ao arquivo\n");

    int posicao = buscarArquivo("teste.txt");

    if (posicao == -1) {
        printf("Arquivo nao encontrado.\n");
        return 1;
    }

    int bloco1 = alocarBloco();
    int bloco2 = alocarBloco();

    if (bloco1 == -1 || bloco2 == -1) {
        printf("Erro ao alocar blocos.\n");

        if (bloco1 != -1)
            liberarBloco(bloco1);

        if (bloco2 != -1)
            liberarBloco(bloco2);

        return 1;
    }

    tabelaInodes[posicao].tamanho = 1000;
    tabelaInodes[posicao].quantidadeBlocos = 2;
    tabelaInodes[posicao].blocos[0] = bloco1;
    tabelaInodes[posicao].blocos[1] = bloco2;

    if (salvarInodes() != 0) {
        printf("Erro ao salvar os Inodes.\n");
        return 1;
    }

    printf("Dois blocos foram associados ao arquivo.\n");
    printf("Blocos utilizados: %d e %d\n\n", bloco1, bloco2);


    // 5. Consultar os metadados
    printf("TESTE 5 - Consulta com stat\n");

    if (consultarArquivo("teste.txt") == 0) {
        printf("Teste de consulta: APROVADO\n\n");
    } else {
        printf("Teste de consulta: FALHOU\n\n");
    }


    // 6. Remover o arquivo
    printf("TESTE 6 - Remocao do arquivo\n");

    if (removerArquivo("teste.txt") == 0) {
        printf("Teste de remocao: APROVADO\n\n");
    } else {
        printf("Teste de remocao: FALHOU\n");
        return 1;
    }


    // 7. Verificar se os blocos foram liberados
    printf("TESTE 7 - Verificacao do bitmap\n");

    if (bitmap[bloco1] == 0 && bitmap[bloco2] == 0) {
        printf("Os dois blocos foram liberados corretamente.\n");
        printf("Teste de liberacao: APROVADO\n\n");
    } else {
        printf("Erro: os blocos ainda estao ocupados.\n");
        printf("Teste de liberacao: FALHOU\n\n");
    }


    // 8. Verificar se o arquivo foi removido da tabela
    printf("TESTE 8 - Verificacao da tabela de Inodes\n");

    if (buscarArquivo("teste.txt") == -1) {
        printf("Arquivo nao encontrado na tabela.\n");
        printf("Teste de remocao do Inode: APROVADO\n\n");
    } else {
        printf("Erro: arquivo ainda consta na tabela.\n");
        printf("Teste de remocao do Inode: FALHOU\n\n");
    }

    printf("====================================\n");
    printf("          TESTES FINALIZADOS\n");
    printf("====================================\n");

    return 0;
}

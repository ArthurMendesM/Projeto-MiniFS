#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "disco.h"
#include "inode.h"
#include "arquivos.h"

#define TAMANHO_LINHA 6144

static void exibirAjuda() {
    printf("\nComandos disponiveis no MiniFS:\n");
    printf("  format <blocos>             - Formata o disco virtual (ex: format 2048)\n");
    printf("  create <arquivo>            - Cria um novo arquivo vazio\n");
    printf("  write <arquivo> \"<texto>\"   - Escreve conteudo em um arquivo (max %d bytes)\n", TAMANHO_MAX_ARQUIVO);
    printf("  read <arquivo>              - Exibe o conteudo de um arquivo\n");
    printf("  ls                          - Lista os arquivos armazenados\n");
    printf("  stat <arquivo>              - Exibe metadados e blocos de um arquivo\n");
    printf("  df                          - Exibe informacoes de espaco do disco\n");
    printf("  rm <arquivo>                - Remove um arquivo e libera seus blocos\n");
    printf("  help                        - Mostra esta lista de comandos\n");
    printf("  exit                        - Encerra o programa\n\n");
}

static char *pularEspacos(char *str) {
    while (*str != '\0' && isspace((unsigned char)*str)) {
        str++;
    }
    return str;
}

static void removerQuebraLinha(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[--len] = '\0';
    }
}

int main() {
    bool discoPronto = false;
    char linha[TAMANHO_LINHA];

    printf("============================================\n");
    printf("        MiniFS - Sistema de Arquivos        \n");
    printf("============================================\n");

    // Tenta restaurar um disco virtual existente para manter persistência
    if (carregarDisco() == 0 && carregarInodes() == 0) {
        discoPronto = true;
        printf("Disco '%s' carregado com sucesso (%u/%u blocos livres).\n",
               NOME_ARQUIVO_DISCO, superbloco.blocosLivres, superbloco.totalBlocos);
    } else {
        printf("Nenhum disco formatado encontrado. Execute 'format <blocos>' (ex: format 2048).\n");
    }
    printf("Digite 'help' para ver os comandos disponiveis.\n\n");

    while (1) {
        printf("minifs> ");
        fflush(stdout);

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            printf("\nEncerrando MiniFS...\n");
            break;
        }

        removerQuebraLinha(linha);
        char *cursor = pularEspacos(linha);

        if (*cursor == '\0') {
            continue;
        }

        // Extrai o primeiro token (nome do comando)
        char *comando = cursor;
        while (*cursor != '\0' && !isspace((unsigned char)*cursor)) {
            cursor++;
        }
        if (*cursor != '\0') {
            *cursor = '\0';
            cursor++;
        }
        char *args = pularEspacos(cursor);

        // Comandos que não exigem disco previamente formatado
        if (strcmp(comando, "exit") == 0 || strcmp(comando, "quit") == 0 || strcmp(comando, "sair") == 0) {
            printf("Encerrando MiniFS...\n");
            break;
        }

        if (strcmp(comando, "help") == 0 || strcmp(comando, "ajuda") == 0) {
            exibirAjuda();
            continue;
        }

        if (strcmp(comando, "format") == 0) {
            if (*args == '\0') {
                printf("Uso: format <total_blocos> (ex: format 2048)\n");
                continue;
            }

            char *fimNumero = NULL;
            long totalBlocos = strtol(args, &fimNumero, 10);
            fimNumero = pularEspacos(fimNumero);

            if (*fimNumero != '\0' || totalBlocos <= BLOCO_INICIO_DADOS || totalBlocos > MAX_BLOCOS) {
                printf("Erro: quantidade de blocos invalida (informe entre %d e %d).\n",
                       BLOCO_INICIO_DADOS + 1, MAX_BLOCOS);
                continue;
            }

            if (formatarDisco((uint32_t)totalBlocos) == 0 && inicializarInodes() == 0) {
                discoPronto = true;
                printf("Disco formatado com %ld blocos (%d bytes/bloco).\n",
                       totalBlocos, TAMANHO_BLOCO);
            } else {
                printf("Erro ao formatar o disco virtual.\n");
            }
            continue;
        }

        // Bloqueia operações caso o disco ainda não tenha sido formatado ou carregado
        if (!discoPronto) {
            printf("Erro: disco nao inicializado. Execute 'format <blocos>' primeiro.\n");
            continue;
        }

        if (strcmp(comando, "ls") == 0) {
            listarArquivos();
        } else if (strcmp(comando, "df") == 0) {
            exibirInfoDisco();
        } else if (strcmp(comando, "create") == 0) {
            if (*args == '\0') {
                printf("Uso: create <nome_arquivo>\n");
                continue;
            }
            criarArquivo(args);
        } else if (strcmp(comando, "rm") == 0) {
            if (*args == '\0') {
                printf("Uso: rm <nome_arquivo>\n");
                continue;
            }
            removerArquivo(args);
        } else if (strcmp(comando, "stat") == 0) {
            if (*args == '\0') {
                printf("Uso: stat <nome_arquivo>\n");
                continue;
            }
            consultarArquivo(args);
        } else if (strcmp(comando, "read") == 0) {
            if (*args == '\0') {
                printf("Uso: read <nome_arquivo>\n");
                continue;
            }
            lerArquivo(args);
        } else if (strcmp(comando, "write") == 0) {
            if (*args == '\0') {
                printf("Uso: write <nome_arquivo> \"<conteudo>\"\n");
                continue;
            }

            // Separa o nome do arquivo
            char *nomeArquivo = args;
            while (*args != '\0' && !isspace((unsigned char)*args)) {
                args++;
            }
            if (*args == '\0') {
                printf("Uso: write <nome_arquivo> \"<conteudo>\"\n");
                continue;
            }
            *args = '\0';
            args++;

            char *conteudo = pularEspacos(args);
            if (*conteudo == '\0') {
                printf("Uso: write <nome_arquivo> \"<conteudo>\"\n");
                continue;
            }

            // Trata conteúdo entre aspas duplas (ex: write arquivo.txt "Conteudo de teste")
            if (*conteudo == '"') {
                conteudo++;
                char *ultimaAspa = strrchr(conteudo, '"');
                if (ultimaAspa == NULL) {
                    printf("Erro: aspas de fechamento ausentes no comando write.\n");
                    continue;
                }
                *ultimaAspa = '\0';
            }

            escreverArquivo(nomeArquivo, conteudo);
        } else {
            printf("Comando desconhecido: '%s'. Digite 'help' para ver as opcoes.\n", comando);
        }
    }

    return 0;
}

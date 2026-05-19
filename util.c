#include "noc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void limparBufferLocal(void) {

    int c;

    while ((c = getchar()) != '\n' && c != EOF);
}


void limparBuffer(void) {

    limparBufferLocal();
}


void obterDataAtual(char *data) {

    time_t t;

    struct tm *tm_info;

    /* Obter tempo atual */

    t = time(NULL);

    /* Converter para estrutura local */

    tm_info = localtime(&t);

    /*
     * Formatar data/hora
     *
     * Exemplo:
     * 15/05/2026 14:35:20
     */

    strftime(data,
             MAX_DATA,
             "%d/%m/%Y %H:%M:%S",
             tm_info);
}


int lerInteiro(const char *prompt,
               int min,
               int max) {

    int valor;

    while (1) {

        printf("%s", prompt);

        if (scanf("%d", &valor) != 1) {

            printf("Entrada invalida!\n");

            limparBuffer();

            continue;
        }

        limparBuffer();

        if (valor < min || valor > max) {

            printf("Valor fora do intervalo permitido!\n");

            continue;
        }

        return valor;
    }
}


void lerString(const char *prompt,
               char *dest,
               int maxLen) {

    printf("%s", prompt);

    fgets(dest, maxLen, stdin);

    /* Remover '\n' */

    dest[strcspn(dest, "\n")] = '\0';
}
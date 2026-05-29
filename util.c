#include "noc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void limparBufferLocal(void) {

    int c;

    while ((c = getchar()) != '\n' && c != EOF);
}


int lerInteiro(const char *prompt, int min, int max)
{
    int valor;
    char lixo;

    while (1)
    {
        printf("%s: ", prompt);
        if (scanf("%d%c", &valor, &lixo) == 2 && lixo == '\n'
            && valor >= min && valor <= max)
        {
            return valor;
        }
        limparBuffer();
        printf("  [!] Valor inválido. Introduza um número entre %d e %d.\n", min, max);
    }
}

void limparBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void limparEcra(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
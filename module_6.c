#include <stdio.h>
#include <stdlib.h>
#include "noc.h"

static void limparBufferLocal(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static void pausarLocal(void) {
    printf("\nPressione ENTER para continuar...");
    limparBufferLocal();
    getchar();
}

static void limparEcraLocal(void) {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void menuRelatorios(Sistema *s) {
    int opcao;
    
    do {
        limparEcraLocal();
        printf("============================================\n");
        printf("       MODULO 6 - RELATORIOS E FICHEIROS     \n");
        printf("============================================\n");
        printf("  1. Guardar dados                           \n");
        printf("  2. Carregar dados                          \n");
        printf("--------------------------------------------\n");
        printf("  0. Voltar ao menu principal                \n");
        printf("============================================\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
        limparBufferLocal();
        
        switch(opcao) {
            case 1:
                guardarFicheiro(s);
                break;
            case 2:
                carregarFicheiro(s);
                break;
            case 0:
                printf("A voltar...\n");
                break;
            default:
                printf("Opcao invalida!\n");
        }
        
        if(opcao != 0) {
            pausarLocal();
        }
    } while(opcao != 0);
}

void gerarRelatorioEstadoRede(int mes, int ano) { 
    printf("Funcionalidade em desenvolvimento...\n"); 
}

void gerarRelatorioMensalIncidentes(int mes, int ano) { 
    printf("Funcionalidade em desenvolvimento...\n"); 
}
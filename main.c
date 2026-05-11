#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "noc.h"
#include "noc.h"
#include "noc.h"
#include "noc.h"
#include "noc.h"
#include "noc.h"

// Declaracoes das funcoes dos modulos
void menuInventario();
void menuConectividade();
void menuSensores();
void menuIncidentes();
void menuConfiguracoes();
void menuRelatorios();
void carregarTodosDados();
void guardarTodosDados();

void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pausar() {
    printf("\nPressione ENTER para continuar...");
    limparBuffer();
    getchar();
}

void limparEcra() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

int main() {
    setlocale(LC_ALL, "Portuguese");
    int opcao;
    
    printf("============================================\n");
    printf("   SISTEMA MINI NOC - MONITORIZACAO DE REDE  \n");
    printf("   Licenciatura em Redes e Sistemas         \n");
    printf("============================================\n");
    printf("\nA carregar dados existentes...\n");
    
    carregarTodosDados();
    
    printf("Dados carregados com sucesso!\n");
    printf("\nPrima ENTER para continuar...");
    limparBuffer();
    getchar();
    
    do {
        limparEcra();
        printf("============================================\n");
        printf("   SISTEMA MINI NOC - MONITORIZACAO DE REDE  \n");
        printf("============================================\n");
        printf("  1. Modulo 1 - Inventario de Equipamentos   \n");
        printf("  2. Modulo 2 - Testes de Conectividade      \n");
        printf("  3. Modulo 3 - Monitorizacao de Sensores    \n");
        printf("  4. Modulo 4 - Incidentes Tecnicos          \n");
        printf("  5. Modulo 5 - Registo de Configuracoes     \n");
        printf("  6. Modulo 6 - Relatorios e Ficheiros       \n");
        printf("--------------------------------------------\n");
        printf("  0. Sair da Aplicacao                       \n");
        printf("============================================\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
        limparBuffer();
        
        switch(opcao) {
            case 1:
                //menuInventario();
                break;
            case 2:
                menuConectividade();
                break;
            case 3:
                //menuSensores();
                break;
            case 4:
                menuIncidentes();
                break;
            case 5:
                //menuConfiguracoes();
                break;
            case 6:
                menuRelatorios();
                break;
            case 0:
                printf("\nA guardar dados antes de sair...\n");
                guardarTodosDados();
                printf("Aplicacao terminada. Ate breve!\n");
                break;
            default:
                printf("\nOpcao invalida! Tente novamente.\n");
                pausar();
        }
    } while(opcao != 0);
    
    return 0;
}
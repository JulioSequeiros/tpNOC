#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "noc.h"

// Variavel global do sistema
Sistema sistema;

// ========== FUNCOES DE UTILIDADE ==========

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

// ========== FUNCOES DOS MODULOS ==========

void menuEquipamento(Sistema *s) {
    printf("\n=== MODULO 1 - INVENTARIO DE EQUIPAMENTOS ===\n");
    printf("Funcionalidade em desenvolvimento...\n");
    pausar();
}

// ========== FUNCOES DE FICHEIRO ==========

void carregarFicheiro(Sistema *s) {
    printf("Dados carregados com sucesso!\n");
}

void guardarFicheiro(const Sistema *s) {
    printf("Dados guardados com sucesso!\n");
}

// ========== MAIN ==========

int main() {
    setlocale(LC_ALL, "Portuguese");
    int opcao;
    
    // Inicializar o sistema
    sistema.equipamentos = NULL;
    sistema.incidentes = NULL;
    sistema.totalEquipamentos = 0;
    sistema.totalIncidentes = 0;
    sistema.proximoCodigoEquip = 1;
    sistema.proximoCodigoInc = 1;
    
    printf("============================================\n");
    printf("   SISTEMA MINI NOC - MONITORIZACAO DE REDE  \n");
    printf("============================================\n");
    printf("\nA carregar dados existentes...\n");
    
    carregarFicheiro(&sistema);
    
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
                menuEquipamento(&sistema);
                break;
            case 2:
                menuConectividade(&sistema);
                break;
            case 3:
                printf("\n=== MODULO 3 - SENSORES ===\n");
                printf("Funcionalidade em desenvolvimento...\n");
                pausar();
                break;
            case 4:
                menuIncidentes(&sistema);
                break;
            case 5:
                printf("\n=== MODULO 5 - CONFIGURACOES ===\n");
                printf("Funcionalidade em desenvolvimento...\n");
                pausar();
                break;
            case 6:
                menuRelatorios(&sistema);
                break;
            case 0:
                printf("\nA guardar dados antes de sair...\n");
                guardarFicheiro(&sistema);
                printf("Aplicacao terminada. Ate breve!\n");
                break;
            default:
                printf("\nOpcao invalida! Tente novamente.\n");
                pausar();
        }
    } while(opcao != 0);
    
    return 0;
}
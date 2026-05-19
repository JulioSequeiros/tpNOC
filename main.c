#include "noc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>


// ================= PROTÓTIPOS =================

void limparBuffer();
void carregarFicheiro(Sistema *s);
void guardarFicheiro(const Sistema *s);

// menus (assumidos existentes)
void menuConectividade(Sistema *s);
void menuIncidentes(Sistema *s);
void menuRelatorios(Sistema *s);

// ================= MAIN =================

int main() {
    setlocale(LC_ALL, "");

    int opcao;

    Sistema sistema = {0};

    printf("A iniciar sistema...\n");
    carregarFicheiro(&sistema);

    do {
        printf("\n============================================\n");
        printf("   SISTEMA MINI NOC - MENU PRINCIPAL\n");
        printf("============================================\n");
        printf("  1. Modulo 1 - Inventario de Equipamentos\n");
        printf("  2. Modulo 2 - Testes de Conectividade\n");
        printf("  3. Modulo 3 - Monitorizacao de Sensores\n");
        printf("  4. Modulo 4 - Incidentes Tecnicos\n");
        printf("  5. Modulo 5 - Configuracoes\n");
        printf("  6. Modulo 6 - Relatorios\n");
        printf("--------------------------------------------\n");
        printf("  0. Sair\n");
        printf("============================================\n");

        printf("Opcao: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Entrada invalida!\n");
            limparBuffer();   // limpa input inválido
            continue;
        }

        limparBuffer(); // evita lixo no buffer após input válido

        switch(opcao) {

            case 1:
                printf("Modulo 1 - Inventario (em desenvolvimento)\n");
                // menuInventario(&sistema);
                break;

            case 2:
                menuConectividade(&sistema);
                break;

            case 3:
                printf("Modulo 3 - Sensores (em desenvolvimento)\n");
                // menuSensores(&sistema);
                break;

            case 4:
                menuIncidentes(&sistema);
                break;

            case 5:
                printf("Modulo 5 - Configuracoes (em desenvolvimento)\n");
                // menuConfiguracoes(&sistema);
                break;

            case 6:
                menuRelatorios(&sistema);
                break;

            case 0:
                printf("\nA guardar dados...\n");
                guardarFicheiro(&sistema);
                printf("Ate breve!\n");
                break;

            default:
                printf("\nOpcao invalida!\n");
        }

    } while(opcao != 0);

    return 0;
}
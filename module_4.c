#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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

void menuIncidentes(Sistema *s) {
    int opcao;
    
    do {
        limparEcraLocal();
        printf("============================================\n");
        printf("        MODULO 4 - INCIDENTES TECNICOS       \n");
        printf("============================================\n");
        printf("  1. Listar incidentes pendentes             \n");
        printf("  2. Listar todos incidentes                 \n");
        printf("--------------------------------------------\n");
        printf("  0. Voltar ao menu principal                \n");
        printf("============================================\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
        limparBufferLocal();
        
        switch(opcao) {
            case 1:
                listarIncidentesPendentes(s);
                break;
            case 2:
                listarIncidentes(s);
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

void listarIncidentesPendentes(const Sistema *s) {
    printf("\n=== INCIDENTES PENDENTES ===\n");
    NodeIncidente *atual = s->incidentes;
    int count = 0;
    
    while(atual != NULL) {
        if(atual->dados.estado == PENDENTE) {
            printf("Incidente #%d - Equipamento: %d - %s\n", 
                   atual->dados.codigo, atual->dados.codigoEquipamento, atual->dados.descricao);
            count++;
        }
        atual = atual->proximo;
    }
    
    if(count == 0) printf("Nenhum incidente pendente.\n");
}

void listarIncidentes(const Sistema *s) {
    printf("\n=== TODOS INCIDENTES ===\n");
    NodeIncidente *atual = s->incidentes;
    int count = 0;
    
    while(atual != NULL) {
        printf("Incidente #%d - Equipamento: %d - Estado: %d - %s\n", 
               atual->dados.codigo, atual->dados.codigoEquipamento, 
               atual->dados.estado, atual->dados.descricao);
        count++;
        atual = atual->proximo;
    }
    
    if(count == 0) printf("Nenhum incidente registado.\n");
}

// Funcoes nao implementadas (placeholders)
void criarIncidenteManual(Sistema *s) { printf("Funcionalidade em desenvolvimento...\n"); }
void criarIncidenteAutomaticoPing(Sistema *s, int codigoEquipamento) { }
void criarIncidenteAutomaticoSensor(Sistema *s, char *codigoSensor, char *descricao) { }
void processarProximoIncidente(Sistema *s) { printf("Funcionalidade em desenvolvimento...\n"); }
void resolverIncidenteWrapper(Sistema *s) { printf("Funcionalidade em desenvolvimento...\n"); }
void listarIncidentesPorPrioridade(Sistema *s) { printf("Funcionalidade em desenvolvimento...\n"); }
void listarIncidentesConcluidos(Sistema *s) { printf("Funcionalidade em desenvolvimento...\n"); }
void listarIncidentesEmCurso(Sistema *s) { printf("Funcionalidade em desenvolvimento...\n"); }
void listarIncidentesPorEquipamento(const Sistema *s) { printf("Funcionalidade em desenvolvimento...\n"); }
const char *prioridadeToString(int prioridade) { return "Desconhecida"; }
int temIncidentePendenteEquipamento(const Sistema *s, int codigoEquipamento) { return 0; }
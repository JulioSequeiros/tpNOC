#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "module_4.h"

// ========== FUNCOES DE UTILIDADE ==========

void inicializarIncidentes(NodeIncidente **lista) {
    *lista = NULL;
}

void obterDataAtualIncidente(char *data) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(data, 20, "%d/%m/%Y %H:%M:%S", tm);
}

void limparBufferIncidente(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int lerInteiroIncidente(const char *prompt, int min, int max) {
    int valor;
    printf("%s", prompt);
    scanf("%d", &valor);
    limparBufferIncidente();
    return valor;
}

void lerStringIncidente(const char *prompt, char *dest, int maxLen) {
    printf("%s", prompt);
    fgets(dest, maxLen, stdin);
    dest[strcspn(dest, "\n")] = 0;
}

const char *estadoIncidenteToString(EstadoIncidente estado) {
    switch(estado) {
        case PENDENTE: return "Pendente";
        case EM_CURSO: return "Em Curso";
        case CONCLUIDO: return "Concluido";
        default: return "Desconhecido";
    }
}

const char *prioridadeToString(PrioridadeIncidente prioridade) {
    switch(prioridade) {
        case BAIXA: return "Baixa";
        case MEDIA: return "Media";
        case ALTA: return "Alta";
        default: return "Desconhecida";
    }
}

// ========== FUNCOES PRINCIPAIS ==========

void criarIncidenteManual(NodeIncidente **lista, int *proximoNumero) {
    printf("=== CRIAR INCIDENTE MANUAL ===\n");
    printf("Funcionalidade em desenvolvimento...\n");
}

void criarIncidenteAutomaticoPing(NodeIncidente **lista, int *proximoNumero, int codigoEquipamento) {
    printf("=== CRIAR INCIDENTE AUTO PING (Equipamento: %d) ===\n", codigoEquipamento);
    printf("Funcionalidade em desenvolvimento...\n");
}

void criarIncidenteAutomaticoSensor(NodeIncidente **lista, int *proximoNumero, char *codigoSensor, char *descricao) {
    printf("=== CRIAR INCIDENTE AUTO SENSOR (%s) ===\n", codigoSensor);
    printf("Funcionalidade em desenvolvimento...\n");
}

void adicionarIncidenteFila(NodeIncidente **lista, Incidente *incidente) {
    printf("Adicionando incidente a fila...\n");
    printf("Funcionalidade em desenvolvimento...\n");
}

void processarProximoIncidente(NodeIncidente **lista) {
    printf("=== PROCESSAR PROXIMO INCIDENTE ===\n");
    printf("Funcionalidade em desenvolvimento...\n");
}

void concluirIncidente(NodeIncidente **lista, int numero) {
    printf("=== CONCLUIR INCIDENTE (Numero: %d) ===\n", numero);
    printf("Funcionalidade em desenvolvimento...\n");
}

void listarTodosIncidentes(NodeIncidente *lista) {
    printf("=== LISTAR TODOS INCIDENTES ===\n");
    printf("Funcionalidade em desenvolvimento...\n");
}

void listarIncidentesPorEstado(NodeIncidente *lista, EstadoIncidente estado) {
    printf("=== LISTAR INCIDENTES POR ESTADO ===\n");
    printf("Funcionalidade em desenvolvimento...\n");
}

void listarIncidentesPorEntidade(NodeIncidente *lista, char *entidadeId) {
    printf("=== LISTAR INCIDENTES POR ENTIDADE (%s) ===\n", entidadeId);
    printf("Funcionalidade em desenvolvimento...\n");
}

void listarIncidentesPorPrioridade(NodeIncidente *lista, PrioridadeIncidente prioridade) {
    printf("=== LISTAR INCIDENTES POR PRIORIDADE ===\n");
    printf("Funcionalidade em desenvolvimento...\n");
}

void listarIncidentesPendentes(NodeIncidente *lista) {
    listarIncidentesPorEstado(lista, PENDENTE);
}

void listarIncidentesEmCurso(NodeIncidente *lista) {
    listarIncidentesPorEstado(lista, EM_CURSO);
}

void listarIncidentesConcluidos(NodeIncidente *lista) {
    listarIncidentesPorEstado(lista, CONCLUIDO);
}

NodeIncidente *encontrarIncidentePorNumero(NodeIncidente *lista, int numero) {
    printf("Procurando incidente %d...\n", numero);
    printf("Funcionalidade em desenvolvimento...\n");
    return NULL;
}

int temIncidentePendenteEquipamento(NodeIncidente *lista, int codigoEquipamento) {
    printf("Verificando incidentes pendentes para equipamento %d...\n", codigoEquipamento);
    printf("Funcionalidade em desenvolvimento...\n");
    return 0;
}

// ========== FUNCOES DE FICHEIRO ==========

void guardarIncidentes(NodeIncidente *lista, const char *nomeFicheiro) {
    printf("Guardando incidentes em %s...\n", nomeFicheiro);
    printf("Funcionalidade em desenvolvimento...\n");
}

void carregarIncidentes(NodeIncidente **lista, int *proximoNumero, const char *nomeFicheiro) {
    printf("Carregando incidentes de %s...\n", nomeFicheiro);
    printf("Funcionalidade em desenvolvimento...\n");
}

void guardarIncidentesBinario(NodeIncidente *lista, const char *nomeFicheiro) {
    printf("Guardando incidentes em ficheiro binario %s...\n", nomeFicheiro);
    printf("Funcionalidade em desenvolvimento...\n");
}

void carregarIncidentesBinario(NodeIncidente **lista, int *proximoNumero, const char *nomeFicheiro) {
    printf("Carregando incidentes de ficheiro binario %s...\n", nomeFicheiro);
    printf("Funcionalidade em desenvolvimento...\n");
}

// ========== FUNCOES DE MENU ==========

void menuIncidentes(NodeIncidente **lista, int *proximoNumero) {
    int opcao;
    
    do {
        system("cls");
        printf("============================================\n");
        printf("        MODULO 4 - INCIDENTES TECNICOS       \n");
        printf("============================================\n");
        printf("  1. Criar incidente manual                  \n");
        printf("  2. Processar proximo incidente da fila     \n");
        printf("  3. Concluir incidente                      \n");
        printf("  4. Listar incidentes pendentes             \n");
        printf("  5. Listar incidentes em curso              \n");
        printf("  6. Listar incidentes concluidos            \n");
        printf("  7. Listar incidentes por equipamento       \n");
        printf("  8. Listar incidentes por prioridade        \n");
        printf("  9. Listar todos incidentes                 \n");
        printf("--------------------------------------------\n");
        printf("  0. Voltar ao menu principal                \n");
        printf("============================================\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
        limparBufferIncidente();
        
        switch(opcao) {
            case 1:
                criarIncidenteManual(lista, proximoNumero);
                break;
            case 2:
                processarProximoIncidente(lista);
                break;
            case 3: {
                int num = lerInteiroIncidente("Numero do incidente: ", 1, 9999);
                concluirIncidente(lista, num);
                break;
            }
            case 4:
                listarIncidentesPendentes(*lista);
                break;
            case 5:
                listarIncidentesEmCurso(*lista);
                break;
            case 6:
                listarIncidentesConcluidos(*lista);
                break;
            case 7: {
                char entidade[MAX_ENTIDADE];
                lerStringIncidente("Codigo do equipamento: ", entidade, MAX_ENTIDADE);
                listarIncidentesPorEntidade(*lista, entidade);
                break;
            }
            case 8: {
                int prioridade = lerInteiroIncidente("Prioridade (1=Baixa, 2=Media, 3=Alta): ", 1, 3);
                listarIncidentesPorPrioridade(*lista, (PrioridadeIncidente)prioridade);
                break;
            }
            case 9:
                listarTodosIncidentes(*lista);
                break;
            case 0:
                printf("A voltar...\n");
                break;
            default:
                printf("Opcao invalida!\n");
        }
        
        if(opcao != 0) {
            printf("\nPressione ENTER para continuar...");
            limparBufferIncidente();
            getchar();
        }
    } while(opcao != 0);
}
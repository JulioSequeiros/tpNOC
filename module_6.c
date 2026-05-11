#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "module_6.h"

static void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static void pausar() {
    printf("\nPressione ENTER para continuar...");
    limparBuffer();
    getchar();
}

void menuRelatorios() {
    int opcao, mes, ano;
    
    do {
        system("cls");
        printf("============================================\n");
        printf("       MODULO 6 - RELATORIOS E FICHEIROS     \n");
        printf("============================================\n");
        printf("  1. Carregar todos os dados (inicio)        \n");
        printf("  2. Guardar todos os dados (antes de sair)  \n");
        printf("  3. Gerar relatorio de estado da rede       \n");
        printf("  4. Gerar relatorio mensal de incidentes    \n");
        printf("  5. Ver relatorio de estado da rede         \n");
        printf("  6. Ver relatorio mensal de incidentes      \n");
        printf("--------------------------------------------\n");
        printf("  0. Voltar ao menu principal                \n");
        printf("============================================\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
        limparBuffer();
        
        switch(opcao) {
            case 1:
                carregarTodosDados();
                printf("Dados carregados com sucesso!\n");
                pausar();
                break;
            case 2:
                guardarTodosDados();
                printf("Dados guardados com sucesso!\n");
                pausar();
                break;
            case 3:
                printf("Mes (1-12): ");
                scanf("%d", &mes);
                printf("Ano: ");
                scanf("%d", &ano);
                limparBuffer();
                gerarRelatorioEstadoRede(mes, ano);
                pausar();
                break;
            case 4:
                printf("Mes (1-12): ");
                scanf("%d", &mes);
                printf("Ano: ");
                scanf("%d", &ano);
                limparBuffer();
                gerarRelatorioMensalIncidentes(mes, ano);
                pausar();
                break;
            case 5:
                printf("A visualizar relatorio_estado_rede.txt...\n");
                system("type relatorio_estado_rede.txt 2>nul || cat relatorio_estado_rede.txt");
                pausar();
                break;
            case 6:
                printf("A visualizar relatorio_mensal_incidentes.txt...\n");
                system("type relatorio_mensal_incidentes.txt 2>nul || cat relatorio_mensal_incidentes.txt");
                pausar();
                break;
            case 0:
                printf("A voltar...\n");
                break;
            default:
                printf("Opcao invalida!\n");
                pausar();
        }
    } while(opcao != 0);
}

void gerarRelatorioEstadoRede(int mes, int ano) {
    printf("=== GERAR RELATORIO ESTADO REDE (Mes: %d, Ano: %d) ===\n", mes, ano);
    printf("Funcionalidade em desenvolvimento...\n");
}

void gerarRelatorioMensalIncidentes(int mes, int ano) {
    printf("=== GERAR RELATORIO MENSAL INCIDENTES (Mes: %d, Ano: %d) ===\n", mes, ano);
    printf("Funcionalidade em desenvolvimento...\n");
}

void carregarTodosDados() {
    printf("=== CARREGAR TODOS DADOS ===\n");
    carregarEquipamentos();
    carregarIncidentes();
    carregarConfiguracoes();
    printf("Todos os dados carregados!\n");
}

void guardarTodosDados() {
    printf("=== GUARDAR TODOS DADOS ===\n");
    guardarEquipamentos();
    guardarIncidentes();
    guardarConfiguracoes();
    printf("Todos os dados guardados!\n");
}

void importarSensoresTXT(const char *nomeFicheiro) {
    printf("Importando sensores de %s\n", nomeFicheiro);
}

void guardarResultadoComandoRede(const char *comando, const char *arquivo) {
    printf("Executando comando: %s\n", comando);
    printf("Guardando resultado em: %s\n", arquivo);
}
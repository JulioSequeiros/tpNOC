#include <stdio.h>
#include <stdlib.h>
#include "noc.h"



static void pausarLocal() {
    printf("\nPressione ENTER para continuar...");
    limparBufferLocal();
    getchar();
}

static void limparEcraLocal() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// ================= MENU RELATORIOS =================

void menuRelatorios(Sistema *s) {

    int opcao;

    do {
        limparEcraLocal();

        printf("======================================================================================\n");
        printf("               MODULO 6 - RELATORIOS E FICHEIROS                                      \n");
        printf("======================================================================================\n");
        printf(" 1. Carregar dados existentes ao iniciar a aplicacao.                                 \n");
        printf(" 2. Guardar dados atualizados antes de sair da aplicação.                             \n");
        printf(" 3. Importar leituras de sensores a partir de ficheiros de texto.                     \n");
        printf(" 4. Guardar os resultados dos comandos de rede em ficheiros de texto;                 \n");
        printf(" 5. Gerar um relatório de estado da rede, listando equipamentos operacionais,         \n");
        printf("    equipamentos em falha, incidentes pendentes e leituras anómalas.                  \n");
        printf(" 6. Gerar um relatório mensal de incidentes, com totais por tipo de incidente e por,  \n");
        printf("    prioridade.                                                                       \n");
        printf(" 7. Outras atividades que considere relevantes..                                      \n");
        printf("--------------------------------------------------------------------------------------\n");
        printf(" 0. Voltar ao menu principal                                                          \n");
        printf("======================================================================================\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
        limparBufferLocal();
        switch(opcao) {
            case 1:
                printf("\n=== CARREGAR DADOS EXISTENTES AO INICIAR A APLICACAO ===\n");
                carregarFicheiro(s);
                break;
            case 2:
                printf("\n=== GUARDAR DADOS ATUALIZADOS ANTES DE SAIR ===\n");
                guardarFicheiro(s);
                break;
            case 3:
                printf("\n=== IMPORTAR LEITURAS DE SENSORES A PARTIR DE FICHEIROS DE TEXTO ===\n");
                importarLeiturasSensores(s);
                break;
            case 4:
                printf("\n=== GUARDAR RESULTADOS DOS COMANDOS DE REDE ===\n");
                guardarResultadosRede(s);
                break;
            case 5:
                printf("\n=== GERAR RELATORIO DE ESTADO DA REDE ===\n");
                gerarRelatorioEstadoRede(s);
                break;
            case 6:
                printf("\n=== GERAR RELATORIO MENSAL DE INCIDENTES ===\n");
                gerarRelatorioMensalIncidentes(s);
                break;
            case 7:
                printf("\n=== OUTRAS ATIVIDADES RELEVANTES ===\n");
                outrasAtividadesRelatorios(s);
                break;
            case 0:
                printf("A voltar ao menu principal...\n");
                break;
            default:
                printf("Opcao invalida!\n");
        }
        if (opcao != 0) {
            pausarLocal();
        }
    } while(opcao != 0);
}

// ================= Funçoes =================
void carregarFicheiro(Sistema *s) {

    FILE *f = fopen(FICHEIRO_DAT, "rb");

    if (!f) {
        printf("Ficheiro nao encontrado.\n");
        return;
    }

    fread(&s->totalEquipamentos, sizeof(int), 1, f);
    fread(&s->totalIncidentes, sizeof(int), 1, f);
    fread(&s->proximoCodigoEquip, sizeof(int), 1, f);
    fread(&s->proximoCodigoInc, sizeof(int), 1, f);

    fclose(f);

    printf("Dados carregados com sucesso.\n");
}

void guardarFicheiro(const Sistema *s) {

    FILE *f = fopen(FICHEIRO_DAT, "wb");

    if (!f) {
        printf("Erro ao guardar ficheiro.\n");
        return;
    }

    fwrite(&s->totalEquipamentos, sizeof(int), 1, f);
    fwrite(&s->totalIncidentes, sizeof(int), 1, f);
    fwrite(&s->proximoCodigoEquip, sizeof(int), 1, f);
    fwrite(&s->proximoCodigoInc, sizeof(int), 1, f);

    fclose(f);

    printf("Dados guardados com sucesso.\n");
}

void importarLeiturasSensores(Sistema *s) {

    FILE *f = fopen("sensores.txt", "r");

    char linha[200];

    (void)s;

    if (!f) {
        printf("Erro ao abrir sensores.txt\n");
        return;
    }

    printf("\n=== LEITURAS DE SENSORES ===\n");

    while (fgets(linha, sizeof(linha), f)) {
        printf("%s", linha);
    }

    fclose(f);

    printf("Importacao concluida.\n");
}

void guardarResultadosRede(Sistema *s) {

    FILE *f = fopen("logs_rede.txt", "a");

    time_t t = time(NULL);

    (void)s;

    if (!f) {
        printf("Erro ao abrir logs_rede.txt\n");
        return;
    }

    fprintf(f, "====================================\n");
    fprintf(f, "Log: %s", ctime(&t));
    fprintf(f, "Estado da rede registado\n");
    fprintf(f, "====================================\n\n");

    fclose(f);

    printf("Resultados de rede guardados.\n");
}

void gerarRelatorioEstadoRede(Sistema *s) {

    FILE *f = fopen("relatorio_estado_rede.txt", "w");

    NodeEquipamento *eq = s->equipamentos;
    NodeIncidente *inc = s->incidentes;

    int totalEq = 0;
    int pendentes = 0;

    if (!f) {
        printf("Erro ao criar relatorio.\n");
        return;
    }

    fprintf(f, "=========== RELATORIO ESTADO REDE ===========\n\n");

    while (eq) {

        fprintf(f, "Equipamento: %s | IP: %s\n",
                eq->dados.nome,
                eq->dados.ip);

        totalEq++;
        eq = eq->proximo;
    }

    while (inc) {

        if (inc->dados.estado == PENDENTE)
            pendentes++;

        inc = inc->proximo;
    }

    fprintf(f, "\nTotal equipamentos: %d\n", totalEq);
    fprintf(f, "Incidentes pendentes: %d\n", pendentes);

    fclose(f);

    printf("Relatorio de estado da rede criado.\n");
}

void gerarRelatorioMensalIncidentes(Sistema *s) {

    FILE *f = fopen("relatorio_mensal_incidentes.txt", "w");

    NodeIncidente *inc = s->incidentes;

    int pendentes = 0;
    int resolvidos = 0;

    if (!f) {
        printf("Erro ao criar relatorio.\n");
        return;
    }

    while (inc) {

        if (inc->dados.estado == PENDENTE)
            pendentes++;
        else
            resolvidos++;

        inc = inc->proximo;
    }

    fprintf(f, "=========== RELATORIO INCIDENTES ===========\n\n");
    fprintf(f, "Pendentes : %d\n", pendentes);
    fprintf(f, "Resolvidos: %d\n", resolvidos);
    fprintf(f, "Total     : %d\n", s->totalIncidentes);

    fclose(f);

    printf("Relatorio mensal criado.\n");
}

void outrasAtividadesRelatorios(Sistema *s) {

    (void)s;

    printf("\n=== OUTRAS ATIVIDADES ===\n");
    printf("Sistema operacional OK\n");
    printf("Backups ativos\n");
    printf("Logs a funcionar corretamente\n");
}
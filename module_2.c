#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "noc.h"

// ========== FUNCOES DE UTILIDADE DO MODULO 2 ==========

static void obterDataAtualLocal(char *data) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(data, 11, "%d/%m/%Y", tm);
}

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

// ========== FUNCOES DO MODULO 2 ==========

void menuConectividade(Sistema *s) {
    int opcao;
    
    do {
        limparEcraLocal();
        printf("============================================\n");
        printf("     MODULO 2 - TESTES DE CONECTIVIDADE      \n");
        printf("============================================\n");
        printf("  1. Executar ping a um equipamento          \n");
        printf("  2. Teste geral da rede (ping a todos)      \n");
        printf("  3. Ver log de monitorizacao                \n");
        printf("--------------------------------------------\n");
        printf("  0. Voltar ao menu principal                \n");
        printf("============================================\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
        limparBufferLocal();
        
        switch(opcao) {
            case 1:
                executarPingEquipamento(s);
                break;
            case 2:
                pingGeral(s);
                break;
            case 3:
                printf("\n=== LOG DE MONITORIZACAO ===\n");
                system("type log_monitorizacao.txt 2>nul");
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

void executarPingEquipamento(Sistema *s) {
    int codigo;
    char comando[256];
    char data[11];
    
    printf("\n=== EXECUTAR PING ===\n");
    printf("Codigo do equipamento: ");
    scanf("%d", &codigo);
    limparBufferLocal();
    
    NodeEquipamento *node = s->equipamentos;
    while(node != NULL && node->dados.codigo != codigo) {
        node = node->proximo;
    }
    
    if(node == NULL) {
        printf("ERRO: Equipamento com codigo %d nao encontrado!\n", codigo);
        return;
    }
    
    printf("Equipamento: %s\n", node->dados.nome);
    printf("IP: %s\n", node->dados.ip);
    printf("A executar ping...\n");
    
    sprintf(comando, "ping -n 4 %s > resultado_ping.txt", node->dados.ip);
    system(comando);
    
    printf("Ping executado. Resultado guardado em resultado_ping.txt\n");
    
    obterDataAtualLocal(data);
    strcpy(node->dados.dataUltimaVerificacao, data);
    printf("Data da ultima verificacao atualizada: %s\n", data);
    
    // Verificar resposta
    FILE *fp = fopen("resultado_ping.txt", "r");
    int respondeu = 0;
    char linha[256];
    
    if(fp != NULL) {
        while(fgets(linha, sizeof(linha), fp)) {
            if(strstr(linha, "Resposta") || strstr(linha, "Reply from") || 
               strstr(linha, "bytes=")) {
                respondeu = 1;
                break;
            }
        }
        fclose(fp);
    }
    
    if(!respondeu) {
        printf("\nEquipamento NAO RESPONDEU ao ping!\n");
        node->dados.estado = EM_FALHA;
        printf("Estado alterado para EM FALHA\n");
    } else {
        printf("\nEquipamento RESPONDEU ao ping!\n");
        if(node->dados.estado == EM_FALHA) {
            node->dados.estado = OPERACIONAL;
            printf("Estado alterado para OPERACIONAL\n");
        }
    }
    
    // Registrar no log
    FILE *log = fopen("log_monitorizacao.txt", "a");
    if(log != NULL) {
        char dataHora[30];
        obterDataAtualLocal(dataHora);
        fprintf(log, "[%s] Ping - Equipamento %d (%s) - %s\n", 
                dataHora, codigo, node->dados.nome, respondeu ? "RESPONDEU" : "NAO RESPONDEU");
        fclose(log);
    }
}

void pingGeral(Sistema *s) {
    char data[11];
    int respostas = 0, falhas = 0;
    
    printf("\n=== TESTE GERAL DA REDE ===\n");
    obterDataAtualLocal(data);
    
    NodeEquipamento *atual = s->equipamentos;
    while(atual != NULL) {
        printf("Testando %s (%s)... ", atual->dados.nome, atual->dados.ip);
        
        char comando[256];
        sprintf(comando, "ping -n 2 %s > temp_ping.txt", atual->dados.ip);
        system(comando);
        
        FILE *fp = fopen("temp_ping.txt", "r");
        int respondeu = 0;
        char linha[256];
        
        if(fp != NULL) {
            while(fgets(linha, sizeof(linha), fp)) {
                if(strstr(linha, "Resposta") || strstr(linha, "Reply from")) {
                    respondeu = 1;
                    break;
                }
            }
            fclose(fp);
        }
        
        strcpy(atual->dados.dataUltimaVerificacao, data);
        
        if(respondeu) {
            printf("RESPONDEU\n");
            respostas++;
            if(atual->dados.estado == EM_FALHA) {
                atual->dados.estado = OPERACIONAL;
            }
        } else {
            printf("NAO RESPONDEU\n");
            falhas++;
            if(atual->dados.estado != EM_FALHA) {
                atual->dados.estado = EM_FALHA;
            }
        }
        
        atual = atual->proximo;
    }
    
    system("del temp_ping.txt 2>nul");
    
    printf("\n=== RESUMO ===\n");
    printf("Total: %d, Respostas: %d, Falhas: %d\n", s->totalEquipamentos, respostas, falhas);
}

void guardarResultadoPing(Sistema *s) { printf("Funcionalidade em desenvolvimento...\n"); }
void verificarRespostaPing(Sistema *s) { printf("Funcionalidade em desenvolvimento...\n"); }
void mostrarEstatisticas(Sistema *s) { printf("Funcionalidade em desenvolvimento...\n"); }
int analisarResultadoPingArquivo(const char *arquivo) { return 0; }
void registarLogMonitorizacaoTexto(const char *mensagem) { printf("Log: %s\n", mensagem); }
void comandoIpConfig(Sistema *s) { printf("Comando ipconfig extra...\n"); }
void comandoArp(Sistema *s) { printf("Comando arp extra...\n"); }
void comandoNslookup(Sistema *s) { printf("Comando nslookup extra...\n"); }
void comandoTracert(Sistema *s) { printf("Comando tracert extra...\n"); }
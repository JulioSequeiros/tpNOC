#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "module_2.h"
#include "noc.h"

// Variavel global do sistema (definida no main)
extern Sistema sistema;

static void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static void pausar() {
    printf("\nPressione ENTER para continuar...");
    limparBuffer();
    getchar();
}

void menuConectividade() {
    int opcao;
    
    do {
        system("cls");
        printf("============================================\n");
        printf("     MODULO 2 - TESTES DE CONECTIVIDADE      \n");
        printf("============================================\n");
        printf("  1. Selecionar equipamento e executar ping  \n");
        printf("  2. Guardar resultado em ficheiro de texto  \n");
        printf("  3. Ler ficheiro e verificar resposta       \n");
        printf("  4. Atualizar data da ultima verificacao    \n");
        printf("  5. Alterar estado para 'Em Falha'          \n");
        printf("  6. Registar teste em log_monitorizacao.txt \n");
        printf("  7. Criar incidente (falha de resposta)     \n");
        printf("  8. Teste geral da rede (ping a todos)      \n");
        printf("  9. Estatisticas e resultados               \n");
        printf("--------------------------------------------\n");
        printf("  0. Voltar ao menu principal                \n");
        printf("============================================\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
        limparBuffer();
        
        switch(opcao) {
            case 1:
                executarPingEquipamento();
                pausar();
                break;
            case 2:
                guardarResultadoPing();
                pausar();
                break;
            case 3:
                verificarRespostaPing();
                pausar();
                break;
            case 4:
                atualizarDataVerificacao();
                pausar();
                break;
            case 5:
                alterarEstadoParaFalha();
                pausar();
                break;
            case 6:
                registarLogMonitorizacao();
                pausar();
                break;
            case 7:
                criarIncidenteFalha();
                pausar();
                break;
            case 8:
                pingGeral();
                pausar();
                break;
            case 9:
                mostrarEstatisticas();
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

// 1. Selecionar equipamento registado e executar ping ao seu endereco IP
void executarPingEquipamento() {
    int codigo;
    char comando[256];
    char data[MAX_DATA];
    
    printf("=== Executar ping ===\n");
    codigo = lerInteiro("Codigo do equipamento: ", 1, 9999);
    
    // Usar funcao do noc.h para encontrar equipamento
    NodeEquipamento *node = encontrarEquipamentoPorCodigo(&sistema, codigo);
    
    if(node == NULL) {
        printf("ERRO: Equipamento com codigo %d nao encontrado!\n", codigo);
        return;
    }
    
    printf("Equipamento: %s\n", node->dados.nome);
    printf("IP: %s\n", node->dados.ip);
    printf("A executar ping...\n");
    
    // Executar ping
    sprintf(comando, "ping -n 4 %s > resultado_ping.txt", node->dados.ip);
    system(comando);
    
    printf("Ping executado. Resultado guardado em resultado_ping.txt\n");
    
    // Atualizar data da ultima verificacao
    obterDataAtual(data);
    strcpy(node->dados.dataUltimaVerificacao, data);
    printf("Data da ultima verificacao atualizada: %s\n", data);
    
    // Verificar resposta
    int respondeu = analisarResultadoPingArquivo("resultado_ping.txt");
    
    if(!respondeu) {
        printf("Equipamento NAO RESPONDEU ao ping!\n");
        node->dados.estado = EM_FALHA;
        printf("Estado alterado para EM FALHA\n");
        
        // Criar incidente automaticamente
        criarIncidenteAutomaticoPing(&sistema, codigo);
    } else {
        printf("Equipamento RESPONDEU ao ping!\n");
        if(node->dados.estado == EM_FALHA) {
            node->dados.estado = OPERACIONAL;
            printf("Estado alterado para OPERACIONAL\n");
        }
    }
    
    // Registar no log
    char logMsg[500];
    sprintf(logMsg, "Ping executado para equipamento %d (%s) - Resultado: %s", 
            codigo, node->dados.nome, respondeu ? "RESPONDEU" : "NAO RESPONDEU");
    registarLogMonitorizacaoTexto(logMsg);
}

// 2. Guardar o resultado bruto do comando num ficheiro de texto
void guardarResultadoPing() {
    char ip[MAX_IP];
    char comando[256];
    char data[MAX_DATA];
    
    printf("=== Guardar resultado ping ===\n");
    lerString("Endereco IP: ", ip, MAX_IP);
    obterDataAtual(data);
    
    printf("A executar ping para IP: %s\n", ip);
    sprintf(comando, "ping -n 4 %s > resultado_ping.txt", ip);
    system(comando);
    
    printf("Resultado guardado em resultado_ping.txt\n");
    
    // Adicionar informacao ao ficheiro
    FILE *fp = fopen("resultado_ping.txt", "a");
    if(fp != NULL) {
        fprintf(fp, "\n--- Informacao do teste ---\n");
        fprintf(fp, "IP testado: %s\n", ip);
        fprintf(fp, "Data e hora: %s\n", data);
        fclose(fp);
    }
}

// 3. Ler o ficheiro de resultado e determinar se o equipamento respondeu
void verificarRespostaPing() {
    analisarResultadoPingArquivo("resultado_ping.txt");
}

// Funcao auxiliar para analisar resultado do ping
int analisarResultadoPingArquivo(const char *arquivo) {
    FILE *fp = fopen(arquivo, "r");
    if(fp == NULL) {
        printf("ERRO: Nao foi possivel abrir %s\n", arquivo);
        return 0;
    }
    
    char linha[256];
    int respondeu = 0;
    
    printf("\n=== ANALISE DO RESULTADO DO PING ===\n");
    printf("Ficheiro: %s\n", arquivo);
    printf("------------------------------------\n");
    
    while(fgets(linha, sizeof(linha), fp)) {
        printf("%s", linha);
        
        // Verificar se houve resposta
        if(strstr(linha, "Resposta") || strstr(linha, "Reply from") || 
           strstr(linha, "bytes=") || strstr(linha, "tempo=")) {
            respondeu = 1;
        }
    }
    
    fclose(fp);
    
    printf("------------------------------------\n");
    if(respondeu) {
        printf("RESULTADO: Equipamento RESPONDEU ao ping\n");
    } else {
        printf("RESULTADO: Equipamento NAO RESPONDEU ao ping\n");
    }
    
    return respondeu;
}

// 4. Atualizar automaticamente a data da ultima verificacao do equipamento
void atualizarDataVerificacao() {
    int codigo;
    char data[MAX_DATA];
    
    printf("=== Atualizar data de verificacao ===\n");
    codigo = lerInteiro("Codigo do equipamento: ", 1, 9999);
    
    NodeEquipamento *node = encontrarEquipamentoPorCodigo(&sistema, codigo);
    
    if(node == NULL) {
        printf("ERRO: Equipamento nao encontrado!\n");
        return;
    }
    
    obterDataAtual(data);
    strcpy(node->dados.dataUltimaVerificacao, data);
    
    printf("Data da ultima verificacao atualizada para: %s\n", data);
    printf("Equipamento: %s\n", node->dados.nome);
}

// 5. Alterar o estado do equipamento para "Em Falha" quando nao existir resposta
void alterarEstadoParaFalha() {
    int codigo;
    
    printf("=== Alterar estado para EM FALHA ===\n");
    codigo = lerInteiro("Codigo do equipamento: ", 1, 9999);
    
    NodeEquipamento *node = encontrarEquipamentoPorCodigo(&sistema, codigo);
    
    if(node == NULL) {
        printf("ERRO: Equipamento nao encontrado!\n");
        return;
    }
    
    node->dados.estado = EM_FALHA;
    printf("Estado do equipamento %s alterado para EM FALHA\n", node->dados.nome);
}

// 6. Registar cada teste realizado num ficheiro de texto denominado log_monitorizacao.txt
void registarLogMonitorizacao() {
    char mensagem[500];
    
    printf("=== Registar log monitorizacao ===\n");
    lerString("Mensagem para registar: ", mensagem, 500);
    registarLogMonitorizacaoTexto(mensagem);
}

// Funcao auxiliar para registar log
void registarLogMonitorizacaoTexto(const char *mensagem) {
    FILE *fp = fopen("log_monitorizacao.txt", "a");
    if(fp == NULL) {
        printf("ERRO: Nao foi possivel abrir log_monitorizacao.txt\n");
        return;
    }
    
    char data[MAX_DATA];
    obterDataAtual(data);
    
    fprintf(fp, "[%s] %s\n", data, mensagem);
    fclose(fp);
    
    printf("Log registado com sucesso em log_monitorizacao.txt\n");
}

// 7. Criar automaticamente um incidente tecnico quando um equipamento nao responde
void criarIncidenteFalha() {
    int codigo;
    
    printf("=== Criar incidente por falha ===\n");
    codigo = lerInteiro("Codigo do equipamento sem resposta: ", 1, 9999);
    
    criarIncidenteAutomaticoPing(&sistema, codigo);
}

// Funcao auxiliar para criar incidente automatico
void criarIncidenteAutomaticoPing(Sistema *s, int codigoEquipamento) {
    NodeEquipamento *node = encontrarEquipamentoPorCodigo(s, codigoEquipamento);
    
    if(node == NULL) {
        printf("ERRO: Equipamento nao encontrado!\n");
        return;
    }
    
    printf("A criar incidente automatico para equipamento %s...\n", node->dados.nome);
    
    // Incrementar contador de incidentes
    s->proximoCodigoInc++;
    
    printf("Incidente #%d criado para equipamento %d - Falha de ping\n", 
           s->proximoCodigoInc, codigoEquipamento);
    printf("Incidente adicionado a fila de atendimento\n");
}

// 8. Permitir um teste geral da rede, executando ping a todos os equipamentos registados
void pingGeral() {
    int respostas = 0;
    int falhas = 0;
    char data[MAX_DATA];
    
    printf("\n=== TESTE GERAL DA REDE ===\n");
    printf("A testar todos os equipamentos...\n\n");
    
    obterDataAtual(data);
    
    NodeEquipamento *atual = sistema.equipamentos;
    while(atual != NULL) {
        printf("%d. %s (%s) - ", 
               atual->dados.codigo, 
               atual->dados.nome, 
               atual->dados.ip);
        
        char comando[256];
        sprintf(comando, "ping -n 2 %s > temp_ping.txt", atual->dados.ip);
        system(comando);
        
        int respondeu = analisarResultadoPingArquivo("temp_ping.txt");
        
        // Atualizar data da ultima verificacao
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
                criarIncidenteAutomaticoPing(&sistema, atual->dados.codigo);
            }
        }
        
        char logMsg[300];
        sprintf(logMsg, "Ping geral - Equipamento %d (%s) - %s", 
                atual->dados.codigo, 
                atual->dados.nome,
                respondeu ? "RESPONDEU" : "NAO RESPONDEU");
        registarLogMonitorizacaoTexto(logMsg);
        
        printf("\n");
        atual = atual->proximo;
    }
    
    // Limpar ficheiro temporario
    system("del temp_ping.txt 2>nul");
    
    printf("\n=== RESUMO DO TESTE GERAL ===\n");
    printf("Total de equipamentos: %d\n", sistema.totalEquipamentos);
    printf("Equipamentos que responderam: %d\n", respostas);
    printf("Equipamentos com falha: %d\n", falhas);
    
    char resumoMsg[200];
    sprintf(resumoMsg, "RESUMO Ping Geral - Total: %d, Respostas: %d, Falhas: %d", 
            sistema.totalEquipamentos, respostas, falhas);
    registarLogMonitorizacaoTexto(resumoMsg);
}

// 9. Outras atividades - Estatisticas e resultados
void mostrarEstatisticas() {
    printf("\n=== ESTATISTICAS E RESULTADOS ===\n");
    printf("------------------------------------\n");
    
    printf("\nTotal de equipamentos registados: %d\n", sistema.totalEquipamentos);
    printf("Total de incidentes: %d\n", sistema.totalIncidentes);
    
    // Mostrar ultimos logs
    printf("\nUltimos registos de monitorizacao:\n");
    FILE *fp = fopen("log_monitorizacao.txt", "r");
    if(fp != NULL) {
        char linha[500];
        int count = 0;
        while(fgets(linha, sizeof(linha), fp) && count < 10) {
            printf("  %s", linha);
            count++;
        }
        fclose(fp);
    } else {
        printf("  Nenhum log encontrado.\n");
    }
    
    printf("\n------------------------------------\n");
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
        printf(" 2. Guardar dados atualizados antes de sair da aplicacao.                             \n");
        printf(" 3. Importar leituras de sensores a partir de ficheiros de texto.                     \n");
        printf(" 4. Guardar os resultados dos comandos de rede em ficheiros de texto.                 \n");
        printf(" 5. Gerar um relatorio de estado da rede.                                             \n");
        printf(" 6. Gerar um relatorio mensal de incidentes.                                          \n");
        printf(" 7. Outras atividades que considere relevantes.                                       \n");
        printf("--------------------------------------------------------------------------------------\n");
        printf(" 0. Voltar ao menu principal                                                          \n");
        printf("======================================================================================\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
        limparBufferLocal();
        
        switch(opcao) {
            case 1:
                printf("\n=== CARREGAR DADOS ===\n");
                carregarTodosDados(s);
                break;
            case 2:
                printf("\n=== GUARDAR DADOS ===\n");
                guardarTodosDados(s);
                break;
            case 3:
                printf("\n=== IMPORTAR SENSORES ===\n");
                importarLeiturasSensores(s);
                break;
            case 4:
                printf("\n=== GUARDAR RESULTADOS DE REDE ===\n");
                guardarResultadosRede(s);
                break;
            case 5:
                printf("\n=== RELATORIO ESTADO DA REDE ===\n");
                gerarRelatorioEstadoRede(s);
                break;
            case 6:
                printf("\n=== RELATORIO MENSAL INCIDENTES ===\n");
                gerarRelatorioMensalIncidentes(s);
                break;
            case 7:
                printf("\n=== OUTRAS ATIVIDADES ===\n");
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

// ================= 1. CARREGAR E GUARDAR DADOS =================

void carregarTodosDados(Sistema *s) {
    carregarEquipamentos(s);
    carregarIncidentes(s);
    printf("✅ Dados carregados com sucesso!\n");
}

void guardarTodosDados(Sistema *s) {
    guardarEquipamentos(s);
    guardarIncidentes(s);
    printf("✅ Dados guardados com sucesso!\n");
}

// Guardar equipamentos em ficheiro binário
void guardarEquipamentos(Sistema *s) {
    FILE *f = fopen("equipamentos.dat", "wb");
    if (f == NULL) {
        printf("Erro ao guardar equipamentos!\n");
        return;
    }
    
    // Guardar total
    fwrite(&s->totalEquipamentos, sizeof(int), 1, f);
    
    // Guardar cada equipamento
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL) {
        fwrite(&atual->dados, sizeof(Equipamento), 1, f);
        atual = atual->proximo;
    }
    
    fclose(f);
    printf("✅ Equipamentos guardados: %d\n", s->totalEquipamentos);
}

// Carregar equipamentos de ficheiro binário
void carregarEquipamentos(Sistema *s) {
    FILE *f = fopen("equipamentos.dat", "rb");
    if (f == NULL) {
        printf("Ficheiro equipamentos.dat nao encontrado. Iniciando vazio.\n");
        return;
    }
    
    // Limpar lista atual
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL) {
        NodeEquipamento *temp = atual;
        atual = atual->proximo;
        free(temp);
    }
    s->equipamentos = NULL;
    s->totalEquipamentos = 0;
    
    // Ler total
    int total;
    if (fread(&total, sizeof(int), 1, f) != 1) {
        fclose(f);
        return;
    }
    
    // Ler equipamentos
    for (int i = 0; i < total; i++) {
        Equipamento temp;
        if (fread(&temp, sizeof(Equipamento), 1, f) != 1) break;
        
        NodeEquipamento *novo = (NodeEquipamento*)malloc(sizeof(NodeEquipamento));
        if (novo == NULL) continue;
        
        novo->dados = temp;
        novo->proximo = s->equipamentos;
        s->equipamentos = novo;
        s->totalEquipamentos++;
        
        if (temp.codigo >= s->proximoCodigoEquip) {
            s->proximoCodigoEquip = temp.codigo + 1;
        }
    }
    
    fclose(f);
    printf("✅ Equipamentos carregados: %d\n", s->totalEquipamentos);
}

// Guardar incidentes
void guardarIncidentes(Sistema *s) {
    FILE *f = fopen("incidentes.dat", "wb");
    if (f == NULL) {
        printf("Erro ao guardar incidentes!\n");
        return;
    }
    
    fwrite(&s->totalIncidentes, sizeof(int), 1, f);
    
    NodeIncidente *atual = s->incidentes;
    while (atual != NULL) {
        fwrite(&atual->dados, sizeof(Incidente), 1, f);
        atual = atual->proximo;
    }
    
    fclose(f);
    printf("✅ Incidentes guardados: %d\n", s->totalIncidentes);
}

// Carregar incidentes
void carregarIncidentes(Sistema *s) {
    FILE *f = fopen("incidentes.dat", "rb");
    if (f == NULL) {
        printf("Ficheiro incidentes.dat nao encontrado. Iniciando vazio.\n");
        return;
    }
    
    // Limpar lista atual
    NodeIncidente *atual = s->incidentes;
    while (atual != NULL) {
        NodeIncidente *temp = atual;
        atual = atual->proximo;
        free(temp);
    }
    s->incidentes = NULL;
    s->totalIncidentes = 0;
    
    int total;
    if (fread(&total, sizeof(int), 1, f) != 1) {
        fclose(f);
        return;
    }
    
    for (int i = 0; i < total; i++) {
        Incidente temp;
        if (fread(&temp, sizeof(Incidente), 1, f) != 1) break;
        
        NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
        if (novo == NULL) continue;
        
        novo->dados = temp;
        novo->proximo = s->incidentes;
        s->incidentes = novo;
        s->totalIncidentes++;
        
        if (temp.codigo >= s->proximoCodigoInc) {
            s->proximoCodigoInc = temp.codigo + 1;
        }
    }
    
    fclose(f);
    printf("✅ Incidentes carregados: %d\n", s->totalIncidentes);
}


// ================= 2. IMPORTAR LEITURAS DE SENSORES =================

void importarLeiturasSensores(Sistema *s) {
    FILE *f = fopen("sensores_rack.txt", "r");
    
    if (f == NULL) {
        printf("[ERRO] Ficheiro 'sensores_rack.txt' nao encontrado!\n");
        printf("Formato esperado: codigo;tipo;valor;unidade;estado\n");
        return;
    }
    
    char linha[200];
    int totalLeituras = 0;
    int incidentesCriados = 0;
    
    printf("\n--- LEITURAS DE SENSORES ---\n");
    
    while (fgets(linha, sizeof(linha), f)) {
        // Remover newline
        linha[strcspn(linha, "\n")] = '\0';
        
        // Parse: codigo;tipo;valor;unidade;estado
        int codigo;
        char tipo[50], valorStr[20], unidade[20], estado[30];
        
        int result = sscanf(linha, "%d;%[^;];%[^;];%[^;];%s", 
                            &codigo, tipo, valorStr, unidade, estado);
        
        if (result != 5) {
            printf("Erro ao processar: %s\n", linha);
            continue;
        }
        
        float valor = atof(valorStr);
        
        printf("Sensor %d | %s | %.2f %s | Estado: %s\n", 
               codigo, tipo, valor, unidade, estado);
        totalLeituras++;
        
        // Registar no log de sensores
        FILE *log = fopen("log_sensores.txt", "a");
        if (log != NULL) {
            time_t t = time(NULL);
            fprintf(log, "[%s] Sensor %d - %s - %.2f %s - %s\n", 
                    ctime(&t), codigo, tipo, valor, unidade, estado);
            fclose(log);
        }
        
        // Verificar estado anomalo e criar incidente
        if (strcmp(estado, "AVISO") == 0 || 
            strcmp(estado, "CRITICO") == 0 || 
            strcmp(estado, "FALHA_REDE") == 0) {
            
            printf("  ⚠️ Estado anomalo! A criar incidente...\n");
            
            NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
            if (novo != NULL) {
                char descricao[MAX_DESC];
                snprintf(descricao, MAX_DESC, 
                         "Sensor %d (%s) - Leitura: %.2f %s - Estado: %s",
                         codigo, tipo, valor, unidade, estado);
                
                novo->dados.codigo = ++s->proximoCodigoInc;
                novo->dados.codigoEquipamento = codigo;
                strcpy(novo->dados.descricao, descricao);
                novo->dados.estado = PENDENTE;
                novo->dados.prioridade = 1;  // Prioridade ALTA
                obterDataAtual(novo->dados.dataAbertura);
                strcpy(novo->dados.dataFecho, "-");
                
                novo->proximo = s->incidentes;
                s->incidentes = novo;
                s->totalIncidentes++;
                incidentesCriados++;
            }
        }
    }
    
    fclose(f);
    
    printf("\n================================================\n");
    printf("✅ IMPORTACAO CONCLUIDA\n");
    printf("Leituras processadas: %d\n", totalLeituras);
    printf("Incidentes criados: %d\n", incidentesCriados);
    printf("================================================\n");
}


// ================= 3. GUARDAR RESULTADOS DE REDE =================

void guardarResultadosRede(Sistema *s) {
    FILE *f = fopen("log_monitorizacao.txt", "a");
    
    if (f == NULL) {
        printf("Erro ao abrir log_monitorizacao.txt\n");
        return;
    }
    
    time_t t = time(NULL);
    fprintf(f, "\n====================================\n");
    fprintf(f, "Log de rede: %s", ctime(&t));
    fprintf(f, "Total equipamentos: %d\n", s->totalEquipamentos);
    fprintf(f, "Total incidentes: %d\n", s->totalIncidentes);
    fprintf(f, "====================================\n");
    
    fclose(f);
    printf("✅ Resultados de rede guardados em log_monitorizacao.txt\n");
}


// ================= 4. RELATORIO ESTADO DA REDE =================

void gerarRelatorioEstadoRede(Sistema *s) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char nomeFicheiro[100];
    char dataRelatorio[30];
    
    // Corrigido: usar strftime corretamente para a data
    strftime(dataRelatorio, sizeof(dataRelatorio), "%d/%m/%Y %H:%M:%S", tm_info);
    
    // Corrigido: usar sprintf em vez de strftime para o nome do ficheiro
    sprintf(nomeFicheiro, "relatorio_estado_rede_%02d_%02d_%04d.txt",
            tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
    
    FILE *f = fopen(nomeFicheiro, "w");
    if (f == NULL) {
        printf("Erro ao criar relatorio!\n");
        return;
    }
    
    // Contar estatisticas
    NodeEquipamento *eq = s->equipamentos;
    int operacionais = 0, emFalha = 0;
    
    while (eq != NULL) {
        if (eq->dados.estado == OPERACIONAL) operacionais++;
        else if (eq->dados.estado == EM_FALHA) emFalha++;
        eq = eq->proximo;
    }
    
    NodeIncidente *inc = s->incidentes;
    int pendentes = 0;
    while (inc != NULL) {
        if (inc->dados.estado == PENDENTE) pendentes++;
        inc = inc->proximo;
    }
    
    // Escrever relatorio
    fprintf(f, "================================================\n");
    fprintf(f, "        RELATORIO DE ESTADO DA REDE\n");
    fprintf(f, "================================================\n");
    fprintf(f, "Data: %s\n", dataRelatorio);
    fprintf(f, "================================================\n\n");
    
    // Equipamentos operacionais
    fprintf(f, "--- EQUIPAMENTOS OPERACIONAIS ---\n");
    eq = s->equipamentos;
    while (eq != NULL) {
        if (eq->dados.estado == OPERACIONAL) {
            fprintf(f, "  %d | %s | IP: %s\n", 
                    eq->dados.codigo, eq->dados.nome, eq->dados.ip);
        }
        eq = eq->proximo;
    }
    fprintf(f, "Total: %d\n\n", operacionais);
    
    // Equipamentos em falha
    fprintf(f, "--- EQUIPAMENTOS EM FALHA ---\n");
    eq = s->equipamentos;
    while (eq != NULL) {
        if (eq->dados.estado == EM_FALHA) {
            fprintf(f, "  %d | %s | IP: %s | Ultima verificacao: %s\n", 
                    eq->dados.codigo, eq->dados.nome, eq->dados.ip,
                    eq->dados.dataUltimaVerificacao);
        }
        eq = eq->proximo;
    }
    fprintf(f, "Total: %d\n\n", emFalha);
    
    // Incidentes pendentes
    fprintf(f, "--- INCIDENTES PENDENTES ---\n");
    inc = s->incidentes;
    while (inc != NULL) {
        if (inc->dados.estado == PENDENTE) {
            fprintf(f, "  #%d | Equip: %d | Prioridade: %s | Aberto: %s\n",
                    inc->dados.codigo, inc->dados.codigoEquipamento,
                    inc->dados.prioridade == 1 ? "ALTA" : 
                    (inc->dados.prioridade == 2 ? "MEDIA" : "BAIXA"),
                    inc->dados.dataAbertura);
        }
        inc = inc->proximo;
    }
    fprintf(f, "Total: %d\n\n", pendentes);
    
    // Classificacao do estado
    fprintf(f, "--- CLASSIFICACAO DO ESTADO DA REDE ---\n");
    if (emFalha == 0 && pendentes == 0) {
        fprintf(f, "ESTADO: NORMAL\n");
    } else if (emFalha <= 2 && pendentes <= 3) {
        fprintf(f, "ESTADO: ATENCAO\n");
    } else {
        fprintf(f, "ESTADO: CRITICO\n");
    }
    
    fprintf(f, "\n================================================\n");
    fclose(f);
    
    printf("Relatorio gerado: %s\n", nomeFicheiro);
    printf("Equipamentos operacionais: %d\n", operacionais);
    printf("Equipamentos em falha: %d\n", emFalha);
    printf("Incidentes pendentes: %d\n", pendentes);
}


// ================= 5. RELATORIO MENSAL DE INCIDENTES =================

void gerarRelatorioMensalIncidentes(Sistema *s) {
    int mes, ano;
    
    printf("Mes (1-12): ");
    scanf("%d", &mes);
    printf("Ano: ");
    scanf("%d", &ano);
    limparBufferLocal();
    
    char nomeFicheiro[100];
    snprintf(nomeFicheiro, sizeof(nomeFicheiro), "relatorio_incidentes_%02d_%04d.txt", mes, ano);
    
    FILE *f = fopen(nomeFicheiro, "w");
    if (f == NULL) {
        printf("Erro ao criar relatorio!\n");
        return;
    }
    
    char mesStr[3], anoStr[5];
    snprintf(mesStr, sizeof(mesStr), "%02d", mes);
    snprintf(anoStr, sizeof(anoStr), "%04d", ano);
    
    NodeIncidente *inc = s->incidentes;
    int total = 0, pendentes = 0, resolvidos = 0;
    int prioridadeAlta = 0, prioridadeMedia = 0, prioridadeBaixa = 0;
    int incidentesPing = 0, incidentesSensor = 0, incidentesManual = 0;
    
    while (inc != NULL) {
        // Extrair mes e ano da data (formato dd/mm/aaaa HH:MM:SS)
        char mesInc[3], anoInc[5];
        snprintf(mesInc, sizeof(mesInc), "%c%c", 
                 inc->dados.dataAbertura[3], inc->dados.dataAbertura[4]);
        snprintf(anoInc, sizeof(anoInc), "%c%c%c%c",
                 inc->dados.dataAbertura[6], inc->dados.dataAbertura[7],
                 inc->dados.dataAbertura[8], inc->dados.dataAbertura[9]);
        
        if (strcmp(mesInc, mesStr) == 0 && strcmp(anoInc, anoStr) == 0) {
            total++;
            
            if (inc->dados.estado == PENDENTE) pendentes++;
            else if (inc->dados.estado == RESOLVIDO) resolvidos++;
            
            if (inc->dados.prioridade == 1) prioridadeAlta++;
            else if (inc->dados.prioridade == 2) prioridadeMedia++;
            else prioridadeBaixa++;
            
            // Contar por tipo (baseado na descricao)
            if (strstr(inc->dados.descricao, "ping") != NULL ||
                strstr(inc->dados.descricao, "PING") != NULL) {
                incidentesPing++;
            } else if (strstr(inc->dados.descricao, "sensor") != NULL ||
                       strstr(inc->dados.descricao, "Sensor") != NULL) {
                incidentesSensor++;
            } else {
                incidentesManual++;
            }
        }
        inc = inc->proximo;
    }
    
    // Escrever relatorio
    fprintf(f, "================================================\n");
    fprintf(f, "     RELATORIO MENSAL DE INCIDENTES\n");
    fprintf(f, "            %02d/%04d\n", mes, ano);
    fprintf(f, "================================================\n\n");
    
    fprintf(f, "--- TOTAIS GERAIS ---\n");
    fprintf(f, "Total incidentes: %d\n", total);
    fprintf(f, "  Pendentes: %d\n", pendentes);
    fprintf(f, "  Resolvidos: %d (%.1f%%)\n", resolvidos,
            total > 0 ? (resolvidos * 100.0 / total) : 0);
    fprintf(f, "\n");
    
    fprintf(f, "--- POR PRIORIDADE ---\n");
    fprintf(f, "Prioridade ALTA:   %d (%.1f%%)\n", prioridadeAlta,
            total > 0 ? (prioridadeAlta * 100.0 / total) : 0);
    fprintf(f, "Prioridade MEDIA:  %d (%.1f%%)\n", prioridadeMedia,
            total > 0 ? (prioridadeMedia * 100.0 / total) : 0);
    fprintf(f, "Prioridade BAIXA:  %d (%.1f%%)\n", prioridadeBaixa,
            total > 0 ? (prioridadeBaixa * 100.0 / total) : 0);
    fprintf(f, "\n");
    
    fprintf(f, "--- POR TIPO ---\n");
    fprintf(f, "Falha de ping: %d\n", incidentesPing);
    fprintf(f, "Sensor anomalo: %d\n", incidentesSensor);
    fprintf(f, "Manual: %d\n", incidentesManual);
    
    fprintf(f, "\n================================================\n");
    fclose(f);
    
    printf("✅ Relatorio mensal gerado: %s\n", nomeFicheiro);
    printf("Total incidentes no periodo: %d\n", total);
}


// ================= 6. OUTRAS ATIVIDADES =================

void outrasAtividadesRelatorios(Sistema *s) {
    int opcao;
    
    do {
        printf("\n================================================\n");
        printf("        ATIVIDADES EXTRA - RELATORIOS           \n");
        printf("================================================\n");
        printf("1. Ver resumo do sistema\n");
        printf("2. Exportar equipamentos para CSV\n");
        printf("3. Exportar incidentes para CSV\n");
        printf("0. Voltar\n");
        printf("================================================\n");
        printf("Opcao: ");
        
        if (scanf("%d", &opcao) != 1) {
            printf("Opcao invalida!\n");
            limparBufferLocal();
            continue;
        }
        limparBufferLocal();
        
        switch(opcao) {
            case 1:
                printf("\n--- RESUMO DO SISTEMA ---\n");
                printf("Equipamentos: %d\n", s->totalEquipamentos);
                printf("Incidentes: %d\n", s->totalIncidentes);
                printf("Proximo codigo equipamento: %d\n", s->proximoCodigoEquip);
                printf("Proximo codigo incidente: %d\n", s->proximoCodigoInc);
                break;
            case 2:
                // Exportar equipamentos para CSV
                {
                    FILE *csv = fopen("equipamentos_export.csv", "w");
                    if (csv) {
                        fprintf(csv, "Codigo;Nome;Tipo;Marca;Modelo;IP;MAC;Localizacao;Estado\n");
                        NodeEquipamento *atual = s->equipamentos;
                        while (atual != NULL) {
                            fprintf(csv, "%d;%s;%d;%s;%s;%s;%s;%s;%d\n",
                                    atual->dados.codigo, atual->dados.nome,
                                    atual->dados.tipo, atual->dados.marca,
                                    atual->dados.modelo, atual->dados.ip,
                                    atual->dados.mac, atual->dados.localizacao,
                                    atual->dados.estado);
                            atual = atual->proximo;
                        }
                        fclose(csv);
                        printf("✅ Exportado para equipamentos_export.csv\n");
                    }
                }
                break;
            case 3:
                // Exportar incidentes para CSV
                {
                    FILE *csv = fopen("incidentes_export.csv", "w");
                    if (csv) {
                        fprintf(csv, "Codigo;Equipamento;Descricao;Estado;Prioridade;DataAbertura;DataFecho\n");
                        NodeIncidente *atual = s->incidentes;
                        while (atual != NULL) {
                            fprintf(csv, "%d;%d;%s;%d;%d;%s;%s\n",
                                    atual->dados.codigo, atual->dados.codigoEquipamento,
                                    atual->dados.descricao, atual->dados.estado,
                                    atual->dados.prioridade, atual->dados.dataAbertura,
                                    atual->dados.dataFecho);
                            atual = atual->proximo;
                        }
                        fclose(csv);
                        printf("✅ Exportado para incidentes_export.csv\n");
                    }
                }
                break;
            case 0:
                printf("A voltar...\n");
                break;
            default:
                printf("Opcao invalida!\n");
        }
        
        if (opcao != 0) {
            pausarLocal();
        }
        
    } while(opcao != 0);
}

void carregarFicheiro(Sistema *s) {
    carregarTodosDados(s);
}

void guardarFicheiro(const Sistema *s) {
    guardarTodosDados((Sistema*)s);  // cast para remover const
}
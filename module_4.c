#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "noc.h"

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

// ================= MENU INCIDENTES =================

void menuIncidente(Sistema *s) {

    int opcao;
    char entidadeId[50];
    int prioridade;

    do {
        limparEcraLocal();

        printf("===============================================================================\n");
        printf("        MODULO 4 - INCIDENTES TECNICOS                                         \n");
        printf("===============================================================================\n");

        printf(" 1. Criar um novo incidente manualmente.\n");
        printf(" 2. Criar incidente automaticamente (falha ping).\n");
        printf(" 3. Criar incidente automaticamente (sensor anomalia).\n");
        printf(" 4. Colocar incidentes em fila de atendimento.\n");
        printf(" 5. Processar proximo incidente (EM CURSO).\n");
        printf(" 6. Concluir incidente (data/hora).\n");
        printf(" 7. Listar incidentes pendentes.\n");
        printf(" 8. Listar incidentes em curso.\n");
        printf(" 9. Listar incidentes concluidos.\n");
        printf("10. Listar por equipamento/sensor.\n");
        printf("11. Listar por prioridade.\n");
        printf("12. Guardar/carregar ficheiro binario.\n");
        printf("13. Outras atividades.\n");
        printf("-------------------------------------------------------------------------------\n");
        printf(" 0. Voltar\n");
        printf("===============================================================================\n");

        printf("Opcao: ");
        scanf("%d", &opcao);
        limparBufferLocal();

        switch(opcao) {
            case 1: 
                criarIncidente(s); 
                break;
            case 2: 
                criarIncidenteAutoPorFalhaNoPing(s); 
                break;
            case 3: 
                criarIncidenteAutoPorLeituraAnomalaDoSensor(s); 
                break;
            case 4: 
                gerirIncidentesPendentesNaFilaDeAtendimento(s); 
                break;
            case 5: 
                processarProximoIncidenteNaFilaDeAtendimento(s); 
                break;
            case 6: 
                resolverEConcluirIncidenteDataHora(s); 
                break;
            case 7: 
                listarIncidentesPendentes(s); 
                break;
            case 8: 
                listarIncidentesEmCurso(s); 
                break;
            case 9:
                listarIncidentesConcluidos(s); 
                break;
            case 10:
                printf("Codigo equipamento/sensor: ");
                fgets(entidadeId, sizeof(entidadeId), stdin);
                entidadeId[strcspn(entidadeId, "\n")] = 0;
                listarIncidentesPorEntidade(s, entidadeId);
                break;

            case 11:
                printf("Prioridade (1=Baixa, 2=Media, 3=Alta): ");
                scanf("%d", &prioridade);
                limparBufferLocal();
                listarIncidentesPorPrioridadeComParam(s, prioridade);
                break;

            case 12:
                guardarCarregarIncidentesFicheiroBinario(s);
                break;

            case 13:
                outrasAtividadesRelevantes(s);   // 👈 FIX: nome correto do header
                break;

            case 0:
                printf("A voltar ao menu principal...\n");
                break;

            default:
                printf("Opcao invalida!\n");
        }

        if (opcao != 0) pausarLocal();

    } while(opcao != 0);
}

// ================= FUNÇÕES =================

void criarIncidente(Sistema *s) {
    
    // Verificar se existem equipamentos
    if (s->equipamentos == NULL) {
        printf("\n[ERRO] Nao existem equipamentos registados!\n");
        printf("Por favor, registe equipamentos no Modulo 1 primeiro.\n");
        pausar();
        return;
    }
    
    // Listar equipamentos disponíveis
    printf("\n=========== EQUIPAMENTOS DISPONIVEIS ===========\n");
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL) {
        printf("  Codigo: %d | Nome: %s | Estado: %s\n",
               atual->dados.codigo,
               atual->dados.nome,
               estadoEquipamentoToString(atual->dados.estado));
        atual = atual->proximo;
    }
    printf("================================================\n");
    
    // Pedir código do equipamento
    int codigoEquip = lerInteiro("Insira o codigo do equipamento", 1, 9999);
    
    // Verificar se equipamento existe
    NodeEquipamento *equip = encontrarPorCodigo(s, codigoEquip);
    if (equip == NULL) {
        printf("\n[ERRO] Equipamento com codigo %d nao encontrado!\n", codigoEquip);
        pausar();
        return;
    }
    
    // Pedir descrição
    char descricao[MAX_DESC];
    printf("\nDescricao do incidente: ");
    fgets(descricao, MAX_DESC, stdin);
    descricao[strcspn(descricao, "\n")] = '\0';
    
    // Pedir prioridade
    printf("\nPrioridade do incidente:\n");
    printf("1. Alta\n");
    printf("2. Media\n");
    printf("3. Baixa\n");
    int prioridade = lerInteiro("Prioridade", 1, 3);
    
    // Criar incidente
    NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
    if (novo == NULL) {
        printf("[ERRO] Falha ao alocar memoria!\n");
        pausar();
        return;
    }
    
    novo->dados.codigo = ++s->proximoCodigoInc;
    novo->dados.codigoEquipamento = codigoEquip;
    strcpy(novo->dados.descricao, descricao);
    novo->dados.estado = PENDENTE;
    novo->dados.prioridade = prioridade;
    obterDataAtual(novo->dados.dataAbertura);
    strcpy(novo->dados.dataFecho, "-");
    
    // Inserir na lista de incidentes
    novo->proximo = s->incidentes;
    s->incidentes = novo;
    s->totalIncidentes++;
    
    // Adicionar à FILA de atendimento
    adicionarNaFilaAtendimento(s, novo->dados.codigo);
    
    printf("\n================================================\n");
    printf(" INCIDENTE CRIADO COM SUCESSO!\n");
    printf("================================================\n");
    printf("ID do incidente: #%d\n", novo->dados.codigo);
    printf("Equipamento: %s (Codigo: %d)\n", equip->dados.nome, codigoEquip);
    printf("Descricao: %s\n", descricao);
    printf("Prioridade: %s\n", prioridade == 1 ? "ALTA" : (prioridade == 2 ? "MEDIA" : "BAIXA"));
    printf("Estado: PENDENTE\n");
    printf("Data de abertura: %s\n", novo->dados.dataAbertura);
    printf("================================================\n");
    
    // Registar no log
    FILE *log = fopen("log_monitorizacao.txt", "a");
    if (log != NULL) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
        fprintf(log, "[%s] Incidente #%d criado manualmente para equipamento %d - Prioridade: %s\n",
                dataHora, novo->dados.codigo, codigoEquip,
                prioridade == 1 ? "ALTA" : (prioridade == 2 ? "MEDIA" : "BAIXA"));
        fclose(log);
    }
    
    guardarFicheiro(s);
    pausar();
}

void criarIncidenteAutoPorFalhaNoPing(Sistema *s) {
    
    if (s->equipamentos == NULL) {
        printf("\n[ERRO] Nao existem equipamentos registados!\n");
        pausar();
        return;
    }
    
    // Listar equipamentos
    printf("\n=========== EQUIPAMENTOS DISPONIVEIS ===========\n");
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL) {
        printf("  Codigo: %d | Nome: %s | IP: %s\n",
               atual->dados.codigo,
               atual->dados.nome,
               atual->dados.ip);
        atual = atual->proximo;
    }
    printf("================================================\n");
    
    int codigoEquip = lerInteiro("Insira o codigo do equipamento que falhou no ping", 1, 9999);
    
    NodeEquipamento *equip = encontrarPorCodigo(s, codigoEquip);
    if (equip == NULL) {
        printf("[ERRO] Equipamento nao encontrado!\n");
        pausar();
        return;
    }
    
    // Criar incidente automatico
    NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
    if (novo == NULL) {
        printf("[ERRO] Falha ao alocar memoria!\n");
        pausar();
        return;
    }
    
    novo->dados.codigo = ++s->proximoCodigoInc;
    novo->dados.codigoEquipamento = codigoEquip;
    strcpy(novo->dados.descricao, "Falha de comunicacao - Equipamento nao respondeu ao ping");
    novo->dados.estado = PENDENTE;
    novo->dados.prioridade = 1;  // Prioridade ALTA para falhas de ping
    obterDataAtual(novo->dados.dataAbertura);
    strcpy(novo->dados.dataFecho, "-");
    
    novo->proximo = s->incidentes;
    s->incidentes = novo;
    s->totalIncidentes++;
    
    // Adicionar à FILA
    adicionarNaFilaAtendimento(s, novo->dados.codigo);
    
    // Alterar estado do equipamento para EM_FALHA
    equip->dados.estado = EM_FALHA;
    
    printf("\n================================================\n");
    printf(" INCIDENTE AUTOMATICO CRIADO (FALHA NO PING)\n");
    printf("================================================\n");
    printf("ID do incidente: #%d\n", novo->dados.codigo);
    printf("Equipamento: %s (Codigo: %d)\n", equip->dados.nome, codigoEquip);
    printf("Descricao: %s\n", novo->dados.descricao);
    printf("Prioridade: ALTA\n");
    printf("Estado do equipamento alterado para: EM_FALHA\n");
    printf("================================================\n");
    
    // Registar no log
    FILE *log = fopen("log_monitorizacao.txt", "a");
    if (log != NULL) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
        fprintf(log, "[%s] Incidente #%d criado AUTOMATICAMENTE (ping falhou) para equipamento %d\n",
                dataHora, novo->dados.codigo, codigoEquip);
        fclose(log);
    }
    
    guardarFicheiro(s);
    pausar();
}


void criarIncidenteAutoPorLeituraAnomalaDoSensor(Sistema *s) {
    
    int codigoSensor;
    char tipoLeitura[50];
    char valorLeitura[20];
    
    printf("\n=========== INCIDENTE POR SENSOR ===========\n");
    printf("Codigo do sensor: ");
    scanf("%d", &codigoSensor);
    limparBuffer();
    
    printf("Tipo de leitura anomalia (ex: Temperatura, Humidade, UPS): ");
    fgets(tipoLeitura, sizeof(tipoLeitura), stdin);
    tipoLeitura[strcspn(tipoLeitura, "\n")] = '\0';
    
    printf("Valor registado: ");
    fgets(valorLeitura, sizeof(valorLeitura), stdin);
    valorLeitura[strcspn(valorLeitura, "\n")] = '\0';
    
    // Criar descrição
    char descricao[MAX_DESC];
    snprintf(descricao, MAX_DESC, "Leitura anomala de sensor [%s] - Sensor %d - Valor: %s (CRITICO/AVISO)",
             tipoLeitura, codigoSensor, valorLeitura);
    
    // Criar incidente
    NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
    if (novo == NULL) {
        printf("[ERRO] Falha ao alocar memoria!\n");
        pausar();
        return;
    }
    
    novo->dados.codigo = ++s->proximoCodigoInc;
    novo->dados.codigoEquipamento = codigoSensor;
    strcpy(novo->dados.descricao, descricao);
    novo->dados.estado = PENDENTE;
    novo->dados.prioridade = 1;  // Prioridade ALTA para anomalias
    obterDataAtual(novo->dados.dataAbertura);
    strcpy(novo->dados.dataFecho, "-");
    
    novo->proximo = s->incidentes;
    s->incidentes = novo;
    s->totalIncidentes++;
    
    // Adicionar à FILA
    adicionarNaFilaAtendimento(s, novo->dados.codigo);
    
    printf("\n================================================\n");
    printf(" INCIDENTE AUTOMATICO CRIADO (LEITURA SENSOR)\n");
    printf("================================================\n");
    printf("ID do incidente: #%d\n", novo->dados.codigo);
    printf("Sensor: %d\n", codigoSensor);
    printf("Descricao: %s\n", descricao);
    printf("Prioridade: ALTA\n");
    printf("================================================\n");
    
    // Registar no log
    FILE *log = fopen("log_sensores.txt", "a");
    if (log != NULL) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
        fprintf(log, "[%s] INCIDENTE #%d - Sensor %d - %s - Valor: %s\n",
                dataHora, novo->dados.codigo, codigoSensor, tipoLeitura, valorLeitura);
        fclose(log);
    }
    
    guardarFicheiro(s);
    pausar();
}

// ============================================================
// FUNÇÕES AUXILIARES PARA A FILA DE ATENDIMENTO
// ============================================================

void inicializarFilaAtendimento(Sistema *s) {
    s->filaAtendimento = NULL;
    s->filaAtendimentoTras = NULL;
}

void adicionarNaFilaAtendimento(Sistema *s, int codigoIncidente) {
    
    FilaAtendimento *novo = (FilaAtendimento*)malloc(sizeof(FilaAtendimento));
    if (novo == NULL) {
        printf("[ERRO] Falha ao adicionar incidente na fila!\n");
        return;
    }
    
    novo->codigoIncidente = codigoIncidente;
    novo->proximo = NULL;
    
    if (s->filaAtendimento == NULL) {
        // Fila vazia
        s->filaAtendimento = novo;
        s->filaAtendimentoTras = novo;
    } else {
        // Adicionar no fim da fila
        s->filaAtendimentoTras->proximo = novo;
        s->filaAtendimentoTras = novo;
    }
    
    printf("[FILA] Incidente #%d adicionado a fila de atendimento.\n", codigoIncidente);
}

int removerDaFilaAtendimento(Sistema *s) {
    
    if (s->filaAtendimento == NULL) {
        return -1;  // Fila vazia
    }
    
    FilaAtendimento *temp = s->filaAtendimento;
    int codigoIncidente = temp->codigoIncidente;
    
    s->filaAtendimento = s->filaAtendimento->proximo;
    
    if (s->filaAtendimento == NULL) {
        s->filaAtendimentoTras = NULL;
    }
    
    free(temp);
    return codigoIncidente;
}

void listarFilaAtendimento(Sistema *s) {
    
    printf("\n=========== FILA DE ATENDIMENTO (FIFO) ===========\n");
    
    if (s->filaAtendimento == NULL) {
        printf("Nenhum incidente na fila de atendimento.\n");
        return;
    }
    
    FilaAtendimento *atual = s->filaAtendimento;
    int posicao = 1;
    
    while (atual != NULL) {
        // Procurar detalhes do incidente
        NodeIncidente *inc = s->incidentes;
        while (inc != NULL && inc->dados.codigo != atual->codigoIncidente) {
            inc = inc->proximo;
        }
        
        printf("%d. Incidente #%d", posicao, atual->codigoIncidente);
        if (inc != NULL) {
            printf(" - Equip: %d - Prioridade: %s",
                   inc->dados.codigoEquipamento,
                   inc->dados.prioridade == 1 ? "ALTA" : (inc->dados.prioridade == 2 ? "MEDIA" : "BAIXA"));
        }
        printf("\n");
        
        atual = atual->proximo;
        posicao++;
    }
    printf("==================================================\n");
}

void gerirIncidentesPendentesNaFilaDeAtendimento(Sistema *s) {
    
    printf("\n=========== GESTAO DA FILA DE ATENDIMENTO ===========\n");
    
    // Mostrar incidentes pendentes (da lista)
    if (s->incidentes == NULL) {
        printf("Nao existem incidentes registados.\n");
        pausar();
        return;
    }
    
    NodeIncidente *atual = s->incidentes;
    int pendentes = 0;
    
    printf("\n--- INCIDENTES PENDENTES ---\n");
    while (atual != NULL) {
        if (atual->dados.estado == PENDENTE) {
            printf("  #%d | Equip: %d | Prioridade: %s | Aberto: %s\n",
                   atual->dados.codigo,
                   atual->dados.codigoEquipamento,
                   atual->dados.prioridade == 1 ? "ALTA" : 
                   (atual->dados.prioridade == 2 ? "MEDIA" : "BAIXA"),
                   atual->dados.dataAbertura);
            pendentes++;
        }
        atual = atual->proximo;
    }
    
    printf("\nTotal de incidentes pendentes: %d\n", pendentes);
    
    // Mostrar fila de atendimento
    printf("\n--- FILA DE ATENDIMENTO (ORDEM DE CHEGADA) ---\n");
    listarFilaAtendimento(s);
    
    printf("==================================================\n");
    pausar();
}

void processarProximoIncidenteNaFilaDeAtendimento(Sistema *s) {
    
    printf("\n=========== PROCESSAR PROXIMO INCIDENTE ===========\n");
    
    // Verificar se há incidentes na fila
    if (s->filaAtendimento == NULL) {
        printf("Nenhum incidente na fila de atendimento!\n");
        pausar();
        return;
    }
    
    // Remover da frente da FILA (FIFO)
    int codigoInc = removerDaFilaAtendimento(s);
    
    if (codigoInc == -1) {
        printf("Erro ao remover incidente da fila!\n");
        pausar();
        return;
    }
    
    // Procurar o incidente na lista
    NodeIncidente *atual = s->incidentes;
    NodeIncidente *encontrado = NULL;
    
    while (atual != NULL) {
        if (atual->dados.codigo == codigoInc) {
            encontrado = atual;
            break;
        }
        atual = atual->proximo;
    }
    
    if (encontrado == NULL) {
        printf("Incidente #%d nao encontrado!\n", codigoInc);
        pausar();
        return;
    }
    
    // Alterar estado para EM_CURSO
    encontrado->dados.estado = EM_CURSO;
    
    printf("\n================================================\n");
    printf(" INCIDENTE EM PROCESSAMENTO\n");
    printf("================================================\n");
    printf("ID do incidente: #%d\n", encontrado->dados.codigo);
    printf("Equipamento: %d\n", encontrado->dados.codigoEquipamento);
    printf("Descricao: %s\n", encontrado->dados.descricao);
    printf("Prioridade: %s\n", encontrado->dados.prioridade == 1 ? "ALTA" : 
           (encontrado->dados.prioridade == 2 ? "MEDIA" : "BAIXA"));
    printf("NOVO ESTADO: EM CURSO\n");
    printf("================================================\n");
    
    // Registar no log
    FILE *log = fopen("log_monitorizacao.txt", "a");
    if (log != NULL) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
        fprintf(log, "[%s] Incidente #%d iniciado (EM CURSO)\n", dataHora, codigoInc);
        fclose(log);
    }
    
    guardarFicheiro(s);
    pausar();
}

void resolverEConcluirIncidenteDataHora(Sistema *s) {
    
    if (s->incidentes == NULL) {
        printf("Nao existem incidentes registados.\n");
        pausar();
        return;
    }
    
    int codigo = lerInteiro("Insira o codigo do incidente a concluir", 1, 9999);
    
    NodeIncidente *atual = s->incidentes;
    NodeIncidente *encontrado = NULL;
    
    while (atual != NULL) {
        if (atual->dados.codigo == codigo) {
            encontrado = atual;
            break;
        }
        atual = atual->proximo;
    }
    
    if (encontrado == NULL) {
        printf("Incidente #%d nao encontrado!\n", codigo);
        pausar();
        return;
    }
    
    if (encontrado->dados.estado == RESOLVIDO) {
        printf("Este incidente ja foi concluido anteriormente!\n");
        pausar();
        return;
    }
    
    // Concluir incidente
    encontrado->dados.estado = RESOLVIDO;
    obterDataAtual(encontrado->dados.dataFecho);
    
    printf("\n================================================\n");
    printf("✅ INCIDENTE CONCLUIDO\n");
    printf("================================================\n");
    printf("ID do incidente: #%d\n", encontrado->dados.codigo);
    printf("Equipamento: %d\n", encontrado->dados.codigoEquipamento);
    printf("Descricao: %s\n", encontrado->dados.descricao);
    printf("Data abertura: %s\n", encontrado->dados.dataAbertura);
    printf("Data fecho: %s\n", encontrado->dados.dataFecho);
    printf("Estado: RESOLVIDO\n");
    printf("================================================\n");
    
    // Registar no log
    FILE *log = fopen("log_monitorizacao.txt", "a");
    if (log != NULL) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
        fprintf(log, "[%s] Incidente #%d concluido (RESOLVIDO)\n", dataHora, codigo);
        fclose(log);
    }
    
    guardarFicheiro(s);
    pausar();
}

void listarIncidentesPendentes(const Sistema *s) {
    
    printf("\n=========== INCIDENTES PENDENTES ===========\n");
    
    if (s->incidentes == NULL) {
        printf("Nenhum incidente registado.\n");
        pausar();
        return;
    }
    
    NodeIncidente *atual = s->incidentes;
    int count = 0;
    
    while (atual != NULL) {
        if (atual->dados.estado == PENDENTE) {
            printf("\n#%d | Equip: %d | Prioridade: %s | Aberto: %s\n",
                   atual->dados.codigo,
                   atual->dados.codigoEquipamento,
                   atual->dados.prioridade == 1 ? "ALTA" : 
                   (atual->dados.prioridade == 2 ? "MEDIA" : "BAIXA"),
                   atual->dados.dataAbertura);
            printf("   Desc: %s\n", atual->dados.descricao);
            printf("-------------------------------------------\n");
            count++;
        }
        atual = atual->proximo;
    }
    
    if (count == 0) {
        printf("Nenhum incidente pendente.\n");
    } else {
        printf("\nTotal: %d incidente(s) pendente(s)\n", count);
    }
    pausar();
}

void listarIncidentesEmCurso(const Sistema *s) {
    
    printf("\n=========== INCIDENTES EM CURSO ===========\n");
    
    if (s->incidentes == NULL) {
        printf("Nenhum incidente registado.\n");
        pausar();
        return;
    }
    
    NodeIncidente *atual = s->incidentes;
    int count = 0;
    
    while (atual != NULL) {
        if (atual->dados.estado == EM_CURSO) {
            printf("\n#%d | Equip: %d | Prioridade: %s | Iniciado: %s\n",
                   atual->dados.codigo,
                   atual->dados.codigoEquipamento,
                   atual->dados.prioridade == 1 ? "ALTA" : 
                   (atual->dados.prioridade == 2 ? "MEDIA" : "BAIXA"),
                   atual->dados.dataAbertura);
            printf("   Desc: %s\n", atual->dados.descricao);
            printf("-------------------------------------------\n");
            count++;
        }
        atual = atual->proximo;
    }
    
    if (count == 0) {
        printf("Nenhum incidente em curso.\n");
    } else {
        printf("\nTotal: %d incidente(s) em curso\n", count);
    }
    pausar();
}

void listarIncidentesConcluidos(const Sistema *s) {
    
    printf("\n=========== INCIDENTES CONCLUIDOS ===========\n");
    
    if (s->incidentes == NULL) {
        printf("Nenhum incidente registado.\n");
        pausar();
        return;
    }
    
    NodeIncidente *atual = s->incidentes;
    int count = 0;
    
    while (atual != NULL) {
        if (atual->dados.estado == RESOLVIDO) {
            printf("\n#%d | Equip: %d | Aberto: %s | Fechado: %s\n",
                   atual->dados.codigo,
                   atual->dados.codigoEquipamento,
                   atual->dados.dataAbertura,
                   atual->dados.dataFecho);
            printf("   Desc: %s\n", atual->dados.descricao);
            printf("-------------------------------------------\n");
            count++;
        }
        atual = atual->proximo;
    }
    
    if (count == 0) {
        printf("Nenhum incidente concluido.\n");
    } else {
        printf("\nTotal: %d incidente(s) concluido(s)\n", count);
    }
    pausar();
}

void listarIncidentesPorEntidade(Sistema *s, char *entidadeId) {
    
    int codigo = atoi(entidadeId);
    
    printf("\n=========== INCIDENTES DO EQUIPAMENTO %d ===========\n", codigo);
    
    if (s->incidentes == NULL) {
        printf("Nenhum incidente registado.\n");
        pausar();
        return;
    }
    
    NodeIncidente *atual = s->incidentes;
    int count = 0;
    
    while (atual != NULL) {
        if (atual->dados.codigoEquipamento == codigo) {
            printf("\n#%d | Estado: ", atual->dados.codigo);
            switch(atual->dados.estado) {
                case PENDENTE: printf("PENDENTE"); break;
                case EM_CURSO: printf("EM CURSO"); break;
                case RESOLVIDO: printf("RESOLVIDO"); break;
                default: printf("DESCONHECIDO");
            }
            printf(" | Prioridade: %s\n", 
                   atual->dados.prioridade == 1 ? "ALTA" : 
                   (atual->dados.prioridade == 2 ? "MEDIA" : "BAIXA"));
            printf("   Desc: %s\n", atual->dados.descricao);
            printf("   Aberto: %s\n", atual->dados.dataAbertura);
            if (atual->dados.estado == RESOLVIDO) {
                printf("   Fechado: %s\n", atual->dados.dataFecho);
            }
            printf("-------------------------------------------\n");
            count++;
        }
        atual = atual->proximo;
    }
    
    if (count == 0) {
        printf("Nenhum incidente encontrado para o equipamento %d.\n", codigo);
    } else {
        printf("\nTotal: %d incidente(s)\n", count);
    }
    pausar();
}

void listarIncidentesPorPrioridadeComParam(Sistema *s, int prioridade) {
    
    const char *nomePrioridade;
    switch(prioridade) {
        case 1: nomePrioridade = "ALTA"; break;
        case 2: nomePrioridade = "MEDIA"; break;
        case 3: nomePrioridade = "BAIXA"; break;
        default: nomePrioridade = "DESCONHECIDA";
    }
    
    printf("\n=========== INCIDENTES COM PRIORIDADE %s ===========\n", nomePrioridade);
    
    if (s->incidentes == NULL) {
        printf("Nenhum incidente registado.\n");
        pausar();
        return;
    }
    
    NodeIncidente *atual = s->incidentes;
    int count = 0;
    
    while (atual != NULL) {
        if (atual->dados.prioridade == prioridade) {
            printf("\n#%d | Equip: %d | Estado: ", 
                   atual->dados.codigo, atual->dados.codigoEquipamento);
            switch(atual->dados.estado) {
                case PENDENTE: printf("PENDENTE"); break;
                case EM_CURSO: printf("EM CURSO"); break;
                case RESOLVIDO: printf("RESOLVIDO"); break;
            }
            printf("\n   Desc: %s\n", atual->dados.descricao);
            printf("   Aberto: %s\n", atual->dados.dataAbertura);
            printf("-------------------------------------------\n");
            count++;
        }
        atual = atual->proximo;
    }
    
    if (count == 0) {
        printf("Nenhum incidente com prioridade %s.\n", nomePrioridade);
    } else {
        printf("\nTotal: %d incidente(s)\n", count);
    }
    pausar();
}

void guardarCarregarIncidentesFicheiroBinario(Sistema *s) {
    
    int opcao = lerInteiro("\n1. Guardar incidentes\n2. Carregar incidentes\nOpcao", 1, 2);
    
    if (opcao == 1) {
        // GUARDAR
        FILE *f = fopen("incidentes.dat", "wb");
        if (f == NULL) {
            printf("[ERRO] Nao foi possivel abrir o ficheiro para guardar!\n");
            pausar();
            return;
        }
        
        // Guardar total
        fwrite(&s->totalIncidentes, sizeof(int), 1, f);
        
        // Percorrer lista e guardar cada incidente
        NodeIncidente *atual = s->incidentes;
        while (atual != NULL) {
            fwrite(&atual->dados, sizeof(Incidente), 1, f);
            atual = atual->proximo;
        }
        
        fclose(f);
        printf("✅ Incidentes guardados com sucesso em 'incidentes.dat'\n");
        
    } else if (opcao == 2) {
        // CARREGAR
        FILE *f = fopen("incidentes.dat", "rb");
        if (f == NULL) {
            printf("[ERRO] Nao foi possivel abrir o ficheiro para carregar!\n");
            pausar();
            return;
        }
        
        // Limpar lista atual de incidentes
        NodeIncidente *atual = s->incidentes;
        while (atual != NULL) {
            NodeIncidente *temp = atual;
            atual = atual->proximo;
            free(temp);
        }
        s->incidentes = NULL;
        s->totalIncidentes = 0;
        
        // Ler total
        int total;
        if (fread(&total, sizeof(int), 1, f) != 1) {
            printf("Ficheiro vazio ou corrompido.\n");
            fclose(f);
            pausar();
            return;
        }
        
        // Ler incidentes (serão carregados pela ordem inversa)
        for (int i = 0; i < total; i++) {
            Incidente temp;
            if (fread(&temp, sizeof(Incidente), 1, f) != 1) {
                break;
            }
            
            NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
            if (novo == NULL) {
                printf("[ERRO] Falha de memoria ao carregar!\n");
                fclose(f);
                pausar();
                return;
            }
            
            novo->dados = temp;
            novo->proximo = s->incidentes;
            s->incidentes = novo;
            s->totalIncidentes++;
            
            // Atualizar próximo código
            if (temp.codigo >= s->proximoCodigoInc) {
                s->proximoCodigoInc = temp.codigo + 1;
            }
        }
        
        fclose(f);
        printf(" Incidentes carregados com sucesso! Total: %d\n", s->totalIncidentes);
    }
    
    pausar();
}

void outrasAtividadesRelevantes(Sistema *s) {
    
    int opcao;
    
    do {
        printf("\n================================================\n");
        printf("        ATIVIDADES EXTRA - INCIDENTES           \n");
        printf("================================================\n");
        printf("1. Estatisticas gerais de incidentes\n");
        printf("2. Limpar incidentes resolvidos antigos\n");
        printf("3. Mostrar resumo da fila de atendimento\n");
        printf("4. Reordenar fila por prioridade\n");
        printf("0. Voltar\n");
        printf("================================================\n");
        printf("Opcao: ");
        
        if (scanf("%d", &opcao) != 1) {
            printf("Opcao invalida!\n");
            limparBuffer();
            continue;
        }
        limparBuffer();
        
        switch(opcao) {
            case 1: {
                // Estatisticas
                int pendentes = 0, emCurso = 0, resolvidos = 0;
                NodeIncidente *atual = s->incidentes;
                while (atual != NULL) {
                    switch(atual->dados.estado) {
                        case PENDENTE: pendentes++; break;
                        case EM_CURSO: emCurso++; break;
                        case RESOLVIDO: resolvidos++; break;
                    }
                    atual = atual->proximo;
                }
                
                printf("\n--- ESTATISTICAS DE INCIDENTES ---\n");
                printf("Total: %d\n", s->totalIncidentes);
                printf("Pendentes: %d\n", pendentes);
                printf("Em Curso: %d\n", emCurso);
                printf("Resolvidos: %d\n", resolvidos);
                if (s->totalIncidentes > 0) {
                    printf("Taxa de resolucao: %.1f%%\n", 
                           (resolvidos * 100.0) / s->totalIncidentes);
                }
                break;
            }
            case 2: {
                // Limpar resolvidos antigos
                NodeIncidente *atual = s->incidentes;
                NodeIncidente *anterior = NULL;
                int removidos = 0;
                
                while (atual != NULL) {
                    if (atual->dados.estado == RESOLVIDO) {
                        NodeIncidente *temp = atual;
                        if (anterior == NULL) {
                            s->incidentes = atual->proximo;
                            atual = s->incidentes;
                        } else {
                            anterior->proximo = atual->proximo;
                            atual = atual->proximo;
                        }
                        free(temp);
                        s->totalIncidentes--;
                        removidos++;
                    } else {
                        anterior = atual;
                        atual = atual->proximo;
                    }
                }
                printf("Removidos %d incidentes resolvidos.\n", removidos);
                guardarFicheiro(s);
                break;
            }
            case 3: {
                // Resumo da fila
                listarFilaAtendimento(s);
                break;
            }
            case 4: {
                printf("Funcao de reordenacao por prioridade (extra).\n");
                break;
            }
            case 0:
                printf("A voltar...\n");
                break;
            default:
                printf("Opcao invalida!\n");
        }
        
        if (opcao != 0 && opcao != 3) {
            pausar();
        }
        
    } while(opcao != 0);
}
// Modulo 4: Incidentes Tecnicos
// Created by Guilherme Fernandes on 24/05/26.
//

#include "noc.h"

/*
 * UTIL - Funções auxiliares específicas do módulo 4
 */

// Função auxiliar para obter string da prioridade
static const char *prioridadeToString(int prioridade)
{
    switch (prioridade)
    {
        case 1: return "ALTA";
        case 2: return "MEDIA";
        case 3: return "BAIXA";
        default: return "DESCONHECIDA";
    }
}

// Função auxiliar para obter string do estado do incidente
static const char *estadoIncidenteParaString(EstadoIncidente estado)
{
    switch (estado)
    {
        case PENDENTE:  return "PENDENTE";
        case EM_CURSO:  return "EM CURSO";
        case CONCLUIDO: return "CONCLUIDO";
        default:        return "DESCONHECIDO";
    }
}

// Função auxiliar para listar equipamentos resumido
static void listarEquipamentosResumidoInc(const Sistema *s)
{
    printf("\n  %-6s %-20s %-16s\n", "Cod.", "Nome", "Estado");
    printf("  ");
    for (int i = 0; i < 50; i++) printf("-");
    printf("\n");
    
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL)
    {
        printf("  %-6d %-20s %-16s\n",
               atual->dados.codigo,
               atual->dados.nome,
               estadoEquipamentoToString(atual->dados.estado));
        atual = atual->proximo;
    }
}

// Função auxiliar para registar no log de incidentes
static void registarLogIncidente(const char *mensagem)
{
    FILE *log = fopen("log_monitorizacao.txt", "a");
    if (log != NULL)
    {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
        fprintf(log, "[%s] %s\n", dataHora, mensagem);
        fclose(log);
    }
}

// Função auxiliar para registar no log de sensores
static void registarLogSensor(const char *mensagem)
{
    FILE *log = fopen("log_sensores.txt", "a");
    if (log != NULL)
    {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
        fprintf(log, "[%s] %s\n", dataHora, mensagem);
        fclose(log);
    }
}

// Função auxiliar para encontrar incidente por código
static NodeIncidente *encontrarIncidentePorCodigo(Sistema *s, int codigo)
{
    NodeIncidente *atual = s->incidentes;
    while (atual != NULL)
    {
        if (atual->dados.codigo == codigo)
            return atual;
        atual = atual->proximo;
    }
    return NULL;
}

// Função auxiliar para contar incidentes por estado
static void contarIncidentesPorEstado(Sistema *s, int *pendentes, int *emCurso, int *resolvidos)
{
    *pendentes = 0;
    *emCurso = 0;
    *resolvidos = 0;
    
    NodeIncidente *atual = s->incidentes;
    while (atual != NULL)
    {
        switch (atual->dados.estado)
        {
            case PENDENTE:  (*pendentes)++; break;
            case EM_CURSO:  (*emCurso)++;   break;
            case CONCLUIDO: (*resolvidos)++; break;
            default: break;
        }
        atual = atual->proximo;
    }
}

/*
 * Operações da Fila de Atendimento (usando FilaIncidentes do noc.h)
 */

void inicializarFilaAtendimento(Sistema *s)
{
    s->filaAtendimento.inicio = NULL;
    s->filaAtendimento.fim = NULL;
    s->filaAtendimento.total = 0;
}

void adicionarNaFilaAtendimento(Sistema *s, int codigoIncidente)
{
    // Encontrar o incidente correspondente
    NodeIncidente *inc = s->incidentes;
    while (inc != NULL && inc->dados.codigo != codigoIncidente)
    {
        inc = inc->proximo;
    }
    
    if (inc == NULL)
    {
        printf("\n  [!] Incidente #%d nao encontrado!\n", codigoIncidente);
        return;
    }
    
    // Adicionar à fila (o nodo já existe, apenas referenciamos)
    if (s->filaAtendimento.inicio == NULL)
    {
        s->filaAtendimento.inicio = inc;
        s->filaAtendimento.fim = inc;
    }
    else
    {
        s->filaAtendimento.fim->proximo = inc;
        s->filaAtendimento.fim = inc;
    }
    
    s->filaAtendimento.total++;
    
    printf("\n  [FILA] Incidente #%d adicionado à fila de atendimento.\n", codigoIncidente);
}

int removerDaFilaAtendimento(Sistema *s)
{
    if (s->filaAtendimento.inicio == NULL)
        return -1;
    
    NodeIncidente *temp = s->filaAtendimento.inicio;
    int codigoIncidente = temp->dados.codigo;
    
    s->filaAtendimento.inicio = s->filaAtendimento.inicio->proximo;
    
    if (s->filaAtendimento.inicio == NULL)
        s->filaAtendimento.fim = NULL;
    
    s->filaAtendimento.total--;
    return codigoIncidente;
}

void listarFilaAtendimento(Sistema *s)
{
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║                 FILA DE ATENDIMENTO (FIFO)                ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    
    if (s->filaAtendimento.inicio == NULL)
    {
        printf("\n  [!] Nenhum incidente na fila de atendimento.\n");
        return;
    }
    
    NodeIncidente *atual = s->filaAtendimento.inicio;
    int posicao = 1;
    
    while (atual != NULL)
    {
        printf("  %d. Incidente #%d - Equip: %d - Prioridade: %s - Estado: %s\n",
               posicao,
               atual->dados.codigo,
               atual->dados.codigoEquipamento,
               prioridadeToString(atual->dados.prioridade),
               estadoIncidenteParaString(atual->dados.estado));
        
        atual = atual->proximo;
        posicao++;
    }
}

/*
 * Operações Lógicas do Módulo 4
 */

// 1. Criar um novo incidente manualmente
void criarIncidente(Sistema *s)
{
    limparEcra();
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║              CRIAR INCIDENTE MANUALMENTE                  ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    
    // Verificar se existem equipamentos
    if (s->equipamentos == NULL)
    {
        printf("\n  [!] Nao existem equipamentos registados!\n");
        printf("  [!] Registar equipamentos no Modulo 1 primeiro.\n");
        pausar();
        return;
    }
    
    // Listar equipamentos disponíveis
    printf("\n  Equipamentos disponiveis:\n");
    listarEquipamentosResumidoInc(s);
    
    // Pedir código do equipamento
    int codigoEquip = lerInteiro("\n  Codigo do equipamento", 1, 9999);
    
    // Verificar se equipamento existe
    NodeEquipamento *equip = encontrarPorCodigo(s, codigoEquip);
    if (equip == NULL)
    {
        printf("\n  [!] Equipamento com codigo %d nao encontrado!\n", codigoEquip);
        pausar();
        return;
    }
    
    // Pedir descrição
    char descricao[MAX_DESC];
    lerString("Descricao do incidente", descricao, MAX_DESC);
    
    // Pedir prioridade
    printf("\n  Prioridade do incidente:\n");
    printf("    1. Alta\n");
    printf("    2. Media\n");
    printf("    3. Baixa\n");
    int prioridade = lerInteiro("  Prioridade", 1, 3);
    
    // Criar incidente
    NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
    if (novo == NULL)
    {
        printf("\n  [!] Falha ao alocar memoria!\n");
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
    novo->proximo = NULL;
    
    // Inserir na lista de incidentes (no início)
    novo->proximo = s->incidentes;
    s->incidentes = novo;
    s->totalIncidentes++;
    
    // Adicionar à fila de atendimento
    adicionarNaFilaAtendimento(s, novo->dados.codigo);
    
    printf("\n  ──────────────────────────────────────────────────────────\n");
    printf("  [OK] INCIDENTE CRIADO COM SUCESSO!\n");
    printf("  ──────────────────────────────────────────────────────────\n");
    printf("  ID do incidente: #%d\n", novo->dados.codigo);
    printf("  Equipamento: %s (Codigo: %d)\n", equip->dados.nome, codigoEquip);
    printf("  Descricao: %s\n", descricao);
    printf("  Prioridade: %s\n", prioridadeToString(prioridade));
    printf("  Estado: PENDENTE\n");
    printf("  Data de abertura: %s\n", novo->dados.dataAbertura);
    printf("  ──────────────────────────────────────────────────────────\n");
    
    // Registar no log
    char logMsg[300];
    snprintf(logMsg, sizeof(logMsg), "Incidente #%d criado manualmente para equipamento %d - Prioridade: %s",
             novo->dados.codigo, codigoEquip, prioridadeToString(prioridade));
    registarLogIncidente(logMsg);
    
    guardarFicheiro(s);
    pausar();
}

// 2. Criar incidente automaticamente (falha ping)
void criarIncidenteAutoPorFalhaNoPing(Sistema *s)
{
    limparEcra();
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║        CRIAR INCIDENTE AUTOMATICO (FALHA PING)            ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    
    if (s->equipamentos == NULL)
    {
        printf("\n  [!] Nao existem equipamentos registados!\n");
        pausar();
        return;
    }
    
    // Listar equipamentos
    printf("\n  Equipamentos disponiveis:\n");
    listarEquipamentosResumidoInc(s);
    
    int codigoEquip = lerInteiro("\n  Codigo do equipamento que falhou no ping", 1, 9999);
    
    NodeEquipamento *equip = encontrarPorCodigo(s, codigoEquip);
    if (equip == NULL)
    {
        printf("\n  [!] Equipamento nao encontrado!\n");
        pausar();
        return;
    }
    
    // Criar incidente automático
    NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
    if (novo == NULL)
    {
        printf("\n  [!] Falha ao alocar memoria!\n");
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
    novo->proximo = NULL;
    
    novo->proximo = s->incidentes;
    s->incidentes = novo;
    s->totalIncidentes++;
    
    // Adicionar à fila
    adicionarNaFilaAtendimento(s, novo->dados.codigo);
    
    // Alterar estado do equipamento para EM_FALHA
    equip->dados.estado = EM_FALHA;
    
    printf("\n  ──────────────────────────────────────────────────────────\n");
    printf("  [AVISO] INCIDENTE AUTOMATICO CRIADO (FALHA NO PING)\n");
    printf("  ──────────────────────────────────────────────────────────\n");
    printf("  ID do incidente: #%d\n", novo->dados.codigo);
    printf("  Equipamento: %s (Codigo: %d)\n", equip->dados.nome, codigoEquip);
    printf("  Descricao: %s\n", novo->dados.descricao);
    printf("  Prioridade: ALTA\n");
    printf("  Estado do equipamento alterado para: EM_FALHA\n");
    printf("  ──────────────────────────────────────────────────────────\n");
    
    // Registar no log
    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Incidente #%d criado AUTOMATICAMENTE (ping falhou) para equipamento %d",
             novo->dados.codigo, codigoEquip);
    registarLogIncidente(logMsg);
    
    guardarFicheiro(s);
    pausar();
}

// 3. Criar incidente automaticamente (sensor anomalia)
void criarIncidenteAutoPorLeituraAnomalaDoSensor(Sistema *s)
{
    limparEcra();
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║       CRIAR INCIDENTE AUTOMATICO (SENSOR ANOMALIA)        ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    
    char tipoLeitura[50];
    char valorLeitura[20];
    
    int codigoSensor = lerInteiro("Codigo do sensor", 1, 9999);
    lerString("Tipo de leitura anomalia (ex: Temperatura, Humidade, UPS)", tipoLeitura, sizeof(tipoLeitura));
    lerString("Valor registado", valorLeitura, sizeof(valorLeitura));
    
    // Criar descrição
    char descricao[MAX_DESC];
    snprintf(descricao, MAX_DESC, "Leitura anomala de sensor [%s] - Sensor %d - Valor: %s (CRITICO/AVISO)",
             tipoLeitura, codigoSensor, valorLeitura);
    
    // Criar incidente
    NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
    if (novo == NULL)
    {
        printf("\n  [!] Falha ao alocar memoria!\n");
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
    novo->proximo = NULL;
    
    novo->proximo = s->incidentes;
    s->incidentes = novo;
    s->totalIncidentes++;
    
    // Adicionar à fila
    adicionarNaFilaAtendimento(s, novo->dados.codigo);
    
    printf("\n  ──────────────────────────────────────────────────────────\n");
    printf("  [AVISO] INCIDENTE AUTOMATICO CRIADO (LEITURA SENSOR)\n");
    printf("  ──────────────────────────────────────────────────────────\n");
    printf("  ID do incidente: #%d\n", novo->dados.codigo);
    printf("  Sensor: %d\n", codigoSensor);
    printf("  Descricao: %s\n", descricao);
    printf("  Prioridade: ALTA\n");
    printf("  ──────────────────────────────────────────────────────────\n");
    
    // Registar no log de sensores
    char logMsg[300];
    snprintf(logMsg, sizeof(logMsg), "INCIDENTE #%d - Sensor %d - %s - Valor: %s",
             novo->dados.codigo, codigoSensor, tipoLeitura, valorLeitura);
    registarLogSensor(logMsg);
    
    guardarFicheiro(s);
    pausar();
}

// 4. Colocar incidentes em fila de atendimento
void gerirIncidentesPendentesNaFilaDeAtendimento(Sistema *s)
{
    limparEcra();
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║              GESTAO DA FILA DE ATENDIMENTO                ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    
    if (s->incidentes == NULL)
    {
        printf("\n  [!] Nao existem incidentes registados.\n");
        pausar();
        return;
    }
    
    // Mostrar incidentes pendentes
    printf("\n  --- INCIDENTES PENDENTES ---\n");
    NodeIncidente *atual = s->incidentes;
    int pendentes = 0;
    
    while (atual != NULL)
    {
        if (atual->dados.estado == PENDENTE)
        {
            printf("  #%d | Equip: %d | Prioridade: %s | Aberto: %s\n",
                   atual->dados.codigo,
                   atual->dados.codigoEquipamento,
                   prioridadeToString(atual->dados.prioridade),
                   atual->dados.dataAbertura);
            pendentes++;
        }
        atual = atual->proximo;
    }
    
    printf("\n  Total de incidentes pendentes: %d\n", pendentes);
    
    // Mostrar fila de atendimento
    printf("\n  --- FILA DE ATENDIMENTO (ORDEM DE CHEGADA) ---\n");
    listarFilaAtendimento(s);
    
    pausar();
}

// 5. Processar próximo incidente (EM CURSO)
void processarProximoIncidenteNaFilaDeAtendimento(Sistema *s)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |              PROCESSAR PROXIMO INCIDENTE                  |\n");
    printf("  =====================================================================\n");
    
    // Verificar se há incidentes na fila
    if (s->filaAtendimento.inicio == NULL)
    {
        printf("\n  [!] Nenhum incidente na fila de atendimento!\n");
        pausar();
        return;
    }
    
    // Remover da frente da fila (FIFO) - retorna o código do incidente
    int codigoIncidente = removerDaFilaAtendimento(s);
    
    if (codigoIncidente == -1)
    {
        printf("\n  [!] Erro ao remover incidente da fila!\n");
        pausar();
        return;
    }
    
    // Procurar o incidente na lista pelo código
    NodeIncidente *incidente = encontrarIncidentePorCodigo(s, codigoIncidente);
    
    if (incidente == NULL)
    {
        printf("\n  [!] Incidente #%d nao encontrado!\n", codigoIncidente);
        pausar();
        return;
    }
    
    // Alterar estado para EM_CURSO
    incidente->dados.estado = EM_CURSO;
    
    printf("\n  ---------------------------------------------------------------------\n");
    printf("  [INFO] INCIDENTE EM PROCESSAMENTO\n");
    printf("  ---------------------------------------------------------------------\n");
    printf("  ID do incidente: #%d\n", incidente->dados.codigo);
    printf("  Equipamento: %d\n", incidente->dados.codigoEquipamento);
    printf("  Descricao: %s\n", incidente->dados.descricao);
    printf("  Prioridade: %s\n", prioridadeToString(incidente->dados.prioridade));
    printf("  NOVO ESTADO: EM CURSO\n");
    printf("  ---------------------------------------------------------------------\n");
    
    // Registar no log
    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Incidente #%d iniciado (EM CURSO)", incidente->dados.codigo);
    registarLogIncidente(logMsg);
    
    guardarFicheiro(s);
    pausar();
}


// 6. Concluir incidente (data/hora)
void resolverEConcluirIncidenteDataHora(Sistema *s)
{
    limparEcra();
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║                 CONCLUIR INCIDENTE                        ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    
    if (s->incidentes == NULL)
    {
        printf("\n  [!] Nao existem incidentes registados.\n");
        pausar();
        return;
    }
    
    int codigo = lerInteiro("Codigo do incidente a concluir", 1, 9999);
    
    NodeIncidente *encontrado = encontrarIncidentePorCodigo(s, codigo);
    
    if (encontrado == NULL)
    {
        printf("\n  [!] Incidente #%d nao encontrado!\n", codigo);
        pausar();
        return;
    }
    
    if (encontrado->dados.estado == CONCLUIDO)
    {
        printf("\n  [!] Este incidente ja foi concluido anteriormente!\n");
        pausar();
        return;
    }
    
    // Concluir incidente
    encontrado->dados.estado = CONCLUIDO;
    obterDataAtual(encontrado->dados.dataFecho);
    
    printf("\n  ──────────────────────────────────────────────────────────\n");
    printf("  [OK] INCIDENTE CONCLUIDO\n");
    printf("  ──────────────────────────────────────────────────────────\n");
    printf("  ID do incidente: #%d\n", encontrado->dados.codigo);
    printf("  Equipamento: %d\n", encontrado->dados.codigoEquipamento);
    printf("  Descricao: %s\n", encontrado->dados.descricao);
    printf("  Data abertura: %s\n", encontrado->dados.dataAbertura);
    printf("  Data fecho: %s\n", encontrado->dados.dataFecho);
    printf("  Estado: CONCLUIDO\n");
    printf("  ──────────────────────────────────────────────────────────\n");
    
    // Registar no log
    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Incidente #%d concluido (CONCLUIDO)", codigo);
    registarLogIncidente(logMsg);
    
    guardarFicheiro(s);
    pausar();
}

// 7. Listar incidentes pendentes
void listarIncidentesPendentes(const Sistema *s)
{
    limparEcra();
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║              INCIDENTES PENDENTES                         ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    
    if (s->incidentes == NULL)
    {
        printf("\n  [!] Nenhum incidente registado.\n");
        pausar();
        return;
    }
    
    NodeIncidente *atual = s->incidentes;
    int count = 0;
    
    while (atual != NULL)
    {
        if (atual->dados.estado == PENDENTE)
        {
            printf("\n  #%d | Equip: %d | Prioridade: %s | Aberto: %s\n",
                   atual->dados.codigo,
                   atual->dados.codigoEquipamento,
                   prioridadeToString(atual->dados.prioridade),
                   atual->dados.dataAbertura);
            printf("  └─ Desc: %s\n", atual->dados.descricao);
            printf("  ──────────────────────────────────────────────────────────\n");
            count++;
        }
        atual = atual->proximo;
    }
    
    if (count == 0)
    {
        printf("\n  [!] Nenhum incidente pendente.\n");
    }
    else
    {
        printf("\n  Total: %d incidente(s) pendente(s)\n", count);
    }
    pausar();
}

// 8. Listar incidentes em curso
void listarIncidentesEmCurso(const Sistema *s)
{
    limparEcra();
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║              INCIDENTES EM CURSO                          ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    
    if (s->incidentes == NULL)
    {
        printf("\n  [!] Nenhum incidente registado.\n");
        pausar();
        return;
    }
    
    NodeIncidente *atual = s->incidentes;
    int count = 0;
    
    while (atual != NULL)
    {
        if (atual->dados.estado == EM_CURSO)
        {
            printf("\n  #%d | Equip: %d | Prioridade: %s | Iniciado: %s\n",
                   atual->dados.codigo,
                   atual->dados.codigoEquipamento,
                   prioridadeToString(atual->dados.prioridade),
                   atual->dados.dataAbertura);
            printf("  └─ Desc: %s\n", atual->dados.descricao);
            printf("  ──────────────────────────────────────────────────────────\n");
            count++;
        }
        atual = atual->proximo;
    }
    
    if (count == 0)
    {
        printf("\n  [!] Nenhum incidente em curso.\n");
    }
    else
    {
        printf("\n  Total: %d incidente(s) em curso\n", count);
    }
    pausar();
}

// 9. Listar incidentes concluídos
void listarIncidentesConcluidos(const Sistema *s)
{
    limparEcra();
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║              INCIDENTES CONCLUIDOS                        ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    
    if (s->incidentes == NULL)
    {
        printf("\n  [!] Nenhum incidente registado.\n");
        pausar();
        return;
    }
    
    NodeIncidente *atual = s->incidentes;
    int count = 0;
    
    while (atual != NULL)
    {
        if (atual->dados.estado == CONCLUIDO)
        {
            printf("\n  #%d | Equip: %d | Aberto: %s | Fechado: %s\n",
                   atual->dados.codigo,
                   atual->dados.codigoEquipamento,
                   atual->dados.dataAbertura,
                   atual->dados.dataFecho);
            printf("  └─ Desc: %s\n", atual->dados.descricao);
            printf("  ──────────────────────────────────────────────────────────\n");
            count++;
        }
        atual = atual->proximo;
    }
    
    if (count == 0)
    {
        printf("\n  [!] Nenhum incidente concluido.\n");
    }
    else
    {
        printf("\n  Total: %d incidente(s) concluido(s)\n", count);
    }
    pausar();
}

// 10. Listar por equipamento/sensor
void listarIncidentesPorEntidade(Sistema *s, char *entidadeId)
{
    limparEcra();
    int codigo = atoi(entidadeId);
    
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║        INCIDENTES DO EQUIPAMENTO/SENSOR %d                ║\n", codigo);
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    
    if (s->incidentes == NULL)
    {
        printf("\n  [!] Nenhum incidente registado.\n");
        pausar();
        return;
    }
    
    NodeIncidente *atual = s->incidentes;
    int count = 0;
    
    while (atual != NULL)
    {
        if (atual->dados.codigoEquipamento == codigo)
        {
            printf("\n  #%d | Estado: %s | Prioridade: %s\n",
                   atual->dados.codigo,
                   estadoIncidenteParaString(atual->dados.estado),
                   prioridadeToString(atual->dados.prioridade));
            printf("  └─ Desc: %s\n", atual->dados.descricao);
            printf("     Aberto: %s\n", atual->dados.dataAbertura);
            if (atual->dados.estado == CONCLUIDO)
            {
                printf("     Fechado: %s\n", atual->dados.dataFecho);
            }
            printf("  ──────────────────────────────────────────────────────────\n");
            count++;
        }
        atual = atual->proximo;
    }
    
    if (count == 0)
    {
        printf("\n  [!] Nenhum incidente encontrado para a entidade %d.\n", codigo);
    }
    else
    {
        printf("\n  Total: %d incidente(s)\n", count);
    }
    pausar();
}

// 11. Listar por prioridade
void listarIncidentesPorPrioridadeComParam(Sistema *s, int prioridade)
{
    limparEcra();
    
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║        INCIDENTES COM PRIORIDADE %s                       ║\n", prioridadeToString(prioridade));
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    
    if (s->incidentes == NULL)
    {
        printf("\n  [!] Nenhum incidente registado.\n");
        pausar();
        return;
    }
    
    NodeIncidente *atual = s->incidentes;
    int count = 0;
    
    while (atual != NULL)
    {
        if (atual->dados.prioridade == prioridade)
        {
            printf("\n  #%d | Equip: %d | Estado: %s\n",
                   atual->dados.codigo,
                   atual->dados.codigoEquipamento,
                   estadoIncidenteParaString(atual->dados.estado));
            printf("  └─ Desc: %s\n", atual->dados.descricao);
            printf("     Aberto: %s\n", atual->dados.dataAbertura);
            printf("  ──────────────────────────────────────────────────────────\n");
            count++;
        }
        atual = atual->proximo;
    }
    
    if (count == 0)
    {
        printf("\n  [!] Nenhum incidente com prioridade %s.\n", prioridadeToString(prioridade));
    }
    else
    {
        printf("\n  Total: %d incidente(s)\n", count);
    }
    pausar();
}

// 12. Guardar/carregar ficheiro binário
void guardarCarregarIncidentesFicheiroBinario(Sistema *s)
{
    limparEcra();
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║         GUARDAR/CARREGAR INCIDENTES (FICHEIRO)            ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    
    int opcao = lerInteiro("\n  1. Guardar incidentes\n  2. Carregar incidentes\n  Opcao", 1, 2);
    
    if (opcao == 1)
    {
        // GUARDAR
        FILE *f = fopen("incidentes.dat", "wb");
        if (f == NULL)
        {
            printf("\n  [!] Nao foi possivel abrir o ficheiro para guardar!\n");
            pausar();
            return;
        }
        
        // Guardar total
        fwrite(&s->totalIncidentes, sizeof(int), 1, f);
        
        // Percorrer lista e guardar cada incidente
        NodeIncidente *atual = s->incidentes;
        while (atual != NULL)
        {
            fwrite(&atual->dados, sizeof(Incidente), 1, f);
            atual = atual->proximo;
        }
        
        fclose(f);
        printf("\n  [OK] Incidentes guardados com sucesso em 'incidentes.dat'\n");
    }
    else if (opcao == 2)
    {
        // CARREGAR
        FILE *f = fopen("incidentes.dat", "rb");
        if (f == NULL)
        {
            printf("\n  [!] Nao foi possivel abrir o ficheiro para carregar!\n");
            pausar();
            return;
        }
        
        // Limpar lista atual de incidentes
        NodeIncidente *atual = s->incidentes;
        while (atual != NULL)
        {
            NodeIncidente *temp = atual;
            atual = atual->proximo;
            free(temp);
        }
        s->incidentes = NULL;
        s->totalIncidentes = 0;
        s->filaAtendimento.inicio = NULL;
        s->filaAtendimento.fim = NULL;
        s->filaAtendimento.total = 0;
        
        // Ler total
        int total;
        if (fread(&total, sizeof(int), 1, f) != 1)
        {
            printf("\n  [!] Ficheiro vazio ou corrompido.\n");
            fclose(f);
            pausar();
            return;
        }
        
        // Ler incidentes
        for (int i = 0; i < total; i++)
        {
            Incidente temp;
            if (fread(&temp, sizeof(Incidente), 1, f) != 1)
            {
                break;
            }
            
            NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
            if (novo == NULL)
            {
                printf("\n  [!] Falha de memoria ao carregar!\n");
                fclose(f);
                pausar();
                return;
            }
            
            novo->dados = temp;
            novo->proximo = s->incidentes;
            s->incidentes = novo;
            s->totalIncidentes++;
            
            // Adicionar à fila se estiver pendente
            if (novo->dados.estado == PENDENTE)
            {
                adicionarNaFilaAtendimento(s, novo->dados.codigo);
            }
            
            // Atualizar próximo código
            if (temp.codigo >= s->proximoCodigoInc)
            {
                s->proximoCodigoInc = temp.codigo + 1;
            }
        }
        
        fclose(f);
        printf("\n  [OK] Incidentes carregados com sucesso! Total: %d\n", s->totalIncidentes);
    }
    
    pausar();
}

// 13. Outras atividades relevantes
void outrasAtividadesRelevantes(Sistema *s)
{
    int opcao;
    
    do
    {
        limparEcra();
        printf("\n  ╔═══════════════════════════════════════════════════════════╗\n");
        printf("  ║              ATIVIDADES EXTRA - INCIDENTES                  ║\n");
        printf("  ╠═══════════════════════════════════════════════════════════╣\n");
        printf("  ║  1. Estatisticas gerais de incidentes                      ║\n");
        printf("  ║  2. Limpar incidentes resolvidos antigos                   ║\n");
        printf("  ║  3. Mostrar resumo da fila de atendimento                  ║\n");
        printf("  ║  4. Reordenar fila por prioridade                          ║\n");
        printf("  ║  0. Voltar                                                ║\n");
        printf("  ╚═══════════════════════════════════════════════════════════╝\n");
        
        opcao = lerInteiro("  Opcao", 0, 4);
        
        switch (opcao)
        {
            case 1:
            {
                int pendentes, emCurso, resolvidos;
                contarIncidentesPorEstado(s, &pendentes, &emCurso, &resolvidos);
                
                printf("\n  ──────────────────────────────────────────────────────────\n");
                printf("  [ESTATISTICAS] ESTATISTICAS DE INCIDENTES\n");
                printf("  ──────────────────────────────────────────────────────────\n");
                printf("  Total: %d\n", s->totalIncidentes);
                printf("  Pendentes: %d\n", pendentes);
                printf("  Em Curso: %d\n", emCurso);
                printf("  Resolvidos: %d\n", resolvidos);
                if (s->totalIncidentes > 0)
                {
                    printf("  Taxa de resolucao: %.1f%%\n", 
                           (resolvidos * 100.0) / s->totalIncidentes);
                }
                printf("  ──────────────────────────────────────────────────────────\n");
                pausar();
                break;
            }
            case 2:
            {
                NodeIncidente *atual = s->incidentes;
                NodeIncidente *anterior = NULL;
                int removidos = 0;
                
                while (atual != NULL)
                {
                    if (atual->dados.estado == CONCLUIDO)
                    {
                        NodeIncidente *temp = atual;
                        if (anterior == NULL)
                        {
                            s->incidentes = atual->proximo;
                            atual = s->incidentes;
                        }
                        else
                        {
                            anterior->proximo = atual->proximo;
                            atual = atual->proximo;
                        }
                        free(temp);
                        s->totalIncidentes--;
                        removidos++;
                    }
                    else
                    {
                        anterior = atual;
                        atual = atual->proximo;
                    }
                }
                
                printf("\n  [OK] Removidos %d incidentes resolvidos.\n", removidos);
                guardarFicheiro(s);
                pausar();
                break;
            }
            case 3:
                listarFilaAtendimento(s);
                pausar();
                break;
            case 4:
                printf("\n  [!] Funcao de reordenacao por prioridade (em desenvolvimento).\n");
                pausar();
                break;
            case 0:
                break;
        }
        
    } while (opcao != 0);
}

/*
 * Menu do Modulo 4
 */

void menuIncidente(Sistema *s)
{
    int opcao;
    
    do
    {
        limparEcra();
        printf("\n  ╔═══════════════════════════════════════════════════════════╗\n");
        printf("  ║              MODULO 4 — INCIDENTES TECNICOS                 ║\n");
        printf("  ╠═══════════════════════════════════════════════════════════╣\n");
        printf("  ║  1. Criar novo incidente manualmente                       ║\n");
        printf("  ║  2. Criar incidente automaticamente (falha ping)          ║\n");
        printf("  ║  3. Criar incidente automaticamente (sensor anomalia)     ║\n");
        printf("  ║  4. Colocar incidentes em fila de atendimento             ║\n");
        printf("  ║  5. Processar proximo incidente (EM CURSO)                ║\n");
        printf("  ║  6. Concluir incidente (data/hora)                        ║\n");
        printf("  ║  7. Listar incidentes pendentes                           ║\n");
        printf("  ║  8. Listar incidentes em curso                            ║\n");
        printf("  ║  9. Listar incidentes concluidos                          ║\n");
        printf("  ║ 10. Listar por equipamento/sensor                         ║\n");
        printf("  ║ 11. Listar por prioridade                                 ║\n");
        printf("  ║ 12. Guardar/carregar ficheiro binario                     ║\n");
        printf("  ║ 13. Outras atividades                                     ║\n");
        printf("  ║  0. Voltar                                                ║\n");
        printf("  ╚═══════════════════════════════════════════════════════════╝\n");
        
        opcao = lerInteiro("  Opcao", 0, 13);
        
        switch (opcao)
        {
            case 1:  criarIncidente(s); break;
            case 2:  criarIncidenteAutoPorFalhaNoPing(s); break;
            case 3:  criarIncidenteAutoPorLeituraAnomalaDoSensor(s); break;
            case 4:  gerirIncidentesPendentesNaFilaDeAtendimento(s); break;
            case 5:  processarProximoIncidenteNaFilaDeAtendimento(s); break;
            case 6:  resolverEConcluirIncidenteDataHora(s); break;
            case 7:  listarIncidentesPendentes(s); break;
            case 8:  listarIncidentesEmCurso(s); break;
            case 9:  listarIncidentesConcluidos(s); break;
            case 10: {
                char entidadeId[50];
                lerString("Codigo do equipamento/sensor", entidadeId, sizeof(entidadeId));
                listarIncidentesPorEntidade(s, entidadeId);
                break;
            }
            case 11: {
                int prioridade = lerInteiro("Prioridade (1=Alta, 2=Media, 3=Baixa)", 1, 3);
                listarIncidentesPorPrioridadeComParam(s, prioridade);
                break;
            }
            case 12: guardarCarregarIncidentesFicheiroBinario(s); break;
            case 13: outrasAtividadesRelevantes(s); break;
            case 0:  break;
        }
        
    } while (opcao != 0);
}
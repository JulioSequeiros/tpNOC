// Modulo 4: Incidentes Tecnicos
// Created by Guilherme Fernandes on 24/05/26.
// Comentários adicionados para estudo

#include "noc.h"  // Inclui o header principal com estruturas e funções globais

/*
 * UTIL - Funções auxiliares específicas do módulo 4
 * Estas funções são static (visíveis apenas neste ficheiro)
 */

// Função auxiliar para converter prioridade numérica em 'string' legível
static const char *prioridadeToString(int prioridade)
{
    switch (prioridade)           // Verifica o valor da prioridade
    {
        case 1: return "ALTA";    // Prioridade 1 = Alta
        case 2: return "MEDIA";   // Prioridade 2 = Média
        case 3: return "BAIXA";   // Prioridade 3 = Baixa
        default: return "DESCONHECIDA";  // Valor inválido
    }
}

// Função auxiliar para converter estado do incidente em 'string'
static const char *estadoIncidenteParaString(EstadoIncidente estado)
{
    switch (estado)               // Verifica o valor do enum
    {
        case PENDENTE:  return "PENDENTE";    // Estado: Pendente
        case EM_CURSO:  return "EM CURSO";    // Estado: Em curso
        case CONCLUIDO: return "CONCLUIDO";   // Estado: Concluído
        default:        return "DESCONHECIDO"; // Valor inválido
    }
}

// Função auxiliar para listar equipamentos de forma resumida (usada ao criar incidentes)
static void listarEquipamentosResumidoInc(const Sistema *s)
{
    // Cabeçalho da tabela formatado com espaçamentos
    printf("\n%-6s %-20s %-20s %-20s %-16s %-18s %-20s %-16s %-20s\n",
           "Tipo", "Codigo", "Nome", "Modelo",
           "IP", "MAC", "Localizacao", "Estado", "Ult.Verificacao");

    // Linha separadora de 160 hífens
    for (int i = 0; i < 160; i++)
        printf("-");
    printf("\n");

    NodeEquipamento *atual = s->equipamentos;  // Ponteiro para início da lista

    while (atual != NULL)  // Percorre toda a lista de equipamentos
    {
        // Imprime cada campo com larguras fixas para alinhamento
        printf("%-6d %-20d %-20s %-20s %-16s %-18s %-20s %-16s %-20s\n",
               atual->dados.tipo,
               atual->dados.codigo,
               atual->dados.nome,
               atual->dados.modelo,
               atual->dados.ip,
               atual->dados.mac,
               atual->dados.localizacao,
               estadoEquipamentoToString(atual->dados.estado),
               atual->dados.dataUltimaVerificacao);

        atual = atual->proximo;  // Avança para o próximo nó
    }
}

// Função auxiliar para registar mensagens no 'log' de monitorização
static void registarLogIncidente(const char *mensagem)
{
    FILE *log = fopen("log_monitorizacao.txt", "a");  // Abre para acrescentar (append)
    if (log != NULL)  // Verifica se o ficheiro abriu corretamente
    {
        time_t t = time(NULL);                       // Obtém timestamp atual
        struct tm *tm_info = localtime(&t);          // Converte para estrutura de tempo
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);  // Formata data/hora
        fprintf(log, "[%s] %s\n", dataHora, mensagem);  // Escreve no 'log'
        fclose(log);  // Fecha o ficheiro
    }
}

// Função auxiliar para registar mensagens no 'log' de sensores (separado)
static void registarLogSensor(const char *mensagem)
{
    FILE *log = fopen("log_sensores.txt", "a");  // Abre ficheiro de 'log' de sensores
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

// Função auxiliar para encontrar um incidente pelo seu código
static NodeIncidente *encontrarIncidentePorCodigo(const Sistema *s, int codigo)
{
    NodeIncidente *atual = s->incidentes;  // Começa do início da lista
    while (atual != NULL)                  // Percorre toda a lista
    {
        if (atual->dados.codigo == codigo) // Se encontrou o código
            return atual;                  // Retorna o ponteiro para o nó
        atual = atual->proximo;            // Avança para o próximo
    }
    return NULL;  // Não encontrou
}

// Função auxiliar para contar incidentes por estado (usando ponteiros como saída)
static void contarIncidentesPorEstado(const Sistema *s, int *pendentes, int *emCurso, int *resolvidos)
{
    *pendentes = 0;    // Inicializa contador de pendentes
    *emCurso = 0;      // Inicializa contador de em curso
    *resolvidos = 0;   // Inicializa contador de concluídos

    NodeIncidente *atual = s->incidentes;
    while (atual != NULL)
    {
        switch (atual->dados.estado)  // Verifica o estado do incidente
        {
            case PENDENTE:  (*pendentes)++; break;   // Incrementa pendentes
            case EM_CURSO:  (*emCurso)++;   break;   // Incrementa em curso
            case CONCLUIDO: (*resolvidos)++; break;  // Incrementa resolvidos
            default: break;
        }
        atual = atual->proximo;
    }
}

/*
 * Operações da Fila de Atendimento (usando FilaIncidentes do noc.h)
 */

// Inicializa a fila de atendimento (vazia)
void inicializarFilaAtendimento(Sistema *s)
{
    s->filaAtendimento.inicio = NULL;  // Primeiro elemento = NULL
    s->filaAtendimento.fim = NULL;     // Último elemento = NULL
    s->filaAtendimento.total = 0;      // Contador = 0
}

// Adiciona um incidente ao final da fila de atendimento (FIFO)
void adicionarNaFilaAtendimento(Sistema *s, int codigoIncidente)
{
    // Encontrar o incidente correspondente pelo código
    NodeIncidente *inc = s->incidentes;
    while (inc != NULL && inc->dados.codigo != codigoIncidente)
    {
        inc = inc->proximo;
    }

    if (inc == NULL)  // Se não encontrou o incidente
    {
        printf("\n  [!] Incidente #%d nao encontrado!\n", codigoIncidente);
        return;
    }

    // Adicionar à fila (usando o mesmo nó, não cria cópia)
    if (s->filaAtendimento.inicio == NULL)  // Se a fila está vazia
    {
        s->filaAtendimento.inicio = inc;    // Início e fim apontam para o mesmo
        s->filaAtendimento.fim = inc;
    }
    else  // Fila não vazia
    {
        s->filaAtendimento.fim->proximo = inc;  // Liga o último ao novo
        s->filaAtendimento.fim = inc;           // Atualiza o fim
    }

    s->filaAtendimento.total++;  // Incrementa contador

    printf("\n  [FILA] Incidente #%d adicionado à fila de atendimento.\n", codigoIncidente);
}

// Remove o incidente da frente da fila (FIFO) e retorna o código
int removerDaFilaAtendimento(Sistema *s)
{
    if (s->filaAtendimento.inicio == NULL)  // Fila vazia
        return -1;  // Retorna '-1' indicando erro

    NodeIncidente *temp = s->filaAtendimento.inicio;  // Guarda o primeiro nó
    int codigoIncidente = temp->dados.codigo;         // Guarda o código

    s->filaAtendimento.inicio = s->filaAtendimento.inicio->proximo;  // Avança o início

    if (s->filaAtendimento.inicio == NULL)  // Se a fila ficou vazia
        s->filaAtendimento.fim = NULL;      // O fim também deve ser NULL

    s->filaAtendimento.total--;  // Decrementa contador
    return codigoIncidente;      // Retorna o código removido
}

// Lista todos os incidentes na fila de atendimento por ordem
void listarFilaAtendimento(const Sistema *s)
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
        // Agora com 8 especificadores para 8 argumentos
        printf("  %d. #%d | Equip:%d | Prior:%s | Estado:%s\n"
               "      Desc: %s\n"
               "      Abertura:%s  Fecho:%s\n",
               posicao,
               atual->dados.codigo,
               atual->dados.codigoEquipamento,
               prioridadeToString(atual->dados.prioridade),
               estadoIncidenteParaString(atual->dados.estado),
               atual->dados.descricao,
               atual->dados.dataAbertura,
               atual->dados.dataFecho);

        atual = atual->proximo;
        posicao++;
    }
}

// Reordenar incidentes por prioridade (ALTA - MEDIA - BAIXA) usando bubble sort
void reordenarIncidentesPorPrioridade(const Sistema *s)
{
    if (s->incidentes == NULL)  // Lista vazia
    {
        printf("\n  [!] Nao existem incidentes registados.\n");
        return;
    }

    int trocou;  // 'Flag' para controlar se houve troca

    do
    {
        trocou = 0;  // Reinicia a "flag"

        NodeIncidente *atual = s->incidentes;  // Começa do início

        while (atual != NULL && atual->proximo != NULL)  // Percorre até o penúltimo
        {
            // Se a prioridade do atual é MAIOR que a do próximo (pior prioridade)
            if (atual->dados.prioridade > atual->proximo->dados.prioridade)
            {
                // Troca os dados entre os dois nós (não os ponteiros)
                Incidente temp = atual->dados;
                atual->dados = atual->proximo->dados;
                atual->proximo->dados = temp;

                trocou = 1;  // Houve troca, precisa continuar
            }

            atual = atual->proximo;  // Avança
        }

    } while (trocou);  // Repete enquanto houve trocas

    printf("\n  [OK] Incidentes reordenados por prioridade.\n");
    printf("       ALTA -> MEDIA -> BAIXA\n");
}

/*
 * Operações Lógicas do Módulo 4
 */

// 1. Criar um incidente manualmente (utilizador fornece os dados)
void criarIncidente(Sistema *s)
{
    limparEcra();  // Limpa o ecrã (função definida em noc.h)
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║              CRIAR INCIDENTE MANUALMENTE                  ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");

    // Verificar se existem equipamentos (não faz sentido criar incidente sem equipamento)
    if (s->equipamentos == NULL)
    {
        printf("\n  [!] Nao existem equipamentos registados!\n");
        printf("  [!] Registar equipamentos no Modulo 1 primeiro.\n");
        pausar();  // Aguarda o utilizador pressionar ENTER
        return;
    }

    // Listar equipamentos disponíveis para o utilizador escolher
    printf("\n  Equipamentos disponiveis:\n");
    listarEquipamentosResumidoInc(s);

    // Pedir código do equipamento (valida entre 1 e 9999)
    int codigoEquip = lerInteiro("\n  Codigo do equipamento", 1, 9999);

    // Verificar se equipamento existe
    NodeEquipamento *equip = encontrarPorCodigo(s, codigoEquip);
    if (equip == NULL)
    {
        printf("\n  [!] Equipamento com codigo %d nao encontrado!\n", codigoEquip);
        pausar();
        return;
    }

    // Pedir descrição do incidente
    char descricao[MAX_DESC];
    lerString("Descricao do incidente", descricao, MAX_DESC);

    // Pedir prioridade (menu de opções)
    printf("\n  Prioridade do incidente:\n");
    printf("    1. Alta\n");
    printf("    2. Media\n");
    printf("    3. Baixa\n");
    int prioridade = lerInteiro("  Prioridade", 1, 3);

    // Criar um novo nó de incidente
    NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
    if (novo == NULL)  // Verifica se a alocação falhou
    {
        printf("\n  [!] Falha ao alocar memoria!\n");
        pausar();
        return;
    }

    // Preencher os dados do novo incidente
    novo->dados.codigo = ++s->proximoCodigoInc;  // Código incremental
    novo->dados.codigoEquipamento = codigoEquip;
    strcpy(novo->dados.descricao, descricao);
    novo->dados.estado = PENDENTE;               // Estado inicial
    novo->dados.prioridade = prioridade;
    obterDataAtual(novo->dados.dataAbertura);    // Data atual
    strcpy(novo->dados.dataFecho, "-");          // Sem data de fecho ainda
    novo->proximo = NULL;

    // Inserir na lista de incidentes (no início - inserção mais rápida)
    novo->proximo = s->incidentes;
    s->incidentes = novo;
    s->totalIncidentes++;

    // Adicionar automaticamente à fila de atendimento
    adicionarNaFilaAtendimento(s, novo->dados.codigo);

    // Mostrar resumo do incidente criado
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

    // Registar no 'log' de monitorização
    char logMsg[300];
    snprintf(logMsg, sizeof(logMsg), "Incidente #%d criado manualmente para equipamento %d - Prioridade: %s",
             novo->dados.codigo, codigoEquip, prioridadeToString(prioridade));
    registarLogIncidente(logMsg);

    guardarFicheiro(s);  // Persiste os dados em ficheiro
    pausar();
}

// 2. Criar incidente automaticamente quando há falha de ping no equipamento
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

    // Listar equipamentos para o utilizador escolher qual falhou
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
    // Descrição pré-definida para falha de ping
    strcpy(novo->dados.descricao, "Falha de comunicacao - Equipamento nao respondeu ao ping");
    novo->dados.estado = PENDENTE;
    novo->dados.prioridade = 1;  // Prioridade ALTA para falhas de ping (mais grave)
    obterDataAtual(novo->dados.dataAbertura);
    strcpy(novo->dados.dataFecho, "-");
    novo->proximo = NULL;

    // Inserir na lista
    novo->proximo = s->incidentes;
    s->incidentes = novo;
    s->totalIncidentes++;

    // Adicionar à fila
    adicionarNaFilaAtendimento(s, novo->dados.codigo);

    // Alterar estado do equipamento para EM_FALHA (consistência do sistema)
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

    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Incidente #%d criado AUTOMATICAMENTE (ping falhou) para equipamento %d",
             novo->dados.codigo, codigoEquip);
    registarLogIncidente(logMsg);

    guardarFicheiro(s);
    pausar();
}

// 3. Criar incidente automaticamente por leitura anómala de sensor
void criarIncidenteAutoPorLeituraAnomalaDoSensor(Sistema *s)
{
    limparEcra();
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║       CRIAR INCIDENTE AUTOMATICO (SENSOR ANOMALIA)        ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");

    char tipoLeitura[50];
    char valorLeitura[20];

    // Dados simulados de leitura anómala
    int codigoSensor = lerInteiro("Codigo do sensor", 1, 9999);
    lerString("Tipo de leitura anomalia (ex: Temperatura, Humidade, UPS)", tipoLeitura, sizeof(tipoLeitura));
    lerString("Valor registado", valorLeitura, sizeof(valorLeitura));

    // Criar descrição dinâmica com os dados fornecidos
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

    adicionarNaFilaAtendimento(s, novo->dados.codigo);

    printf("\n  ──────────────────────────────────────────────────────────\n");
    printf("  [AVISO] INCIDENTE AUTOMATICO CRIADO (LEITURA SENSOR)\n");
    printf("  ──────────────────────────────────────────────────────────\n");
    printf("  ID do incidente: #%d\n", novo->dados.codigo);
    printf("  Sensor: %d\n", codigoSensor);
    printf("  Descricao: %s\n", descricao);
    printf("  Prioridade: ALTA\n");
    printf("  ──────────────────────────────────────────────────────────\n");

    // Registar no log específico de sensores
    char logMsg[300];
    snprintf(logMsg, sizeof(logMsg), "INCIDENTE #%d - Sensor %d - %s - Valor: %s",
             novo->dados.codigo, codigoSensor, tipoLeitura, valorLeitura);
    registarLogSensor(logMsg);

    guardarFicheiro(s);
    pausar();
}

// 4. Mostrar incidentes pendentes e a fila de atendimento (gestão)
void gerirIncidentesPendentesNaFilaDeAtendimento(const Sistema *s)
{
    limparEcra();
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║              GESTÃO DA FILA DE ATENDIMENTO                ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");

    if (s->incidentes == NULL)
    {
        printf("\n  [!] Nao existem incidentes registados.\n");
        pausar();
        return;
    }

    // Mostrar incidentes pendentes (não estão todos necessariamente na fila)
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

// 5. Processar próximo incidente na fila (muda de PENDENTE para EM_CURSO)
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

    // Alterar estado para EM_CURSO (está a ser tratado)
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

    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Incidente #%d iniciado (EM CURSO)", incidente->dados.codigo);
    registarLogIncidente(logMsg);

    guardarFicheiro(s);
    pausar();
}

// 6. Concluir incidente (muda de EM_CURSO para CONCLUIDO e regista data de fecho)
void resolverEConcluirIncidenteDataHora(const Sistema *s)
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

    if (encontrado->dados.estado == CONCLUIDO)  // Já está concluído
    {
        printf("\n  [!] Este incidente ja foi concluido anteriormente!\n");
        pausar();
        return;
    }

    // Concluir incidente
    encontrado->dados.estado = CONCLUIDO;
    obterDataAtual(encontrado->dados.dataFecho);  // Regista data/hora de fecho

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

    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Incidente #%d concluido (CONCLUIDO)", codigo);
    registarLogIncidente(logMsg);

    guardarFicheiro(s);
    pausar();
}

// 7. Listar incidentes pendentes (estado = PENDENTE)
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

// 8. Listar incidentes em curso (estado = EM_CURSO)
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

// 9. Listar incidentes concluídos (estado = CONCLUIDO)
void listarIncidentesConcluidos(const Sistema *s)
{
    limparEcra();
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║              LISTAR INCIDENTES CONCLUÍDOS                 ║\n");
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

// 10. Listar incidentes por equipamento/sensor (filtro por código)
void listarIncidentesPorEntidade(const Sistema *s, char *entidadeId)
{
    limparEcra();
    int codigo = atoi(entidadeId);  // Converte 'string' para inteiro

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
        // Filtra pelo código do equipamento/sensor
        if (atual->dados.codigoEquipamento == codigo)
        {
            printf("\n  #%d | Estado: %s | Prioridade: %s\n",
                   atual->dados.codigo,
                   estadoIncidenteParaString(atual->dados.estado),
                   prioridadeToString(atual->dados.prioridade));
            printf("  └─ Desc: %s\n", atual->dados.descricao);
            printf("     Aberto: %s\n", atual->dados.dataAbertura);
            if (atual->dados.estado == CONCLUIDO)  // Só mostra fecho se concluído
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

// 11. Listar incidentes por prioridade (filtro por 1, 2 ou 3)
void listarIncidentesPorPrioridadeComParam(const Sistema *s, int prioridade)
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
        if (atual->dados.prioridade == prioridade)  // Filtra pela prioridade
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

// 12. Guardar/Carregar incidentes em ficheiro binário (persistência)
void guardarCarregarIncidentesFicheiroBinario(Sistema *s)
{
    limparEcra();
    printf("\n  ═══════════════════════════════════════════════════════════╗\n");
    printf("  ║         GUARDAR/CARREGAR INCIDENTES (FICHEIRO)            ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");

    int opcao = lerInteiro("\n  1. Guardar incidentes\n  2. Carregar incidentes\n  Opcao", 1, 2);

    if (opcao == 1)
    {
        // GUARDAR incidentes em ficheiro binário
        FILE *f = fopen("incidentes.dat", "wb");  // "wb" = write binary
        if (f == NULL)
        {
            printf("\n  [!] Nao foi possivel abrir o ficheiro para guardar!\n");
            pausar();
            return;
        }

        // Guardar o total de incidentes primeiro
        fwrite(&s->totalIncidentes, sizeof(int), 1, f);

        // Percorrer a lista e guardar cada incidente
        NodeIncidente *atual = s->incidentes;
        while (atual != NULL)
        {
            fwrite(&atual->dados, sizeof(Incidente), 1, f);  // Escreve a struct
            atual = atual->proximo;
        }

        fclose(f);
        printf("\n  [OK] Incidentes guardados com sucesso em 'incidentes.dat'\n");
    }
    else if (opcao == 2)
    {
        // CARREGAR incidentes de ficheiro binário
        FILE *f = fopen("incidentes.dat", "rb");  // "rb" = read binary
        if (f == NULL)
        {
            printf("\n  [!] Nao foi possivel abrir o ficheiro para carregar!\n");
            pausar();
            return;
        }

        // Limpar a lista atual de incidentes (libertar memória)
        NodeIncidente *atual = s->incidentes;
        while (atual != NULL)
        {
            NodeIncidente *temp = atual;
            atual = atual->proximo;
            free(temp);  // Liberta cada nó
        }
        s->incidentes = NULL;
        s->totalIncidentes = 0;
        // Reiniciar a fila de atendimento
        s->filaAtendimento.inicio = NULL;
        s->filaAtendimento.fim = NULL;
        s->filaAtendimento.total = 0;

        // Ler o total de incidentes do ficheiro
        int total;
        if (fread(&total, sizeof(int), 1, f) != 1)
        {
            printf("\n  [!] Ficheiro vazio ou corrompido.\n");
            fclose(f);
            pausar();
            return;
        }

        // Ler cada incidente e recriar a lista
        for (int i = 0; i < total; i++)
        {
            Incidente temp;
            if (fread(&temp, sizeof(Incidente), 1, f) != 1)
            {
                break;  // Fim do ficheiro ou erro
            }

            // Criar um novo nó
            NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
            if (novo == NULL)
            {
                printf("\n  [!] Falha de memoria ao carregar!\n");
                fclose(f);
                pausar();
                return;
            }

            novo->dados = temp;        // Copia os dados
            novo->proximo = s->incidentes;  // Insere no início
            s->incidentes = novo;
            s->totalIncidentes++;

            // Se o incidente está pendente, adicionar à fila de atendimento
            if (novo->dados.estado == PENDENTE)
            {
                adicionarNaFilaAtendimento(s, novo->dados.codigo);
            }

            // Atualizar o próximo código disponível (evitar duplicados)
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

// 13. Outras atividades relevantes (estatísticas, limpeza, reordenação)
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
            case 1:  // Estatísticas gerais
            {
                int pendentes, emCurso, resolvidos;
                contarIncidentesPorEstado(s, &pendentes, &emCurso, &resolvidos);

                printf("\n  ──────────────────────────────────────────────────────────\n");
                printf("  ESTATÍSTICAS DE INCIDENTES\n");
                printf("  ──────────────────────────────────────────────────────────\n");
                printf("  Total: %d\n", s->totalIncidentes);
                printf("  Pendentes: %d\n", pendentes);
                printf("  Em Curso: %d\n", emCurso);
                printf("  Resolvidos: %d\n", resolvidos);
                if (s->totalIncidentes > 0)
                {
                    // Calcula percentagem de resolução
                    printf("  Taxa de resolucao: %.1f%%\n",
                           (resolvidos * 100.0) / s->totalIncidentes);
                }
                printf("  ──────────────────────────────────────────────────────────\n");
                pausar();
                break;
            }
            case 2:  // Limpar incidentes resolvidos antigos (liberta memória)
            {
                NodeIncidente *atual = s->incidentes;
                NodeIncidente *anterior = NULL;
                int removidos = 0;

                while (atual != NULL)
                {
                    if (atual->dados.estado == CONCLUIDO)
                    {
                        NodeIncidente *temp = atual;
                        if (anterior == NULL)  // É o primeiro nó
                        {
                            s->incidentes = atual->proximo;
                            atual = s->incidentes;
                        }
                        else  // Nó no meio ou fim
                        {
                            anterior->proximo = atual->proximo;
                            atual = atual->proximo;
                        }
                        free(temp);  // Liberta memória
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
            case 3:  // Mostrar resumo da fila
                listarFilaAtendimento(s);
                pausar();
                break;
            case 4:  // Reordenar incidentes por prioridade
                reordenarIncidentesPorPrioridade(s);
                guardarFicheiro(s);
                pausar();
                break;
            case 0:
                break;
        }

    } while (opcao != 0);
}

/*
 * Menu do Módulo 4
 * Apresenta as opções e chama as funções correspondentes
 */

void menuIncidente(Sistema *s)
{
    int opcao;

    do
    {
        limparEcra();
        printf("\n  ╔═══════════════════════════════════════════════════════════╗\n");
        printf("  ║              MODULO 4 — INCIDENTES TÉCNICOS                 ║\n");
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
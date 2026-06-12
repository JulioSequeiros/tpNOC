// Modulo 6: Relatorios e Ficheiros
// Created by Guilherme Fernandes on 24/05/26.
//

#include "noc.h"

/*
 * UTIL - Funções auxiliares específicas do módulo 6
 */

// Função auxiliar para obter 'string' do estado do equipamento para relatórios
static const char *estadoEquipamentoParaRelatorio(EstadoEquipamento estado)
{
    switch (estado)
    {
        case OPERACIONAL:   return "OPERACIONAL";
        case EM_FALHA:      return "EM FALHA";
        case EM_MANUTENCAO: return "EM MANUTENCAO";
        case DESATIVADO:    return "DESATIVADO";
        default:            return "DESCONHECIDO";
    }
}

// Função auxiliar para obter 'string' da prioridade para relatórios
static const char *prioridadeParaRelatorio(int prioridade)
{
    switch (prioridade)
    {
        case 1: return "ALTA";
        case 2: return "MEDIA";
        case 3: return "BAIXA";
        default: return "DESCONHECIDA";
    }
}

// Função auxiliar para extrair mês e ano de uma data (formato dd-mm-aaaa)
static int extrairMesData(const char *data)
{
    if (data == NULL || strlen(data) < 5) return 0;
    int mes;
    sscanf(data + 3, "%2d", &mes);
    return mes;
}

static int extrairAnoData(const char *data)
{
    if (data == NULL || strlen(data) < 10) return 0;
    int ano;
    sscanf(data + 6, "%4d", &ano);
    return ano;
}

// Função auxiliar para verificar se incidente pertence a um determinado mês/ano
static int incidenteNoPeriodo(const Incidente *inc, int mes, int ano)
{
    int mesInc = extrairMesData(inc->dataAbertura);
    int anoInc = extrairAnoData(inc->dataAbertura);
    return (mesInc == mes && anoInc == ano);
}

// Função auxiliar para classificar tipo de incidente pela descrição
static const char *classificarTipoIncidente(const char *descricao)
{
    if (strstr(descricao, "ping") != NULL || strstr(descricao, "PING") != NULL)
        return "Falha de ping";
    else if (strstr(descricao, "sensor") != NULL || strstr(descricao, "Sensor") != NULL)
        return "Sensor anomalo";
    else
        return "Manual";
}

/*
 * Estrutura auxiliar para ordenacao de equipamentos
 */
typedef struct {
    int codigo;
    char nome[MAX_NOME];
    TipoEquipamento tipo;
    char tipoStr[30];
    char marca[MAX_MARCA];
    char modelo[MAX_MODELO];
    char ip[MAX_IP];
    char mac[MAX_MAC];
    char localizacao[MAX_LOCAL];
    EstadoEquipamento estado;
    char estadoStr[30];
    char dataUltimaVerificacao[MAX_DATA];
} EquipamentoOrdenado;

/*
 * Funcoes de comparacao para qsort
 */
static int compararPorEstado(const void *a, const void *b)
{
    EquipamentoOrdenado *eqA = (EquipamentoOrdenado*)a;
    EquipamentoOrdenado *eqB = (EquipamentoOrdenado*)b;
    return eqA->estado - eqB->estado;
}

static int compararPorTipo(const void *a, const void *b)
{
    EquipamentoOrdenado *eqA = (EquipamentoOrdenado*)a;
    EquipamentoOrdenado *eqB = (EquipamentoOrdenado*)b;
    return eqA->tipo - eqB->tipo;
}

static int compararPorLocalizacao(const void *a, const void *b)
{
    EquipamentoOrdenado *eqA = (EquipamentoOrdenado*)a;
    EquipamentoOrdenado *eqB = (EquipamentoOrdenado*)b;
    return strcmp(eqA->localizacao, eqB->localizacao);
}

/*
 * Operacoes de Persistencia (Carregar/Guardar)
 */

// Carregar equipamentos de ficheiro binario
static void carregarEquipamentosBin(Sistema *s)
{
    FILE *f = fopen("equipamentos.dat", "rb");
    if (f == NULL)
    {
        printf("\n  [!] Ficheiro 'equipamentos.dat' nao encontrado. Iniciando vazio.\n");
        return;
    }
    
    // Limpar lista atual
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL)
    {
        NodeEquipamento *temp = atual;
        atual = atual->proximo;
        free(temp);
    }
    s->equipamentos = NULL;
    s->totalEquipamentos = 0;
    
    // Ler total
    int total;
    if (fread(&total, sizeof(int), 1, f) != 1)
    {
        fclose(f);
        return;
    }
    
    // Ler equipamentos
    for (int i = 0; i < total; i++)
    {
        Equipamento temp;
        if (fread(&temp, sizeof(Equipamento), 1, f) != 1) break;
        
        NodeEquipamento *novo = (NodeEquipamento*)malloc(sizeof(NodeEquipamento));
        if (novo == NULL) continue;
        
        novo->dados = temp;
        novo->proximo = s->equipamentos;
        s->equipamentos = novo;
        s->totalEquipamentos++;
        
        if (temp.codigo >= s->proximoCodigoEquip)
        {
            s->proximoCodigoEquip = temp.codigo + 1;
        }
    }
    
    fclose(f);
    printf("\n  [OK] Equipamentos carregados: %d\n", s->totalEquipamentos);
}

// Guardar equipamentos em ficheiro binario
static void guardarEquipamentosBin(const Sistema *s)
{
    FILE *f = fopen("equipamentos.dat", "wb");
    if (f == NULL)
    {
        printf("\n  [!] Erro ao guardar equipamentos!\n");
        return;
    }
    
    // Guardar total
    fwrite(&s->totalEquipamentos, sizeof(int), 1, f);
    
    // Guardar cada equipamento
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL)
    {
        fwrite(&atual->dados, sizeof(Equipamento), 1, f);
        atual = atual->proximo;
    }
    
    fclose(f);
    printf("\n  [OK] Equipamentos guardados: %d\n", s->totalEquipamentos);
}

// Carregar incidentes de ficheiro binario
void carregarIncidentes(Sistema *s)
{
    FILE *f = fopen("incidentes.dat", "rb");
    if (f == NULL)
    {
        printf("\n  [!] Ficheiro 'incidentes.dat' nao encontrado. Iniciando vazio.\n");
        return;
    }
    
    // Limpar lista atual
    NodeIncidente *atual = s->incidentes;
    while (atual != NULL)
    {
        NodeIncidente *temp = atual;
        atual = atual->proximo;
        free(temp);
    }
    s->incidentes = NULL;
    s->totalIncidentes = 0;
    
    int total;
    if (fread(&total, sizeof(int), 1, f) != 1)
    {
        fclose(f);
        return;
    }
    
    for (int i = 0; i < total; i++)
    {
        Incidente temp;
        if (fread(&temp, sizeof(Incidente), 1, f) != 1) break;
        
        NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
        if (novo == NULL) continue;
        
        novo->dados = temp;
        novo->proximo = s->incidentes;
        s->incidentes = novo;
        s->totalIncidentes++;
        
        if (temp.codigo >= s->proximoCodigoInc)
        {
            s->proximoCodigoInc = temp.codigo + 1;
        }
    }
    
    fclose(f);
    printf("\n  [OK] Incidentes carregados: %d\n", s->totalIncidentes);
}

// Guardar incidentes em ficheiro binario
void guardarIncidentes(const Sistema *s)
{
    FILE *f = fopen("incidentes.dat", "wb");
    if (f == NULL)
    {
        printf("\n  [!] Erro ao guardar incidentes!\n");
        return;
    }
    
    fwrite(&s->totalIncidentes, sizeof(int), 1, f);
    
    NodeIncidente *atual = s->incidentes;
    while (atual != NULL)
    {
        fwrite(&atual->dados, sizeof(Incidente), 1, f);
        atual = atual->proximo;
    }
    
    fclose(f);
    printf("\n  [OK] Incidentes guardados: %d\n", s->totalIncidentes);
}

/*
 * Operacoes Logicas do Módulo 6
 */

// 1. Carregar dados existentes ao iniciar a aplicacao
void carregarTodosDados(Sistema *s)
{
    carregarEquipamentosBin(s);
    carregarIncidentes(s);
    carregarSensoresFicheiro(s);
    carregarConfiguracoesFicheiro(s);
    printf("\n  Dados carregados com sucesso!\n");
}

// 2. Guardar dados atualizados antes de sair da aplicacao
void guardarTodosDados(const Sistema *s)
{
    guardarEquipamentosBin(s);
    guardarIncidentes(s);
    guardarSensoresFicheiro(s);
    guardarConfiguracoesFicheiro(s);
    printf("\n  Dados guardados com sucesso!\n");
}

// 3. Importar leituras de sensores a partir de ficheiros de texto
void importarLeiturasSensores(Sistema *s)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |           IMPORTAR LEITURAS DE SENSORES                   |\n");
    printf("  =====================================================================\n");
    
    FILE *f = fopen("sensores_rack.txt", "r");
    
    if (f == NULL)
    {
        printf("\n  [!] Ficheiro 'sensores_rack.txt' nao encontrado!\n");
        printf("  [!] Formato esperado: codigo;tipo;valor;unidade;estado\n");
        pausar();
        return;
    }
    
    char linha[200];
    int totalLeituras = 0;
    int incidentesCriados = 0;
    
    printf("\n  --- LEITURAS DE SENSORES ---\n");
    printf("  ---------------------------------------------------------------------\n");
    
    while (fgets(linha, sizeof(linha), f))
    {
        // Remover newline
        linha[strcspn(linha, "\n")] = '\0';
        
        // Parse: codigo;tipo;valor;unidade;estado
        int codigo;
        char tipo[50], valorStr[20], unidade[20], estado[30];
        
        int result = sscanf(linha, "%d;%[^;];%[^;];%[^;];%s", 
                            &codigo, tipo, valorStr, unidade, estado);
        
        if (result != 5)
        {
            printf("  [!] Erro ao processar: %s\n", linha);
            continue;
        }
        
        float valor = atof(valorStr);
        
        printf("  Sensor %d | %s | %.2f %s | Estado: %s\n", 
               codigo, tipo, valor, unidade, estado);
        totalLeituras++;
        
        // Registar no 'log' de sensores
        FILE *log = fopen("log_sensores.txt", "a");
        if (log != NULL)
        {
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char dataHora[50];
            strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
            fprintf(log, "[%s] Sensor %d - %s - %.2f %s - %s\n", 
                    dataHora, codigo, tipo, valor, unidade, estado);
            fclose(log);
        }
        
        // Verificar estado anomalo e criar incidente
        if (strcmp(estado, "AVISO") == 0 || 
            strcmp(estado, "CRITICO") == 0 || 
            strcmp(estado, "FALHA_REDE") == 0)
        {
            printf("     [AVISO] Estado anomalo! A criar incidente...\n");
            
            NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
            if (novo != NULL)
            {
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
                
                // Adicionar a fila de atendimento
                adicionarNaFilaAtendimento(s, novo->dados.codigo);
            }
        }
    }
    
    fclose(f);
    
    printf("\n  ---------------------------------------------------------------------\n");
    printf("  [OK] IMPORTACAO CONCLUIDA\n");
    printf("  Leituras processadas: %d\n", totalLeituras);
    printf("  Incidentes criados: %d\n", incidentesCriados);
    printf("  ---------------------------------------------------------------------\n");
    
    guardarFicheiro(s);
    pausar();
}

// 4. Guardar os resultados dos comandos de rede em ficheiros de texto
void guardarResultadosRede(const Sistema *s)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |           GUARDAR RESULTADOS DE REDE                      |\n");
    printf("  =====================================================================\n");
    
    FILE *f = fopen("log_monitorizacao.txt", "a");
    
    if (f == NULL)
    {
        printf("\n  [!] Erro ao abrir log_monitorizacao.txt\n");
        pausar();
        return;
    }
    
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char dataHora[50];
    strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
    
    fprintf(f, "\n====================================\n");
    fprintf(f, "Log de rede: %s\n", dataHora);
    fprintf(f, "Total equipamentos: %d\n", s->totalEquipamentos);
    fprintf(f, "Total incidentes: %d\n", s->totalIncidentes);
    fprintf(f, "====================================\n");
    
    fclose(f);
    
    printf("\n  [OK] Resultados de rede guardados em log_monitorizacao.txt\n");
    pausar();
}

// 5. Gerar um relatorio de estado da rede
void gerarRelatorioEstadoRede(const Sistema *s)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |           RELATORIO DE ESTADO DA REDE                     |\n");
    printf("  =====================================================================\n");
    
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char nomeFicheiro[100];
    char dataRelatorio[50];
    
    strftime(dataRelatorio, sizeof(dataRelatorio), "%d/%m/%Y %H:%M:%S", tm_info);
    snprintf(nomeFicheiro, sizeof(nomeFicheiro), "relatorio_estado_rede_%02d_%02d_%04d.txt",
             tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
    
    FILE *f = fopen(nomeFicheiro, "w");
    if (f == NULL)
    {
        printf("\n  [!] Erro ao criar relatorio!\n");
        pausar();
        return;
    }
    
    // Contar estatisticas
    NodeEquipamento *eq = s->equipamentos;
    int operacionais = 0, emFalha = 0, emManutencao = 0, desativados = 0;
    
    while (eq != NULL)
    {
        switch (eq->dados.estado)
        {
            case OPERACIONAL:   operacionais++; break;
            case EM_FALHA:      emFalha++; break;
            case EM_MANUTENCAO: emManutencao++; break;
            case DESATIVADO:    desativados++; break;
            default: break;
        }
        eq = eq->proximo;
    }
    
    NodeIncidente *inc = s->incidentes;
    int pendentes = 0, emCurso = 0, resolvidos = 0;
    while (inc != NULL)
    {
        switch (inc->dados.estado)
        {
            case PENDENTE:  pendentes++; break;
            case EM_CURSO:  emCurso++; break;
            case CONCLUIDO: resolvidos++; break;
            default: break;
        }
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
    while (eq != NULL)
    {
        if (eq->dados.estado == OPERACIONAL)
        {
            fprintf(f, "  %d | %s | IP: %s | Local: %s\n", 
                    eq->dados.codigo, eq->dados.nome, eq->dados.ip, eq->dados.localizacao);
        }
        eq = eq->proximo;
    }
    fprintf(f, "Total: %d\n\n", operacionais);
    
    // Equipamentos em falha
    fprintf(f, "--- EQUIPAMENTOS EM FALHA ---\n");
    eq = s->equipamentos;
    while (eq != NULL)
    {
        if (eq->dados.estado == EM_FALHA)
        {
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
    while (inc != NULL)
    {
        if (inc->dados.estado == PENDENTE)
        {
            fprintf(f, "  #%d | Equip: %d | Prioridade: %s | Aberto: %s\n",
                    inc->dados.codigo, inc->dados.codigoEquipamento,
                    prioridadeParaRelatorio(inc->dados.prioridade),
                    inc->dados.dataAbertura);
        }
        inc = inc->proximo;
    }
    fprintf(f, "Total: %d\n\n", pendentes);
    
    // Resumo do estado
    fprintf(f, "--- RESUMO DO ESTADO DA REDE ---\n");
    fprintf(f, "Equipamentos:\n");
    fprintf(f, "  Operacionais:   %d\n", operacionais);
    fprintf(f, "  Em falha:       %d\n", emFalha);
    fprintf(f, "  Em manutencao:  %d\n", emManutencao);
    fprintf(f, "  Desativados:    %d\n", desativados);
    fprintf(f, "\nIncidentes:\n");
    fprintf(f, "  Pendentes:      %d\n", pendentes);
    fprintf(f, "  Em curso:       %d\n", emCurso);
    fprintf(f, "  Resolvidos:     %d\n", resolvidos);
    fprintf(f, "\n");
    
    // Classificacao do estado
    fprintf(f, "--- CLASSIFICACAO DO ESTADO DA REDE ---\n");
    if (emFalha == 0 && pendentes == 0)
    {
        fprintf(f, "ESTADO: NORMAL\n");
    }
    else if (emFalha <= 2 && pendentes <= 3)
    {
        fprintf(f, "ESTADO: ATENCAO\n");
    }
    else
    {
        fprintf(f, "ESTADO: CRITICO\n");
    }
    
    fprintf(f, "\n================================================\n");
    fclose(f);
    
    printf("\n  [OK] Relatorio gerado: %s\n", nomeFicheiro);
    printf("  Equipamentos operacionais: %d\n", operacionais);
    printf("  Equipamentos em falha: %d\n", emFalha);
    printf("  Incidentes pendentes: %d\n", pendentes);
    pausar();
}

// 6. Gerar um relatorio mensal de incidentes
void gerarRelatorioMensalIncidentes(const Sistema *s)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |           RELATORIO MENSAL DE INCIDENTES                  |\n");
    printf("  =====================================================================\n");
    
    int mes = lerInteiro("  Mes (1-12)", 1, 12);
    int ano = lerInteiro("  Ano", 2000, 2100);
    
    char nomeFicheiro[100];
    snprintf(nomeFicheiro, sizeof(nomeFicheiro), "relatorio_incidentes_%02d_%04d.txt", mes, ano);
    
    FILE *f = fopen(nomeFicheiro, "w");
    if (f == NULL)
    {
        printf("\n  [!] Erro ao criar relatorio!\n");
        pausar();
        return;
    }
    
    NodeIncidente *inc = s->incidentes;
    int total = 0, pendentes = 0, emCurso = 0, resolvidos = 0;
    int prioridadeAlta = 0, prioridadeMedia = 0, prioridadeBaixa = 0;
    int incidentesPing = 0, incidentesSensor = 0, incidentesManual = 0;
    
    while (inc != NULL)
    {
        if (incidenteNoPeriodo(&inc->dados, mes, ano))
        {
            total++;
            
            switch (inc->dados.estado)
            {
                case PENDENTE:  pendentes++; break;
                case EM_CURSO:  emCurso++; break;
                case CONCLUIDO: resolvidos++; break;
                default: break;
            }
            
            if (inc->dados.prioridade == 1) prioridadeAlta++;
            else if (inc->dados.prioridade == 2) prioridadeMedia++;
            else prioridadeBaixa++;
            
            // Contar por tipo
            const char *tipo = classificarTipoIncidente(inc->dados.descricao);
            if (strcmp(tipo, "Falha de ping") == 0) incidentesPing++;
            else if (strcmp(tipo, "Sensor anomalo") == 0) incidentesSensor++;
            else incidentesManual++;
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
    fprintf(f, "  Pendentes:  %d\n", pendentes);
    fprintf(f, "  Em curso:   %d\n", emCurso);
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
    fprintf(f, "Falha de ping:   %d\n", incidentesPing);
    fprintf(f, "Sensor anomalo:  %d\n", incidentesSensor);
    fprintf(f, "Manual:          %d\n", incidentesManual);
    
    fprintf(f, "\n================================================\n");
    fclose(f);
    
    printf("\n  [OK] Relatorio mensal gerado: %s\n", nomeFicheiro);
    printf("  Total incidentes no periodo: %d\n", total);
    pausar();
}

// 7. Listar equipamentos ordenados
void listarEquipamentosOrdenados(const Sistema *s)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |           LISTAR EQUIPAMENTOS ORDENADOS                    |\n");
    printf("  =====================================================================\n");
    
    // Verificar se existem equipamentos
    if (s->equipamentos == NULL)
    {
        printf("\n  [!] Nao existem equipamentos registados.\n");
        pausar();
        return;
    }
    
    // Perguntar criterio de ordenacao
    printf("\n  Ordenar por:\n");
    printf("    1. Estado (Operacional, Em Falha, Em Manutencao, Desativado)\n");
    printf("    2. Tipo (Router, Switch, Access Point, etc.)\n");
    printf("    3. Localizacao (ordem alfabetica)\n");
    
    int criterio = lerInteiro("\n  Opcao", 1, 3);
    
    // Contar numero de equipamentos
    int total = 0;
    NodeEquipamento *aux = s->equipamentos;
    while (aux != NULL)
    {
        total++;
        aux = aux->proximo;
    }
    
    // Alocar array para ordenacao
    EquipamentoOrdenado *equipamentos = (EquipamentoOrdenado*)malloc(total * sizeof(EquipamentoOrdenado));
    if (equipamentos == NULL)
    {
        printf("\n  [!] Erro ao alocar memoria!\n");
        pausar();
        return;
    }
    
    // Copiar dados para o array
    int i = 0;
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL)
    {
        equipamentos[i].codigo = atual->dados.codigo;
        strcpy(equipamentos[i].nome, atual->dados.nome);
        equipamentos[i].tipo = atual->dados.tipo;
        strcpy(equipamentos[i].tipoStr, tipoToString(atual->dados.tipo));
        strcpy(equipamentos[i].marca, atual->dados.marca);
        strcpy(equipamentos[i].modelo, atual->dados.modelo);
        strcpy(equipamentos[i].ip, atual->dados.ip);
        strcpy(equipamentos[i].mac, atual->dados.mac);
        strcpy(equipamentos[i].localizacao, atual->dados.localizacao);
        equipamentos[i].estado = atual->dados.estado;
        strcpy(equipamentos[i].estadoStr, estadoEquipamentoToString(atual->dados.estado));
        strcpy(equipamentos[i].dataUltimaVerificacao, atual->dados.dataUltimaVerificacao);
        i++;
        atual = atual->proximo;
    }
    
    // Ordenar conforme criterio
    switch (criterio)
    {
        case 1:
            qsort(equipamentos, total, sizeof(EquipamentoOrdenado), compararPorEstado);
            printf("\n  [LISTA] EQUIPAMENTOS ORDENADOS POR ESTADO:\n");
            break;
        case 2:
            qsort(equipamentos, total, sizeof(EquipamentoOrdenado), compararPorTipo);
            printf("\n  [LISTA] EQUIPAMENTOS ORDENADOS POR TIPO:\n");
            break;
        case 3:
            qsort(equipamentos, total, sizeof(EquipamentoOrdenado), compararPorLocalizacao);
            printf("\n  [LISTA] EQUIPAMENTOS ORDENADOS POR LOCALIZACAO:\n");
            break;
    }
    
    printf("\n  %-6s %-22s %-18s %-16s %-16s\n",
           "Cod.", "Nome", (criterio == 1 ? "Estado" : "Tipo"), "Localizacao", "IP");
    printf("  ");
    for (int i = 0; i < 85; i++) printf("-");
    printf("\n");
    
    // Exibir equipamentos ordenados
    for (int i = 0; i < total; i++)
    {
        if (criterio == 1)
        {
            printf("  %-6d %-22s %-18s %-16s %-16s\n",
                   equipamentos[i].codigo,
                   equipamentos[i].nome,
                   equipamentos[i].estadoStr,
                   equipamentos[i].localizacao,
                   equipamentos[i].ip);
        }
        else
        {
            printf("  %-6d %-22s %-18s %-16s %-16s\n",
                   equipamentos[i].codigo,
                   equipamentos[i].nome,
                   equipamentos[i].tipoStr,
                   equipamentos[i].localizacao,
                   equipamentos[i].ip);
        }
    }
    
    printf("\n  Total de equipamentos: %d\n", total);
    
    // Opcao para guardar em ficheiro
    int guardar = lerInteiro("\n  Deseja guardar esta lista em ficheiro? (1-Sim, 2-Nao)", 1, 2);
    if (guardar == 1)
    {
        char nomeFicheiro[100];
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        
        switch (criterio)
        {
            case 1:
                snprintf(nomeFicheiro, sizeof(nomeFicheiro), "equipamentos_ordenados_estado_%02d%02d%04d.txt",
                         tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
                break;
            case 2:
                snprintf(nomeFicheiro, sizeof(nomeFicheiro), "equipamentos_ordenados_tipo_%02d%02d%04d.txt",
                         tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
                break;
            case 3:
                snprintf(nomeFicheiro, sizeof(nomeFicheiro), "equipamentos_ordenados_local_%02d%02d%04d.txt",
                         tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
                break;
        }
        
        FILE *f = fopen(nomeFicheiro, "w");
        if (f != NULL)
        {
            fprintf(f, "================================================\n");
            fprintf(f, "        LISTA DE EQUIPAMENTOS ORDENADOS\n");
            fprintf(f, "================================================\n");
            fprintf(f, "Data: %02d/%02d/%04d %02d:%02d:%02d\n",
                    tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900,
                    tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
            fprintf(f, "Criterio: ");
            switch (criterio)
            {
                case 1: fprintf(f, "Estado\n"); break;
                case 2: fprintf(f, "Tipo\n"); break;
                case 3: fprintf(f, "Localizacao\n"); break;
            }
            fprintf(f, "================================================\n\n");
            fprintf(f, "%-6s %-22s %-18s %-16s %-16s\n",
                    "Cod.", "Nome", (criterio == 1 ? "Estado" : "Tipo"), "Localizacao", "IP");
            fprintf(f, "------------------------------------------------\n");
            
            for (int i = 0; i < total; i++)
            {
                if (criterio == 1)
                {
                    fprintf(f, "%-6d %-22s %-18s %-16s %-16s\n",
                           equipamentos[i].codigo,
                           equipamentos[i].nome,
                           equipamentos[i].estadoStr,
                           equipamentos[i].localizacao,
                           equipamentos[i].ip);
                }
                else
                {
                    fprintf(f, "%-6d %-22s %-18s %-16s %-16s\n",
                           equipamentos[i].codigo,
                           equipamentos[i].nome,
                           equipamentos[i].tipoStr,
                           equipamentos[i].localizacao,
                           equipamentos[i].ip);
                }
            }
            
            fprintf(f, "\nTotal de equipamentos: %d\n", total);
            fprintf(f, "================================================\n");
            fclose(f);
            
            printf("\n  [OK] Lista guardada em: %s\n", nomeFicheiro);
        }
        else
        {
            printf("\n  [!] Erro ao guardar ficheiro!\n");
        }
    }
    
    free(equipamentos);
    pausar();
}

// 9. Criar resumo textual do estado da rede (Normal, Aviso, Critico) - NOVA FUNCAO
void resumoTextualEstadoRede(const Sistema *s)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |           RESUMO TEXTUAL DO ESTADO DA REDE                  |\n");
    printf("  =====================================================================\n");
    
    // Verificar se existem equipamentos
    if (s->equipamentos == NULL)
    {
        printf("\n  [!] Nao existem equipamentos registados.\n");
        printf("  [!] Registe equipamentos no Modulo 1 primeiro.\n");
        pausar();
        return;
    }
    
    // Contar estatisticas dos equipamentos
    NodeEquipamento *eq = s->equipamentos;
    int totalEquipamentos = 0;
    int operacionais = 0;
    int emFalha = 0;
    int emManutencao = 0;
    int desativados = 0;
    
    while (eq != NULL)
    {
        totalEquipamentos++;
        switch (eq->dados.estado)
        {
            case OPERACIONAL:   operacionais++; break;
            case EM_FALHA:      emFalha++; break;
            case EM_MANUTENCAO: emManutencao++; break;
            case DESATIVADO:    desativados++; break;
            default: break;
        }
        eq = eq->proximo;
    }
    
    // Contar incidentes pendentes
    NodeIncidente *inc = s->incidentes;
    int incidentesPendentes = 0;
    int incidentesPrioridadeAlta = 0;
    
    while (inc != NULL)
    {
        if (inc->dados.estado == PENDENTE)
        {
            incidentesPendentes++;
            if (inc->dados.prioridade == 1)
            {
                incidentesPrioridadeAlta++;
            }
        }
        inc = inc->proximo;
    }
    
    // Calcular percentagem de equipamentos operacionais
    float percentagemOperacional = (totalEquipamentos > 0) ? 
        (operacionais * 100.0 / totalEquipamentos) : 0;
    
    // Determinar o estado geral da rede
    char estadoGeral[20];
    char cor[10];
    char recomendacao[200];
    
    if (percentagemOperacional >= 90 && emFalha == 0 && incidentesPrioridadeAlta == 0)
    {
        strcpy(estadoGeral, "NORMAL");
        strcpy(cor, "VERDE");
        strcpy(recomendacao, "Rede operacional sem anomalias. Mantenha a monitorizacao de rotina.");
    }
    else if (percentagemOperacional >= 70 && emFalha <= 2 && incidentesPendentes <= 3)
    {
        strcpy(estadoGeral, "ATENCAO");
        strcpy(cor, "AMARELO");
        strcpy(recomendacao, "Existem algumas anomalias na rede. Recomenda-se investigar os equipamentos em falha e os incidentes pendentes.");
    }
    else if (percentagemOperacional >= 50 && emFalha <= 5)
    {
        strcpy(estadoGeral, "CRITICO");
        strcpy(cor, "LARANJA");
        strcpy(recomendacao, "A rede apresenta problemas significativos. Intervencao necessaria em breve.");
    }
    else
    {
        strcpy(estadoGeral, "GRAVE");
        strcpy(cor, "VERMELHO");
        strcpy(recomendacao, "A rede encontra-se em estado grave com multiplas falhas. Intervencao imediata necessaria!");
    }
    
    // Exibir resumo textual formatado
    printf("\n  =====================================================================\n");
    printf("  |                    RESUMO DO ESTADO DA REDE                      |\n");
    printf("  =====================================================================\n\n");
    
    printf("  [DADOS GERAIS]\n");
    printf("  ---------------------------------------------------------------------\n");
    printf("  Total de equipamentos:        %d\n", totalEquipamentos);
    printf("  Equipamentos operacionais:    %d (%.1f%%)\n", operacionais, percentagemOperacional);
    printf("  Equipamentos em falha:        %d\n", emFalha);
    printf("  Equipamentos em manutencao:   %d\n", emManutencao);
    printf("  Equipamentos desativados:     %d\n", desativados);
    printf("  Incidentes pendentes:         %d\n", incidentesPendentes);
    printf("  Incidentes de prioridade ALTA: %d\n", incidentesPrioridadeAlta);
    printf("  ---------------------------------------------------------------------\n\n");
    
    printf("  [CLASSIFICACAO GERAL]\n");
    printf("  ---------------------------------------------------------------------\n");
    printf("  ESTADO DA REDE: %s [%s]\n", estadoGeral, cor);
    printf("  ---------------------------------------------------------------------\n\n");
    
    printf("  [ANALISE DETALHADA]\n");
    printf("  ---------------------------------------------------------------------\n");
    
    // Análise da percentagem de equipamentos operacionais
    if (percentagemOperacional >= 95)
    {
        printf("  - Excelente: %.1f%% dos equipamentos estao operacionais.\n", percentagemOperacional);
    }
    else if (percentagemOperacional >= 80)
    {
        printf("  - Bom: %.1f%% dos equipamentos estao operacionais.\n", percentagemOperacional);
    }
    else if (percentagemOperacional >= 60)
    {
        printf("  - Razoavel: %.1f%% dos equipamentos estao operacionais.\n", percentagemOperacional);
    }
    else
    {
        printf("  - Critico: Apenas %.1f%% dos equipamentos estao operacionais.\n", percentagemOperacional);
    }
    
    // Análise dos equipamentos em falha
    if (emFalha > 0)
    {
        printf("  - %d equipamento(s) em estado de FALHA.\n", emFalha);
        if (emFalha > 3)
        {
            printf("    > Atencao: Multiplas falhas na rede.\n");
        }
    }
    else
    {
        printf("  - Nenhum equipamento em estado de falha.\n");
    }
    
    // Análise dos incidentes pendentes
    if (incidentesPendentes > 0)
    {
        printf("  - %d incidente(s) pendentes.\n", incidentesPendentes);
        if (incidentesPrioridadeAlta > 0)
        {
            printf("    > URGENTE: %d incidente(s) de prioridade ALTA aguardam resolucao.\n", 
                   incidentesPrioridadeAlta);
        }
    }
    else
    {
        printf("  - Nenhum incidente pendente.\n");
    }
    
    printf("  ---------------------------------------------------------------------\n\n");
    
    printf("  [RECOMENDACAO]\n");
    printf("  ---------------------------------------------------------------------\n");
    printf("  %s\n", recomendacao);
    printf("  ---------------------------------------------------------------------\n\n");
    
    // Sugestões especificas
    printf("  [SUGESTOES]\n");
    printf("  ---------------------------------------------------------------------\n");
    
    if (emFalha > 0)
    {
        printf("  - Verificar os equipamentos em estado de FALHA.\n");
        printf("  - Executar testes de ping aos equipamentos offline.\n");
    }
    
    if (incidentesPrioridadeAlta > 0)
    {
        printf("  - Priorizar a resolucao dos incidentes de prioridade ALTA.\n");
    }
    
    if (percentagemOperacional < 70)
    {
        printf("  - Acionar plano de contingencia da rede.\n");
        printf("  - Contactar suporte tecnico especializado.\n");
    }
    
    if (emFalha == 0 && incidentesPendentes == 0 && percentagemOperacional == 100)
    {
        printf("  - Manter a monitorizacao preventiva da rede.\n");
        printf("  - Documentar as configuracoes atuais.\n");
    }
    
    printf("  ---------------------------------------------------------------------\n");
    
    // Opcao para guardar resumo em ficheiro
    int guardar = lerInteiro("\n  Deseja guardar este resumo em ficheiro? (1-Sim, 2-Nao)", 1, 2);
    
    if (guardar == 1)
    {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char nomeFicheiro[100];
        
        snprintf(nomeFicheiro, sizeof(nomeFicheiro), "resumo_estado_rede_%02d%02d%04d.txt",
                 tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
        
        FILE *f = fopen(nomeFicheiro, "w");
        if (f != NULL)
        {
            fprintf(f, "================================================\n");
            fprintf(f, "        RESUMO DO ESTADO DA REDE\n");
            fprintf(f, "================================================\n");
            fprintf(f, "Data: %02d/%02d/%04d %02d:%02d:%02d\n",
                    tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900,
                    tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
            fprintf(f, "================================================\n\n");
            
            fprintf(f, "ESTADO DA REDE: %s\n\n", estadoGeral);
            
            fprintf(f, "--- DADOS GERAIS ---\n");
            fprintf(f, "Total equipamentos: %d\n", totalEquipamentos);
            fprintf(f, "Operacionais: %d (%.1f%%)\n", operacionais, percentagemOperacional);
            fprintf(f, "Em falha: %d\n", emFalha);
            fprintf(f, "Em manutencao: %d\n", emManutencao);
            fprintf(f, "Desativados: %d\n", desativados);
            fprintf(f, "Incidentes pendentes: %d\n", incidentesPendentes);
            fprintf(f, "Incidentes ALTA prioridade: %d\n\n", incidentesPrioridadeAlta);
            
            fprintf(f, "--- RECOMENDACAO ---\n");
            fprintf(f, "%s\n", recomendacao);
            fprintf(f, "\n================================================\n");
            
            fclose(f);
            printf("\n  [OK] Resumo guardado em: %s\n", nomeFicheiro);
        }
        else
        {
            printf("\n  [!] Erro ao guardar ficheiro!\n");
        }
    }
    
    pausar();
}

// 8. Outras atividades relevantes
void outrasAtividadesRelatorios(Sistema *s)
{
    int opcao;
    
    do
    {
        limparEcra();
        printf("\n  ================================================================\n");
        printf("  |              ATIVIDADES EXTRA - RELATORIOS                  |\n");
        printf("  ================================================================\n");
        printf("  |  1. Ver resumo do sistema                                  |\n");
        printf("  |  2. Exportar equipamentos para CSV                         |\n");
        printf("  |  3. Exportar incidentes para CSV                           |\n");
        printf("  |  4. Listar equipamentos ordenados                          |\n");
        printf("  |  5. Resumo textual do estado da rede (NORMAL/AVISO/CRITICO) |\n");
        printf("  |  0. Voltar                                                |\n");
        printf("  ================================================================\n");
        
        opcao = lerInteiro("  Opcao", 0, 5);
        
        switch (opcao)
        {
            case 1:
                printf("\n  ---------------------------------------------------------------------\n");
                printf("  RESUMO DO SISTEMA\n");
                printf("  ---------------------------------------------------------------------\n");
                printf("  Equipamentos: %d\n", s->totalEquipamentos);
                printf("  Incidentes: %d\n", s->totalIncidentes);
                printf("  Proximo codigo equipamento: %d\n", s->proximoCodigoEquip);
                printf("  Proximo codigo incidente: %d\n", s->proximoCodigoInc);
                printf("  ---------------------------------------------------------------------\n");
                pausar();
                break;
                
            case 2:
            {
                FILE *csv = fopen("equipamentos_export.csv", "w");
                if (csv)
                {
                    fprintf(csv, "Codigo;Nome;Tipo;Marca;Modelo;IP;MAC;Localizacao;Estado;UltimaVerificacao\n");
                    NodeEquipamento *atual = s->equipamentos;
                    while (atual != NULL)
                    {
                        fprintf(csv, "%d;%s;%s;%s;%s;%s;%s;%s;%s;%s\n",
                                atual->dados.codigo,
                                atual->dados.nome,
                                tipoToString(atual->dados.tipo),
                                atual->dados.marca,
                                atual->dados.modelo,
                                atual->dados.ip,
                                atual->dados.mac,
                                atual->dados.localizacao,
                                estadoEquipamentoParaRelatorio(atual->dados.estado),
                                atual->dados.dataUltimaVerificacao);
                        atual = atual->proximo;
                    }
                    fclose(csv);
                    printf("\n  [OK] Exportado para equipamentos_export.csv\n");
                }
                else
                {
                    printf("\n  [!] Erro ao criar ficheiro CSV!\n");
                }
                pausar();
                break;
            }
                
            case 3:
            {
                FILE *csv = fopen("incidentes_export.csv", "w");
                if (csv)
                {
                    fprintf(csv, "Codigo;Equipamento;Descricao;Estado;Prioridade;DataAbertura;DataFecho\n");
                    NodeIncidente *atual = s->incidentes;
                    while (atual != NULL)
                    {
                        fprintf(csv, "%d;%d;%s;%s;%s;%s;%s\n",
                                atual->dados.codigo,
                                atual->dados.codigoEquipamento,
                                atual->dados.descricao,
                                estadoIncidenteToString(atual->dados.estado),
                                prioridadeParaRelatorio(atual->dados.prioridade),
                                atual->dados.dataAbertura,
                                atual->dados.dataFecho);
                        atual = atual->proximo;
                    }
                    fclose(csv);
                    printf("\n  [OK] Exportado para incidentes_export.csv\n");
                }
                else
                {
                    printf("\n  [!] Erro ao criar ficheiro CSV!\n");
                }
                pausar();
                break;
            }
            
            case 4:
                listarEquipamentosOrdenados(s);
                break;
            
            case 5:
                resumoTextualEstadoRede(s);
                break;
                
            case 0:
                break;
        }
        
    } while (opcao != 0);
}

/*
 * Funcoes de ‘Interface’ com o Módulo Principal
 */

void carregarFicheiro(Sistema *s)
{
    carregarTodosDados(s);
}

void guardarFicheiro(const Sistema *s)
{
    guardarTodosDados((Sistema*)s);
}

/*
 * Menu do Módulo 6
 */

void menuRelatorios(Sistema *s)
{
    int opcao;
    
    do
    {
        _sleep(40);
        limparEcra();
        printf("\n  ================================================================\n");
        printf("  |           MODULO 6 - RELATORIOS E FICHEIROS                |\n");
        printf("  ================================================================\n");
        printf("  |  1. Carregar dados existentes ao iniciar a aplicacao       |\n");
        printf("  |  2. Guardar dados atualizados antes de sair da aplicacao   |\n");
        printf("  |  3. Importar leituras de sensores                          |\n");
        printf("  |  4. Guardar resultados dos comandos de rede                |\n");
        printf("  |  5. Gerar relatorio de estado da rede                      |\n");
        printf("  |  6. Gerar relatorio mensal de incidentes                   |\n");
        printf("  |  7. Outras atividades                                      |\n");
        printf("  |  0. Voltar ao menu principal                               |\n");
        printf("  ================================================================\n");
        
        opcao = lerInteiro("  Opcao", 0, 7);
        
        switch (opcao)
        {
            case 1: carregarTodosDados(s); break;
            case 2: guardarTodosDados(s); break;
            case 3: importarLeiturasSensores(s); break;
            case 4: guardarResultadosRede(s); break;
            case 5: gerarRelatorioEstadoRede(s); break;
            case 6: gerarRelatorioMensalIncidentes(s); break;
            case 7: outrasAtividadesRelatorios(s); break;
            case 0: break;
        }
        
        if (opcao != 0 && opcao != 1 && opcao != 2)
        {
            pausar();
        }
        
    } while (opcao != 0);
}
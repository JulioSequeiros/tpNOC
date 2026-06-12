//
// Created by julio on 21/05/26.
//
// Modulo 3: Monitorização de Sensores

#include "noc.h"

void obterDataHoraAtual(char *datahora)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(datahora, MAX_DATAHORA, "%d-%m-%Y %H:%M:%S", tm);
}

// Converte 'string' de estado para enum
EstadoSensor parseEstadoSensor(const char *estadoStr)
{
    if (strcmp(estadoStr, "AVISO") == 0)      return SENSOR_AVISO;
    if (strcmp(estadoStr, "CRITICO") == 0)    return SENSOR_CRITICO;
    if (strcmp(estadoStr, "FALHA_REDE") == 0) return SENSOR_FALHA_REDE;
    return SENSOR_NORMAL;
}

// Regista mensagem no 'log' dos sensores
void registarLogSensores(const char *mensagem)
{
    FILE *f = fopen(FICH_LOG_SENSORES, "a");
    if (f == NULL) return;
    char datahora[MAX_DATAHORA];
    obterDataHoraAtual(datahora);
    fprintf(f, "[%s] %s\n", datahora, mensagem);
    fclose(f);
}

// Encontrar sensor por codigo
NodeSensor *encontrarSensorPorCodigo(const Sistema *s, const char *codigo)
{
    NodeSensor *atual = s->sensores;
    while (atual != NULL)
    {
        if (strcmp(atual->dados.codigo, codigo) == 0)
            return atual;
        atual = atual->proximo;
    }
    return NULL;
}

// Cria incidente automatico para leitura anómala
static void criarIncidenteSensorAuto(Sistema *s, const Sensor *sensor)
{
    NodeIncidente *novo = (NodeIncidente *)malloc(sizeof(NodeIncidente));
    if (novo == NULL) return;

    memset(novo, 0, sizeof(NodeIncidente));
    novo->dados.codigo            = ++s->proximoCodigoInc;
    novo->dados.codigoEquipamento = sensor->codigoEquipamento;

    snprintf(novo->dados.descricao, MAX_DESC,
             "Leitura anomala no sensor %s: %.2f%s [%s]",
             sensor->codigo,
             sensor->valorAtual,
             sensor->unidade,
             sensor->estado);

    novo->dados.estado     = PENDENTE;
    novo->dados.prioridade = (strcmp(sensor->estado, "CRITICO") == 0 ||
                               strcmp(sensor->estado, "FALHA_REDE") == 0) ? 3 : 2;
    obterDataAtual(novo->dados.dataAbertura);
    strcpy(novo->dados.dataFecho, "-");

    novo->proximo  = s->incidentes;
    s->incidentes  = novo;
    s->totalIncidentes++;

    adicionarNaFilaAtendimento(s, novo->dados.codigo);

    printf("  [AUTO] Incidente #%d criado para sensor %s\n",
           novo->dados.codigo, sensor->codigo);
}

/*
 * 1. Importar sensores do ficheiro texto
 */
void importarSensores(Sistema *s)
{
    limparEcra();
    printf("\n  ╔══════════════════════════════════════╗\n");
    printf("  ║    IMPORTAR SENSORES DO FICHEIRO     ║\n");
    printf("  ╚══════════════════════════════════════╝\n");

    FILE *f = fopen(FICH_SENSORES_IN, "r");
    if (f == NULL)
    {
        printf("\n  [!] Ficheiro \"%s\" nao encontrado.\n", FICH_SENSORES_IN);
        printf("      Formato esperado: codigo;tipo;valor;unidade;estado\n");
        printf("      Exemplo: TEMP_RACK;Temperatura da rack;38.5;C;CRITICO\n");
        pausar();
        return;
    }

    char linha[256];
    int importados = 0, atualizados = 0, incidentes = 0;

    while (fgets(linha, sizeof(linha), f) != NULL)
    {
        if (linha[0] == '\n' || linha[0] == '#') continue;
        linha[strcspn(linha, "\n")] = '\0';

        char codSensor[MAX_COD_SENSOR];
        char tipo[50];
        float valor;
        char unidade[20];
        char estado[20];

        int campos = sscanf(linha, "%29[^;];%49[^;];%f;%19[^;];%19s",
                            codSensor, tipo, &valor, unidade, estado);

        if (campos != 5)
        {
            printf("\n  [!] Linha com formato invalido: %s\n", linha);
            continue;
        }

        NodeSensor *existente = encontrarSensorPorCodigo(s, codSensor);
        if (existente != NULL)
        {
            existente->dados.valorAtual = valor;
            strncpy(existente->dados.estado, estado, sizeof(existente->dados.estado) - 1);
            obterDataAtual(existente->dados.ultimaLeitura);
            atualizados++;

            char logMsg[MAX_DESC];
            snprintf(logMsg, MAX_DESC, "Sensor %s atualizado: %.2f%s [%s]",
                     codSensor, valor, unidade, estado);
            registarLogSensores(logMsg);

            if (strcmp(estado, "NORMAL") != 0)
            {
                criarIncidenteSensorAuto(s, &existente->dados);
                incidentes++;
            }
        }
        else
        {
            NodeSensor *novo = (NodeSensor *)malloc(sizeof(NodeSensor));
            if (novo == NULL)
            {
                printf("\n  [!] Erro ao alocar memoria para sensor %s\n", codSensor);
                continue;
            }
            memset(novo, 0, sizeof(NodeSensor));

            strncpy(novo->dados.codigo,  codSensor, MAX_COD_SENSOR - 1);
            strncpy(novo->dados.tipo,    tipo,       49);
            strncpy(novo->dados.unidade, unidade,    19);
            strncpy(novo->dados.estado,  estado,     19);
            novo->dados.valorAtual        = valor;
            novo->dados.codigoEquipamento = 0;
            obterDataAtual(novo->dados.ultimaLeitura);

            novo->proximo  = s->sensores;
            s->sensores    = novo;
            s->totalSensores++;
            importados++;

            char logMsg[MAX_DESC];
            snprintf(logMsg, MAX_DESC, "Sensor %s importado: %.2f%s [%s]",
                     codSensor, valor, unidade, estado);
            registarLogSensores(logMsg);

            if (strcmp(estado, "NORMAL") != 0)
            {
                criarIncidenteSensorAuto(s, &novo->dados);
                incidentes++;
            }
        }
    }

    fclose(f);

    printf("\n  [OK] Importacao concluida:\n");
    printf("       Novos sensores  : %d\n", importados);
    printf("       Atualizados     : %d\n", atualizados);
    printf("       Incidentes auto : %d\n", incidentes);
    pausar();
}

/*
 * 2. Listar todos os sensores
 */
void listarSensores(const Sistema *s)
{
    limparEcra();
    printf("\n  ╔══════════════════════════════════════╗\n");
    printf("  ║    LISTA DE SENSORES                 ║\n");
    printf("  ╚══════════════════════════════════════╝\n");

    if (s->sensores == NULL)
    {
        printf("\n  [!] Nao existem sensores registados.\n");
        pausar();
        return;
    }

    printf("\n  %-28s %-22s %10s %-8s %-14s %s\n",
           "Codigo", "Tipo", "Valor", "Unidade", "Estado", "Ultima Leitura");
    printf("  ");
    for (int i = 0; i < 100; i++) printf("-");
    printf("\n");

    NodeSensor *atual = s->sensores;
    while (atual != NULL)
    {
        Sensor *sen = &atual->dados;
        printf("  %-28s %-22s %10.2f %-8s %-14s %s\n",
               sen->codigo, sen->tipo, sen->valorAtual,
               sen->unidade, sen->estado, sen->ultimaLeitura);
        atual = atual->proximo;
    }

    printf("\n  Total de sensores: %d\n", s->totalSensores);
    pausar();
}

/*
 * 3. Pesquisar sensor por codigo
 */
void pesquisarSensor(const Sistema *s)
{
    limparEcra();
    printf("\n  ╔══════════════════════════════════════╗\n");
    printf("  ║    PESQUISAR SENSOR                  ║\n");
    printf("  ╚══════════════════════════════════════╝\n");

    if (s->sensores == NULL)
    {
        printf("\n  [!] Nao existem sensores registados.\n");
        pausar();
        return;
    }

    char codSensor[MAX_COD_SENSOR];
    lerString("  Codigo do sensor", codSensor, MAX_COD_SENSOR);

    NodeSensor *no = encontrarSensorPorCodigo(s, codSensor);
    if (no == NULL)
    {
        printf("\n  [!] Sensor com codigo \"%s\" nao encontrado.\n", codSensor);
        pausar();
        return;
    }

    Sensor *sen = &no->dados;
    printf("\n  ╔══════════════════════════════════════════════════╗\n");
    printf("  ║           FICHA DO SENSOR                       ║\n");
    printf("  ╚══════════════════════════════════════════════════╝\n");
    printf("  Codigo            : %s\n", sen->codigo);
    printf("  Tipo              : %s\n", sen->tipo);
    printf("  Equipamento (cod) : %d\n", sen->codigoEquipamento);
    printf("  Valor Atual       : %.2f %s\n", sen->valorAtual, sen->unidade);
    printf("  Valor Minimo      : %.2f %s\n", sen->valorMinimo, sen->unidade);
    printf("  Valor Maximo      : %.2f %s\n", sen->valorMaximo, sen->unidade);
    printf("  Estado            : %s\n", sen->estado);
    printf("  Ultima Leitura    : %s\n", sen->ultimaLeitura);
    printf("  ────────────────────────────────────────────────────\n");
    pausar();
}

/*
 * 4. Listar sensores anomalos
 */
void listarSensoresAnomalos(const Sistema *s)
{
    limparEcra();
    printf("\n  ╔══════════════════════════════════════╗\n");
    printf("  ║    SENSORES COM LEITURAS ANOMALAS    ║\n");
    printf("  ╚══════════════════════════════════════╝\n");

    if (s->sensores == NULL)
    {
        printf("\n  [!] Nao existem sensores registados.\n");
        pausar();
        return;
    }

    printf("\n  %-28s %-22s %10s %-8s %-14s\n",
           "Codigo", "Tipo", "Valor", "Unidade", "Estado");
    printf("  ");
    for (int i = 0; i < 86; i++) printf("-");
    printf("\n");

    int count = 0;
    NodeSensor *atual = s->sensores;
    while (atual != NULL)
    {
        Sensor *sen = &atual->dados;
        if (strcmp(sen->estado, "NORMAL") != 0)
        {
            printf("  %-28s %-22s %10.2f %-8s %-14s\n",
                   sen->codigo, sen->tipo, sen->valorAtual,
                   sen->unidade, sen->estado);
            count++;
        }
        atual = atual->proximo;
    }

    if (count == 0)
        printf("\n  [OK] Nao existem leituras anomalas.\n");
    else
        printf("\n  Total de sensores anomalos: %d\n", count);

    pausar();
}

/*
 * 5. Guardar sensores em ficheiro binario
 */
void guardarSensoresFicheiro(const Sistema *s)
{
    FILE *f = fopen(FICH_SENSORES_DAT, "wb");
    if (f == NULL)
    {
        printf("\n  [!] Erro ao abrir o ficheiro \"%s\" para escrita\n", FICH_SENSORES_DAT);
        return;
    }

    fwrite(&s->totalSensores,       sizeof(int), 1, f);
    fwrite(&s->proximoCodigoSensor, sizeof(int), 1, f);

    NodeSensor *atual = s->sensores;
    while (atual != NULL)
    {
        fwrite(&atual->dados, sizeof(Sensor), 1, f);
        atual = atual->proximo;
    }

    fclose(f);
    printf("\n  [OK] %d sensor(es) guardado(s) em \"%s\".\n",
           s->totalSensores, FICH_SENSORES_DAT);
}

/*
 * 6. Carregar sensores de ficheiro binario
 */
void carregarSensoresFicheiro(Sistema *s)
{
    FILE *f = fopen(FICH_SENSORES_DAT, "rb");
    if (f == NULL)
    {
        printf("\n  [!] Ficheiro \"%s\" nao encontrado. A iniciar com lista vazia.\n", FICH_SENSORES_DAT);
        return;
    }

    int total = 0, proxCod = 0;
    fread(&total,   sizeof(int), 1, f);
    fread(&proxCod, sizeof(int), 1, f);

    s->totalSensores       = total;
    s->proximoCodigoSensor = proxCod;

    NodeSensor **ultimo = &s->sensores;
    for (int i = 0; i < total; i++)
    {
        NodeSensor *no = (NodeSensor *)malloc(sizeof(NodeSensor));
        if (no == NULL) break;
        fread(&no->dados, sizeof(Sensor), 1, f);
        no->proximo = NULL;
        *ultimo     = no;
        ultimo      = &no->proximo;
    }

    fclose(f);
    printf("\n  [OK] %d sensor(es) carregado(s) de \"%s\".\n", total, FICH_SENSORES_DAT);
}

/* ─── API pública de sensores ───────────────────────────────────────────── */

#define API_LEGACY "https://sensorlab.innominatum.pt/v1/sensors/export/legacy"
#define API_TXT    "https://sensorlab.innominatum.pt/v1/sensors/export/txt"
#define FICH_API_TMP "sensores_api_tmp.txt"

static int descarregarAPI(const char *url)
{
    char cmd[512];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd),
             "curl -s --max-time 10 \"%s\" -o \"%s\" 2>nul", url, FICH_API_TMP);
#else
    snprintf(cmd, sizeof(cmd),
             "curl -s --max-time 10 \"%s\" -o \"%s\" 2>/dev/null", url, FICH_API_TMP);
#endif
    return system(cmd);
}

/*
 * 7. Importar leituras da API
 */
void importarSensoresAPI(Sistema *s)
{
    limparEcra();
    printf("\n  Importar Sensores da API\n");
    printf("  URL: %s\n\n", API_LEGACY);
    printf("  [*] A ligar...\n");
    fflush(stdout);

    if (descarregarAPI(API_LEGACY) != 0) {
        printf("  [!] Erro ao aceder a API. Verifique a ligacao a internet.\n");
        remove(FICH_API_TMP);
        pausar();
        return;
    }

    FILE *f = fopen(FICH_API_TMP, "r");
    if (f == NULL) {
        printf("  [!] Erro ao abrir ficheiro temporario.\n");
        pausar();
        return;
    }

    char linha[256];
    int importados = 0, atualizados = 0, incidentes = 0;

    while (fgets(linha, sizeof(linha), f) != NULL)
    {
        if (linha[0] == '\n' || linha[0] == '#') continue;
        linha[strcspn(linha, "\n")] = '\0';

        char codSensor[MAX_COD_SENSOR], tipo[50], unidade[20], estado[20];
        float valor;
        int campos = sscanf(linha, "%29[^;];%49[^;];%f;%19[^;];%19s",
                            codSensor, tipo, &valor, unidade, estado);
        if (campos != 5) continue;

        NodeSensor *existente = encontrarSensorPorCodigo(s, codSensor);
        if (existente != NULL)
        {
            existente->dados.valorAtual = valor;
            strncpy(existente->dados.estado, estado, sizeof(existente->dados.estado) - 1);
            obterDataAtual(existente->dados.ultimaLeitura);
            atualizados++;

            char logMsg[MAX_DESC];
            snprintf(logMsg, MAX_DESC, "[API] Sensor %s atualizado: %.2f%s [%s]",
                     codSensor, valor, unidade, estado);
            registarLogSensores(logMsg);

            if (strcmp(estado, "NORMAL") != 0) {
                criarIncidenteSensorAuto(s, &existente->dados);
                incidentes++;
            }
        }
        else
        {
            NodeSensor *novo = (NodeSensor *)malloc(sizeof(NodeSensor));
            if (novo == NULL) continue;
            memset(novo, 0, sizeof(NodeSensor));
            strncpy(novo->dados.codigo,  codSensor, MAX_COD_SENSOR - 1);
            strncpy(novo->dados.tipo,    tipo,       49);
            strncpy(novo->dados.unidade, unidade,    19);
            strncpy(novo->dados.estado,  estado,     19);
            novo->dados.valorAtual        = valor;
            novo->dados.codigoEquipamento = 0;
            obterDataAtual(novo->dados.ultimaLeitura);
            novo->proximo  = s->sensores;
            s->sensores    = novo;
            s->totalSensores++;
            importados++;

            char logMsg[MAX_DESC];
            snprintf(logMsg, MAX_DESC, "[API] Sensor %s importado: %.2f%s [%s]",
                     codSensor, valor, unidade, estado);
            registarLogSensores(logMsg);

            if (strcmp(estado, "NORMAL") != 0) {
                criarIncidenteSensorAuto(s, &novo->dados);
                incidentes++;
            }
        }
    }

    fclose(f);
    remove(FICH_API_TMP);

    printf("  [OK] Importacao da API concluida:\n");
    printf("       Novos sensores  : %d\n", importados);
    printf("       Atualizados     : %d\n", atualizados);
    printf("       Incidentes auto : %d\n", incidentes);
    pausar();
}

/*
 * 8. Consultar estado atual da API (sem importar)
 */
void consultarEstadoAPI(void)
{
    limparEcra();
    printf("\n  Estado Atual dos Sensores — API RACK-01\n");
    printf("  URL: %s\n\n", API_TXT);
    printf("  [*] A ligar...\n");
    fflush(stdout);

    if (descarregarAPI(API_TXT) != 0) {
        printf("  [!] Erro ao aceder a API. Verifique a ligacao a internet.\n");
        remove(FICH_API_TMP);
        pausar();
        return;
    }

    FILE *f = fopen(FICH_API_TMP, "r");
    if (f == NULL) {
        printf("  [!] Erro ao abrir ficheiro temporario.\n");
        pausar();
        return;
    }

    limparEcra();
    printf("\n  ╔══════════════════════════════════════════════════════════════════╗\n");
    printf("  ║   ESTADO ATUAL DOS SENSORES — API RACK-01                       ║\n");
    printf("  ╚══════════════════════════════════════════════════════════════════╝\n");

    char linha[512];
    int header_done = 0;

    while (fgets(linha, sizeof(linha), f) != NULL)
    {
        linha[strcspn(linha, "\n")] = '\0';

        if (linha[0] == '#') {
            /* Mostrar apenas metadados relevantes */
            if (strstr(linha, "local_time=") || strstr(linha, "day_profile=") ||
                strstr(linha, "site=")        || strstr(linha, "rack="))
                printf("  %s\n", linha + 2);  /* +2 salta "# " */
            continue;
        }

        /* Primeira linha não comentario = cabecalho */
        if (!header_done) {
            header_done = 1;
            printf("\n  %-14s %-22s %8s %-7s %-10s %s\n",
                   "Sensor", "Tipo", "Valor", "Unidade", "Estado", "Mensagem");
            printf("  ");
            for (int i = 0; i < 85; i++) printf("-");
            printf("\n");
            continue;
        }

        /* Linhas de dados: tokenizar por
         * Campos (0-indexed): 0=timestamp 1=seq 2=site 3=rack
         *   4=sensor_code 5=sensor_label 6=sensor_kind
         *   7=value 8=unit 9=status... 14=message */
        char buf[512];
        strncpy(buf, linha, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        char *tok[16];
        int ntok = 0;
        char *p = strtok(buf, ";");
        while (p != NULL && ntok < 16) {
            tok[ntok++] = p;
            p = strtok(NULL, ";");
        }

        if (ntok >= 10) {
            printf("  %-14s %-22s %8s %-7s %-10s %s\n",
                   tok[4], tok[6], tok[7], tok[8], tok[9],
                   ntok >= 15 ? tok[14] : "");
        }
    }

    fclose(f);
    remove(FICH_API_TMP);
    printf("\n");
    pausar();
}

/*
 * Menu
 */
void menuSensores(Sistema *s)
{
    int opcao;
    do
    {
        limparEcra();
        printf("\n  ╔══════════════════════════════════════╗\n");
        printf("  ║    MÓDULO 3 — SENSORES               ║\n");
        printf("  ╠══════════════════════════════════════╣\n");
        printf("  ║  1. Importar sensores (ficheiro)     ║\n");
        printf("  ║  2. Listar todos os sensores         ║\n");
        printf("  ║  3. Pesquisar sensor por codigo      ║\n");
        printf("  ║  4. Listar sensores anomalos         ║\n");
        printf("  ║  5. Guardar sensores (binario)       ║\n");
        printf("  ║  6. Carregar sensores (binario)      ║\n");
        printf("  ║  7. Importar da API (RACK-01)        ║\n");
        printf("  ║  8. Consultar estado atual (API)     ║\n");
        printf("  ║  0. Voltar                           ║\n");
        printf("  ╚══════════════════════════════════════╝\n");
        printf("  Sensores registados: %d\n", s->totalSensores);

        opcao = lerInteiro("  Opcao", 0, 8);

        switch (opcao)
        {
        case 1: importarSensores(s);         break;
        case 2: listarSensores(s);           break;
        case 3: pesquisarSensor(s);          break;
        case 4: listarSensoresAnomalos(s);   break;
        case 5: guardarSensoresFicheiro(s);  pausar(); break;
        case 6: carregarSensoresFicheiro(s); pausar(); break;
        case 7: importarSensoresAPI(s);      break;
        case 8: consultarEstadoAPI();        break;
        case 0: break;
        }
    } while (opcao != 0);
}

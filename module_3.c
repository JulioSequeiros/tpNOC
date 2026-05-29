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

// Converte string de estado para enum
EstadoSensor parseEstadoSensor(const char *estadoStr)
{
    if (strcmp(estadoStr, "AVISO") == 0)      return SENSOR_AVISO;
    if (strcmp(estadoStr, "CRITICO") == 0)    return SENSOR_CRITICO;
    if (strcmp(estadoStr, "FALHA_REDE") == 0) return SENSOR_FALHA_REDE;
    return SENSOR_NORMAL;
}

// Regista mensagem no log dos sensores
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
        printf("  ║  0. Voltar                           ║\n");
        printf("  ╚══════════════════════════════════════╝\n");
        printf("  Sensores registados: %d\n", s->totalSensores);

        opcao = lerInteiro("  Opcao", 0, 6);

        switch (opcao)
        {
        case 1: importarSensores(s);         break;
        case 2: listarSensores(s);           break;
        case 3: pesquisarSensor(s);          break;
        case 4: listarSensoresAnomalos(s);   break;
        case 5: guardarSensoresFicheiro(s);  pausar(); break;
        case 6: carregarSensoresFicheiro(s); pausar(); break;
        case 0: break;
        }
    } while (opcao != 0);
}

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
EstadoSensor paraEstadoSensor(const char *estadoStr)
{
    if (strcmp(estadoStr, "AVISO") == 0) return SENSOR_AVISO;
    if (strcmp(estadoStr, "CRITICO") == 0) return SENSOR_CRITICO;
    if (strcmp(estadoStr, "FALHA_REDE") == 0) return SENSOR_FALHA_REDE;
    return SENSOR_NORMAL; // Default
}

// Regista mensagem no log do sensores
void registarLogSensores(const char *mensagem)
{
    FILE *f = fopen(FICH_LOG_SENSORES, "a");
    if (f == NULL) return;
    char datahora[MAX_DATAHORA];
    obterDataHoraAtual(datahora);
    fprintf(f, "[%s] %s\n", datahora, mensagem);  // Adiciona a mensagem
    fclose(f);
}

// Encontrar sensor por codigo
NodeSensor *encontrarSensorPorCodigo(const Sistema *s, const char *codigo)
{
    NodeSensor *atual = s->sensores;
    while (atual != NULL)
    {
        // Converter o código recebido (string) para inteiro
        int codigoInt = atoi(codigo);
        
        // Comparar com o campo 'codigo' (que é int)
        if (atual->dados.codigo == codigoInt)
            return atual;
        atual = atual->proximo;
    }
    return NULL;
}

// Cria incidente automatico para leitura anómala (sem input do utilizador)
static void __attribute__((unused)) criarIncidenteSensorAuto(Sistema *s, const LeituraSensor *leitura)
{
    NodeIncidente *novo = (NodeIncidente *)malloc(sizeof(NodeIncidente));
    if (novo == NULL) return;

    memset(novo, 0, sizeof(NodeIncidente));
    novo->dados.codigo = ++s->proximoCodigoInc;
    novo->dados.codigoEquipamento = leitura->codigo;

    // Converter o estado de string para enum antes de usar
    EstadoSensor estadoSensor = SENSOR_NORMAL;
    if (strcmp(leitura->estado, "AVISO") == 0)
        estadoSensor = SENSOR_AVISO;
    else if (strcmp(leitura->estado, "CRITICO") == 0)
        estadoSensor = SENSOR_CRITICO;
    else if (strcmp(leitura->estado, "FALHA_REDE") == 0)
        estadoSensor = SENSOR_FALHA_REDE;

    snprintf(novo->dados.descricao, MAX_DESC,
             "Leitura anomala no sensor %d: %.2f%s [%s]",
             leitura->codigo,
             leitura->valor,
             leitura->unidade,
             estadoSensorToString(estadoSensor));

    novo->dados.estado = PENDENTE;
    novo->dados.prioridade = 1;
    obterDataAtual(novo->dados.dataAbertura);
    strcpy(novo->dados.dataFecho, "-");

    novo->proximo = s->incidentes;
    s->incidentes = novo;
    s->totalIncidentes++;

    adicionarNaFilaAtendimento(s, novo->dados.codigo);

    printf(" [AUTO] Incidente #%d criado para sensor %d\n",
           novo->dados.codigo, leitura->codigo);
}
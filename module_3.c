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

// Resgista mensagem no log do sensores
void registarLogSensores(const char *mensagem)
{
    FILE *f = fopen(FICH_LOG_SENSORES, "a");
    if (f == NULL) return;
    char datahora[MAX_DATAHORA];
    obterDataHoraAtual(datahora);
    fprintf(f, "%s\n", datahora);
    fclose(f);
}

//Encontrar sensor por codigo
NodeSensor *encontrarSensorPorCodigo(const Sistema *s, const char *codigo)
{
    NodeSensor *atual = s->sensores;
    while (atual != NULL)
    {
        if (strcmp(atual->dados.codigoSensor, codigo) == 0)
            return atual;
        atual = atual->proximo;
    }
    return NULL;
}

// Cria incidente automatico para leitura anómala (sem input do utilizador)
static void criarInciddenteSensorAuto(Sistema *s, const LeituraSensor *leitura)
{
    NodeIncidente *novo =  (NodeIncidente *)malloc(sizeof(NodeIncidente));
    if (novo == NULL) return;

    memset(novo, 0, sizeof(NodeIncidente));
    novo->dados.codigo = s->proximoCodigoInc++;
    novo->dados.codigoEquipamento = 0;

    snprintf(novo->dados.descricao, MAX_DESC,
    "Leitura anómala no sensor %s: %.2f%s [%s]",
        leitura->codigoSensor,
        leitura->valor,
        leitura->unidade,
        estadoSensorToString(leitura->estado));

    novo->dados.estado = PENDENTE;
    obterDataAtual(novo->dados.dataAbertura);
    strcpy(novo->dados.dataFecho, "-");

    novo->proximo = s->incidentes;
    s->incidentes = novo;
    s->totalIncidentes++;

    printf(" [AUTO] Incidente #%d criado para sensor %s\n",
        novo->dados.codigo, leitura->codigoSensor);
}


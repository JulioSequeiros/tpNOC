#ifndef TPNOC_NOC_H
#define TPNOC_NOC_H

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ================= CONSTANTES ================= */

#define MAX_NOME        50
#define MAX_MARCA       30
#define MAX_MODELO      30
#define MAX_IP          16
#define MAX_LOCAL       50
#define MAX_MAC         18
#define MAX_DATA        11      // DD-MM-AAAA\0
#define MAX_HORA        9       // HH:MM:SS\0
#define MAX_DATAHORA    20      // DD-MM-AAAA HH:MM:SS\0
#define MAX_DESC        200
#define MAX_TECNICO     50
#define MAX_TIPO_CONFIG 50
#define MAX_VALOR       100
#define MAX_COD_SENSOR  30
#define MAX_UNIDADE     10

// Binários
#define FICH_EQUIPAMENTOS   "equipamentos.dat"
#define FICH_INCIDENTES     "incidentes.dat"
#define FICH_CONFIGURACOES  "configuracoes.dat"
#define FICH_SENSORES_DAT   "leituras_sensores.dat"

// Texto
#define FICH_PING           "resultado_ping.txt"
#define FICH_SENSORES_IN    "sensores_rack.txt"
#define FICH_LOG_MON        "log_monitorizacao.txt"
#define FICH_LOG_SENSORES   "log_sensores.txt"

/* ================= ENUMERAÇÕES ================= */

typedef enum {
    ROUTER = 1,
    SWITCH_L2,
    ACCESS_POINT,
    SERVIDOR_NAS,
    IMPRESSORA_REDE,
    CAMERA_IP,
    SENSOR,
    UPS
} TipoEquipamento;

typedef enum {
    OPERACIONAL = 1,
    EM_FALHA,
    EM_MANUTENCAO,
    DESATIVADO
} EstadoEquipamento;

typedef enum {
    PENDENTE = 1,
    EM_CURSO,
    RESOLVIDO
} EstadoIncidente;

// Módulo 3 — estado da leitura do sensor
typedef enum
{
    SENSOR_NORMAL = 1,
    SENSOR_AVISO,
    SENSOR_CRITICO,
    SENSOR_FALHA_REDE
} EstadoSensor;

/* ================= STRUCTS ================= */
// Modulo 1
typedef struct Equipamento {
    TipoEquipamento tipo;
    int codigo;
    char nome[MAX_NOME];
    char marca[MAX_MARCA];
    char modelo[MAX_MODELO];
    char ip[MAX_IP];
    char mac[MAX_MAC];
    char localizacao[MAX_LOCAL];
    EstadoEquipamento estado;
    char dataUltimaVerificacao[MAX_DATA];
} Equipamento;

typedef struct NodeEquipamento {
    Equipamento dados;
    struct NodeEquipamento *proximo;
} NodeEquipamento;

// Modulo 3
typedef struct
{
    char         codigoSensor[MAX_COD_SENSOR];
    char         tipo[MAX_NOME];            // ex: "Temperatura da rack"
    float        valor;
    char         unidade[MAX_UNIDADE];      // ex: "C", "%", "-"
    EstadoSensor estado;
    char         dataHoraLeitura[MAX_DATAHORA];
} LeituraSensor;

typedef struct NodeSensor
{
    LeituraSensor      dados;
    struct NodeSensor *proximo;
} NodeSensor;

//Modulo 4
typedef struct Incidente{
    int codigo;
    int codigoEquipamento;
    char descricao[MAX_DESC];
    EstadoIncidente estado;
    char dataAbertura[MAX_DATA];
    char dataFecho[MAX_DATA];
} Incidente;

typedef struct NodeIncidente {
    Incidente dados;
    struct NodeIncidente *proximo;
} NodeIncidente;

// Fila de atendimento FIFO (incidentes pendentes) — Módulo 4
typedef struct
{
    NodeIncidente *inicio;
    NodeIncidente *fim;
    int            total;
} FilaIncidentes;

//Modulo 5
typedef struct
{
    int  codigoEquipamento;
    char tipoConfiguracao[MAX_TIPO_CONFIG]; // ex: "IP", "Estado", "Localização"
    char valorAnterior[MAX_VALOR];
    char novoValor[MAX_VALOR];
    char tecnico[MAX_TECNICO];
    char dataHora[MAX_DATAHORA];
} Configuracao;

typedef struct NodeConfiguracao
{
    Configuracao            dados;
    struct NodeConfiguracao *proximo;       // topo da pilha = cabeça da lista
} NodeConfiguracao;

// Pilha de reversão LIFO — Módulo 5
typedef struct
{
    NodeConfiguracao *topo;
    int               total;
} PilhaConfiguracoes;

typedef struct
{
    // Módulo 1 — lista ligada de equipamentos
    NodeEquipamento   *equipamentos;
    int                totalEquipamentos;
    int                proximoCodigoEquip;

    // Módulo 3 — lista ligada de leituras de sensores
    NodeSensor        *sensores;
    int                totalSensores;

    // Módulo 4 — lista histórica + fila de atendimento
    NodeIncidente     *incidentes;
    FilaIncidentes     filaAtendimento;
    int                totalIncidentes;
    int                proximoCodigoInc;

    // Módulo 5 — pilha de configurações
    PilhaConfiguracoes pilhaConfiguracoes;
    int                totalConfiguracoes;
    int                proximoCodigoCfg;
} Sistema;

/* ================= PROTOTIPOS AUXILIARES ================= */
TipoEquipamento selecionarTipo(void);
EstadoEquipamento selecionarEstado(void);
int temIncidentePendente(Sistema *s, int codigo);
NodeEquipamento *encontrarPorCodigo(Sistema *s, int codigo);
void imprimirEquipamento(Equipamento *e);
const char *estadoEquipamentoToString(EstadoEquipamento estado);
const char *tipoToString(TipoEquipamento t);
NodeEquipamento *encontrarPorIP(Sistema *s, const char *ip);
NodeEquipamento *encontrarPorMAC(Sistema *s, const char *mac);


// ========== MENU PRINCIPAL =================//
void menuPrincipal(Sistema *s);
void menuEquipamentos(Sistema *s);
void menuConectividade(Sistema *s);
void menuIncidente(Sistema *s);
void menuRelatorios(Sistema *s);
void menuFicheiro(Sistema *s);
void menuEstatistica(const Sistema *s);

void menuSair(void);

/* ================= MODULO 1 ================= */

void             adicionarEquipamento(Sistema *s);
void             removerEquipamento(Sistema *s);
void             alterarEquipamento(Sistema *s);
void             alterarEstado(Sistema *s);
void             listarTodos(const Sistema *s);
void             listarPorTipo(const Sistema *s);
void             listarPorEstado(const Sistema *s);
void             pesquisarEquipamento(const Sistema *s);
// NodeEquipamento *encontrarPorCodigo(const Sistema *s, int codigo);
// void             imprimirEquipamento(const Equipamento *e);
void             imprimirCabecalho(void);
void             menuEstatistica(const Sistema *s);
void             menuEquipamento(Sistema *s);
void             guardarFicheiro(const Sistema *s);
void             carregarFicheiro(Sistema *s);

void              obterDataAtual(char *data);
void              obterDataHoraAtual(char *datahora);
void              limparBuffer(void);
int               lerInteiro(const char *prompt, int min, int max);
void              lerString(const char *prompt, char *dest, int maxLen);
const char       *tipoToString(TipoEquipamento tipo);
const char       *estadoEquipamentoToString(EstadoEquipamento estado);
const char       *estadoIncidenteToString(EstadoIncidente estado);
const char       *estadoSensorToString(EstadoSensor estado);
TipoEquipamento   selecionarTipo(void);
EstadoEquipamento selecionarEstado(void);
void              limparEcra(void);
void              pausar(void);

// Módulo 1 — lista ligada de equipamentos
NodeEquipamento   *equipamentos;
int                totalEquipamentos;
int                proximoCodigoEquip;

/* ================= MODULO 2 - CONECTIVIDADE ================= */

void executarPingEquipamento(Sistema *s);
void criarIncidenteAutomatico(Sistema *s);
void guardarResultadoPing(Sistema *s);
void verificarRespostaPing(Sistema *s);
void atualizarDataVerificacao(Sistema *s);
void alterarEstadoFalha(Sistema *s);
void registarTesteLog(Sistema *s);
void criarIncidenteAutoPorFalhaNoPing(Sistema *s);
void pingGeral(Sistema *s);
void mostrarFerramentasExtras(Sistema *s);

/* ================= MODULO 3 ================= */

void          importarSensores(Sistema *s);
void          listarSensores(const Sistema *s);
void          pesquisarSensor(const Sistema *s);
void          listarSensoresAnomalos(const Sistema *s);
NodeSensor   *encontrarSensorPorCodigo(const Sistema *s, const char *codigo);
void          guardarSensoresFicheiro(const Sistema *s);
void          carregarSensoresFicheiro(Sistema *s);
void          registarLogSensores(const char *mensagem);
EstadoSensor  parseEstadoSensor(const char *str);
void          menuSensores(Sistema *s);

/* ================= MODULO 4 - INCIDENTES ================= */

void criarIncidente(Sistema *s);
void criarIncidenteAutoPorFalhaNoPing(Sistema *s);
void criarIncidenteAutoPorLeituraAnomalaDoSensor(Sistema *s);

void gerirIncidentesPendentesNaFilaDeAtendimento(Sistema *s);
void processarProximoIncidenteNaFilaDeAtendimento(Sistema *s);
void resolverEConcluirIncidenteDataHora(Sistema *s);

void listarIncidentesPendentes(const Sistema *s);
void listarIncidentesEmCurso(const Sistema *s);
void listarIncidentesConcluidos(const Sistema *s);

void listarIncidentesPorEntidade(Sistema *s, char *entidadeId);
void listarIncidentesPorPrioridadeComParam(Sistema *s, int prioridade);

void guardarCarregarIncidentesFicheiroBinario(Sistema *s);

void outrasAtividadesRelevantes(Sistema *s);

/* ================= MODULO 5 ================= */
void registarConfiguracao(Sistema *s);
void consultarUltimaConfiguracao(const Sistema *s);
void consultarNConfiguracoes(const Sistema *s);
void reverterUltimaConfiguracao(Sistema *s);
void consultarHistoricoEquipamento(const Sistema *s);
void limparConfiguracoes(Sistema *s);
void guardarConfiguracoesFicheiro(const Sistema *s);
void carregarConfiguracoesFicheiro(Sistema *s);
void menuConfiguracoes(Sistema *s);

/* ================= MODULO 6 - RELATORIOS ================= */
void carregarFicheiro(Sistema *s);
void guardarFicheiro(const Sistema *s);
void importarLeiturasSensores(Sistema *s);
void guardarResultadosRede(Sistema *s);
void gerarRelatorioEstadoRede(Sistema *s);
void gerarRelatorioMensalIncidentes(Sistema *s);
void outrasAtividadesRelatorios(Sistema *s);

#endif
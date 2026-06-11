#ifndef TPNOC_NOC_H
#define TPNOC_NOC_H

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

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
#define MAX_VALOR       500
#define MAX_COD_SENSOR  30
#define MAX_UNIDADE     10

// Binários
#define FICHEIRO_DAT        "inventario.dat"
#define FICH_EQUIPAMENTOS   "equipamentos.dat"
#define FICH_INCIDENTES     "incidentes.dat"
#define FICH_CONFIGURACOES  "configuracoes.dat"
#define FICH_SENSORES_DAT   "leituras_sensores.dat"
#define FICH_TECNICOS       "tecnicos.dat"

// Texto
#define FICH_PING           "resultado_ping.txt"
#define FICH_SENSORES_IN    "sensores_rack.txt"
#define FICH_LOG_MON        "log_monitorizacao.txt"
#define FICH_LOG_SENSORES   "log_sensores.txt"

/* ================= ENUMERAÇÕES ================= */

typedef enum
{
    ROUTER = 1,
    SWITCH_L2,
    ACCESS_POINT,
    SERVIDOR_NAS,
    IMPRESSORA_REDE,
    CAMERA_IP,
    SENSOR,
    UPS
} TipoEquipamento;

typedef enum
{
    OPERACIONAL = 1,
    EM_FALHA,
    EM_MANUTENCAO,
    DESATIVADO
} EstadoEquipamento;

// Módulo 4 — estados do incidente (3 estados conforme enunciado)
typedef enum
{
    PENDENTE = 1,
    EM_CURSO,
    CONCLUIDO
} EstadoIncidente;

// Módulo 4 — prioridade
typedef enum
{
    PRIORIDADE_BAIXA = 1,
    PRIORIDADE_MEDIA,
    PRIORIDADE_ALTA,
    PRIORIDADE_CRITICA
} PrioridadeIncidente;

// Módulo 4 — origem do incidente
typedef enum
{
    ORIGEM_MANUAL = 1,
    ORIGEM_PING,
    ORIGEM_SENSOR
} OrigemIncidente;

// Módulo 3 — estado da leitura do sensor
typedef enum
{
    SENSOR_NORMAL = 1,
    SENSOR_AVISO,
    SENSOR_CRITICO,
    SENSOR_FALHA_REDE
} EstadoSensor;

/* ================= STRUCTS ================= */

// Modulo 1 - Equipamento
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

// Modulo 3 - Sensor (versão completa)
typedef struct Sensor {
    char codigo[MAX_COD_SENSOR];
    int codigoEquipamento;
    char tipo[50];          // temperatura, humidade, corrente, etc.
    float valorMinimo;
    float valorMaximo;
    float valorAtual;
    char unidade[20];
    char estado[20];        // normal, alerta, critico
    char ultimaLeitura[MAX_DATA];
} Sensor;

typedef struct NodeSensor {
    Sensor dados;
    struct NodeSensor *proximo;
} NodeSensor;

// Modulo 3 - LeituraSensor (para histórico)
typedef struct LeituraSensor {
    int codigo;
    char tipo[50];
    float valor;
    char unidade[10];
    char estado[20];
    char dataHora[MAX_DATA];
    struct LeituraSensor *proximo;
} LeituraSensor;

// Modulo 4 - Incidente (com prioridade)
typedef struct Incidente {
    int codigo;
    int codigoEquipamento;
    char descricao[MAX_DESC];
    EstadoIncidente estado;
    int prioridade;         // 1=Baixa, 2=Média, 3=Alta, 4=Crítica
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

// Fila simples para atendimento (versão alternativa)
typedef struct FilaAtendimento {
    int codigoIncidente;
    struct FilaAtendimento *proximo;
} FilaAtendimento;

// Modulo 5 - Configuração
typedef struct
{
    int  codigoEquipamento;
    char tipoConfiguracao[MAX_TIPO_CONFIG];
    char valorAnterior[MAX_VALOR];
    char novoValor[MAX_VALOR];
    char tecnico[MAX_TECNICO];
    char dataHora[MAX_DATAHORA];
} Configuracao;

typedef struct NodeConfiguracao
{
    Configuracao            dados;
    struct NodeConfiguracao *proximo;
} NodeConfiguracao;

// Pilha de reversão LIFO — Módulo 5
typedef struct
{
    NodeConfiguracao *topo;
    int               tamanho;
} PilhaConfiguracoes;

// Estruturas para Testes Pendentes (Módulo 2)
typedef struct TestePendente {
    int codigoEquipamento;
    char ip[20];
    char nomeEquip[MAX_NOME];
    struct TestePendente *proximo;
} TestePendente;

// Estruturas para Histórico de Comandos (Módulo 2)
typedef struct ComandoExecutado {
    char comando[300];
    char dataHora[MAX_DATA];
    int codigoEquipamento;
    int respondeu;          // 1 = online, 0 = offline
    struct ComandoExecutado *anterior;
} ComandoExecutado;

/* ================= ESTRUTURA PRINCIPAL SISTEMA ================= */
typedef struct Sistema {
    // Módulo 1 — lista ligada de equipamentos
    NodeEquipamento   *equipamentos;
    int                totalEquipamentos;
    int                proximoCodigoEquip;

    // Módulo 3 — lista ligada de sensores
    NodeSensor        *sensores;
    int                totalSensores;
    int                proximoCodigoSensor;

    // Módulo 4 — lista histórica + fila de atendimento
    NodeIncidente     *incidentes;
    FilaIncidentes     filaAtendimento;
    int                totalIncidentes;
    int                proximoCodigoInc;

    // Módulo 5 — pilha de configurações
    PilhaConfiguracoes pilhaConfiguracoes;
    int                totalConfiguracoes;
    int                proximoCodigoCfg;

    // Módulo 2 — fila de testes pendentes
    struct {
        struct TestePendente *frente;
        struct TestePendente *tras;
        int tamanho;
    } filaTestes;

    // Módulo 2 — pilha de comandos executados
    struct {
        struct ComandoExecutado *topo;
        int tamanho;
    } pilhaComandos;

    // Módulo 4 — fila de atendimento alternativa
    FilaAtendimento *filaAtendimentoPtr;
    FilaAtendimento *filaAtendimentoTras;

    // Autenticação — técnico com sessão activa
    char tecnicoLogado[MAX_TECNICO];

} Sistema;

// Credencial de técnico (guardada em ficheiro binário)
typedef struct {
    char         username[MAX_NOME];
    char         nomeCompleto[MAX_NOME];
    unsigned int passwordHash;
} Credencial;

/* ================= ALIASES DE COMPATIBILIDADE ================= */
#define INC_PENDENTE   PENDENTE
#define INC_EM_CURSO   EM_CURSO
#define INC_CONCLUIDO  CONCLUIDO
#define RESOLVIDO      CONCLUIDO

/* ================= MENU PRINCIPAL ================= */
void menuPrincipal(Sistema *s);



/* ================= AUTENTICACAO ================= */
void inicializarTecnicos(void);
int  autenticarTecnico(Sistema *s);
void menuGestaoPerfil(Sistema *s);

/* ================= MODULO 1 - EQUIPAMENTOS ================= */
void adicionarEquipamento(Sistema *s);
void removerEquipamento(Sistema *s);
void alterarEquipamento(Sistema *s);
void alterarEstado(Sistema *s);
void listarTodos(const Sistema *s);
void listarPorTipo(const Sistema *s);
void listarPorEstado(const Sistema *s);
void pesquisarEquipamento(Sistema *s);
void listarEquipamentos(Sistema *s);  // alias para listarTodos
NodeEquipamento *encontrarPorCodigo(Sistema *s, int codigo);
NodeEquipamento *encontrarPorIP(Sistema *s, const char *ip);
NodeEquipamento *encontrarPorMAC(Sistema *s, const char *mac);
void imprimirEquipamento(Equipamento *e);
void imprimirCabecalho(void);
void menuEstatistica(const Sistema *s);
void menuEquipamento(Sistema *s);
void guardarEquipamento(const Sistema *s);
void carregarEquipamento(Sistema *s);
void guardarInventario(Sistema *s);   // alias

/* ================= MODULO 2 - CONECTIVIDADE ================= */
void menuConectividade(Sistema *s);
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
void executarComandosRedeExtra(Sistema *s);
void obterInfoRedeLocal(void);
void obterTabelaARP(void);
void obterResolucaoDNS(void);
void obterRotaDestino(void);

/* ================= MODULO 3 - SENSORES ================= */
void menuSensores(Sistema *s);
void importarSensores(Sistema *s);
void listarSensores(const Sistema *s);
void pesquisarSensor(const Sistema *s);
void listarSensoresAnomalos(const Sistema *s);
NodeSensor *encontrarSensorPorCodigo(const Sistema *s, const char *codigo);
void guardarSensoresFicheiro(const Sistema *s);
void carregarSensoresFicheiro(Sistema *s);
void registarLogSensores(const char *mensagem);
EstadoSensor parseEstadoSensor(const char *str);
void importarSensoresAPI(Sistema *s);
void consultarEstadoAPI(void);


/* ================= MODULO 4 - INCIDENTES ================= */
void menuIncidente(Sistema *s);
void criarIncidente(Sistema *s);
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

/* Funções da fila de atendimento */
void inicializarFilaAtendimento(Sistema *s);
void adicionarNaFilaAtendimento(Sistema *s, int codigoIncidente);
int removerDaFilaAtendimento(Sistema *s);
void listarFilaAtendimento(Sistema *s);

/* ================= MODULO 5 - CONFIGURAÇÕES ================= */
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
void menuRelatorios(Sistema *s);
void carregarFicheiro(Sistema *s);
void guardarFicheiro(const Sistema *s);
void importarLeiturasSensores(Sistema *s);
void guardarResultadosRede(Sistema *s);
void gerarRelatorioEstadoRede(Sistema *s);
void gerarRelatorioMensalIncidentes(Sistema *s);
void outrasAtividadesRelatorios(Sistema *s);
void listarEquipamentosOrdenados(Sistema *s);
void resumoTextualEstadoRede(Sistema *s);

/* Funções para ficheiros binários completos */
void carregarTodosDados(Sistema *s);
void guardarTodosDados(Sistema *s);
void carregarEquipamentos(Sistema *s);
void guardarEquipamentos(Sistema *s);
void carregarIncidentes(Sistema *s);
void guardarIncidentes(Sistema *s);

/* ================= Modulo 7 - Ficheiros ================= */
void menuFicheiro(Sistema *s);

/* ================= FUNÇÕES UTILITÁRIAS ================= */
void obterDataAtual(char *data);
void obterDataHoraAtual(char *datahora);
void limparBuffer(void);
void limparBufferLocal(void);
int lerInteiro(const char *prompt, int min, int max);
void lerString(const char *prompt, char *dest, int maxLen);
const char *tipoToString(TipoEquipamento tipo);
const char *estadoEquipamentoToString(EstadoEquipamento estado);
const char *estadoIncidenteToString(EstadoIncidente estado);
const char *estadoSensorToString(EstadoSensor estado);
TipoEquipamento selecionarTipo(void);
EstadoEquipamento selecionarEstado(void);
int temIncidentePendente(Sistema *s, int codigo);
void limparEcra(void);
void pausar(void);

#endif /* TPNOC_NOC_H */
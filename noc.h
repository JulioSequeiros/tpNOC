#ifndef TPNOC_NOC_H
#define TPNOC_NOC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ================= CONSTANTES ================= */

#define MAX_NOME 50
#define MAX_MARCA 30
#define MAX_MODELO 30
#define MAX_IP 16
#define MAX_LOCAL 80
#define MAX_MAC 18
#define MAX_DATA 20
#define MAX_DESC 100

#define FICHEIRO_DAT "inventario.dat"

/* ================= ENUMS ================= */

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

/* ================= STRUCTS ================= */

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

typedef struct {

    int codigo;
    int codigoEquipamento;

    char descricao[MAX_DESC];

    EstadoIncidente estado;
    
    int prioridade;  // ← NOVO: 1=Alta, 2=Média, 3=Baixa

    char dataAbertura[MAX_DATA];
    char dataFecho[MAX_DATA];

} Incidente;

typedef struct NodeEquipamento {

    Equipamento dados;
    struct NodeEquipamento *proximo;

} NodeEquipamento;

typedef struct NodeIncidente {

    Incidente dados;
    struct NodeIncidente *proximo;

} NodeIncidente;

/* ================= ESTRUTURAS PARA FILA (Testes Pendentes) ================= */

typedef struct TestePendente {
    int codigoEquipamento;
    char ip[20];
    char nomeEquip[MAX_NOME];
    struct TestePendente *proximo;
} TestePendente;

/* ================= ESTRUTURAS PARA PILHA (Histórico de Comandos) ================= */

typedef struct ComandoExecutado {
    char comando[300];
    char dataHora[MAX_DATA];
    int codigoEquipamento;
    int respondeu;          // 1 = online, 0 = offline
    struct ComandoExecutado *anterior;
} ComandoExecutado;

/* ================= ESTRUTURAS PARA FILA DE ATENDIMENTO DE INCIDENTES ================= */

typedef struct FilaAtendimento {
    int codigoIncidente;
    struct FilaAtendimento *proximo;
} FilaAtendimento;

/* ================= ESTRUTURA SISTEMA (CORRIGIDA) ================= */

typedef struct Sistema {
    NodeEquipamento *equipamentos;
    NodeIncidente *incidentes;
    
    int totalEquipamentos;
    int totalIncidentes;
    
    int proximoCodigoEquip;
    int proximoCodigoInc;
    
    /* FILA para testes pendentes (Módulo 2) */
    struct {
        struct TestePendente *frente;
        struct TestePendente *tras;
        int tamanho;
    } filaTestes;
    
    /* PILHA para histórico de comandos (Módulo 2) */
    struct {
        struct ComandoExecutado *topo;
        int tamanho;
    } pilhaComandos;
    
    /* ========== NOVOS CAMPOS PARA O MÓDULO 4 ========== */
    /* FILA para atendimento de incidentes */
    FilaAtendimento *filaAtendimento;      // Cabeça da fila (FIFO)
    FilaAtendimento *filaAtendimentoTras;  // Fim da fila
    
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



/* ================= UTIL ================= */
void imprimirCabecalho(void);
void obterDataAtual(char *data);
void limparEcra(void);
void pausar(void);

void limparBuffer(void);
void limparBufferLocal(void);

int lerInteiro(const char *prompt, int min, int max);

void lerString(const char *prompt, char *dest, int maxLen);


// ========== MENU PRINCIPAL =================//
void menuPrincipal(Sistema *s);
void menuEquipamentos(Sistema *s);
void menuConectividade(Sistema *s);
void menuIncidente(Sistema *s);
void menuRelatorios(Sistema *s);
void menuFicheiro(Sistema *s);
void menuEstatistica(const Sistema *s);   
void menuSair(void);







/* ================= MODULO 1 - (VAZIO) ================= */
void adicionarEquipamento(Sistema *s);
void removerEquipamento(Sistema *s);
void alterarEquipamento(Sistema *s);
void alterarEstado(Sistema *s);
void listarEquipamentos(Sistema *s);
void listarPorTipo(Sistema *s);
void listarPorEstado(Sistema *s);
void pesquisarEquipamento(Sistema *s);
void guardarInventario(Sistema *s);


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
void executarComandosRedeExtra(Sistema *s);
void obterInfoRedeLocal(void);
void obterTabelaARP(void);
void obterResolucaoDNS(void);
void obterRotaDestino(void);

/* ================= MODULO 3 - (VAZIO) ================= */








/* ================= MODULO 4 - INCIDENTES ================= */

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

/* ========== NOVOS PROTÓTIPOS PARA A FILA ========== */
void inicializarFilaAtendimento(Sistema *s);
void adicionarNaFilaAtendimento(Sistema *s, int codigoIncidente);
int removerDaFilaAtendimento(Sistema *s);
void listarFilaAtendimento(Sistema *s);

/* ================= MODULO 5 - (VAZIO) ================= */



/* ================= MODULO 6 - RELATORIOS ================= */
/* ================= MODULO 6 - RELATORIOS ================= */

/* Estrutura para leituras de sensores (opcional - para histórico) */
typedef struct LeituraSensor {
    int codigo;
    char tipo[50];
    float valor;
    char unidade[10];
    char estado[20];
    char dataHora[MAX_DATA];
    struct LeituraSensor *proximo;
} LeituraSensor;

/* Funções principais do Módulo 6 */
void carregarFicheiro(Sistema *s);
void guardarFicheiro(const Sistema *s);
void importarLeiturasSensores(Sistema *s);
void guardarResultadosRede(Sistema *s);
void gerarRelatorioEstadoRede(Sistema *s);
void gerarRelatorioMensalIncidentes(Sistema *s);
void outrasAtividadesRelatorios(Sistema *s);

/* ========== NOVAS FUNÇÕES PARA FICHEIROS BINÁRIOS COMPLETOS ========== */
void carregarTodosDados(Sistema *s);
void guardarTodosDados(Sistema *s);

void carregarEquipamentos(Sistema *s);
void guardarEquipamentos(Sistema *s);

void carregarIncidentes(Sistema *s);
void guardarIncidentes(Sistema *s);

#endif
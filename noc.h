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
#define MAX_LOCAL 50
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

typedef struct {

    NodeEquipamento *equipamentos;
    NodeIncidente *incidentes;

    int totalEquipamentos;
    int totalIncidentes;

    int proximoCodigoEquip;
    int proximoCodigoInc;

} Sistema;

/* ================= UTIL ================= */

void obterDataAtual(char *data);

void limparBuffer(void);
void limparBufferLocal(void);

int lerInteiro(const char *prompt, int min, int max);

void lerString(const char *prompt, char *dest, int maxLen);


/* ================= MENUS ================= */

void menuPrincipal(Sistema *s);
void menuEquipamento(Sistema *s);
void menuConectividade(Sistema *s);
void menuIncidentes(Sistema *s);
void menuRelatorios(Sistema *s);


/* ================= MODULO 1 - (VAZIO) ================= */



/* ================= MODULO 2 - CONECTIVIDADE ================= */

void executarPingEquipamento(Sistema *s);
void guardarResultadoPing(Sistema *s);
void verificarRespostaPing(Sistema *s);
void atualizarDataVerificacao(Sistema *s);
void alterarEstadoFalha(Sistema *s);
void registarTesteLog(Sistema *s);
void criarIncidenteAutoPorFalhaNoPing(Sistema *s);
void pingGeral(Sistema *s);
void mostrarFerramentasExtras(Sistema *s);

/* ================= MODULO 3 - (VAZIO) ================= */



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

/* ================= MODULO 5 - (VAZIO) ================= */



/* ================= MODULO 6 - RELATORIOS ================= */
void carregarFicheiro(Sistema *s);
void guardarFicheiro(const Sistema *s);
void importarLeiturasSensores(Sistema *s);
void guardarResultadosRede(Sistema *s);
void gerarRelatorioEstadoRede(Sistema *s);
void gerarRelatorioMensalIncidentes(Sistema *s);
void outrasAtividadesRelatorios(Sistema *s);

#endif
//
// Created by surty on 08/05/26.
//

#ifndef TPNOC_NOC_H
#define TPNOC_NOC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NOME 50
#define MAX_MARCA 30
#define MAX_MODELO 30
#define MAX_IP 16
#define MAX_LOCAL 50
#define MAX_MAC 18
#define MAX_DATA 11
#define MAX_DESC 200
#define FICHEIRO_DAT "iventario.dat"

//enums
typedef enum
{
    ROUTER=1,
    SWITCH_L2,
    ACESS_POINT,
    SERVIDOR_NAS,
    IMPRESSORA_REDE,
    CAMERA_IP,
    SENSOR,
    UPS
}TipoEquipamento;

typedef enum
{
    OPERACIONAL = 1,
    EM_FALHA,
    EM_MANUTENCAO,
    DESATIVADO
}EstadoEquipamento;

typedef enum
{
    PENDENTE = 1,
    RESOLVIDO
}EstadoIncidente;

// ESTRETURA DE DADOS

// IDENTIFICACAO DO EQUIPAMENTO
typedef struct Equipamentos
{
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
}Equipamento;

//
typedef struct
{
    int codigo;
    int codigoEquipamento;
    char descricao[MAX_DESC];
    EstadoIncidente estado;
    char dataAbertura[MAX_DATA];
    char dataFecho[MAX_DATA];
}Incidente;

typedef struct NodeEquipamento
{
    Equipamento dados;
    struct NodeEquipamento *proximo;
}NodeEquipamento;

typedef struct NodeIncidente
{
    Incidente dados;
    struct NodeIncidente *proximo;
}NodeIncidente;

typedef struct
{
    NodeEquipamento *equipamentos;
    NodeIncidente *incidentes;
    int totalEquipamentos;
    int totalIncidentes;
    int proximoCodigoEquip;
    int proximoCodigoInc;
}Sistema;

// ========== UTIL ==========
void obterDataAtual(char *data);
void limparBuffer(void);
int lerInteiro(const char *prompt, int min, int max);
void lerString(const char *prompt, char *dest, int maxLen);
const char *tipoToString(TipoEquipamento tipo);
const char *estadoEquipamentoToString(EstadoEquipamento estado);
const char *estadoIncidenteToString(EstadoIncidente estado);
TipoEquipamento selecionarTipo(void);
EstadoEquipamento selecionarEstado(void);
void limparEcra(void);
void pausar(void);

// ========== EQUIPAMENTOS ==========
void adicionarEquipamento(Sistema *s);
void removerEquipamento(Sistema *s);
void alterarEquipamento(Sistema *s);
void alterarEstado(Sistema *s);
void listarTodos(const Sistema *s);
void listarPorTipo(const Sistema *s);
void listarPorEstado(const Sistema *s);
void pesquisarEquipamento(const Sistema *s);
NodeEquipamento *encontrarPorCodigo(const Sistema *s, int codigo);
void imprimirEquipamento(const Equipamento *e);
void imprimirCabecalho(void);

// ========== INCIDENTES ==========
void adicionarIncidentes(Sistema *s);
void resolverIncidente(Sistema *s);
void listarIncidentes(const Sistema *s);
void listarIncidentesPorEquipamento(const Sistema *s);
void listarIncidentesPendentes(const Sistema *s);
int temIncidentePendente(const Sistema *s, int codigoEquipamento);
NodeIncidente *encontrarPorCodigoInc(const Sistema *s, int codigo);

// ========== FICHEIROS ==========
void guardarFicheiro(const Sistema *s);
void carregarFicheiro(Sistema *s);

// ========== MODULO 2 - CONECTIVIDADE ==========
void menuConectividade(Sistema *s);
void executarPingEquipamento(Sistema *s);
void guardarResultadoPing(Sistema *s);
void verificarRespostaPing(Sistema *s);
void pingGeral(Sistema *s);
void mostrarEstatisticas(Sistema *s);
int analisarResultadoPingArquivo(const char *arquivo);
void registarLogMonitorizacaoTexto(const char *mensagem);
void comandoIpConfig(Sistema *s);
void comandoArp(Sistema *s);
void comandoNslookup(Sistema *s);
void comandoTracert(Sistema *s);

// ========== MODULO 4 - INCIDENTES (completo) ==========
void menuIncidentes(Sistema *s);
void criarIncidenteManual(Sistema *s);
void criarIncidenteAutomaticoPing(Sistema *s, int codigoEquipamento);
void criarIncidenteAutomaticoSensor(Sistema *s, char *codigoSensor, char *descricao);
void processarProximoIncidente(Sistema *s);
void resolverIncidenteWrapper(Sistema *s);
void listarIncidentesPorPrioridade(Sistema *s);
void listarIncidentesConcluidos(Sistema *s);
void listarIncidentesEmCurso(Sistema *s);
const char *prioridadeToString(int prioridade);
int temIncidentePendenteEquipamento(const Sistema *s, int codigoEquipamento);

// ========== MODULO 6 - RELATORIOS ==========
void menuRelatorios(Sistema *s);
void gerarRelatorioEstadoRede(int mes, int ano);
void gerarRelatorioMensalIncidentes(int mes, int ano);

// ========== MENU PRINCIPAL ==========
void menuPrincipal(Sistema *s);
void menuEquipamento(Sistema *s);
void menuIncidente(Sistema *s);
void menuFicheiro(Sistema *s);
void menuEstatistica(const Sistema *s);
void menuSair(void);

#endif //TPNOC_NOC_H
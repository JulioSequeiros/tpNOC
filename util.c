#include "noc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void limparBufferLocal(void) {

    int c;

    while ((c = getchar()) != '\n' && c != EOF);
}


void limparBuffer(void) {

    limparBufferLocal();
}


void obterDataAtual(char *data) {

    time_t t;

    struct tm *tm_info;

    /* Obter tempo atual */

    t = time(NULL);

    /* Converter para estrutura local */

    tm_info = localtime(&t);

    /*
     * Formatar data/hora
     *
     * Exemplo:
     * 15/05/2026 14:35:20
     */

    strftime(data,
             MAX_DATA,
             "%d/%m/%Y %H:%M:%S",
             tm_info);
}


int lerInteiro(const char *prompt,
               int min,
               int max) {

    int valor;

    while (1) {

        printf("%s", prompt);

        if (scanf("%d", &valor) != 1) {

            printf("Entrada invalida!\n");

            limparBuffer();

            continue;
        }

        limparBuffer();

        if (valor < min || valor > max) {

            printf("Valor fora do intervalo permitido!\n");

            continue;
        }

        return valor;
    }
}


void lerString(const char *prompt, char *dest, int maxLen)
{
    printf("%s: ", prompt);
    fflush(stdout);

    if (fgets(dest, maxLen, stdin) == NULL)
    {
        dest[0] = '\0';
        return;
    }

    dest[strcspn(dest, "\n")] = '\0';
}

void limparEcra(void)
{
    system("cls"); // Windows
}
void pausar(void)
{
    system("pause");
}

const char *tipoToString(TipoEquipamento t)
{
    switch (t)
    {
        case ROUTER: return "ROUTER";
        case SWITCH_L2: return "SWITCH_L2";
        case ACCESS_POINT: return "ACCESS_POINT";
        case SERVIDOR_NAS: return "SERVIDOR_NAS";
        case IMPRESSORA_REDE: return "IMPRESSORA_REDE";
        case CAMERA_IP: return "CAMERA_IP";
        case SENSOR: return "SENSOR";
        case UPS: return "UPS";
        default: return "DESCONHECIDO";
    }
}
TipoEquipamento selecionarTipo(void) {
    int op;

    printf("\n=== Selecionar Tipo de Equipamento ===\n");
    printf("1 - Router\n");
    printf("2 - Switch L2\n");
    printf("3 - Access Point\n");
    printf("4 - Servidor NAS\n");
    printf("5 - Impressora de Rede\n");
    printf("6 - Camera IP\n");
    printf("7 - Sensor\n");
    printf("8 - UPS\n");

    op = lerInteiro("Escolha o tipo: ", 1, 8);

    switch (op) {
        case 1: return ROUTER;
        case 2: return SWITCH_L2;
        case 3: return ACCESS_POINT;
        case 4: return SERVIDOR_NAS;
        case 5: return IMPRESSORA_REDE;
        case 6: return CAMERA_IP;
        case 7: return SENSOR;
        case 8: return UPS;
        default: return ROUTER; // fallback por segurança
    }
}
EstadoEquipamento selecionarEstado(void) {
    int op;

    printf("\n=== Selecionar Estado do Equipamento ===\n");
    printf("1 - Operacional\n");
    printf("2 - Em Falha\n");
    printf("3 - Em Manutencao\n");
    printf("4 - Desativado\n");

    op = lerInteiro("Escolha o estado: ", 1, 4);

    switch (op) {
        case 1: return OPERACIONAL;
        case 2: return EM_FALHA;
        case 3: return EM_MANUTENCAO;
        case 4: return DESATIVADO;
        default: return OPERACIONAL; // segurança
    }
}

int temIncidentePendente(Sistema *s, int codigo)
{
    NodeIncidente *atual = s->incidentes;

    while (atual != NULL)
    {
        if (atual->dados.codigoEquipamento == codigo &&
            atual->dados.estado != RESOLVIDO)
        {
            return 1;
        }

        atual = atual->proximo;
    }

    return 0;
}

NodeEquipamento *encontrarPorCodigo(Sistema *s, int codigo){
    NodeEquipamento *atual = s->equipamentos;

    while (atual != NULL)
    {
        if (atual->dados.codigo == codigo)
        {
            return atual;
        }

        atual = atual->proximo;
    }

    return NULL;
}

void imprimirEquipamento(Equipamento *e){
    printf("Codigo: %d\n", e->codigo);
    printf("Nome: %s\n", e->nome);
    printf("Tipo: %s\n", tipoToString(e->tipo));
    printf("Marca: %s\n", e->marca);
    printf("Modelo: %s\n", e->modelo);
    printf("IP: %s\n", e->ip);
    printf("MAC: %s\n", e->mac);
    printf("Localizacao: %s\n", e->localizacao);
    printf("Estado: %s\n", estadoEquipamentoToString(e->estado));
    printf("Data da ultima verificacao: %s\n", e->dataUltimaVerificacao);
}

const char *estadoEquipamentoToString(EstadoEquipamento estado){
    switch (estado)
    {
        case OPERACIONAL: return "OPERACIONAL";
        case EM_FALHA: return "EM_FALHA";
        case EM_MANUTENCAO: return "EM_MANUTENCAO";
        case DESATIVADO: return "DESATIVADO";
        default: return "DESCONHECIDO";
    }

}

NodeEquipamento *encontrarPorIP(Sistema *s, const char *ip){
    NodeEquipamento *atual = s->equipamentos;

    while (atual != NULL)
    {
        if (strcmp(atual->dados.ip, ip) == 0)
        {
            return atual;
        }

        atual = atual->proximo;
    }

    return NULL;
}

NodeEquipamento *encontrarPorMAC(Sistema *s, const char *mac){
    NodeEquipamento *atual = s->equipamentos;

    while (atual != NULL)
    {
        if (strcmp(atual->dados.mac, mac) == 0)
        {
            return atual;
        }

        atual = atual->proximo;
    }

    return NULL;
}

void imprimirCabecalho(void)
{
    printf("=========================================\n");
    printf("      INVENTARIO DE EQUIPAMENTOS        \n");
    printf("=========================================\n");
}
//
// Created by surty on 29/05/26.
//
// Modulo 5: Registo de configurações e pilha de reversao

#include "noc.h"

/*
 *  Helpers Privados
 */

static void empilhar(PilhaConfiguracoes *pilha, Configuracao dados)
{
    NodeConfiguracao *novo = malloc(sizeof(NodeConfiguracao));
    if (novo == NULL) return;
    novo->dados   = dados;
    novo->proximo = pilha->topo;
    pilha->topo   = novo;
    pilha->tamanho++;
}

static NodeConfiguracao *desempilhar(PilhaConfiguracoes *pilha)
{
    if (pilha->topo == NULL) return NULL;
    NodeConfiguracao *no = pilha->topo;
    pilha->topo = no->proximo;
    pilha->tamanho--;
    return no;  /* caller must free */
}

static void imprimirConfiguracaoDetalhe(const Configuracao *c, int seq)
{
    printf("  ┌─────────────────────────────────────────────────────┐\n");
    printf("  │ #%-3d  Equip #%-4d   %-31s│\n",
           seq, c->codigoEquipamento, c->dataHora);
    printf("  │ Tipo   : %-44s│\n", c->tipoConfiguracao);
    printf("  │ Antes  : %-44s│\n", c->valorAnterior);
    printf("  │ Depois : %-44s│\n", c->novoValor);
    printf("  │ Tecnico: %-44s│\n", c->tecnico);
    printf("  └─────────────────────────────────────────────────────┘\n");
}

/* Tenta aplicar valorAnterior ao campo correto do equipamento.
 * Retorna 1 se aplicado com sucesso, 0 se tipo desconhecido. */

static int aplicarReversao(NodeEquipamento *eq, const Configuracao *cfg)
{
    char tipo[MAX_TIPO_CONFIG];
    strncpy(tipo, cfg->tipoConfiguracao, MAX_TIPO_CONFIG - 1);
    tipo[MAX_TIPO_CONFIG - 1] = '\0';
    for (int i = 0; tipo[i]; i++)
        tipo[i] = (char)toupper((unsigned char)tipo[i]);

    if (strcmp(tipo, "IP") == 0)
    {
        strncpy(eq->dados.ip, cfg->valorAnterior, MAX_IP - 1);
        eq->dados.ip[MAX_IP - 1] = '\0';
        return 1;
    }
    if (strcmp(tipo, "NOME") == 0)
    {
        strncpy(eq->dados.nome, cfg->valorAnterior, MAX_NOME - 1);
        eq->dados.nome[MAX_NOME - 1] = '\0';
        return 1;
    }
    if (strcmp(tipo, "LOCALIZACAO") == 0)
    {
        strncpy(eq->dados.localizacao, cfg->valorAnterior, MAX_LOCAL - 1);
        eq->dados.localizacao[MAX_LOCAL - 1] = '\0';
        return 1;
    }
    if (strcmp(tipo, "MAC") == 0)
    {
        strncpy(eq->dados.mac, cfg->valorAnterior, MAX_MAC - 1);
        eq->dados.mac[MAX_MAC - 1] = '\0';
        return 1;
    }
    return 0;
}

/*
 *  1. Registar nova configuração
 */

void registarConfiguracao(Sistema *s)
{
    if (s->equipamentos == NULL) {
        printf("\n  [!] Nao existem equipamentos registados.\n");
        pausar();
        return;
    }

    limparEcra();
    printf("\n  ╔══════════════════════════════════════════════════╗\n");
    printf("  ║   Registar Configuracao                          ║\n");
    printf("  ╚══════════════════════════════════════════════════╝\n\n");

    int cod = lerInteiro("  Codigo do equipamento", 1, s->proximoCodigoEquip - 1);
    NodeEquipamento *eq = encontrarPorCodigo(s, cod);
    if (eq == NULL)
    {
        printf("\n  [!] Equipamento #%d nao encontrado.\n", cod);
        pausar();
        return;
    }
    printf("  Equipamento: %s\n\n", eq->dados.nome);

    Configuracao cfg;
    memset(&cfg, 0, sizeof(Configuracao));
    cfg.codigoEquipamento = cod;

    lerString("  Tipo de configuracao (e.g., IP, NOME, LOCALIZACAO, MAC)",
        cfg.tipoConfiguracao, MAX_TIPO_CONFIG);
    lerString("  Valor anterior", cfg.valorAnterior, MAX_VALOR);
    lerString("  Novo valor", cfg.novoValor, MAX_VALOR);
    lerString("  Localizacao", cfg.tecnico, MAX_LOCAL);
    obterDataHoraAtual(cfg.dataHora);

    empilhar(&s->pilhaConfiguracoes, cfg);
    s->totalConfiguracoes = s->pilhaConfiguracoes.tamanho;
    s->proximoCodigoCfg++;

    printf("  [OK] Configuracao registada. Pilha: %d entrada(s).\n",
           s->pilhaConfiguracoes.tamanho);
    pausar();
}

/*
 *  2. Consultar ultima configuração (topo da pilha)
 */

void consultarUltimaConfiguracao(const Sistema *s)
{
    limparEcra();
    printf("\n  ╔══════════════════════════════════════════════════╗\n");
    printf("  ║   Consultar Ultima Configuracao (topo da pilha) ║\n");
    printf("  ╚══════════════════════════════════════════════════╝\n\n");

    if (s->pilhaConfiguracoes.topo == NULL) {
        printf("  [!] Pilha vazia — nenhuma configuracao registada.\n");
        pausar();
        return;
    }

    imprimirConfiguracaoDetalhe(&s->pilhaConfiguracoes.topo->dados, 1);
    pausar();
}

/*
 * 3. Consultar N configurações mais recentes
 */

void consultarNConfigurações(const Sistema *s)
{
    limparEcra();
    printf("\n  ╔══════════════════════════════════════════════════╗\n");
    printf("  ║   N Configuracoes Mais Recentes                  ║\n");
    printf("  ╚══════════════════════════════════════════════════╝\n\n");

    if (s->pilhaConfiguracoes.topo == NULL) {
        printf("  [!] Pilha vazia — nenhuma configuracao registada.\n");
        pausar();
        return;
    }

    int total = s->pilhaConfiguracoes.tamanho;
    printf("  Total na pilha: %d\n\n", total);
    int n = lerInteiro("  Quantas configurações mostrar", 1, total);

    const NodeConfiguracao *atual = s->pilhaConfiguracoes.topo;
    for (int i = 0; atual != NULL && i <= n; i++, atual = atual->proximo)
        imprimirConfiguracaoDetalhe(&atual->dados, i);

    pausar();
}

/*
 *  4.  Reverter ultima configuração (pop + restaurar equipamento)
 */

void reverterUltimaConfiguracao(Sistema *s)
{
    limparEcra();
    printf("\n  ╔══════════════════════════════════════════════════╗\n");
    printf("  ║   Reverter Ultima Configuracao                   ║\n");
    printf("  ╚══════════════════════════════════════════════════╝\n\n");

    if (s->pilhaConfiguracoes.topo == NULL)
    {
        printf("  [!] Pilha vazia — nenhuma configuracao para reverter.\n");
        pausar();
        return;
    }

    printf("  A reverter ultima configuracao...\n");
    imprimirConfiguracaoDetalhe(&s->pilhaConfiguracoes.topo->dados, 1);

    int conf = lerInteiro("\n  Deseja reverter esta configuracao? (1-Sim, 2-Nao)", 1, 2);
    if (conf == 2)
    {
        printf("  [--] Operacao cancelada.\n");
        pausar();
        return;
    }

    NodeConfiguracao *no = desempilhar(&s->pilhaConfiguracoes);
    s->totalConfiguracoes = s->pilhaConfiguracoes.tamanho;

    NodeEquipamento *eq = encontrarPorCodigo(s, no->dados.codigoEquipamento);
    if (eq != NULL)
    {
        if (aplicarReversao(eq, &no->dados))
            printf("  [OK] Campo '%s' do equipamento #%d restaurado para \"%s\".\n",
                   no->dados.tipoConfiguracao,
                   no->dados.codigoEquipamento,
                   no->dados.valorAnterior);
        else
            printf("  [--] Tipo '%s' nao mapeado automaticamente.\n"
                   "       Restaure o valor \"%s\" manualmente.\n",
                   no->dados.tipoConfiguracao, no->dados.valorAnterior);
    } else
    {
        printf("  [!] Equipamento #%d nao esta na lista atual.\n",
               no->dados.codigoEquipamento);
    }

    free(no);
    printf("  [OK] Configuracao removida da pilha. Pilha: %d entrada(s).\n",
           s->pilhaConfiguracoes.tamanho);
    pausar();
}

/*
 *  5. Historico de configurações por equipamento
 */


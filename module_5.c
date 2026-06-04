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
    strncpy(cfg.tecnico, s->tecnicoLogado, MAX_TECNICO - 1);
    cfg.tecnico[MAX_TECNICO - 1] = '\0';
    printf("  Tecnico: %s\n", cfg.tecnico);
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

void consultarNConfiguracoes(const Sistema *s)
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

void consultarHistoricoEquipamento(const Sistema *s)
{
    limparEcra();
    printf("\n  ╔══════════════════════════════════════════════════╗\n");
    printf("  ║   Historico por Equipamento                      ║\n");
    printf("  ╚══════════════════════════════════════════════════╝\n\n");

    if (s->pilhaConfiguracoes.topo == NULL) {
        printf("  [!] Pilha vazia — nenhuma configuracao registada.\n");
        pausar();
        return;
    }

    int cod = lerInteiro("  Codigo do equipamento", 1, s->proximoCodigoEquip - 1);

    int seq = 0;
    const NodeConfiguracao *atual = s->pilhaConfiguracoes.topo;
    while (atual != NULL) {
        if (atual->dados.codigoEquipamento == cod) {
            seq++;
            imprimirConfiguracaoDetalhe(&atual->dados, seq);
        }
        atual = atual->proximo;
    }

    if (seq == 0)
        printf("  [!] Nenhuma configuracao encontrada para o equipamento #%d.\n", cod);
    else
        printf("\n  Total: %d configuracao(oes) para equipamento #%d.\n", seq, cod);

    pausar();
}

/*
 *  6. Guardar/carregar ficheiro binario
 */

void guardarConfiguracoesFicheiro(const Sistema *s)
{
    int total = s->pilhaConfiguracoes.tamanho;

    FILE *f = fopen(FICH_CONFIGURACOES, "wb");
    if (f == NULL) {
        printf("\n  [!] Erro ao abrir o ficheiro \"%s\" para escrita\n", FICH_CONFIGURACOES);
        return;
    }

    fwrite(&total, sizeof(int), 1, f);

    if (total > 0)
    {
        /* Guardar da mais antiga para a mais recente para que ao carregar
         * e empilhar sequencialmente o topo seja restaurado corretamente. */

        Configuracao *buf = malloc((size_t)total * sizeof(Configuracao));
        if (buf == NULL) {
            fclose(f);
            printf("  [!] Erro de memoria.\n");
            return;
        }

        const NodeConfiguracao *atual = s->pilhaConfiguracoes.topo;
        for (int i = total - 1; atual != NULL; i--, atual = atual->proximo)
            buf[i] = atual->dados;
        fwrite(buf, sizeof(Configuracao), (size_t)total, f);
        free(buf);
    }

    fclose(f);
    printf("  [OK] %d configuracao(oes) guardada(s) em '%s'.\n",
           total, FICH_CONFIGURACOES);
}

void carregarConfiguracoesFicheiro(Sistema *s)
{
    FILE *f = fopen(FICH_CONFIGURACOES, "rb");
    if (f == NULL) {
        printf("  [!] Ficheiro '%s' nao encontrado.\n", FICH_CONFIGURACOES);
        return;
    }

    int total;
    if (fread(&total, sizeof(int), 1, f) != 1 || total < 0) {
        fclose(f);
        printf("  [!] Ficheiro corrompido.\n");
        return;
    }

    /* Limpar pilha atual */
    NodeConfiguracao *no = s->pilhaConfiguracoes.topo;
    while (no) {
        NodeConfiguracao *tmp = no->proximo;
        free(no);
        no = tmp;
    }
    s->pilhaConfiguracoes.topo    = NULL;
    s->pilhaConfiguracoes.tamanho = 0;
    s->totalConfiguracoes         = 0;

    Configuracao cfg;
    for (int i = 0; i < total; i++) {
        if (fread(&cfg, sizeof(Configuracao), 1, f) != 1) break;
        empilhar(&s->pilhaConfiguracoes, cfg);
    }

    fclose(f);
    s->totalConfiguracoes = s->pilhaConfiguracoes.tamanho;
    printf("  [OK] %d configuracao(oes) carregada(s) de '%s'.\n",
           s->totalConfiguracoes, FICH_CONFIGURACOES);
}

/*
 *  7. Limpar configuracoes
 */

void limparConfiguracoes(Sistema *s)
{
    limparEcra();
    printf("\n  ╔══════════════════════════════════════════════════╗\n");
    printf("  ║   Limpar Registo de Configuracoes                ║\n");
    printf("  ╚══════════════════════════════════════════════════╝\n\n");

    if (s->pilhaConfiguracoes.topo == NULL) {
        printf("  [!] A pilha ja esta vazia.\n");
        pausar();
        return;
    }

    printf("  [!] Esta operacao elimina %d configuracao(oes). Nao e reversivel.\n\n",
           s->pilhaConfiguracoes.tamanho);

    int conf = lerInteiro("  Confirmar limpeza? (1-Sim  2-Nao)", 1, 2);
    if (conf == 2) {
        printf("  [--] Operacao cancelada.\n");
        pausar();
        return;
    }

    NodeConfiguracao *no = s->pilhaConfiguracoes.topo;
    while (no) {
        NodeConfiguracao *tmp = no->proximo;
        free(no);
        no = tmp;
    }
    s->pilhaConfiguracoes.topo    = NULL;
    s->pilhaConfiguracoes.tamanho = 0;
    s->totalConfiguracoes         = 0;

    printf("  [OK] Pilha de configuracoes limpa.\n");
    pausar();
}

/*
 *  8. Menu de configuracoes
 */

void menuConfiguracoes(Sistema *s)
{
    int opcao;
    do {
        limparEcra();
        printf("\n  ╔══════════════════════════════════════════════════╗\n");
        printf("  ║   M5 — Registo de Configuracoes                  ║\n");
        printf("  ╠══════════════════════════════════════════════════╣\n");
        printf("  ║  1. Registar nova configuracao                    ║\n");
        printf("  ║  2. Consultar ultima configuracao                 ║\n");
        printf("  ║  3. Consultar N configuracoes mais recentes       ║\n");
        printf("  ║  4. Reverter ultima configuracao                  ║\n");
        printf("  ║  5. Historico por equipamento                     ║\n");
        printf("  ║  6. Guardar configuracoes em ficheiro             ║\n");
        printf("  ║  7. Carregar configuracoes de ficheiro            ║\n");
        printf("  ║  8. Limpar configuracoes                          ║\n");
        printf("  ║  0. Voltar                                        ║\n");
        printf("  ╠══════════════════════════════════════════════════╣\n");
        printf("  ║  Configuracoes na pilha: %-4d                     ║\n",
               s->pilhaConfiguracoes.tamanho);
        printf("  ╚══════════════════════════════════════════════════╝\n");

        opcao = lerInteiro("  Opcao", 0, 8);
        switch (opcao) {
        case 1: registarConfiguracao(s);          break;
        case 2: consultarUltimaConfiguracao(s);   break;
        case 3: consultarNConfiguracoes(s);        break;
        case 4: reverterUltimaConfiguracao(s);    break;
        case 5: consultarHistoricoEquipamento(s);  break;
        case 6: guardarConfiguracoesFicheiro(s);  pausar(); break;
        case 7: carregarConfiguracoesFicheiro(s); pausar(); break;
        case 8: limparConfiguracoes(s);            break;
        case 0: break;
        }
    } while (opcao != 0);
}
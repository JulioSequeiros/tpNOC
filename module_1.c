// Modulo 1 : Inventario de Equipamentos da Rede
// Created by Julio on 15/05/26.
//

#include "noc.h"

// 1.Adicionar equipamento
void adicionarEquipamento(Sistema *s)
{
    limparEcra();

    printf("Adicionar Equipamento\n");

    printf("Insira o nome do equipamento: ");

    // Invoca um novo indice para o array da memoria alocada
    Equipamento novo;
    memset(&novo, 0, sizeof(Equipamento));
    novo.codigo = s->proximoCodigoEquip;

    // Identifica do equipamento
    lerString("Insira o nome do Equipamento",novo.nome,MAX_NOME);

    printf("Qual e o tipo de equipamento?");
    novo.tipo = selecionarTipo();

    lerString("Qual é a marca do equipamento?",novo.marca,MAX_MARCA);
    lerString("Qual é o modelo do equipamento?",novo.marca,MAX_MODELO);

    lerString("Qual é o IP do equipamento?",novo.ip,MAX_IP);
    lerString("Qual é o MAC do equipamento?",novo.mac,MAX_MAC);
    lerString("Qual é a localização do equipamento?",novo.localizacao,MAX_LOCAL);

    printf("Qual é o estado operacional do equipamento?");
    novo.estado = selecionarEstado();
    obterDataAtual(novo.dataUltimaVerificacao);

    NodeEquipamento *novoNode = (NodeEquipamento *)malloc(sizeof(NodeEquipamento));
    if (novoNode == NULL)
    {
        printf("Erro ao alocar memoria");
        return;
    }
    novoNode->dados = novo;
    novoNode->proximo = s->equipamentos;
    s->equipamentos = novoNode;

    s->totalEquipamentos++;
    s->proximoCodigoEquip++;

    printf("[OK] Equipamento \"%s\" adicionado com sucesso, com codigo %d\n", novo.nome, novo.codigo);
    pausar();
}

// 2.Remover equipamento
void removerEquipamento(Sistema *s)
{
    limparEcra();
    printf("Remover Equipamento\n");

    /*
     * Realiza um check automatico na lista ligada se tem algum equipamento alocado,
     * se nao tiver devolve uma mensagem.
     */

    if (s->equipamentos == NULL)
    {
        printf("\n [!] Não existem equipamentos registados.\n")
        pausar();
        return;
    }

    /*
     * Pergunta ao utilizador qual o codigo do equipamento a remover,
     * e depois percorre a lista ligada para encontrar o equipamento com o codigo correspondente.
     */

    int codigo = lerInteiro("Insira o codigo do equipamento a remover", 1, s->proximoCodigoEquip);

    /* Se esse equipamento tiver um incidente pendente,
     * vai ser avisado ao utilizador que não consegue remover o mesmo
     */

    if (temIncidentePendente(s, codigo))
    {
        printf("\n [!] Não é possível remover o equipamento, pois existem incidentes pendentes associados a ele.\n");
        pausar();
        return;
    }

    /*
     * Procura o codigo do equipamento e realiza a pesquisa na lista ligada, se encontrar o equipamento,
     * remove o mesmo da lista ligada e libera a memoria alocada.
     */

    NodeEquipamento *atual = s->equipamentos;
    NodeEquipamento *anterior = NULL;

    while (atual != NULL && atual->dados.codigo != codigo)
    {
        anterior = atual;
        atual = atual->proximo;
    }

    if (atual == NULL)
    {
        printf("\n [!] Equipamento com codigo %d nao encontrado.\n", codigo);
        pausar();
        return;
    }

    printf("\n Equipamento encontrado: %s\n", atual->dados.nome);
    int conf = lerInteiro("Tem a certeza que deseja remover este equipamento? (1-Sim, 2-Nao)", 1, 2);
    if (conf != 1)
    {
        printf("\n Remocao cancelada.\n");
        pausar();
        return;
    }

    if (atual->proximo == NULL)
        s->equipamentos = atual->proximo;
    else
        anterior->proximo = atual->proximo;

    printf("\n [OK] Equipamento encontrado com sucesso.\n");
    pausar();
}


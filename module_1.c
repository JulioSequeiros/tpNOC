// Modulo 1 : Inventario de Equipamentos da Rede
// Created by Julio on 15/05/26.
//

#include "noc.h"

// Adicionar equipamento
void adicionarEquipamento(Sistema *s)
{
    limparEcra();
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

// Remover equipamento
void removerEquipamento(Sistema *s)
{

}

// Modulo 1: Inventario de Equipamentos da Rede
// Created by Julio on 15/05/26.
//

#include "noc.h"

// ================= MENU EQUIPAMENTOS =================

void menuEquipamentos(Sistema *s)
{
    int opcao;

    do
    {
        printf("===============================================================================================\n");
        printf("                              MODULO 1 - GESTAO DE EQUIPAMENTOS                               \n");
        printf("===============================================================================================\n");
        printf("1. Adicionar um novo equipamento de rede.                                                      \n");
        printf("2. Remover um equipamento (sem incidentes pendentes).                                         \n");
        printf("3. Alterar os dados de um equipamento.                                                        \n");
        printf("4. Alterar o estado de um equipamento (Operacional, Em Falha, Em Manutencao, Desativado).     \n");
        printf("5. Listar todos os equipamentos.                                                              \n");
        printf("6. Listar equipamentos por tipo.                                                              \n");
        printf("7. Listar equipamentos por estado.                                                           \n");
        printf("8. Pesquisar equipamento por codigo, IP ou MAC.                                              \n");
        printf("9. Guardar e carregar inventario em ficheiro binario.                                        \n");
        printf("10. Outras atividades relevantes.                                                            \n");
        printf("-----------------------------------------------------------------------------------------------\n");
        printf("0. Voltar                                                                                      \n");
        printf("===============================================================================================\n");
        printf("Opcao: ");

        if (scanf("%d", &opcao) != 1)
        {
            printf("Entrada invalida!\n");
            limparBuffer();
            continue;
        }

        limparBuffer();

        switch (opcao)
        {
            case 1:
                adicionarEquipamento(s);
                break;

            case 2:
                removerEquipamento(s);
                break;

            case 3:
                alterarEquipamento(s);
                break;

            case 4:
                alterarEstado(s);
                break;

            case 5:
                listarEquipamentos(s);
                break;

            case 6:
                listarPorTipo(s);
                break;

            case 7:
                listarPorEstado(s);
                break;

            case 8:
                pesquisarEquipamento(s);
                break;

            case 9:
                guardarInventario(s);
                break;

            case 10:
                outrasAtividadesRelevantes(s);
                break;

            case 0:
                printf("A voltar...\n");
                break;

            default:
                printf("Opcao invalida!\n");
        }

    } while (opcao != 0);
}

// ================= Funcoes do Modulo 1 =================
// 1. Adicionar equipamento
void adicionarEquipamento(Sistema *s)
{
    limparEcra();

    printf("Adicionar Equipamento\n");

    // Invoca um novo indice para o array da memoria alocada
    Equipamento novo;
    memset(&novo, 0, sizeof(Equipamento));
    novo.codigo = s->proximoCodigoEquip;

    // Identificação do equipamento
    lerString("Insira o nome do Equipamento", novo.nome, MAX_NOME);

    printf("\nQual e o tipo de equipamento:\n");
    novo.tipo = selecionarTipo();

    lerString("Qual e a marca do equipamento", novo.marca, MAX_MARCA);
    lerString("Qual e o modelo do equipamento", novo.modelo, MAX_MODELO);

    lerString("Qual e o IP do equipamento", novo.ip, MAX_IP);
    lerString("Qual e o MAC do equipamento", novo.mac, MAX_MAC);

    lerString("Qual e a localizacao do equipamento", novo.localizacao, MAX_LOCAL);

    printf("\nQual e o estado operacional do equipamento\n");
    novo.estado = selecionarEstado();

    obterDataAtual(novo.dataUltimaVerificacao);

    NodeEquipamento *novoNode = (NodeEquipamento *)malloc(sizeof(NodeEquipamento));
    if (novoNode == NULL)
    {
        printf("Erro ao alocar memoria\n");
        pausar();
        return;
    }

    novoNode->dados = novo;
    novoNode->proximo = s->equipamentos;
    s->equipamentos = novoNode;

    s->totalEquipamentos++;
    s->proximoCodigoEquip++;

    printf("\n[OK] Equipamento \"%s\" adicionado com sucesso, com codigo %d\n",
           novo.nome, novo.codigo);

    pausar();
}

// 2. Remover equipamento
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
        printf("\n [!] Não existem equipamentos registados.\n");
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

//3. Alterar os dados de um equipamento
void alterarEquipamento(Sistema *s)
{
    limparEcra();
    printf("Alterar Dados de Equipamento\n");

    /*
     * Pergunta ao utilizador o codigo do equipamento que quer alterar os dados
     */
    int codigo = lerInteiro("Insira o codigo do equipamento a alterar", 1, s->proximoCodigoEquip);

    /*
     * Procura na lista ligada, se esse codigo do equipamento e existente,
     * se nao for e devolvido um aviso a dizer que o equipamento nao existe.
     */
    NodeEquipamento *no = encontrarPorCodigo(s, codigo);
    if (no == NULL)
    {
        printf("\n [!] Equipamento com codigo %d nao encontrado.\n", codigo);
        pausar();
        return;
    }

    /*
     * Se o equipamento existier vai para este passo, onde vai apresentar o equipamento que vai ser atualizado;
     * Depois pergunta ao utilizador quais os dados que quer alterar, e depois atualiza os dados do equipamento.
     */
    Equipamento *e = &no->dados;
    imprimirEquipamento(e);
    printf("\n Deixe em branco (Enter) para manter o valor atual. \n\n");

    char buffer[MAX_DESC];

    printf(" Nome ['\%s\']: ", e->nome);
    fgets(buffer, MAX_DESC, stdin);
    if (buffer[0] == '\n')
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        strncpy(e->nome, buffer, MAX_DESC - 1);
    }

    int altTipo = lerInteiro(" Alterar tipo? (1-Sim, 2-Nao)", 1, 2);
    if (altTipo == 1)
    {
        e->tipo = selecionarTipo();
    }

    printf(" Marca [\"%s\"]: ", e->marca);
    fgets(buffer, MAX_DESC, stdin);
    if (buffer[0] == '\n')
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        strncpy(e->marca, buffer, MAX_DESC - 1);
    }

    printf(" Endereço IP ['\%s\']: ", e->ip);
    fgets(buffer, MAX_DESC, stdin);
    if (buffer[0] == '\n')
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        strncpy(e->ip, buffer, MAX_DESC - 1);
    }

    printf(" Endereço MAC ['\%s\']: ", e->mac);
    fgets(buffer, MAX_DESC, stdin);
    if (buffer[0] == '\n')
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        strncpy(e->mac, buffer, MAX_DESC - 1);
    }

    printf(" Localização ['\%s\']: ", e->localizacao);
    fgets(buffer, MAX_DESC, stdin);
    if (buffer[0] == '\n')
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        strncpy(e->localizacao, buffer, MAX_DESC - 1);
    }

    obterDataAtual(e->dataUltimaVerificacao);

    printf("\n [OK] Dados do equipamento atualizados com sucesso.\n");
    pausar();
}

//4. Alterar o estado de um equipamento (Operacional / Em Falha / Em Manutenção / Desativado)
void alterarEstado(Sistema *s)
{
    limparEcra();
    printf("Alterar Estado de Equipamento\n");

    /*
     * Pergunta ao utilizador o codigo do equipamento que quer alterar o estado
     */
    int codigo = lerInteiro("Insira o codigo do equipamento a alterar o estado", 1, s->proximoCodigoEquip);

    /*
     * Procura na lista ligada, se esse codigo do equipamento e existente,
     * se nao for e devolvido um aviso a dizer que o equipamento nao existe.
     */
    NodeEquipamento *no = encontrarPorCodigo(s, codigo);
    if (no == NULL)
    {
        printf("\n [!] Equipamento com codigo %d nao encontrado.\n", codigo);
        pausar();
        return;
    }

    printf("\n Equipamento: %s\n", no->dados.nome);
    printf("\n Estado atual: %s\n ", estadoEquipamentoToString(no->dados.estado));
    printf("\n Novo estado: \n");
    no->dados.estado = selecionarTipo();
    obterDataAtual(no->dados.dataUltimaVerificacao);

    printf("\n [OK] Estado do equipamento atualizado para \"%s\".\n", estadoEquipamentoToString(no->dados.estado));
    pausar();
}

// 5. Listar Todos os Equipamentos
void listarEquipamentos(Sistema *s)
{
    limparEcra();
    printf("Lista de Equipamentos\n");

    if (s->equipamentos == NULL)
    {
        printf("\n [!] Não existem equipamentos registados.\n");
        pausar();
        return;
    }

    imprimirCabecalho();
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL)
    {
        imprimirEquipamento(&atual->dados);
        printf("\n-----------------------------\n");
        atual = atual->proximo;
    }
    printf("\n Total de equipamentos: %d\n", s->totalEquipamentos);
    pausar();
}

// 6. Listar por Tipo
void listarPorTipo(Sistema *s)
{
    limparEcra();
    printf("Listar Equipamentos por Tipo\n");

    if (s->equipamentos == NULL)
    {
        printf("\n [!] Não existem equipamentos registados.\n");
        pausar();
        return;
    }

    printf("Selecione o tipo de equipamento para listar:\n");
    TipoEquipamento tipo = selecionarTipo();

    imprimirCabecalho();
    int count = 0;
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL)
    {
        if (atual->dados.tipo == tipo)
        {
            imprimirEquipamento(&atual->dados);
            printf("\n-----------------------------\n");
            count++;
        }
        atual = atual->proximo;
    }

    if (count == 0)
    {
        printf("\n [!] Não existem equipamentos do tipo \"%s\" encontrado.\n", tipoToString(tipo));
    }

    printf("\n Total de equipamentos do tipo \"%s\": %d\n", tipoToString(tipo), count);
    pausar();
}

//7. Listar por Estado
void listarPorEstado(Sistema *s)
{
    limparEcra();
    printf("Listar Equipamentos por Estado\n");

    if (s->equipamentos == NULL)
    {
        printf("\n [!] Não existem equipamentos registados.\n");
        pausar();
        return;
    }

    printf("Selecione o estado de equipamento para listar:\n");
    EstadoEquipamento estado = selecionarEstado();

    imprimirCabecalho();
    int count = 0;
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL)
    {
        if (atual->dados.estado == estado)
        {
            imprimirEquipamento(&atual->dados);
            printf("\n-----------------------------\n");
            count++;
        }
        atual = atual->proximo;
    }

    if (count == 0)
    {
        printf("\n [!] Não existem equipamentos com estado \"%s\" encontrado.\n", estadoEquipamentoToString(estado));
    }
    printf("\n Total de equipamentos com estado \"%s\": %d\n", estadoEquipamentoToString(estado), count);
    pausar();
}

//8. Pesquisar equipamento por codigo, IP, MAC
void pesquisarEquipamento(Sistema *s)
{
    limparEcra();
    printf("Pesquisar Equipamento\n");

    printf("Pesquisar por:\n1. Código\n2. Endereço IP\n3. Endereço MAC\n");

    int opcao = lerInteiro("Selecione a opção de pesquisa", 1, 3);

    if (opcao == 1)
    {
        int codigo = lerInteiro("Insira o codigo do equipamento a pesquisar", 1, 1000000);

        NodeEquipamento *no = encontrarPorCodigo(s, codigo);

        if (no == NULL)
            printf("\n [!] Equipamento com codigo %d nao encontrado.\n", codigo);
        else
        {
            imprimirCabecalho();
            imprimirEquipamento(&no->dados);
        }
    }
    else if (opcao == 2)
    {
        char ip[MAX_IP];
        lerString("Insira o endereco IP do equipamento a pesquisar", ip, MAX_IP);

        NodeEquipamento *no = encontrarPorIP(s, ip);

        if (no == NULL)
            printf("\n [!] Equipamento com endereco IP \"%s\" nao encontrado.\n", ip);
        else
        {
            imprimirCabecalho();
            imprimirEquipamento(&no->dados);
        }
    }
    else if (opcao == 3)
    {
        char mac[MAX_MAC];
        lerString("Insira o endereco MAC do equipamento a pesquisar", mac, MAX_MAC);

        NodeEquipamento *no = encontrarPorMAC(s, mac);

        if (no == NULL)
            printf("\n [!] Equipamento com endereco MAC \"%s\" nao encontrado.\n", mac);
        else
        {
            imprimirCabecalho();
            imprimirEquipamento(&no->dados);
        }
    }
}
void guardarInventario(Sistema *s)
{
    limparEcra();

    int opcao = lerInteiro(
        "Selecione a opção\n1. Guardar Inventário\n2. Carregar Inventário\n", 1, 2);

    if (opcao == 1)
    {
        
    }

    (void)s;
}
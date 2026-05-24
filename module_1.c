// Modulo 1: Inventario de Equipamentos da Rede
// Created by Julio on 15/05/26.
//

#include "noc.h"

/*
 * UTIL
 */

void obterDataAtual(char *data)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(data, MAX_DATA, "%d-%m-%Y", tm_info);
}


int lerInteiro(const char *prompt, int min, int max)
{
    int valor;
    char lixo;

    while (1)
    {
        printf("%s: ", prompt);
        if (scanf("%d%c", &valor, &lixo) == 2 && lixo == '\n'
            && valor >= min && valor <= max)
        {
            return valor;
        }
        limparBuffer();
        printf("  [!] Valor inválido. Introduza um número entre %d e %d.\n", min, max);
    }
}

void limparBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}



void lerString(const char *prompt, char *dest, int maxLen)
{
    while (1)
    {
        printf("%s: ", prompt);
        if (fgets(dest, maxLen, stdin) != NULL)
        {
            size_t len = strlen(dest);
            if ( len > 0 && dest[len - 1] == '\n')
            {
                dest[len - 1] = '\0';
            }
            if (strlen(dest) > 0)
            {
                return;
            }
        }
        printf("  [!] Campo obrigatório. Tente novamente.\n");
    }
}

const char *tipoToString(TipoEquipamento tipo)
{
    switch (tipo)
    {
    case ROUTER:          return "Router";
    case SWITCH_L2:       return "Switch L2";
    case ACCESS_POINT:    return "Access Point";
    case SERVIDOR_NAS:    return "Servidor/NAS";
    case IMPRESSORA_REDE: return "Impressora de Rede";
    case CAMERA_IP:       return "Câmara IP";
    case SENSOR:          return "Sensor";
    case UPS:             return "UPS";
    default:              return "Desconhecido";
    }
}

const char *estadoEquipamentoToString(EstadoEquipamento estado)
{
    switch (estado)
    {
    case OPERACIONAL:  return "Operacional";
    case EM_FALHA:       return "Em Falha";
    case EM_MANUTENCAO:  return "Em Manutenção";
    case DESATIVADO:     return "Desativado";
    default:             return "Desconhecido";
    }
}

const char *estadoIncidenteToString(EstadoIncidente estado)
{
    switch (estado)
    {
    case PENDENTE:  return "Pendente";
    case EM_CURSO:  return "Em Curso";
    case CONCLUIDO: return "Concluído";
    default:            return "Desconhecido";
    }
}

const char *estadoSensorToString(EstadoSensor estado)
{
    switch (estado)
    {
    case SENSOR_NORMAL:    return "Normal";
    case SENSOR_AVISO:     return "Aviso";
    case SENSOR_CRITICO:   return "Crítico";
    case SENSOR_FALHA_REDE:return "Falha Rede";
    default:               return "Desconhecido";
    }
}

TipoEquipamento selecionarTipo(void)
{
    printf("\n  Tipos de Equipamento:\n");
    printf("  1. Router\n");
    printf("  2. Switch L2\n");
    printf("  3. Access Point\n");
    printf("  4. Servidor/NAS\n");
    printf("  5. Impressora de Rede\n");
    printf("  6. Câmara IP\n");
    printf("  7. Sensor\n");
    printf("  8. UPS\n");
    return (TipoEquipamento) lerInteiro("  Opção: ", 1, 8);
}

EstadoEquipamento selecionarEstado(void)
{
    printf("\n  Estados disponíveis:\n");
    printf("  1. Operacional\n");
    printf("  2. Em Falha\n");
    printf("  3. Em Manutenção\n");
    printf("  4. Desativado\n");
    return (EstadoEquipamento) lerInteiro("  Opção: ", 1, 4);
}

void limparEcra(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausar(void)
{
    printf("\n  Prima ENTER para continuar...");
    limparBuffer();
}

NodeEquipamento *encontrarPorCodigo(Sistema *s, int codigo)
{
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL)
    {
        if (atual->dados.codigo == codigo)
            return atual;
        atual = atual->proximo;
    }
    return NULL;
}

NodeEquipamento *encontrarPorIP(Sistema *s, const char *ip)
{
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL)
    {
        if (strcmp(atual->dados.ip, ip) == 0)
            return atual;
        atual = atual->proximo;
    }
    return NULL;
}

NodeEquipamento *encontrarPorMAC(Sistema *s, const char *mac)
{
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL)
    {
        if (strcmp(atual->dados.mac, mac) == 0)
            return atual;
        atual = atual->proximo;
    }
    return NULL;
}

int temIncidentePendente(Sistema *s, int codigo)
{
    NodeIncidente *atual = s->incidentes;
    while (atual != NULL)
    {
        if (atual->dados.codigoEquipamento == codigo &&
            atual->dados.estado != CONCLUIDO)
            return 1;
        atual = atual->proximo;
    }
    return 0;
}

/*
 * Prints
 */

void imprimirCabecalho(void)
{
    printf("\n  %-6s %-20s %-18s %-15s %-16s %-18s %-17s %-16s\n",
           "Cód.", "Nome", "Tipo", "Marca",
           "IP", "MAC", "Localização", "Estado");
    printf("  ");
    for (int i = 0; i < 128; i++) printf("-");
    printf("\n");
}

void imprimirEquipamento(Equipamento *e)
{
    printf("  %-6d %-20s %-18s %-15s %-16s %-18s %-17s %-16s\n",
           e->codigo, e->nome, tipoToString(e->tipo), e->marca,
           e->ip, e->mac, e->localizacao,
           estadoEquipamentoToString(e->estado));
}

static void imprimirFichaEquipamento(const Equipamento *e)
{
    printf("\n  ╔══════════════════════════════════════════════════╗\n");
    printf("  ║           FICHA DE EQUIPAMENTO                  ║\n");
    printf("  ╚══════════════════════════════════════════════════╝\n");
    printf("  Código            : %d\n",  e->codigo);
    printf("  Nome              : %s\n",  e->nome);
    printf("  Tipo              : %s\n",  tipoToString(e->tipo));
    printf("  Marca             : %s\n",  e->marca);
    printf("  Modelo            : %s\n",  e->modelo);
    printf("  Endereço IP       : %s\n",  e->ip);
    printf("  Endereço MAC      : %s\n",  e->mac);
    printf("  Localização       : %s\n",  e->localizacao);
    printf("  Estado            : %s\n",  estadoEquipamentoToString(e->estado));
    printf("  Última verificação: %s\n",  e->dataUltimaVerificacao);
    printf("  ────────────────────────────────────────────────────\n");
}

/*
 * Operação Logica
 */

// 1. Adicionar equipamento
void adicionarEquipamento(Sistema *s)
{
    limparEcra();
    printf("\n  ══════════════════════════════════════\n");
    printf("    ADICIONAR NOVO EQUIPAMENTO\n");
    printf("  ══════════════════════════════════════\n");

    // Invoca um novo indice para o array na memória alocada
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

// 2. Remover equipamento
void removerEquipamento(Sistema *s)
{
    limparEcra();
    printf("\n  ══════════════════════════════════════\n");
    printf("    REMOVER EQUIPAMENTO\n");
    printf("  ══════════════════════════════════════\n");

    /*
     * Realiza um check automatico na lista ligada se tem algum equipamento alocado,
     * se nao tiver devolve uma mensagem.
     */

    if (s->equipamentos == NULL)
    {
        printf("\n [!] Não existem equipamentos registados.\n");
        pausar();return;
    }

    /*
     * Pergunta ao utilizador qual o codigo do equipamento a remover,
     * e depois percorre a lista ligada para encontrar o equipamento com o codigo correspondente.
     */

    int codigo = lerInteiro("Insira o codigo do equipamento a remover", 1, s->proximoCodigoEquip - 1);

    /* Se esse equipamento tiver um incidente pendente,
     * vai ser avisado ao utilizador que não consegue remover o mesmo
     */

    if (temIncidentePendente(s, codigo))
    {
        printf("\n [!] Não é possível remover o equipamento, pois existem incidentes pendentes associados a ele.\n");
        pausar();return;
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
        pausar();return;
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

    free(atual);
    s->totalEquipamentos--;
    printf("\n [OK] Equipamento encontrado com sucesso.\n");
    pausar();
}

//3. Alterar os dados de um equipamento
void alterarEquipamento(Sistema *s)
{
    limparEcra();
    printf("\n  ══════════════════════════════════════\n");
    printf("    ALTERAR DADOS DE EQUIPAMENTO\n");
    printf("  ══════════════════════════════════════\n");

    /*
     * Pergunta ao utilizador o codigo do equipamento que quer alterar os dados
     */
    int codigo = lerInteiro("Insira o codigo do equipamento a alterar", 1, s->proximoCodigoEquip - 1);

    /*
     * Procura na lista ligada, se esse codigo do equipamento e existente,
     * se nao for e devolvido um aviso a dizer que o equipamento nao existe.
     */
    NodeEquipamento *no = encontrarPorCodigo(s, codigo);
    if (no == NULL)
    {
        printf("\n [!] Equipamento com codigo %d nao encontrado.\n", codigo);
        pausar();return;
    }

    /*
     * Se o equipamento existier vai para este passo, onde vai apresentar o equipamento que vai ser atualizado;
     * Depois pergunta ao utilizador quais os dados que quer alterar, e depois atualiza os dados do equipamento.
     */
    Equipamento *e = &no->dados;
    imprimirEquipamento(e);
    printf("\n Deixe em branco (Enter) para manter o valor atual. \n\n");

    char buffer[MAX_DESC];

    printf(" Nome [%s]: ", e->nome);
    fgets(buffer, MAX_DESC, stdin);
    if (buffer[0] == '\n')
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        strncpy(e->nome, buffer, sizeof(e->nome) - 1);
        e->nome[sizeof(e->nome) - 1] = '\0';
    }

    if (lerInteiro(" Alterar tipo? (1-Sim, 2-Nao)", 1, 2) == 1)
        e->tipo = selecionarTipo();

    printf(" Marca [%s]: ", e->marca);
    fgets(buffer, MAX_DESC, stdin);
    if (buffer[0] == '\n')
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        strncpy(e->marca, buffer, sizeof(e->marca) - 1);
        e->marca[sizeof(e->marca) - 1] = '\0';
    }

    printf("  Modelo [%s]: ", e->modelo);
    fgets(buffer, MAX_MODELO, stdin);
    if (buffer[0] != '\n')
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        strncpy(e->modelo, buffer, sizeof(e->modelo) - 1);
        e->modelo[sizeof(e->modelo) - 1] = '\0';
    }

    printf(" Endereço IP [%s]: ", e->ip);
    fgets(buffer, MAX_DESC, stdin);
    if (buffer[0] == '\n')
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        strncpy(e->ip, buffer, sizeof(e->ip) - 1);
        e->ip[sizeof(e->ip) - 1] = '\0';
    }

    printf(" Endereço MAC [%s]: ", e->mac);
    fgets(buffer, MAX_MAC, stdin);
    if (buffer[0] == '\n')
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        strncpy(e->mac, buffer, sizeof(e->mac) - 1);
        e->mac[sizeof(e->mac) - 1] = '\0';
    }

    printf(" Localização [%s]: ", e->localizacao);
    fgets(buffer, MAX_LOCAL, stdin);
    if (buffer[0] == '\n')
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        strncpy(e->localizacao, buffer, sizeof(e->localizacao) - 1);
        e->localizacao[sizeof(e->localizacao) - 1] = '\0';
    }

    obterDataAtual(e->dataUltimaVerificacao);

    printf("\n [OK] Dados do equipamento atualizados com sucesso.\n");
    pausar();
}

//4. Alterar o estado de um equipamento (Operacional / Em Falha / Em Manutenção / Desativado)
void alterarEstado(Sistema *s)
{
    limparEcra();
    printf("\n  ══════════════════════════════════════\n");
    printf("    ALTERAR ESTADO DO EQUIPAMENTO\n");
    printf("  ══════════════════════════════════════\n");

    /*
     * Pergunta ao utilizador o codigo do equipamento que quer alterar o estado
     */
    int codigo = lerInteiro("Insira o codigo do equipamento a alterar o estado", 1, s->proximoCodigoEquip - 1);

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
    no->dados.estado = selecionarEstado();
    obterDataAtual(no->dados.dataUltimaVerificacao);

    printf("\n [OK] Estado do equipamento atualizado para \"%s\".\n", estadoEquipamentoToString(no->dados.estado));
    pausar();
}

// 5. Listar Todos os Equipamentos
void listarTodos(const Sistema *s)
{
    limparEcra();
    printf("\n  ══════════════════════════════════════\n");
    printf("    LISTA DE TODOS OS EQUIPAMENTOS (%d)\n", s->totalEquipamentos);
    printf("  ══════════════════════════════════════\n");

    if (s->equipamentos == NULL)
    {
        printf("\n [!] Não existem equipamentos registados.\n");
        pausar();return;
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
void listarPorTipo(const Sistema *s)
{
    limparEcra();
    printf("\n  ══════════════════════════════════════\n");
    printf("    LISTAR EQUIPAMENTOS POR TIPO\n");
    printf("  ══════════════════════════════════════\n");

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
void listarPorEstado(const Sistema *s)
{
    limparEcra();
    printf("\n  ══════════════════════════════════════\n");
    printf("    LISTAR EQUIPAMENTOS POR ESTADO\n");
    printf("  ══════════════════════════════════════\n");

    if (s->equipamentos == NULL)
    {
        printf("\n [!] Não existem equipamentos registados.\n");
        pausar();
        return;
    }

    printf("Selecione o estado: ");
    EstadoEquipamento estado = selecionarEstado();

    imprimirCabecalho();
    int count = 0;
    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL)
    {
        if (atual->dados.estado == estado)
        {
            imprimirEquipamento(&atual->dados);
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
    printf("\n  ══════════════════════════════════════\n");
    printf("    PESQUISAR EQUIPAMENTO\n");
    printf("  ══════════════════════════════════════\n");

    printf("Pesquisar por:\n1. Código\n2. Endereço IP\n3. Endereço MAC\n");
    int opcao = lerInteiro("Selecione a opção de pesquisa", 1,3);
    Equipamento *encontrado = NULL;

    if (opcao == 1)
    {
        int codigo = lerInteiro("  Codigo: ", 1, s->proximoCodigoEquip - 1);
        NodeEquipamento *no = encontrarPorCodigo(s, codigo);
        if (no)
        {
            encontrado = &no->dados;
        }
    }
    else
    {
        char termo[MAX_MAC];
        if (opcao == 2)
        {
            lerString("  Endereço IP: ", termo, MAX_IP);
        }
        lerString("  Endereço MAC: ", termo, MAX_MAC);

        NodeEquipamento *atual = s->equipamentos;
        while (atual != NULL)
        {
            int match = (opcao == 2) ? (strcmp(atual->dados.ip, termo) == 0) : (strcmp(atual->dados.mac, termo) == 0);
            if (match)
            {
                encontrado = &atual->dados;
                break;
            }
            atual = atual->proximo;
        }
    }

    if (encontrado == NULL)
    {
        printf("\n [!] Equipamento não encontrado.\n");
    }
    else
    {
        imprimirFichaEquipamento(encontrado);
    }
    pausar();
}

//9. Guardar e carregar o ficheiro binario
void guardarEquipamento(const Sistema *s)
{
    FILE *f = fopen(FICH_EQUIPAMENTOS, "wb");
    if (f == NULL)
    {
        printf("\n [!] Não foi possível abrir %s.\n", FICH_EQUIPAMENTOS);
        return;
    }

    // Escreve o número total de equipamentos
    fwrite(&s->totalEquipamentos, sizeof(int), 1, f);
    fwrite(&s->proximoCodigoEquip, sizeof(int), 1, f);

    // Escreve cada equipamento
    NodeEquipamento *eq = s->equipamentos;
    while (eq != NULL)
    {
        fwrite(&eq->dados, sizeof(Equipamento), 1, f);
        eq = eq->proximo;
    }

    fclose(f);
    printf("\n [OK] Equipamentos guardado com sucesso em \"%s\".\n",FICH_EQUIPAMENTOS);
}

void carregarEquipameto(Sistema *s)
{
    FILE *f = fopen(FICH_EQUIPAMENTOS, "rb");
    if (f == NULL)
    {
        printf("\n  [!] \"%s\" não encontrado. A iniciar vazio.\n", FICH_EQUIPAMENTOS);
        return;
    }

    int total = 0, proxCod = 1;
    fread(&total, sizeof(int), 1, f);
    fread(&proxCod, sizeof(int), 1, f);

    s->totalEquipamentos = total;
    s->proximoCodigoEquip = proxCod;

    NodeEquipamento **ultimo = &s->equipamentos;
    for (int i = 0; i < total; i++)
    {
        NodeEquipamento *no = (NodeEquipamento *)malloc(sizeof(NodeEquipamento));
        if (no == NULL) break;
        fread(&no->dados, sizeof(Equipamento), 1, f);
        no->proximo = NULL;
        *ultimo = no;
        ultimo = &no->proximo;
    }

    fclose(f);
    printf("\n  [OK] %d equipamento(s) carregado(s) de \"%s\".\n", total, FICH_EQUIPAMENTOS);
}

// 10. Stats geral do modulo

void menuEstatistica(const Sistema *s)
{
    limparEcra();
    printf("\n  ══════════════════════════════════════\n");
    printf("    ESTATÍSTICAS DO INVENTÁRIO\n");
    printf("  ══════════════════════════════════════\n");


    int porTipo[9] = {0}, porEstado[5] = {0};

    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL)
    {
        porTipo[atual->dados.tipo]++;
        porEstado[atual->dados.estado]++;
        atual = atual->proximo;
    }

    printf("\n Total de equipamentos : %d\n", s->totalEquipamentos);
    printf("\n Total de incidentes : %d\n\n", s->totalIncidentes);

    printf("\n Por tipo:\n");
    for (int i = 1; i <= 8; i++)
    {
        if (porTipo[i] > 0)
        {
            printf(" %-20s : %d\n", tipoToString((TipoEquipamento)i), porTipo[i]);
        }
    }
    printf("\n Por Estado:\n");
    for (int i = 1; i <= 4; i++)
    {
        printf(" %-20s : %d\n", estadoEquipamentoToString((EstadoEquipamento)i), porEstado[i]);
    }
    pausar();
}

// Menu do Modulo

void menuEquipamento(Sistema *s)
{
    int opcao;
    do
    {
        limparEcra();
        printf("\n  ╔══════════════════════════════════════╗\n");
        printf("  ║    MÓDULO 1 — INVENTÁRIO             ║\n");
        printf("  ╠══════════════════════════════════════╣\n");
        printf("  ║  1. Adicionar equipamento            ║\n");
        printf("  ║  2. Remover equipamento              ║\n");
        printf("  ║  3. Alterar dados                    ║\n");
        printf("  ║  4. Alterar estado                   ║\n");
        printf("  ║  5. Listar todos                     ║\n");
        printf("  ║  6. Listar por tipo                  ║\n");
        printf("  ║  7. Listar por estado                ║\n");
        printf("  ║  8. Pesquisar equipamento            ║\n");
        printf("  ║  9. Estatísticas                     ║\n");
        printf("  ║  0. Voltar                           ║\n");
        printf("  ╚══════════════════════════════════════╝\n");

        opcao = lerInteiro("  Opção: ", 0, 9);

        switch (opcao)
        {
        case 1: adicionarEquipamento(s); break;
        case 2: removerEquipamento(s);   break;
        case 3: alterarEquipamento(s);   break;
        case 4: alterarEstado(s);        break;
        case 5: listarTodos(s);          break;
        case 6: listarPorTipo(s);        break;
        case 7: listarPorEstado(s);      break;
        case 8: pesquisarEquipamento(s); break;
        case 9: menuEstatistica(s);      break;
        case 0: break;
        }
    } while (opcao != 0);
}
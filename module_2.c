// Modulo 2: Testes de Conectividade
// Created by Guilherme Fernandes on 24/05/26.
//
#include "noc.h"

// Inclusões específicas para compatibilidade entre Windows e Unix/Linux
#ifdef _WIN32
    #include <windows.h>  // Funções específicas do Windows (Sleep, etc.)
#else
    #include <unistd.h>   // Funções POSIX (usleep, etc.)
#endif

/*
 * UTIL - Funções auxiliares específicas do módulo 2
 */

// Estrutura interna para resultados de teste (não vai para o ".h")
// Usada para armazenar resultados temporários dos testes de ping
typedef struct ResultadoTeste {
    int codigo;                 // Código do equipamento
    char nome[MAX_NOME];        // Nome do equipamento
    char ip[MAX_IP];            // Endereço IP
    int respondeu;              // 'Flag': 1 se responder, 0 se não responder
} ResultadoTeste;

// Estrutura para estatisticas do ping
// Armazena todas as métricas coletadas durante o teste de ping
typedef struct {
    int respondeu;              // Se houve resposta (1) ou não (0)
    int pacotesEnviados;        // Número de pacotes enviados
    int pacotesRecebidos;       // Número de pacotes recebidos
    int pacotesPerdidos;        // Número de pacotes perdidos
    float percentagemPerda;     // Percentagem de perda (0-100)
    float tempoMinimo;          // Menor tempo de resposta (ms)
    float tempoMaximo;          // Maior tempo de resposta (ms)
    float tempoMedio;           // Tempo médio de resposta (ms)
} PingEstatisticas;

// Função auxiliar para executar ping e obter resultado
// Executa um ping simples (1 pacote) e guarda o resultado em arquivo
// Atributo __attribute__((unused)) evita warning de função não utilizada
static int __attribute__((unused)) executarPingSimples(const char *ip, char *ficheiroSaida)
{
    char comando[300];

#ifdef _WIN32
    // Windows: ping -n 1 (1 pacote)
    snprintf(comando, sizeof(comando), "ping -n 1 %s > %s", ip, ficheiroSaida);
#else
    // Linux/Unix: ping -c 1 (1 pacote)
    snprintf(comando, sizeof(comando), "ping -c 1 %s > %s", ip, ficheiroSaida);
#endif

    return system(comando);  // Executa o comando no sistema operacional
}

// Função auxiliar para executar ping com 4 pacotes (para estatisticas)
// Usa 4 pacotes para obter métricas mais precisas (média, min, max)
static int executarPingEstatisticas(const char *ip, char *ficheiroSaida)
{
    char comando[300];

#ifdef _WIN32
    // Windows: ping -n 4 (4 pacotes)
    snprintf(comando, sizeof(comando), "ping -n 4 %s > %s", ip, ficheiroSaida);
#else
    // Linux/Unix: ping -c 4 (4 pacotes)
    snprintf(comando, sizeof(comando), "ping -c 4 %s > %s", ip, ficheiroSaida);
#endif

    return system(comando);
}

// Função auxiliar para analisar se houve resposta ao ping
// Lê o arquivo de saída do ping e verifica se encontrou indicadores de resposta
static int __attribute__((unused)) analisarRespostaPing(const char *ficheiro)
{
    FILE *f = fopen(ficheiro, "r");  // Abre arquivo para leitura
    if (f == NULL) return 0;          // Arquivo não existe

    char linha[300];
    int respondeu = 0;

    // Procura por padrões comuns de resposta em diferentes sistemas
    while (fgets(linha, sizeof(linha), f) != NULL)
    {
        // Windows: "Reply from" ou "Resposta de"
        // Linux: "bytes from" ou "TTL="
        if (strstr(linha, "TTL=") != NULL ||
            strstr(linha, "ttl=") != NULL ||
            strstr(linha, "Reply from") != NULL ||
            strstr(linha, "Resposta de") != NULL ||
            strstr(linha, "bytes from") != NULL)
        {
            respondeu = 1;
            break;
        }
    }

    fclose(f);
    return respondeu;
}

// Função auxiliar para extrair estatisticas do ping
// Analisa o arquivo de saída e extrai todas as métricas disponíveis
static PingEstatisticas extrairEstatisticasPing(const char *ficheiro)
{
    FILE *f = fopen(ficheiro, "r");
    PingEstatisticas stats;

    // Inicializar estatisticas com valores padrão
    stats.respondeu = 0;
    stats.pacotesEnviados = 4;      // Por padrão, 4 pacotes
    stats.pacotesRecebidos = 0;
    stats.pacotesPerdidos = 4;
    stats.percentagemPerda = 100.0f;
    stats.tempoMinimo = 0;
    stats.tempoMaximo = 0;
    stats.tempoMedio = 0;

    if (f == NULL) return stats;    // Arquivo não encontrado

    char linha[300];
    int tempos[4];                  // Array para armazenar tempos individuais
    int numTempos = 0;              // Número de tempos coletados

    // Processa cada linha do arquivo
    while (fgets(linha, sizeof(linha), f) != NULL)
    {
        // Verificar padrões que indicam que houve resposta ao ping
        if (strstr(linha, "TTL=") != NULL ||
            strstr(linha, "ttl=") != NULL ||
            strstr(linha, "Reply from") != NULL ||
            strstr(linha, "Resposta de") != NULL ||
            strstr(linha, "bytes from") != NULL)
        {
            stats.respondeu = 1;    // Marcamos que houve resposta
            stats.pacotesRecebidos++;

            // Extrair tempo de resposta (funciona em Windows e Linux)
            int tempo = 0;

            // Formato Windows: "tempo=10ms" ou "time=10ms"
            if (strstr(linha, "tempo=") != NULL)
            {
                sscanf(strstr(linha, "tempo="), "tempo=%dms", &tempo);
            }
            else if (strstr(linha, "time=") != NULL)
            {
                sscanf(strstr(linha, "time="), "time=%dms", &tempo);
            }
            // Formato Linux: "time=10 ms"
            else if (strstr(linha, "time=") != NULL && strstr(linha, " ms") != NULL)
            {
                sscanf(strstr(linha, "time="), "time=%d ms", &tempo);
            }

            // Armazena o tempo se for válido
            if (tempo > 0 && numTempos < 4)
            {
                tempos[numTempos++] = tempo;
            }
        }

        // Extrair estatisticas finais (formato Windows)
        if (strstr(linha, "Perda =") != NULL)
        {
            int perdidos;
            sscanf(linha, "Perda = %d", &perdidos);
            stats.pacotesPerdidos = perdidos;
            stats.pacotesRecebidos = 4 - perdidos;
            stats.percentagemPerda = (perdidos * 100.0) / 4;
        }
        else if (strstr(linha, "loss") != NULL)  // Formato alternativo
        {
            int perdidos;
            sscanf(linha, "%d%% loss", &perdidos);
            stats.percentagemPerda = perdidos;
            stats.pacotesPerdidos = (int)(4 * perdidos / 100.0);
            stats.pacotesRecebidos = 4 - stats.pacotesPerdidos;
        }

        // Extrair estatisticas finais (formato Linux)
        if (strstr(linha, "received") != NULL)
        {
            int recebidos, perdidos;
            sscanf(linha, "%d packets transmitted, %d received, %d%% loss",
                   &stats.pacotesEnviados, &recebidos, &perdidos);
            stats.pacotesRecebidos = recebidos;
            stats.pacotesPerdidos = stats.pacotesEnviados - recebidos;
            stats.percentagemPerda = perdidos;
        }

        // Extrair tempos minimo, maximo e medio (formato Windows)
        if (strstr(linha, "Minimo =") != NULL || strstr(linha, "Minimum =") != NULL)
        {
            int min, max, media;
            if (strstr(linha, "Minimo =") != NULL)
            {
                sscanf(linha, "Minimo = %dms, Maximo = %dms, Media = %dms",
                       &min, &max, &media);
            }
            else
            {
                sscanf(linha, "Minimum = %dms, Maximum = %dms, Average = %dms",
                       &min, &max, &media);
            }
            stats.tempoMinimo = min;
            stats.tempoMaximo = max;
            stats.tempoMedio = media;
        }
    }

    // Se não encontrou as estatisticas finais, calcular a partir dos tempos coletados
    if (stats.tempoMedio == 0 && numTempos > 0)
    {
        int soma = 0;
        stats.tempoMinimo = tempos[0];
        stats.tempoMaximo = tempos[0];

        for (int i = 0; i < numTempos; i++)
        {
            soma += tempos[i];
            if (tempos[i] < stats.tempoMinimo) stats.tempoMinimo = tempos[i];
            if (tempos[i] > stats.tempoMaximo) stats.tempoMaximo = tempos[i];
        }
        stats.tempoMedio = (float)soma / numTempos;  // Média aritmética
    }

    fclose(f);
    return stats;
}

// Função auxiliar para mostrar estatisticas do ping formatadas
// Exibe os resultados de forma organizada para o utilizador
static void mostrarEstatisticasPing(const PingEstatisticas *stats)
{
    printf("\n  --- ESTATÍSTICAS DO PING ---\n");
    printf("  Pacotes enviados:   %d\n", stats->pacotesEnviados);
    printf("  Pacotes recebidos:  %d\n", stats->pacotesRecebidos);
    printf("  Pacotes perdidos:   %d\n", stats->pacotesPerdidos);
    printf("  Percentagem de perda: %.1f%%\n", stats->percentagemPerda);

    // Mostra tempos apenas se houver resposta
    if (stats->respondeu && stats->tempoMedio > 0)
    {
        printf("\n  --- TEMPOS DE RESPOSTA ---\n");
        printf("  Tempo minimo:   %.1f ms\n", stats->tempoMinimo);
        printf("  Tempo maximo:   %.1f ms\n", stats->tempoMaximo);
        printf("  Tempo medio:    %.1f ms\n", stats->tempoMedio);
    }

    // Classificacao da qualidade da rede baseada nas métricas
    printf("\n  --- CLASSIFICACAO ---\n");
    if (stats->percentagemPerda == 0)  // Sem perda de pacotes
    {
        if (stats->tempoMedio <= 10)
            printf("  Qualidade: EXCELENTE\n");
        else if (stats->tempoMedio <= 50)
            printf("  Qualidade: BOA\n");
        else if (stats->tempoMedio <= 100)
            printf("  Qualidade: RAZOAVEL\n");
        else
            printf("  Qualidade: LENTA\n");
    }
    else if (stats->percentagemPerda <= 10)  // Perda moderada
    {
        printf("  Qualidade: PERDA PARCIAL (rede instavel)\n");
    }
    else  // Perda alta
    {
        printf("  Qualidade: PERDA ELEVADA (verificar rede)\n");
    }
}

// Função auxiliar para registar no "log" de monitorizacao
// Adiciona uma entrada no arquivo de 'log' com timestamp
static void registarLog(const char *mensagem)
{
    FILE *log = fopen("log_monitorizacao.txt", "a");  // "a" = append (adicionar)
    if (log != NULL)
    {
        // Obtém data e hora atual
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);

        // Escreve no 'log' no formato: [data/hora] mensagem
        fprintf(log, "[%s] %s\n", dataHora, mensagem);
        fclose(log);
    }
}

// Função auxiliar para listar equipamentos (formato resumido)
// Exibe uma tabela com informações principais dos equipamentos
static void listarEquipamentosResumido(const Sistema *s)
{
    printf("\n  %-6s %-20s %-15s %-16s %-16s\n",
           "Cod.", "Nome", "IP", "Estado", "Data Verif.");
    printf("  ");
    for (int i = 0; i < 80; i++) printf("-");
    printf("\n");

    NodeEquipamento *atual = s->equipamentos;
    while (atual != NULL)
    {
        // Exibe cada equipamento formatado em colunas
        printf("  %-6d %-20s %-15s %-16s %-16s\n",
               atual->dados.codigo,
               atual->dados.nome,
               atual->dados.ip,
               estadoEquipamentoToString(atual->dados.estado),
               atual->dados.dataUltimaVerificacao);
        atual = atual->proximo;
    }
}

/*
 * Operacoes Logicas do Módulo 2
 */

// 1. Selecionar um equipamento registado e executar um teste ping ao seu endereco IP
// Função principal para testar conectividade com um equipamento específico
void executarPingEquipamento(Sistema *s)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |              TESTE DE PING A EQUIPAMENTO                  |\n");
    printf("  =====================================================================\n");

    // Verifica se existem equipamentos cadastrados
    if (s->equipamentos == NULL)
    {
        printf("\n  [!] Nao existem equipamentos registados.\n");
        printf("  [!] Registe equipamentos no Modulo 1 primeiro.\n");
        pausar();
        return;
    }

    // Lista equipamentos disponiveis para o utilizador escolher
    printf("\n  Equipamentos disponiveis:\n");
    listarEquipamentosResumido(s);

    // Solicita o código do equipamento a ser testado
    int codigo = lerInteiro("\n  Insira o codigo do equipamento para testar",
                            1, s->proximoCodigoEquip - 1);

    // Procura o equipamento na lista encadeada
    NodeEquipamento *equipamento = encontrarPorCodigo(s, codigo);
    if (equipamento == NULL)
    {
        printf("\n  [!] Equipamento com codigo %d nao encontrado!\n", codigo);
        pausar();
        return;
    }

    // Executa o ping
    printf("\n  ---------------------------------------------------------------------\n");
    printf("  A testar equipamento: %s\n", equipamento->dados.nome);
    printf("  Endereco IP: %s\n", equipamento->dados.ip);
    printf("  ---------------------------------------------------------------------\n");

    // Cria nome de arquivo temporário para o resultado
    char ficheiroResultado[100];
    snprintf(ficheiroResultado, sizeof(ficheiroResultado), "ping_temp_%d.txt", codigo);

    // Executar ping com 4 pacotes para obter estatisticas
    executarPingEstatisticas(equipamento->dados.ip, ficheiroResultado);

    // Analisar resultado e extrair estatisticas
    PingEstatisticas stats = extrairEstatisticasPing(ficheiroResultado);

    // Mostrar estatisticas formatadas
    mostrarEstatisticasPing(&stats);

    // Copiar para o ficheiro padrao (sobrescreve resultado anterior)
    FILE *origem = fopen(ficheiroResultado, "r");
    FILE *destino = fopen("resultado_ping.txt", "w");
    if (origem != NULL && destino != NULL)
    {
        char linha[300];
        while (fgets(linha, sizeof(linha), origem) != NULL)
        {
            fputs(linha, destino);
        }
        fclose(origem);
        fclose(destino);
    }

    // Remover ficheiro temporario (limpeza)
    remove(ficheiroResultado);

    // Atualizar data da ultima verificacao do equipamento
    obterDataAtual(equipamento->dados.dataUltimaVerificacao);

    // Se não respondeu, alterar estado para EM_FALHA
    if (!stats.respondeu)
    {
        equipamento->dados.estado = EM_FALHA;
        printf("\n  [AVISO] Estado do equipamento alterado para EM_FALHA\n");

        // Criar incidente automaticamente (apenas se não existir um pendente)
        if (!temIncidentePendente(s, codigo))
        {
            NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
            if (novo != NULL)
            {
                novo->dados.codigo = ++s->proximoCodigoInc;
                novo->dados.codigoEquipamento = codigo;
                strcpy(novo->dados.descricao, "Falha de comunicacao - Equipamento nao respondeu ao ping");
                novo->dados.estado = PENDENTE;
                novo->dados.prioridade = 1;  // Prioridade ALTA
                obterDataAtual(novo->dados.dataAbertura);
                strcpy(novo->dados.dataFecho, "-");

                // Insere no início da lista de incidentes
                novo->proximo = s->incidentes;
                s->incidentes = novo;
                s->totalIncidentes++;

                // Adiciona à fila de atendimento
                adicionarNaFilaAtendimento(s, novo->dados.codigo);

                printf("  [OK] Incidente #%d criado automaticamente\n", novo->dados.codigo);
            }
        }
    }

    // Registar no "log" de monitorização
    char logMsg[300];
    snprintf(logMsg, sizeof(logMsg), "Ping para %s (%s) - Perda: %.1f%% - Tempo medio: %.1fms",
             equipamento->dados.nome, equipamento->dados.ip,
             stats.percentagemPerda, stats.tempoMedio);
    registarLog(logMsg);

    // Guardar alteracoes (estado do equipamento, data, incidente)
    guardarFicheiro(s);

    printf("\n  [OK] Ping executado. Resultado guardado em 'resultado_ping.txt'\n");
    pausar();
}

// 2. Guardar o resultado bruto do comando num ficheiro de texto
// Salva o resultado do último ping num arquivo com timestamp
void guardarResultadoPing(const Sistema *s)
{
    (void)s;  // Cast para void evita warning de parâmetro não utilizado

    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |              GUARDAR RESULTADO DO PING                    |\n");
    printf("  =====================================================================\n");

    // Verificar se o ficheiro de resultado existe (ping já foi executado)
    FILE *origem = fopen("resultado_ping.txt", "r");
    if (origem == NULL)
    {
        printf("\n  [!] Nenhum resultado de ping encontrado!\n");
        printf("  [!] Execute primeiro o teste de ping (opcao 1).\n");
        pausar();
        return;
    }

    // Criar nome do ficheiro com timestamp para não sobrescrever
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char nomeFicheiro[100];

    // Formato: ping_AAAAMMDD_HHMMSS.txt
    snprintf(nomeFicheiro, sizeof(nomeFicheiro),
             "ping_%04d%02d%02d_%02d%02d%02d.txt",
             tm_info->tm_year + 1900,
             tm_info->tm_mon + 1,
             tm_info->tm_mday,
             tm_info->tm_hour,
             tm_info->tm_min,
             tm_info->tm_sec);

    // Copiar conteudo para novo ficheiro
    FILE *destino = fopen(nomeFicheiro, "w");
    if (destino == NULL)
    {
        printf("\n  [!] Nao foi possivel criar o ficheiro %s!\n", nomeFicheiro);
        fclose(origem);
        pausar();
        return;
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), origem) != NULL)
    {
        fputs(linha, destino);
    }

    fclose(origem);
    fclose(destino);

    printf("\n  [OK] Resultado do ping guardado com sucesso!\n");
    printf("  [OK] Ficheiro: %s\n", nomeFicheiro);

    // Registar no "log"
    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Resultado do ping guardado em: %s", nomeFicheiro);
    registarLog(logMsg);

    pausar();
}

// 3. Ler o ficheiro de resultado e determinar se o equipamento respondeu (com estatisticas)
// Função para analisar e interpretar os resultados do último ping
void verificarRespostaPing(const Sistema *s)
{
    (void)s;  // Evita warning

    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |              VERIFICAR RESPOSTA DO PING                   |\n");
    printf("  =====================================================================\n");

    // Verificar se o ficheiro existe
    FILE *f = fopen("resultado_ping.txt", "r");
    if (f == NULL)
    {
        printf("\n  [!] Nenhum resultado de ping encontrado!\n");
        printf("  [!] Execute primeiro o teste de ping (opcao 1).\n");
        pausar();
        return;
    }
    fclose(f);

    // Extrair estatisticas do arquivo
    PingEstatisticas stats = extrairEstatisticasPing("resultado_ping.txt");

    printf("\n  ---------------------------------------------------------------------\n");
    if (stats.respondeu)
    {
        printf("  ESTADO: EQUIPAMENTO RESPONDEU (ONLINE)\n");
        printf("  O equipamento esta ativo na rede.\n");
    }
    else
    {
        printf("  ESTADO: EQUIPAMENTO NAO RESPONDEU (OFFLINE)\n");
        printf("  O equipamento pode estar desligado ou com problemas.\n");
    }
    printf("  ---------------------------------------------------------------------\n");

    // Mostrar estatisticas detalhadas
    mostrarEstatisticasPing(&stats);

    // Registar no "log"
    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Verificacao de ping: %s - Perda: %.1f%% - Tempo medio: %.1fms",
             stats.respondeu ? "ONLINE" : "OFFLINE",
             stats.percentagemPerda, stats.tempoMedio);
    registarLog(logMsg);

    pausar();
}

// 4. Atualizar automaticamente a data da última verificacao do equipamento
// Permite atualizar manualmente a data de verificação de um equipamento
void atualizarDataVerificacao(Sistema *s)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |           ATUALIZAR DATA DE VERIFICACAO                   |\n");
    printf("  =====================================================================\n");

    // Verificar se existem equipamentos
    if (s->equipamentos == NULL)
    {
        printf("\n  [!] Nao existem equipamentos registados.\n");
        pausar();
        return;
    }

    // Listar equipamentos
    printf("\n  Equipamentos disponiveis:\n");
    listarEquipamentosResumido(s);

    // Solicitar código
    int codigo = lerInteiro("\n  Insira o codigo do equipamento", 1, s->proximoCodigoEquip - 1);

    // Procurar equipamento
    NodeEquipamento *encontrado = encontrarPorCodigo(s, codigo);
    if (encontrado == NULL)
    {
        printf("\n  [!] Equipamento com codigo %d nao encontrado!\n", codigo);
        pausar();
        return;
    }

    // Mostrar dados atuais para confirmação
    printf("\n  ---------------------------------------------------------------------\n");
    printf("  Equipamento: %s\n", encontrado->dados.nome);
    printf("  Data atual da ultima verificacao: %s\n", encontrado->dados.dataUltimaVerificacao);
    printf("  ---------------------------------------------------------------------\n");

    // Confirmar atualizacao
    int confirmar = lerInteiro("\n  Deseja atualizar a data para o momento atual? (1-Sim, 2-Nao)", 1, 2);

    if (confirmar == 1)
    {
        obterDataAtual(encontrado->dados.dataUltimaVerificacao);  // Atualiza para data/hora atual

        printf("\n  [OK] Data atualizada com sucesso!\n");
        printf("  [OK] Nova data: %s\n", encontrado->dados.dataUltimaVerificacao);

        // Registar no "log"
        char logMsg[200];
        snprintf(logMsg, sizeof(logMsg), "Data de verificacao atualizada para equipamento %d (%s)",
                 encontrado->dados.codigo, encontrado->dados.nome);
        registarLog(logMsg);

        // Guardar alteracoes
        guardarFicheiro(s);
    }
    else
    {
        printf("\n  [!] Atualizacao cancelada.\n");
    }

    pausar();
}

// 5. Alterar o estado do equipamento para EM_FALHA quando não existir resposta
// Função manual para marcar um equipamento como em falha
void alterarEstadoFalha(Sistema *s)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |           ALTERAR ESTADO PARA EM_FALHA                    |\n");
    printf("  =====================================================================\n");

    // Verificar se existem equipamentos
    if (s->equipamentos == NULL)
    {
        printf("\n  [!] Nao existem equipamentos registados.\n");
        pausar();
        return;
    }

    // Listar equipamentos
    printf("\n  Equipamentos disponiveis:\n");
    listarEquipamentosResumido(s);

    // Solicitar codigo
    int codigo = lerInteiro("\n  Insira o codigo do equipamento para alterar para EM_FALHA",
                            1, s->proximoCodigoEquip - 1);

    // Procurar equipamento
    NodeEquipamento *encontrado = encontrarPorCodigo(s, codigo);
    if (encontrado == NULL)
    {
        printf("\n  [!] Equipamento com codigo %d nao encontrado!\n", codigo);
        pausar();
        return;
    }

    // Verificar estado atual - evita alteração desnecessária
    if (encontrado->dados.estado == EM_FALHA)
    {
        printf("\n  [!] Equipamento ja esta em estado EM_FALHA!\n");
        pausar();
        return;
    }

    // Confirmar alteracao
    printf("\n  Equipamento: %s\n", encontrado->dados.nome);
    printf("  Estado atual: %s\n", estadoEquipamentoToString(encontrado->dados.estado));

    int confirmar = lerInteiro("\n  Deseja alterar o estado para EM_FALHA? (1-Sim, 2-Nao)", 1, 2);

    if (confirmar == 1)
    {
        encontrado->dados.estado = EM_FALHA;
        obterDataAtual(encontrado->dados.dataUltimaVerificacao);  // Atualiza data também

        printf("\n  [OK] Estado alterado para EM_FALHA com sucesso!\n");

        // Registar no "log"
        char logMsg[200];
        snprintf(logMsg, sizeof(logMsg), "Estado do equipamento %d (%s) alterado para EM_FALHA",
                 encontrado->dados.codigo, encontrado->dados.nome);
        registarLog(logMsg);

        // Guardar alteracoes
        guardarFicheiro(s);
    }
    else
    {
        printf("\n  [!] Alteracao cancelada.\n");
    }

    pausar();
}

// 6. Registar cada teste realizado num ficheiro de texto denominado log_monitorizacao.txt
// Função para registrar manualmente o último teste no 'log'
void registarTesteLog(const Sistema *s)
{
    (void)s;  // Evita warning

    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |              REGISTAR TESTE NO LOG                        |\n");
    printf("  =====================================================================\n");

    // Verificar se existe resultado de ping
    FILE *f = fopen("resultado_ping.txt", "r");
    if (f == NULL)
    {
        printf("\n  [!] Nenhum resultado de ping encontrado!\n");
        printf("  [!] Execute primeiro o teste de ping (opcao 1).\n");
        pausar();
        return;
    }
    fclose(f);

    // Extrair estatisticas
    PingEstatisticas stats = extrairEstatisticasPing("resultado_ping.txt");

    // Obter timestamp para mostrar ao utilizador
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char dataHora[50];
    strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);

    // Registar no "log"
    char logMsg[300];
    snprintf(logMsg, sizeof(logMsg), "Teste de ping executado - Resultado: %s - Perda: %.1f%% - Tempo medio: %.1fms",
             stats.respondeu ? "RESPONDEU" : "NAO RESPONDEU",
             stats.percentagemPerda, stats.tempoMedio);
    registarLog(logMsg);

    printf("\n  ---------------------------------------------------------------------\n");
    printf("  [OK] Log registado com sucesso!\n");
    printf("  [OK] Ficheiro: log_monitorizacao.txt\n");
    printf("  [OK] Data/Hora: %s\n", dataHora);
    printf("  [OK] Resultado: %s\n", stats.respondeu ? "RESPONDEU" : "NAO RESPONDEU");
    printf("  [OK] Perda de pacotes: %.1f%%\n", stats.percentagemPerda);
    if (stats.tempoMedio > 0)
    {
        printf("  [OK] Tempo medio de resposta: %.1f ms\n", stats.tempoMedio);
    }
    printf("  ---------------------------------------------------------------------\n");

    pausar();
}

// 7. Criar automaticamente um incidente tecnico quando um equipamento não responde
// Função para criar incidente baseado no último resultado de ping
void criarIncidenteAutomatico(Sistema *s)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |           CRIAR INCIDENTE AUTOMATICAMENTE                 |\n");
    printf("  =====================================================================\n");

    // Verificar se existem equipamentos
    if (s->equipamentos == NULL)
    {
        printf("\n  [!] Nao existem equipamentos registados.\n");
        pausar();
        return;
    }

    // Verificar se existe resultado de ping
    FILE *f = fopen("resultado_ping.txt", "r");
    if (f == NULL)
    {
        printf("\n  [!] Nenhum resultado de ping encontrado!\n");
        printf("  [!] Execute primeiro o teste de ping (opcao 1).\n");
        pausar();
        return;
    }
    fclose(f);

    // Extrair estatisticas
    PingEstatisticas stats = extrairEstatisticasPing("resultado_ping.txt");

    // Se respondeu, não precisa criar incidente
    if (stats.respondeu)
    {
        printf("\n  [OK] O equipamento respondeu ao ping. Nao e necessario criar incidente.\n");
        printf("  [OK] Perda de pacotes: %.1f%%\n", stats.percentagemPerda);
        pausar();
        return;
    }

    // Listar equipamentos para o utilizador selecionar
    printf("\n  Equipamentos disponiveis:\n");
    listarEquipamentosResumido(s);

    // Solicitar codigo do equipamento que falhou
    int codigo = lerInteiro("\n  Insira o codigo do equipamento que falhou no ping",
                            1, s->proximoCodigoEquip - 1);

    // Procurar equipamento
    NodeEquipamento *equipEncontrado = encontrarPorCodigo(s, codigo);
    if (equipEncontrado == NULL)
    {
        printf("\n  [!] Equipamento com codigo %d nao encontrado!\n", codigo);
        pausar();
        return;
    }

    // Verificar se já existe incidente pendente para evitar duplicação
    if (temIncidentePendente(s, codigo))
    {
        printf("\n  [!] Ja existe um incidente pendente para este equipamento.\n");
        pausar();
        return;
    }

    // Criar incidente
    NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
    if (novo == NULL)
    {
        printf("\n  [!] Erro ao alocar memoria para o incidente.\n");
        pausar();
        return;
    }

    // Construir descrição com estatísticas
    char descricao[MAX_DESC];
    snprintf(descricao, MAX_DESC,
             "Falha de comunicacao - Equipamento nao respondeu ao ping (Perda: %.1f%%)",
             stats.percentagemPerda);

    // Preencher dados do incidente
    novo->dados.codigo = ++s->proximoCodigoInc;
    novo->dados.codigoEquipamento = codigo;
    strcpy(novo->dados.descricao, descricao);
    novo->dados.estado = PENDENTE;
    novo->dados.prioridade = 1;  // Prioridade ALTA para falhas de ping
    obterDataAtual(novo->dados.dataAbertura);
    strcpy(novo->dados.dataFecho, "-");

    // Inserir na lista ligada (no início)
    novo->proximo = s->incidentes;
    s->incidentes = novo;
    s->totalIncidentes++;

    // Adicionar a fila de atendimento
    adicionarNaFilaAtendimento(s, novo->dados.codigo);

    // Mostrar resultado
    printf("\n  ---------------------------------------------------------------------\n");
    printf("    INCIDENTE CRIADO AUTOMATICAMENTE (FALHA NO PING)\n");
    printf("  ---------------------------------------------------------------------\n");
    printf("  ID do incidente: #%d\n", novo->dados.codigo);
    printf("  Equipamento: %s (Codigo: %d)\n", equipEncontrado->dados.nome, codigo);
    printf("  Descricao: %s\n", novo->dados.descricao);
    printf("  Prioridade: ALTA\n");
    printf("  Estado: PENDENTE\n");
    printf("  Data de abertura: %s\n", novo->dados.dataAbertura);
    printf("  Perda de pacotes: %.1f%%\n", stats.percentagemPerda);
    printf("  ---------------------------------------------------------------------\n");

    // Registar no "log"
    char logMsg[300];
    snprintf(logMsg, sizeof(logMsg), "INCIDENTE AUTOMATICO #%d - Equipamento %d (%s) nao respondeu ao ping (Perda: %.1f%%)",
             novo->dados.codigo, codigo, equipEncontrado->dados.nome, stats.percentagemPerda);
    registarLog(logMsg);

    // Guardar alteracoes
    guardarFicheiro(s);

    pausar();
}

// 8. Permitir um teste geral da rede, executando ping a todos os equipamentos registados
// Função que testa todos os equipamentos da rede e gera relatório completo
void pingGeral(Sistema *s)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |                 TESTE GERAL DA REDE                       |\n");
    printf("  =====================================================================\n");

    // Verificar se existem equipamentos
    if (s->equipamentos == NULL)
    {
        printf("\n  [!] Nao existem equipamentos registados.\n");
        pausar();
        return;
    }

    printf("\n  A testar todos os equipamentos registados...\n");
    printf("  ---------------------------------------------------------------------\n");

    // Contadores para estatísticas
    int online = 0;
    int offline = 0;
    int incidentesCriados = 0;
    int contador = 0;
    float perdaTotal = 0;
    int equipamentosComPerda = 0;

    // Alocar array para resultados (tamanho dinâmico)
    ResultadoTeste *resultados = (ResultadoTeste*)malloc(s->totalEquipamentos * sizeof(ResultadoTeste));
    if (resultados == NULL)
    {
        printf("\n  [!] Erro ao alocar memoria!\n");
        pausar();
        return;
    }

    NodeEquipamento *atual = s->equipamentos;

    // Testa cada equipamento individualmente
    while (atual != NULL)
    {
        printf("\n  [%d/%d] Testando: %s\n", contador + 1, s->totalEquipamentos, atual->dados.nome);
        printf("         IP: %s\n", atual->dados.ip);

        // Ficheiro temporario para resultado deste equipamento
        char ficheiroTemp[100];
        snprintf(ficheiroTemp, sizeof(ficheiroTemp), "temp_ping_%d.txt", atual->dados.codigo);

        // Executar ping com 4 pacotes para estatisticas
        executarPingEstatisticas(atual->dados.ip, ficheiroTemp);

        // Extrair estatisticas
        PingEstatisticas stats = extrairEstatisticasPing(ficheiroTemp);

        // Guardar resultado no array
        resultados[contador].codigo = atual->dados.codigo;
        strcpy(resultados[contador].nome, atual->dados.nome);
        strcpy(resultados[contador].ip, atual->dados.ip);
        resultados[contador].respondeu = stats.respondeu;

        // Atualizar data da última verificacao
        obterDataAtual(atual->dados.dataUltimaVerificacao);

        if (stats.respondeu)
        {
            printf("         ONLINE (Perda: %.1f%%, Tempo medio: %.1fms)\n",
                   stats.percentagemPerda, stats.tempoMedio);
            online++;
            if (stats.percentagemPerda > 0)
            {
                perdaTotal += stats.percentagemPerda;
                equipamentosComPerda++;
            }
        }
        else
        {
            printf("         OFFLINE (Perda total: 100%%)\n");
            offline++;
            perdaTotal += 100;
            equipamentosComPerda++;

            // Alterar estado para EM_FALHA automaticamente
            atual->dados.estado = EM_FALHA;

            // Verificar se já existe incidente pendente para este equipamento
            if (!temIncidentePendente(s, atual->dados.codigo))
            {
                // Criar incidente automaticamente
                NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
                if (novo != NULL)
                {
                    char descricao[MAX_DESC];
                    snprintf(descricao, MAX_DESC,
                             "Equipamento falhou no teste geral da rede (Sem resposta ao ping)");

                    novo->dados.codigo = ++s->proximoCodigoInc;
                    novo->dados.codigoEquipamento = atual->dados.codigo;
                    strcpy(novo->dados.descricao, descricao);
                    novo->dados.estado = PENDENTE;
                    novo->dados.prioridade = 1;  // Prioridade ALTA
                    obterDataAtual(novo->dados.dataAbertura);
                    strcpy(novo->dados.dataFecho, "-");

                    // Insere na lista
                    novo->proximo = s->incidentes;
                    s->incidentes = novo;
                    s->totalIncidentes++;
                    incidentesCriados++;

                    printf("         Incidente #%d criado\n", novo->dados.codigo);

                    // Adicionar a fila de atendimento
                    adicionarNaFilaAtendimento(s, novo->dados.codigo);
                }
            }
        }

        // Remover ficheiro temporario (limpeza)
        remove(ficheiroTemp);

        // Pequena pausa entre pings para não sobrecarregar a rede
        #ifdef _WIN32
            Sleep(500);  // Windows: 500ms
        #else
            usleep(500000);  // Linux/Unix: 500.000 microssegundos = 500ms
        #endif

        atual = atual->proximo;
        contador++;
    }

    // Calcular perda média (apenas equipamentos com perda > 0)
    const float perdaMedia = (equipamentosComPerda > 0) ? (perdaTotal / (float)equipamentosComPerda) : 0.0f;

    // Relatorio final com todas as estatísticas
    printf("\n  ---------------------------------------------------------------------\n");
    printf("  RELATORIO DO TESTE GERAL\n");
    printf("  ---------------------------------------------------------------------\n");
    printf("  Total de equipamentos testados: %d\n", s->totalEquipamentos);
    printf("  Equipamentos ONLINE : %d\n", online);
    printf("  Equipamentos OFFLINE: %d\n", offline);
    printf("  Incidentes criados: %d\n", incidentesCriados);
    printf("  Perda media de pacotes: %.1f%%\n", perdaMedia);
    printf("  ---------------------------------------------------------------------\n");

    // Classificacao da qualidade da rede baseada nos resultados
    printf("\n  --- CLASSIFICACAO DA QUALIDADE DA REDE ---\n");
    if (offline == 0 && perdaMedia == 0)
    {
        printf("  QUALIDADE: EXCELENTE (Todos os equipamentos online sem perda)\n");
    }
    else if (offline == 0 && perdaMedia <= 5)
    {
        printf("  QUALIDADE: BOA (Todos online, pouca perda de pacotes)\n");
    }
    else if (offline <= 2 && perdaMedia <= 10)
    {
        printf("  QUALIDADE: RAZOAVEL (Alguns equipamentos com perda)\n");
    }
    else if (offline <= 5)
    {
        printf("  QUALIDADE: ATENCAO (Varios equipamentos offline)\n");
    }
    else
    {
        printf("  QUALIDADE: CRITICA (Rede com problemas graves)\n");
    }

    // Listar equipamentos 'offline' para fácil identificação
    if (offline > 0)
    {
        printf("\n  EQUIPAMENTOS OFFLINE:\n");
        for (int i = 0; i < contador; i++)
        {
            if (!resultados[i].respondeu)
            {
                printf("     Codigo: %d | Nome: %s | IP: %s\n",
                       resultados[i].codigo, resultados[i].nome, resultados[i].ip);
            }
        }
    }

    // Registar no "log"
    char logMsg[300];
    snprintf(logMsg, sizeof(logMsg), "TESTE GERAL: %d equipamentos, %d online, %d offline, %d incidentes, Perda media: %.1f%%",
             s->totalEquipamentos, online, offline, incidentesCriados, perdaMedia);
    registarLog(logMsg);

    // Guardar alteracoes (estados, datas, incidentes)
    guardarFicheiro(s);

    free(resultados);  // Libera memória alocada

    printf("\n  [OK] Teste geral concluido!\n");
    pausar();
}

// 9. Outras atividades que considere relevantes (comandos de rede adicionais)
// Menu com comandos extras de diagnóstico de rede
void executarComandosRedeExtra(const Sistema *s)
{
    (void)s;  // Evita warning

    int opcao;

    do
    {
        limparEcra();
        printf("\n  ================================================================\n");
        printf("  |              COMANDOS DE REDE EXTRA                        |\n");
        printf("  ================================================================\n");
        printf("  |  1. Informacao da rede local (ipconfig / ip addr)         |\n");
        printf("  |  2. Tabela ARP (arp -a)                                   |\n");
        printf("  |  3. Resolucao DNS (nslookup)                              |\n");
        printf("  |  4. Rota ate destino (tracert / traceroute)               |\n");
        printf("  |  0. Voltar                                                |\n");
        printf("  ================================================================\n");

        opcao = lerInteiro("  Opcao", 0, 4);

        switch(opcao)
        {
            case 1: obterInfoRedeLocal(); break;
            case 2: obterTabelaARP(); break;
            case 3: obterResolucaoDNS(); break;
            case 4: obterRotaDestino(); break;
        }

        if (opcao != 0) pausar();

    } while (opcao != 0);
}

// Funcoes auxiliares para os comandos extra

// Obtém informações da rede local (ipconfig no Windows, ip addr no Linux)
void obterInfoRedeLocal(void){
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |              INFORMACAO DA REDE LOCAL                     |\n");
    printf("  =====================================================================\n");

    printf("\n  A obter informacao da rede local...\n");

#ifdef _WIN32
    system("ipconfig > resultado_rede_local.txt");  // Windows
#else
    system("ip addr > resultado_rede_local.txt");   // Linux/Unix
#endif

    printf("\n  [OK] Informacao guardada em: resultado_rede_local.txt\n");
    registarLog("Informacao da rede local obtida");
}

// Obtém a tabela ARP (mapeamento IP - MAC)
void obterTabelaARP(void){
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |                    TABELA ARP                             |\n");
    printf("  =====================================================================\n");

    printf("\n  A obter tabela ARP...\n");
    system("arp -a > resultado_arp.txt");  // Comando funciona em ambos OS

    printf("\n  [OK] Tabela ARP guardada em: resultado_arp.txt\n");
    registarLog("Tabela ARP obtida");
}

// Realiza resolução DNS (converte domínio para IP)
void obterResolucaoDNS(void){
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |                  RESOLUCAO DNS                            |\n");
    printf("  =====================================================================\n");

    char dominio[100];
    lerString("  Insira o dominio (ex: google.com)", dominio, sizeof(dominio));

    printf("\n  A resolver %s...\n", dominio);

    // nslookup funciona em ambos os sistemas
    char comando[200];
    snprintf(comando, sizeof(comando), "nslookup %s > resultado_dns.txt", dominio);

    system(comando);

    printf("\n  [OK] Resolucao DNS guardada em: resultado_dns.txt\n");

    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Resolucao DNS para %s", dominio);
    registarLog(logMsg);
}

// Rastreia a rota até um destino (tracert/traceroute)
void obterRotaDestino(void){
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |              ROTA ATE DESTINO                             |\n");
    printf("  =====================================================================\n");

    char destino[100];
    lerString("  Insira o IP ou dominio (ex: 8.8.8.8 ou google.com)", destino, sizeof(destino));

    printf("\n  A tracar rota ate %s...\n", destino);

#ifdef _WIN32
    char comando[200];
    snprintf(comando, sizeof(comando), "tracert %s > resultado_rota.txt", destino);  // Windows
#else
    char comando[200];
    snprintf(comando, sizeof(comando), "traceroute %s > resultado_rota.txt", destino);  // Linux
#endif

    system(comando);

    printf("\n  [OK] Rota guardada em: resultado_rota.txt\n");

    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Rota ate %s", destino);
    registarLog(logMsg);
}

/*
 * Menu do Módulo 2
 * Menu principal com todas as opções de testes de conectividade
 */
void menuConectividade(Sistema *s)
{
    int opcao;

    do
    {
        _sleep(20);  // Pequena pausa para estabilidade
        limparEcra();
        printf("\n  ╔════════════════════════════════════════════════════════════════╗\n");
        printf("  ║              MODULO 2 - TESTES DE CONECTIVIDADE              ║\n");
        printf("  ╠════════════════════════════════════════════════════════════════╣\n");
        printf("  ║  1. Executar ping a um equipamento                            ║\n");
        printf("  ║  2. Guardar resultado do ping em ficheiro                     ║\n");
        printf("  ║  3. Verificar se equipamento respondeu ao ping                ║\n");
        printf("  ║  4. Atualizar data da ultima verificacao                      ║\n");
        printf("  ║  5. Alterar estado para EM_FALHA                              ║\n");
        printf("  ║  6. Registar teste no log de monitorizacao                    ║\n");
        printf("  ║  7. Criar incidente automaticamente (falha no ping)           ║\n");
        printf("  ║  8. Teste geral da rede (ping a todos)                        ║\n");
        printf("  ║  9. Comandos de rede extra                                    ║\n");
        printf("  ║  0. Voltar                                                    ║\n");
        printf("  ╚════════════════════════════════════════════════════════════════╝\n");

        // Nota: esta linha está duplicada no código original, mantida para fidelidade
        opcao = lerInteiro("  Opcao", 0, 9);
        
        opcao = lerInteiro("  Opcao", 0, 9);
        
        switch (opcao)
        {
            case 1: executarPingEquipamento(s); break;
            case 2: guardarResultadoPing(s); break;
            case 3: verificarRespostaPing(s); break;
            case 4: atualizarDataVerificacao(s); break;
            case 5: alterarEstadoFalha(s); break;
            case 6: registarTesteLog(s); break;
            case 7: criarIncidenteAutomatico(s); break;
            case 8: pingGeral(s); break;
            case 9: executarComandosRedeExtra(s); break;
            case 0: break;
        }
        
    } while (opcao != 0);
}
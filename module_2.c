// Modulo 2: Testes de Conectividade
// Created by Guilherme Fernandes on 24/05/26.
//

#include "noc.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

/*
 * UTIL - Funções auxiliares específicas do módulo 2
 */

// Estrutura interna para resultados de teste (não vai para o .h)
typedef struct ResultadoTeste {
    int codigo;
    char nome[MAX_NOME];
    char ip[MAX_IP];
    int respondeu;
} ResultadoTeste;

// Estrutura para estatisticas do ping
typedef struct {
    int respondeu;
    int pacotesEnviados;
    int pacotesRecebidos;
    int pacotesPerdidos;
    float percentagemPerda;
    float tempoMinimo;
    float tempoMaximo;
    float tempoMedio;
} PingEstatisticas;

// Função auxiliar para executar ping e obter resultado
static int __attribute__((unused)) executarPingSimples(const char *ip, char *ficheiroSaida)
{
    char comando[300];
    
#ifdef _WIN32
    snprintf(comando, sizeof(comando), "ping -n 1 %s > %s", ip, ficheiroSaida);
#else
    snprintf(comando, sizeof(comando), "ping -c 1 %s > %s", ip, ficheiroSaida);
#endif
    
    return system(comando);
}

// Função auxiliar para executar ping com 4 pacotes (para estatisticas)
static int executarPingEstatisticas(const char *ip, char *ficheiroSaida)
{
    char comando[300];
    
#ifdef _WIN32
    snprintf(comando, sizeof(comando), "ping -n 4 %s > %s", ip, ficheiroSaida);
#else
    snprintf(comando, sizeof(comando), "ping -c 4 %s > %s", ip, ficheiroSaida);
#endif
    
    return system(comando);
}

// Função auxiliar para analisar se houve resposta ao ping
static int __attribute__((unused)) analisarRespostaPing(const char *ficheiro)
{
    FILE *f = fopen(ficheiro, "r");
    if (f == NULL) return 0;
    
    char linha[300];
    int respondeu = 0;
    
    while (fgets(linha, sizeof(linha), f) != NULL)
    {
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
static PingEstatisticas extrairEstatisticasPing(const char *ficheiro)
{
    FILE *f = fopen(ficheiro, "r");
    PingEstatisticas stats;
    
    // Inicializar estatisticas
    stats.respondeu = 0;
    stats.pacotesEnviados = 4;
    stats.pacotesRecebidos = 0;
    stats.pacotesPerdidos = 4;
    stats.percentagemPerda = 100.0;
    stats.tempoMinimo = 0;
    stats.tempoMaximo = 0;
    stats.tempoMedio = 0;
    
    if (f == NULL) return stats;
    
    char linha[300];
    int tempos[4];
    int numTempos = 0;
    
    while (fgets(linha, sizeof(linha), f) != NULL)
    {
        // Verificar se houve resposta
        if (strstr(linha, "TTL=") != NULL || 
            strstr(linha, "ttl=") != NULL ||
            strstr(linha, "Reply from") != NULL ||
            strstr(linha, "Resposta de") != NULL ||
            strstr(linha, "bytes from") != NULL)
        {
            stats.respondeu = 1;
            stats.pacotesRecebidos++;
            
            // Extrair tempo de resposta (Windows e Linux)
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
            
            if (tempo > 0 && numTempos < 4)
            {
                tempos[numTempos++] = tempo;
            }
        }
        
        // Extrair estatisticas finais (Windows)
        if (strstr(linha, "Perda =") != NULL)
        {
            int perdidos;
            sscanf(linha, "Perda = %d", &perdidos);
            stats.pacotesPerdidos = perdidos;
            stats.pacotesRecebidos = 4 - perdidos;
            stats.percentagemPerda = (perdidos * 100.0) / 4;
        }
        else if (strstr(linha, "loss") != NULL)
        {
            int perdidos;
            sscanf(linha, "%d%% loss", &perdidos);
            stats.percentagemPerda = perdidos;
            stats.pacotesPerdidos = (int)(4 * perdidos / 100.0);
            stats.pacotesRecebidos = 4 - stats.pacotesPerdidos;
        }
        
        // Extrair estatisticas finais (Linux)
        if (strstr(linha, "received") != NULL)
        {
            int recebidos, perdidos;
            sscanf(linha, "%d packets transmitted, %d received, %d%% loss",
                   &stats.pacotesEnviados, &recebidos, &perdidos);
            stats.pacotesRecebidos = recebidos;
            stats.pacotesPerdidos = stats.pacotesEnviados - recebidos;
            stats.percentagemPerda = perdidos;
        }
        
        // Extrair tempos minimo, maximo e medio (Windows)
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
    
    // Se nao encontrou as estatisticas finais, calcular a partir dos tempos
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
        stats.tempoMedio = (float)soma / numTempos;
    }
    
    fclose(f);
    return stats;
}

// Função auxiliar para mostrar estatisticas do ping
static void mostrarEstatisticasPing(const PingEstatisticas *stats)
{
    printf("\n  --- ESTATISTICAS DO PING ---\n");
    printf("  Pacotes enviados:   %d\n", stats->pacotesEnviados);
    printf("  Pacotes recebidos:  %d\n", stats->pacotesRecebidos);
    printf("  Pacotes perdidos:   %d\n", stats->pacotesPerdidos);
    printf("  Percentagem de perda: %.1f%%\n", stats->percentagemPerda);
    
    if (stats->respondeu && stats->tempoMedio > 0)
    {
        printf("\n  --- TEMPOS DE RESPOSTA ---\n");
        printf("  Tempo minimo:   %.1f ms\n", stats->tempoMinimo);
        printf("  Tempo maximo:   %.1f ms\n", stats->tempoMaximo);
        printf("  Tempo medio:    %.1f ms\n", stats->tempoMedio);
    }
    
    // Classificacao da qualidade da rede
    printf("\n  --- CLASSIFICACAO ---\n");
    if (stats->percentagemPerda == 0)
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
    else if (stats->percentagemPerda <= 10)
    {
        printf("  Qualidade: PERDA PARCIAL (rede instavel)\n");
    }
    else
    {
        printf("  Qualidade: PERDA ELEVADA (verificar rede)\n");
    }
}

// Função auxiliar para registar no log de monitorizacao
static void registarLog(const char *mensagem)
{
    FILE *log = fopen("log_monitorizacao.txt", "a");
    if (log != NULL)
    {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
        
        fprintf(log, "[%s] %s\n", dataHora, mensagem);
        fclose(log);
    }
}

// Função auxiliar para listar equipamentos (formato resumido)
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
void executarPingEquipamento(Sistema *s)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |              TESTE DE PING A EQUIPAMENTO                  |\n");
    printf("  =====================================================================\n");
    
    // Verifica se existem equipamentos
    if (s->equipamentos == NULL)
    {
        printf("\n  [!] Nao existem equipamentos registados.\n");
        printf("  [!] Registe equipamentos no Modulo 1 primeiro.\n");
        pausar();
        return;
    }
    
    // Lista equipamentos disponiveis - PERCORRER LISTA
    printf("\n  Equipamentos disponiveis:\n");
    listarEquipamentosResumido(s);
    
    // Solicita o codigo do equipamento
    int codigo = lerInteiro("\n  Insira o codigo do equipamento para testar", 
                            1, s->proximoCodigoEquip - 1);
    
    // Procura o equipamento - FUNÇÃO PROCURAR
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
    
    char ficheiroResultado[100];
    snprintf(ficheiroResultado, sizeof(ficheiroResultado), "ping_temp_%d.txt", codigo);
    
    // Executar ping com 4 pacotes para obter estatisticas
    executarPingEstatisticas(equipamento->dados.ip, ficheiroResultado);
    
    // Analisar resultado e extrair estatisticas
    PingEstatisticas stats = extrairEstatisticasPing(ficheiroResultado);
    
    // Mostrar estatisticas
    mostrarEstatisticasPing(&stats);
    
    // Copiar para o ficheiro padrao
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
    
    // Remover ficheiro temporario
    remove(ficheiroResultado);
    
    // Atualizar data da ultima verificacao
    obterDataAtual(equipamento->dados.dataUltimaVerificacao);
    
    // Se não respondeu, alterar estado para EM_FALHA
    if (!stats.respondeu)
    {
        equipamento->dados.estado = EM_FALHA;
        printf("\n  [AVISO] Estado do equipamento alterado para EM_FALHA\n");
        
        // Criar incidente automaticamente
        if (!temIncidentePendente(s, codigo))
        {
            NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
            if (novo != NULL)
            {
                novo->dados.codigo = ++s->proximoCodigoInc;
                novo->dados.codigoEquipamento = codigo;
                strcpy(novo->dados.descricao, "Falha de comunicacao - Equipamento nao respondeu ao ping");
                novo->dados.estado = PENDENTE;
                novo->dados.prioridade = 1;
                obterDataAtual(novo->dados.dataAbertura);
                strcpy(novo->dados.dataFecho, "-");
                
                novo->proximo = s->incidentes;
                s->incidentes = novo;
                s->totalIncidentes++;
                
                adicionarNaFilaAtendimento(s, novo->dados.codigo);
                
                printf("  [OK] Incidente #%d criado automaticamente\n", novo->dados.codigo);
            }
        }
    }
    
    // Registar no log
    char logMsg[300];
    snprintf(logMsg, sizeof(logMsg), "Ping para %s (%s) - Perda: %.1f%% - Tempo medio: %.1fms",
             equipamento->dados.nome, equipamento->dados.ip,
             stats.percentagemPerda, stats.tempoMedio);
    registarLog(logMsg);
    
    // Guardar alteracoes
    guardarFicheiro(s);
    
    printf("\n  [OK] Ping executado. Resultado guardado em 'resultado_ping.txt'\n");
    pausar();
}

// 2. Guardar o resultado bruto do comando num ficheiro de texto
void guardarResultadoPing(Sistema *s)
{
    (void)s;  // Parametro nao utilizado
    
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |              GUARDAR RESULTADO DO PING                    |\n");
    printf("  =====================================================================\n");
    
    // Verificar se o ficheiro de resultado existe
    FILE *origem = fopen("resultado_ping.txt", "r");
    if (origem == NULL)
    {
        printf("\n  [!] Nenhum resultado de ping encontrado!\n");
        printf("  [!] Execute primeiro o teste de ping (opcao 1).\n");
        pausar();
        return;
    }
    
    // Criar nome do ficheiro com timestamp
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char nomeFicheiro[100];
    
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
    
    // Registar no log
    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Resultado do ping guardado em: %s", nomeFicheiro);
    registarLog(logMsg);
    
    pausar();
}

// 3. Ler o ficheiro de resultado e determinar se o equipamento respondeu (com estatisticas)
void verificarRespostaPing(Sistema *s)
{
    (void)s;  // Parametro nao utilizado
    
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
    
    // Extrair estatisticas
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
    
    // Mostrar estatisticas
    mostrarEstatisticasPing(&stats);
    
    // Registar no log
    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Verificacao de ping: %s - Perda: %.1f%% - Tempo medio: %.1fms",
             stats.respondeu ? "ONLINE" : "OFFLINE",
             stats.percentagemPerda, stats.tempoMedio);
    registarLog(logMsg);
    
    pausar();
}

// 4. Atualizar automaticamente a data da ultima verificacao do equipamento
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
    
    // Solicitar codigo
    int codigo = lerInteiro("\n  Insira o codigo do equipamento", 1, s->proximoCodigoEquip - 1);
    
    // Procurar equipamento
    NodeEquipamento *encontrado = encontrarPorCodigo(s, codigo);
    if (encontrado == NULL)
    {
        printf("\n  [!] Equipamento com codigo %d nao encontrado!\n", codigo);
        pausar();
        return;
    }
    
    // Mostrar dados atuais
    printf("\n  ---------------------------------------------------------------------\n");
    printf("  Equipamento: %s\n", encontrado->dados.nome);
    printf("  Data atual da ultima verificacao: %s\n", encontrado->dados.dataUltimaVerificacao);
    printf("  ---------------------------------------------------------------------\n");
    
    // Confirmar atualizacao
    int confirmar = lerInteiro("\n  Deseja atualizar a data para o momento atual? (1-Sim, 2-Nao)", 1, 2);
    
    if (confirmar == 1)
    {
        obterDataAtual(encontrado->dados.dataUltimaVerificacao);
        
        printf("\n  [OK] Data atualizada com sucesso!\n");
        printf("  [OK] Nova data: %s\n", encontrado->dados.dataUltimaVerificacao);
        
        // Registar no log
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

// 5. Alterar o estado do equipamento para EM_FALHA quando nao existir resposta
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
    
    // Verificar estado atual
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
        obterDataAtual(encontrado->dados.dataUltimaVerificacao);
        
        printf("\n  [OK] Estado alterado para EM_FALHA com sucesso!\n");
        
        // Registar no log
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
void registarTesteLog(Sistema *s)
{
    (void)s;  // Parametro nao utilizado
    
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
    
    // Obter timestamp para mostrar
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char dataHora[50];
    strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
    
    // Registar no log
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

// 7. Criar automaticamente um incidente tecnico quando um equipamento nao responde
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
    
    // Verificar se ja existe incidente pendente
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
    
    char descricao[MAX_DESC];
    snprintf(descricao, MAX_DESC, 
             "Falha de comunicacao - Equipamento nao respondeu ao ping (Perda: %.1f%%)",
             stats.percentagemPerda);
    
    novo->dados.codigo = ++s->proximoCodigoInc;
    novo->dados.codigoEquipamento = codigo;
    strcpy(novo->dados.descricao, descricao);
    novo->dados.estado = PENDENTE;
    novo->dados.prioridade = 1;  // Prioridade ALTA
    obterDataAtual(novo->dados.dataAbertura);
    strcpy(novo->dados.dataFecho, "-");
    
    // Inserir na lista ligada
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
    
    // Registar no log
    char logMsg[300];
    snprintf(logMsg, sizeof(logMsg), "INCIDENTE AUTOMATICO #%d - Equipamento %d (%s) nao respondeu ao ping (Perda: %.1f%%)",
             novo->dados.codigo, codigo, equipEncontrado->dados.nome, stats.percentagemPerda);
    registarLog(logMsg);
    
    // Guardar alteracoes
    guardarFicheiro(s);
    
    pausar();
}

// 8. Permitir um teste geral da rede, executando ping a todos os equipamentos registados
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
    
    // Contadores
    int online = 0;
    int offline = 0;
    int incidentesCriados = 0;
    int contador = 0;
    float perdaTotal = 0;
    int equipamentosComPerda = 0;
    
    // Alocar array para resultados
    ResultadoTeste *resultados = (ResultadoTeste*)malloc(s->totalEquipamentos * sizeof(ResultadoTeste));
    if (resultados == NULL)
    {
        printf("\n  [!] Erro ao alocar memoria!\n");
        pausar();
        return;
    }
    
    NodeEquipamento *atual = s->equipamentos;
    
    while (atual != NULL)
    {
        printf("\n  [%d/%d] Testando: %s\n", contador + 1, s->totalEquipamentos, atual->dados.nome);
        printf("         IP: %s\n", atual->dados.ip);
        
        // Ficheiro temporario
        char ficheiroTemp[100];
        snprintf(ficheiroTemp, sizeof(ficheiroTemp), "temp_ping_%d.txt", atual->dados.codigo);
        
        // Executar ping com 4 pacotes para estatisticas
        executarPingEstatisticas(atual->dados.ip, ficheiroTemp);
        
        // Extrair estatisticas
        PingEstatisticas stats = extrairEstatisticasPing(ficheiroTemp);
        
        // Guardar resultado
        resultados[contador].codigo = atual->dados.codigo;
        strcpy(resultados[contador].nome, atual->dados.nome);
        strcpy(resultados[contador].ip, atual->dados.ip);
        resultados[contador].respondeu = stats.respondeu;
        
        // Atualizar data da ultima verificacao
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
            
            // Alterar estado para EM_FALHA
            atual->dados.estado = EM_FALHA;
            
            // Verificar se ja existe incidente pendente
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
                    novo->dados.prioridade = 1;
                    obterDataAtual(novo->dados.dataAbertura);
                    strcpy(novo->dados.dataFecho, "-");
                    
                    novo->proximo = s->incidentes;
                    s->incidentes = novo;
                    s->totalIncidentes++;
                    incidentesCriados++;
                    
                    printf("         Incidente #%d criado\n", novo->dados.codigo);
                    
                    // Adicionar a fila
                    adicionarNaFilaAtendimento(s, novo->dados.codigo);
                }
            }
        }
        
        // Remover ficheiro temporario
        remove(ficheiroTemp);
        
        // Pequena pausa entre pings
        #ifdef _WIN32
            Sleep(500);
        #else
            usleep(500000);
        #endif
        
        atual = atual->proximo;
        contador++;
    }
    
    // Calcular perda media
    float perdaMedia = (equipamentosComPerda > 0) ? (perdaTotal / equipamentosComPerda) : 0;
    
    // Relatorio final
    printf("\n  ---------------------------------------------------------------------\n");
    printf("  RELATORIO DO TESTE GERAL\n");
    printf("  ---------------------------------------------------------------------\n");
    printf("  Total de equipamentos testados: %d\n", s->totalEquipamentos);
    printf("  Equipamentos ONLINE : %d\n", online);
    printf("  Equipamentos OFFLINE: %d\n", offline);
    printf("  Incidentes criados: %d\n", incidentesCriados);
    printf("  Perda media de pacotes: %.1f%%\n", perdaMedia);
    printf("  ---------------------------------------------------------------------\n");
    
    // Classificacao da qualidade da rede
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
    
    // Registar no log
    char logMsg[300];
    snprintf(logMsg, sizeof(logMsg), "TESTE GERAL: %d equipamentos, %d online, %d offline, %d incidentes, Perda media: %.1f%%",
             s->totalEquipamentos, online, offline, incidentesCriados, perdaMedia);
    registarLog(logMsg);
    
    // Guardar alteracoes
    guardarFicheiro(s);
    
    free(resultados);
    
    printf("\n  [OK] Teste geral concluido!\n");
    pausar();
}

// 9. Outras atividades que considere relevantes (comandos de rede adicionais)
void executarComandosRedeExtra(Sistema *s)
{
    (void)s;  // Parametro nao utilizado para evitar warning
    
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
void obterInfoRedeLocal(void)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |              INFORMACAO DA REDE LOCAL                     |\n");
    printf("  =====================================================================\n");
    
    printf("\n  A obter informacao da rede local...\n");
    
#ifdef _WIN32
    system("ipconfig > resultado_rede_local.txt");
#else
    system("ip addr > resultado_rede_local.txt");
#endif
    
    printf("\n  [OK] Informacao guardada em: resultado_rede_local.txt\n");
    registarLog("Informacao da rede local obtida");
}

void obterTabelaARP(void)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |                    TABELA ARP                             |\n");
    printf("  =====================================================================\n");
    
    printf("\n  A obter tabela ARP...\n");
    system("arp -a > resultado_arp.txt");
    
    printf("\n  [OK] Tabela ARP guardada em: resultado_arp.txt\n");
    registarLog("Tabela ARP obtida");
}

void obterResolucaoDNS(void)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |                  RESOLUCAO DNS                            |\n");
    printf("  =====================================================================\n");
    
    char dominio[100];
    lerString("  Insira o dominio (ex: google.com)", dominio, sizeof(dominio));
    
    printf("\n  A resolver %s...\n", dominio);
    
#ifdef _WIN32
    char comando[200];
    snprintf(comando, sizeof(comando), "nslookup %s > resultado_dns.txt", dominio);
#else
    char comando[200];
    snprintf(comando, sizeof(comando), "nslookup %s > resultado_dns.txt", dominio);
#endif
    
    system(comando);
    
    printf("\n  [OK] Resolucao DNS guardada em: resultado_dns.txt\n");
    
    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Resolucao DNS para %s", dominio);
    registarLog(logMsg);
}

void obterRotaDestino(void)
{
    limparEcra();
    printf("\n  =====================================================================\n");
    printf("  |              ROTA ATE DESTINO                             |\n");
    printf("  =====================================================================\n");
    
    char destino[100];
    lerString("  Insira o IP ou dominio (ex: 8.8.8.8 ou google.com)", destino, sizeof(destino));
    
    printf("\n  A tracar rota ate %s...\n", destino);
    
#ifdef _WIN32
    char comando[200];
    snprintf(comando, sizeof(comando), "tracert %s > resultado_rota.txt", destino);
#else
    char comando[200];
    snprintf(comando, sizeof(comando), "traceroute %s > resultado_rota.txt", destino);
#endif
    
    system(comando);
    
    printf("\n  [OK] Rota guardada em: resultado_rota.txt\n");
    
    char logMsg[200];
    snprintf(logMsg, sizeof(logMsg), "Rota ate %s", destino);
    registarLog(logMsg);
}

/*
 * Menu do Modulo 2
 */

void menuConectividade(Sistema *s)
{
    int opcao;
    
    do
    {
        limparEcra();
        printf("\n  ================================================================\n");
        printf("  |              MODULO 2 - TESTES DE CONECTIVIDADE            |\n");
        printf("  ================================================================\n");
        printf("  |  1. Executar ping a um equipamento                         |\n");
        printf("  |  2. Guardar resultado do ping em ficheiro                 |\n");
        printf("  |  3. Verificar se equipamento respondeu ao ping            |\n");
        printf("  |  4. Atualizar data da ultima verificacao                  |\n");
        printf("  |  5. Alterar estado para EM_FALHA                          |\n");
        printf("  |  6. Registar teste no log de monitorizacao                |\n");
        printf("  |  7. Criar incidente automaticamente (falha no ping)       |\n");
        printf("  |  8. Teste geral da rede (ping a todos)                    |\n");
        printf("  |  9. Comandos de rede extra                                |\n");
        printf("  |  0. Voltar                                                |\n");
        printf("  ================================================================\n");
        
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
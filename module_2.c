#include "noc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

/* ========================================================= */
/* MENU PRINCIPAL                                            */
/* ========================================================= */

void menuConectividade(Sistema *s) {

    int opcao;

    do {

        printf("===============================================================================================\n");
        printf("                        MODULO 2 - TESTES DE CONECTIVIDADE                                     \n");
        printf("===============================================================================================\n");
        printf("1. Selecionar um equipamento registado e executar um teste ping ao seu endereco IP.            \n");
        printf("2. Guardar o resultado bruto do comando num ficheiro de texto.                                 \n");
        printf("3. Ler o ficheiro de resultado e determinar se o equipamento respondeu.                        \n");
        printf("4. Atualizar automaticamente a data da ultima verificacao do equipamento.                      \n");
        printf("5. Alterar o estado do equipamento para EM_FALHA quando nao existir resposta.                  \n");
        printf("6. Registar cada teste realizado num ficheiro de texto denominado log_monitorizacao.txt.       \n");
        printf("7. Criar automaticamente um incidente tecnico quando um equipamento nao responde.              \n");
        printf("8. Permitir um teste geral da rede, executando ping a todos os equipamentos registados.        \n");
        printf("9. Outras atividades que considere relevantes.                                                 \n");
        printf("-----------------------------------------------------------------------------------------------\n");
        printf("0. Voltar\n");
        printf("===============================================================================================\n");
        printf("Opcao: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Entrada invalida!\n");
            limparBuffer();
            continue;
        }

        limparBuffer();

        switch(opcao) {

            case 1:
                executarPingEquipamento(s);
                break;

            case 2:
                guardarResultadoPing(s);
                break;

            case 3:
                verificarRespostaPing(s);
                break;

            case 4:
                atualizarDataVerificacao(s);
                break;

           case 5: {
                alterarEstadoFalha(s);
                break;
            }
            
            case 6:
                registarTesteLog(s);
                break;

            case 7:
                criarIncidenteAutomatico(s);
                break;

            case 8:
                pingGeral(s);
                break;

            case 9:
                executarComandosRedeExtra(s);
                break;

            case 0:
                printf("A voltar...\n");
                break;

            default:
                printf("Opcao invalida!\n");
        }

    } while(opcao != 0);
}

void executarPingEquipamento(Sistema *s) {
    (void)s;  // Parâmetro não utilizado (evita warning)
    // Declarar variáveis no início (estilo professor)
    NodeEquipamento *posicao = NULL;
    NodeEquipamento *equipamento = NULL;
    int codigo;
    char comando[300];
    
    // 1. Verifica se existem equipamentos registados
    if (s->equipamentos == NULL) {
        printf("Nao existem equipamentos registados!\n");
        printf("Por favor, registe equipamentos no Modulo 1 primeiro.\n");
        pausar();
        return;
    }

    // 2. Lista equipamentos disponiveis
    printf("\n=========== EQUIPAMENTOS DISPONIVEIS ===========\n");
    posicao = s->equipamentos;
    while (posicao != NULL) {
        printf("Codigo: %d | Nome: %s | IP: %s\n",
               posicao->dados.codigo,
               posicao->dados.nome,
               posicao->dados.ip);
        posicao = posicao->proximo;
    }
    printf("================================================\n");

    // 3. Solicita o codigo do equipamento
    codigo = lerInteiro("Insira o codigo do equipamento para testar", 1, 9999);

    // 4. Procura o equipamento
     equipamento = encontrarPorCodigo(s, codigo);
    if (equipamento == NULL) {
        printf("Equipamento com codigo %d nao encontrado!\n", codigo);
        pausar();
        return;
    }
    
    // 5. Executa o ping e guarda em ficheiro
    printf("\n================================================\n");
    printf("A testar equipamento: %s\n", equipamento->dados.nome);
    printf("Endereco IP: %s\n", equipamento->dados.ip);
    printf("================================================\n");
    
    #ifdef _WIN32
        snprintf(comando, sizeof(comando), "ping -n 4 %s > resultado_ping.txt", equipamento->dados.ip);
    #else
        snprintf(comando, sizeof(comando), "ping -c 4 %s > resultado_ping.txt", equipamento->dados.ip);
    #endif
    
    system(comando);
    printf("\nPing executado. Resultado guardado em resultado_ping.txt\n");
    
    pausar();
}



void guardarResultadoPing(Sistema *s) {
    
    (void)s;  // Parâmetro não utilizado (evita warning)
    
    FILE *origem = NULL;
    FILE *destino = NULL;
    FILE *log = NULL;
    char nomeFicheiro[100];
    char linha[256];
    time_t t;
    struct tm *tm_info;
    
    // Verificar se o ficheiro de resultado existe
    origem = fopen("resultado_ping.txt", "r");
    if (origem == NULL) {
        printf("\n[ERRO] Nenhum resultado de ping encontrado!\n");
        printf("Execute primeiro o teste de ping (opcao 1).\n");
        return;
    }
    
    // Criar nome do ficheiro com timestamp
    t = time(NULL);
    tm_info = localtime(&t);
    
    snprintf(nomeFicheiro, sizeof(nomeFicheiro), 
             "ping_%04d%02d%02d_%02d%02d%02d.txt",
             tm_info->tm_year + 1900,
             tm_info->tm_mon + 1,
             tm_info->tm_mday,
             tm_info->tm_hour,
             tm_info->tm_min,
             tm_info->tm_sec);
    
    // Criar ficheiro de destino
    destino = fopen(nomeFicheiro, "w");
    if (destino == NULL) {
        printf("[ERRO] Nao foi possivel criar o ficheiro %s!\n", nomeFicheiro);
        fclose(origem);
        return;
    }
    
    // Copiar conteudo
    while (fgets(linha, sizeof(linha), origem) != NULL) {
        fputs(linha, destino);
    }
    
    // Fechar ficheiros
    fclose(origem);
    fclose(destino);
    
    printf("\n================================================\n");
    printf("Resultado do ping guardado com sucesso!\n");
    printf("Ficheiro: %s\n", nomeFicheiro);
    printf("================================================\n");
    
    // Registar no log
    log = fopen("log_monitorizacao.txt", "a");
    if (log != NULL) {
        time_t t2 = time(NULL);
        struct tm *tm_info2 = localtime(&t2);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info2);
        
        fprintf(log, "[%s] Resultado do ping guardado em: %s\n", dataHora, nomeFicheiro);
        fclose(log);
    }
    
    pausar();
}

       

void verificarRespostaPing(Sistema *s) {
    (void)s;  // Parâmetro não utilizado (evita warning)
    char linha[300];
    int respondeu = 0;
    
    // Abrir o ficheiro de resultado do ping
    FILE *f = fopen("resultado_ping.txt", "r");
    if (f == NULL) {
        printf("\n[ERRO] Nenhum resultado de ping encontrado!\n");
        printf("Execute primeiro o teste de ping (opcao 1).\n");
        pausar();
        return;
    }
    
    // Analisar linha por linha
    while (fgets(linha, sizeof(linha), f) != NULL) {
        
        // Verificar indicadores de resposta (Windows)
        if (strstr(linha, "TTL=") != NULL || 
            strstr(linha, "ttl=") != NULL ||
            strstr(linha, "Reply from") != NULL ||
            strstr(linha, "Resposta de") != NULL) {
            respondeu = 1;
            break;
        }
        
        // Verificar indicadores de resposta (Linux/Unix)
        if (strstr(linha, "bytes from") != NULL) {
            respondeu = 1;
            break;
        }
    }
    
    fclose(f);
    
    // Mostrar resultado formatado
    printf("\n================================================\n");
    printf("              RESULTADO DO PING                  \n");
    printf("================================================\n");
    
    if (respondeu) {
        printf("ESTADO: EQUIPAMENTO RESPONDEU (ONLINE)\n");
        printf("O equipamento esta ativo na rede.\n");
    } else {
        printf("ESTADO: EQUIPAMENTO NAO RESPONDEU (OFFLINE)\n");
        printf("O equipamento pode estar desligado ou com problemas.\n");
    }
    printf("================================================\n");
    
    // Registar a verificacao no log
    FILE *log = fopen("log_monitorizacao.txt", "a");
    if (log != NULL) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
        
        fprintf(log, "[%s] Verificacao do ping: %s\n", 
                dataHora, 
                respondeu ? "ONLINE" : "OFFLINE");
        fclose(log);
    }
    pausar();
}

void alterarEstadoFalha(Sistema *s) {
    int codigoEquipamento;
    NodeEquipamento *posicao = NULL;
    NodeEquipamento *encontrado = NULL;
    
    // Pedir o código do equipamento dentro da função
    printf("Insira o codigo do equipamento para alterar para EM_FALHA: ");
    scanf("%d", &codigoEquipamento);
    
    // Verificar se existem equipamentos
    if (s->equipamentos == NULL) {
        return;
    }
    
    // PERCORRER A LISTA LIGADA para encontrar o equipamento
    posicao = s->equipamentos;
    while (posicao != NULL) {
        if (posicao->dados.codigo == codigoEquipamento) {
            encontrado = posicao;
            break;
        }
        posicao = posicao->proximo;
    }
    
    if (encontrado == NULL) {
        printf("Equipamento não encontrado!\n");
        return;
    }
    
    // Verificar se já está em falha
    if (encontrado->dados.estado == EM_FALHA) {
        printf("Equipamento já está em estado EM_FALHA!\n");
        return;
    }
    
    // ALTERAR O ESTADO PARA EM_FALHA
    encontrado->dados.estado = EM_FALHA;
    
    // Atualizar a data da última verificação
    obterDataAtual(encontrado->dados.dataUltimaVerificacao);
    
    // Registar no log
    FILE *log = fopen("log_monitorizacao.txt", "a");
    if (log != NULL) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
        
        fprintf(log, "[%s] Estado do equipamento %d (%s) alterado para EM_FALHA (falha no ping)\n",
                dataHora, 
                encontrado->dados.codigo, 
                encontrado->dados.nome);
        fclose(log);
    }
    pausar();
    printf("Estado do equipamento %d alterado para EM_FALHA com sucesso!\n", codigoEquipamento);
}

void registarTesteLog(Sistema *s) {
    (void)s;  // Parâmetro não utilizado (evita warning)
    FILE *log = NULL;
    time_t t;
    struct tm *tm_info;
    char dataHora[50];
    char linha[300];
    int respondeu = 0;
    
    // Verificar se existe resultado de ping
    FILE *f = fopen("resultado_ping.txt", "r");
    if (f == NULL) {
        printf("\n[ERRO] Nenhum resultado de ping encontrado!\n");
        printf("Execute primeiro o teste de ping (opcao 1).\n");
        pausar();
        return;
    }
    
    // Analisar o resultado do ping para saber se respondeu
    while (fgets(linha, sizeof(linha), f) != NULL) {
        if (strstr(linha, "TTL=") != NULL || 
            strstr(linha, "ttl=") != NULL ||
            strstr(linha, "Reply from") != NULL ||
            strstr(linha, "bytes from") != NULL) {
            respondeu = 1;
            break;
        }
    }
    fclose(f);
    
    // Abrir ficheiro de log em modo append
    log = fopen("log_monitorizacao.txt", "a");
    if (log == NULL) {
        printf("\n[ERRO] Nao foi possivel abrir o ficheiro de log!\n");
        pausar();
        return;
    }
    
    // Obter data e hora atual
    t = time(NULL);
    tm_info = localtime(&t);
    strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
    
    // Escrever no log (formato automático, sem perguntar)
    fprintf(log, "[%s] Teste de ping executado - Resultado: %s\n",
            dataHora,
            respondeu ? "RESPONDEU" : "NAO RESPONDEU");
    
    fclose(log);
    
    printf("\n================================================\n");
    printf("Log registado com sucesso!\n");
    printf("Ficheiro: log_monitorizacao.txt\n");
    printf("Data/Hora: %s\n", dataHora);
    printf("Resultado: %s\n", respondeu ? "RESPONDEU" : "NAO RESPONDEU");
    printf("================================================\n");
    
    pausar();
}

void criarIncidenteAutomatico(Sistema *s) {
    
    NodeEquipamento *posicao = NULL;
    NodeEquipamento *equipEncontrado = NULL;
    NodeIncidente *novo = NULL;
    NodeIncidente *incAtual = NULL;
    FILE *f = NULL;
    char linha[300];
    int respondeu = 0;
    int codigoEquipamento = -1;
    
    // Verificar se existem equipamentos
    if (s->equipamentos == NULL) {
        return;  // Sai silenciosamente
    }
    
    // Verificar se existe resultado de ping
    f = fopen("resultado_ping.txt", "r");
    if (f == NULL) {
        return;  // Nenhum teste de ping executado
    }
    
    // Analisar se o ping respondeu
    while (fgets(linha, sizeof(linha), f) != NULL) {
        if (strstr(linha, "TTL=") != NULL || 
            strstr(linha, "ttl=") != NULL ||
            strstr(linha, "Reply from") != NULL ||
            strstr(linha, "bytes from") != NULL) {
            respondeu = 1;
            break;
        }
    }
    fclose(f);
    
    // Se respondeu, não criar incidente
    if (respondeu) {
        return;
    }
    
    // Para saber qual equipamento falhou, precisamos do último IP testado
    // Vamos procurar nos equipamentos qual foi o último testado?
    // Alternativa: usar o ficheiro de log ou passar como parâmetro
    
    // Como a função não recebe parâmetro, vamos usar uma abordagem diferente:
    // Percorrer todos os equipamentos e verificar se algum está em estado EM_FALHA
    // mas sem incidente associado
    
    posicao = s->equipamentos;
    while (posicao != NULL) {
        if (posicao->dados.estado == EM_FALHA) {
            // Verificar se já existe incidente pendente para este equipamento
            incAtual = s->incidentes;
            int incidenteExiste = 0;
            
            while (incAtual != NULL) {
                if (incAtual->dados.codigoEquipamento == posicao->dados.codigo && 
                    incAtual->dados.estado == PENDENTE) {
                    incidenteExiste = 1;
                    break;
                }
                incAtual = incAtual->proximo;
            }
            
            if (!incidenteExiste) {
                codigoEquipamento = posicao->dados.codigo;
                equipEncontrado = posicao;
                break;
            }
        }
        posicao = posicao->proximo;
    }
    
    if (equipEncontrado == NULL) {
        return;  // Nenhum equipamento em falha sem incidente
    }
    
    // =========================================================
    // CRIAR INCIDENTE AUTOMATICAMENTE (SEM PERGUNTAR)
    // =========================================================
    
    novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
    if (novo == NULL) {
        return;
    }
    
    // Preencher os dados do incidente (automático)
    novo->dados.codigo = ++s->proximoCodigoInc;
    novo->dados.codigoEquipamento = codigoEquipamento;
    strcpy(novo->dados.descricao, "Falha de comunicacao - Equipamento nao respondeu ao ping");
    novo->dados.estado = PENDENTE;
    novo->dados.prioridade = 1;  // Prioridade ALTA para falha de ping
    obterDataAtual(novo->dados.dataAbertura);
    strcpy(novo->dados.dataFecho, "-");
    
    // INSERIR NA LISTA LIGADA DE INCIDENTES
    novo->proximo = s->incidentes;
    s->incidentes = novo;
    s->totalIncidentes++;
    
    // Adicionar à FILA de atendimento
    adicionarNaFilaAtendimento(s, novo->dados.codigo);
    
    // Mostrar mensagem ao utilizador
    printf("\n================================================\n");
    printf(" INCIDENTE CRIADO AUTOMATICAMENTE (FALHA NO PING)\n");
    printf("================================================\n");
    printf("ID do incidente: #%d\n", novo->dados.codigo);
    printf("Equipamento: %s (Codigo: %d)\n", equipEncontrado->dados.nome, codigoEquipamento);
    printf("Descricao: %s\n", novo->dados.descricao);
    printf("Prioridade: ALTA\n");
    printf("Estado: PENDENTE\n");
    printf("Data de abertura: %s\n", novo->dados.dataAbertura);
    printf("================================================\n");
    
    // Registar no log
    FILE *log = fopen("log_monitorizacao.txt", "a");
    if (log != NULL) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
        
        fprintf(log, "[%s] INCIDENTE AUTOMATICO #%d - Equipamento %d (%s) nao respondeu ao ping\n",
                dataHora, novo->dados.codigo, codigoEquipamento, equipEncontrado->dados.nome);
        fclose(log);
    }
    
    // Guardar alterações
    guardarFicheiro(s);
}


void pingGeral(Sistema *s) {
    
    NodeEquipamento *atual = NULL;
    int online = 0;
    int offline = 0;
    int contador = 0;
    int incidentesCriados = 0;
    
    // Verificar se existem equipamentos
    if (s->equipamentos == NULL) {
        printf("\n[ERRO] Nao existem equipamentos registados!\n");
        pausar();
        return;
    }
    
    printf("\n================================================\n");
    printf("           TESTE GERAL DA REDE                  \n");
    printf("================================================\n");
    printf("A testar todos os equipamentos registados...\n");
    printf("================================================\n");
    
    // Estrutura para guardar resultados
    typedef struct ResultadoTeste {
        int codigo;
        char nome[MAX_NOME];
        char ip[MAX_IP];
        int respondeu;
    } ResultadoTeste;
    
    ResultadoTeste *resultados = (ResultadoTeste*)malloc(s->totalEquipamentos * sizeof(ResultadoTeste));
    if (resultados == NULL) {
        printf("[ERRO] Falha ao alocar memoria!\n");
        pausar();
        return;
    }
    
    atual = s->equipamentos;
    
    while (atual != NULL) {
        
        printf("\n--- [%d/%d] Testando: %s ---\n", 
               contador + 1, s->totalEquipamentos, atual->dados.nome);
        printf("  IP: %s\n", atual->dados.ip);
        
        // Executar ping (1 pacote)
        char ficheiroTemp[100];
        snprintf(ficheiroTemp, sizeof(ficheiroTemp), "temp_ping_%d.txt", atual->dados.codigo);
        
#ifdef _WIN32
        char comando[300];
        snprintf(comando, sizeof(comando), "ping -n 1 %s > %s", atual->dados.ip, ficheiroTemp);
        system(comando);
#else
        char comando[300];
        snprintf(comando, sizeof(comando), "ping -c 1 %s > %s", atual->dados.ip, ficheiroTemp);
        system(comando);
#endif
        
        // Analisar resultado
        FILE *f = fopen(ficheiroTemp, "r");
        int respondeu = 0;
        
        if (f != NULL) {
            char linha[300];
            while (fgets(linha, sizeof(linha), f) != NULL) {
                if (strstr(linha, "TTL=") != NULL || 
                    strstr(linha, "ttl=") != NULL ||
                    strstr(linha, "bytes from") != NULL ||
                    strstr(linha, "Reply from") != NULL) {
                    respondeu = 1;
                    break;
                }
            }
            fclose(f);
        }
        
        // Guardar resultado
        resultados[contador].codigo = atual->dados.codigo;
        strcpy(resultados[contador].nome, atual->dados.nome);
        strcpy(resultados[contador].ip, atual->dados.ip);
        resultados[contador].respondeu = respondeu;
        
        // Atualizar data da ultima verificacao
        obterDataAtual(atual->dados.dataUltimaVerificacao);
        
        if (respondeu) {
            printf("  ✅ RESULTADO: ONLINE\n");
            online++;
        } else {
            printf("  ❌ RESULTADO: OFFLINE\n");
            offline++;
            
            // Alterar estado para EM_FALHA
            atual->dados.estado = EM_FALHA;
            printf("  📌 Estado alterado para EM_FALHA\n");
            
            // Criar incidente automaticamente
            NodeIncidente *novo = (NodeIncidente*)malloc(sizeof(NodeIncidente));
            if (novo != NULL) {
                novo->dados.codigo = ++s->proximoCodigoInc;
                novo->dados.codigoEquipamento = atual->dados.codigo;
                strcpy(novo->dados.descricao, "Equipamento falhou no teste geral da rede");
                novo->dados.estado = PENDENTE;
                novo->dados.prioridade = 1;  // ALTA
                obterDataAtual(novo->dados.dataAbertura);
                strcpy(novo->dados.dataFecho, "-");
                
                novo->proximo = s->incidentes;
                s->incidentes = novo;
                s->totalIncidentes++;
                incidentesCriados++;
                
                printf("  📌 Incidente #%d criado\n", novo->dados.codigo);
            }
        }
        
        // Remover ficheiro temporário
        remove(ficheiroTemp);
        
        // Pequena pausa (opcional)
    #ifdef _WIN32
            Sleep(50);
    #else
            usleep(50000);
    #endif
            
        atual = atual->proximo;
        contador++;
    }
    
    // Relatorio final
    printf("\n================================================\n");
    printf("           RELATORIO DO TESTE GERAL             \n");
    printf("================================================\n");
    printf("Total de equipamentos testados: %d\n", s->totalEquipamentos);
    printf("Equipamentos ONLINE : %d\n", online);
    printf("Equipamentos OFFLINE: %d\n", offline);
    printf("Incidentes criados: %d\n", incidentesCriados);
    printf("================================================\n");
    
    if (offline > 0) {
        printf("\n--- EQUIPAMENTOS OFFLINE ---\n");
        for (int i = 0; i < contador; i++) {
            if (!resultados[i].respondeu) {
                printf("  Codigo: %d | Nome: %s | IP: %s\n", 
                       resultados[i].codigo, resultados[i].nome, resultados[i].ip);
            }
        }
    }
    
    // Registar no log
    FILE *log = fopen("log_monitorizacao.txt", "a");
    if (log != NULL) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dataHora[50];
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
        
        fprintf(log, "[%s] TESTE GERAL: %d equipamentos, %d online, %d offline, %d incidentes\n",
                dataHora, s->totalEquipamentos, online, offline, incidentesCriados);
        fclose(log);
    }
    
    guardarFicheiro(s);
    free(resultados);
    
    printf("\nTeste geral concluido!\n");
    pausar();
}

void executarComandosRedeExtra(Sistema *s) {
    
    (void)s;  // Parâmetro não utilizado
    
    int opcao;
    
    do {
        printf("\n=========== COMANDOS DE REDE EXTRA ===========\n");
        printf("1. Informacao da rede local (ipconfig / ip addr)\n");
        printf("2. Tabela ARP (arp -a)\n");
        printf("3. Resolucao DNS (nslookup)\n");
        printf("4. Rota ate destino (tracert / traceroute)\n");
        printf("0. Voltar\n");
        printf("===============================================\n");
        
        opcao = lerInteiro("Escolha uma opcao", 0, 4);
        
        switch(opcao) {
            case 1:
                obterInfoRedeLocal();
                break;
            case 2:
                obterTabelaARP();
                break;
            case 3:
                obterResolucaoDNS();
                break;
            case 4:
                obterRotaDestino();
                break;
        }
    } while (opcao != 0);
}

void obterInfoRedeLocal() {
    printf("\nA obter informacao da rede local...\n");
    #ifdef _WIN32
        system("ipconfig > resultado_rede_local.txt");
        printf("Informacao guardada em: resultado_rede_local.txt\n");
    #else
        system("ip addr > resultado_rede_local.txt");
        printf("Informacao guardada em: resultado_rede_local.txt\n");
    #endif
}

void obterTabelaARP() {
    printf("\nA obter tabela ARP...\n");
    system("arp -a > resultado_arp.txt");
    printf("Tabela ARP guardada em: resultado_arp.txt\n");
}

void obterResolucaoDNS() {
    char dominio[100];
    printf("\nInsira o dominio (ex: google.com): ");
    scanf("%s", dominio);
    
    #ifdef _WIN32
        char comando[200];
        snprintf(comando, sizeof(comando), "nslookup %s > resultado_dns.txt", dominio);
        system(comando);
    #else
        char comando[200];
        snprintf(comando, sizeof(comando), "nslookup %s > resultado_dns.txt", dominio);
        system(comando);
    #endif
    
    printf("Resolucao DNS guardada em: resultado_dns.txt\n");
}

void obterRotaDestino() {
    char destino[100];
    printf("\nInsira o IP ou dominio (ex: 8.8.8.8 ou google.com): ");
    scanf("%s", destino);
    
    #ifdef _WIN32
        char comando[200];
        snprintf(comando, sizeof(comando), "tracert %s > resultado_rota.txt", destino);
        system(comando);
    #else
        char comando[200];
        snprintf(comando, sizeof(comando), "traceroute %s > resultado_rota.txt", destino);
        system(comando);
    #endif
        
        printf("Rota guardada em: resultado_rota.txt\n");
}

void atualizarDataVerificacao(Sistema *s) {
    
    (void)s;  // Parâmetro não utilizado (evita warning)
    
    NodeEquipamento *posicao = NULL;
    NodeEquipamento *encontrado = NULL;
    int codigo;
    
    // Verificar se existem equipamentos
    if (s->equipamentos == NULL) {
        printf("\n[ERRO] Nao existem equipamentos registados!\n");
        pausar();
        return;
    }
    
    // Listar equipamentos disponíveis
    printf("\n=========== EQUIPAMENTOS DISPONIVEIS ===========\n");
    posicao = s->equipamentos;
    while (posicao != NULL) {
        printf("  Codigo: %d | Nome: %s | Data verificacao: %s\n",
               posicao->dados.codigo,
               posicao->dados.nome,
               posicao->dados.dataUltimaVerificacao);
        posicao = posicao->proximo;
    }
    printf("================================================\n");
    
    // Pedir código do equipamento
    codigo = lerInteiro("Insira o codigo do equipamento", 1, 9999);
    
    // Procurar equipamento
    posicao = s->equipamentos;
    while (posicao != NULL) {
        if (posicao->dados.codigo == codigo) {
            encontrado = posicao;
            break;
        }
        posicao = posicao->proximo;
    }
    
    if (encontrado == NULL) {
        printf("\n[ERRO] Equipamento com codigo %d nao encontrado!\n", codigo);
        pausar();
        return;
    }
    
    // Mostrar dados atuais
    printf("\n================================================\n");
    printf("Equipamento encontrado: %s\n", encontrado->dados.nome);
    printf("Data atual da ultima verificacao: %s\n", encontrado->dados.dataUltimaVerificacao);
    printf("================================================\n");
    
    // Confirmar atualização
    int confirmar = lerInteiro("Deseja atualizar a data para o momento atual? (1-Sim, 2-Nao)", 1, 2);
    
    if (confirmar == 1) {
        // Atualizar data
        obterDataAtual(encontrado->dados.dataUltimaVerificacao);
        
        printf("\n[OK] Data atualizada com sucesso!\n");
        printf("Nova data: %s\n", encontrado->dados.dataUltimaVerificacao);
        
        // Registar no log
        FILE *log = fopen("log_monitorizacao.txt", "a");
        if (log != NULL) {
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char dataHora[50];
            strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);
            
            fprintf(log, "[%s] Data de verificacao atualizada para equipamento %d (%s)\n",
                    dataHora, encontrado->dados.codigo, encontrado->dados.nome);
            fclose(log);
        }
        
        // Guardar alterações
        guardarFicheiro(s);
        
    } else {
        printf("\nAtualizacao cancelada.\n");
    }
    
    pausar();
}
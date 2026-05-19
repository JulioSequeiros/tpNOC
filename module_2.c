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

#include "noc.h"

/* ========================================================= */
/* VALIDACAO DE INPUT                                        */
/* ========================================================= */

int entradaValida(const char *s) {

    if (s == NULL || strlen(s) == 0) {

        return 0;
    }

    while (*s) {

        /*
         * Apenas permitir:
         * letras
         * numeros
         * .
         * -
         * :
         */

        if (!(isalnum((unsigned char)*s) ||
              *s == '.' ||
              *s == '-' ||
              *s == ':')) {

            return 0;
        }

        s++;
    }

    return 1;
}

/* ========================================================= */
/* EXECUTAR PING SEGURO                                      */
/* ========================================================= */

int executarPingSeguro(const char *ip,
                       const char *ficheiroSaida,
                       int quantidade) {

    char comando[300];

    if (!entradaValida(ip)) {

        printf("IP ou dominio invalido!\n");

        return 0;
    }

#ifdef _WIN32

    snprintf(comando,
             sizeof(comando),
             "ping -n %d %s > %s",
             quantidade,
             ip,
             ficheiroSaida);

#else

    snprintf(comando,
             sizeof(comando),
             "ping -c %d %s > %s",
             quantidade,
             ip,
             ficheiroSaida);

#endif

    return system(comando);
}

/* ========================================================= */
/* ANALISAR RESULTADO DO PING                                */
/* ========================================================= */

int verificarRespostaFicheiro(const char *nomeFicheiro) {

    FILE *f;

    char linha[300];

    int respondeu = 0;

    f = fopen(nomeFicheiro, "r");

    if (f == NULL) {

        return 0;
    }

    while (fgets(linha, sizeof(linha), f) != NULL) {

        if (strstr(linha, "TTL=") != NULL ||
            strstr(linha, "ttl=") != NULL) {

            respondeu = 1;
        }
    }

    fclose(f);

    return respondeu;
}

/* ========================================================= */
/* REGISTAR LOG                                              */
/* ========================================================= */

void escreverLog(const char *mensagem) {

    FILE *log;

    time_t t;

    struct tm *tm_info;

    char data[50];

    log = fopen("log_monitorizacao.txt", "a");

    if (log == NULL) {

        return;
    }

    t = time(NULL);

    tm_info = localtime(&t);

    strftime(data,
             sizeof(data),
             "%d/%m/%Y %H:%M:%S",
             tm_info);

    fprintf(log,
            "[%s] %s\n",
            data,
            mensagem);

    fclose(log);
}

/* ========================================================= */
/* MENU PRINCIPAL                                            */
/* ========================================================= */

void menuConectividade(Sistema *s) {

    int opcao;

    do {

        printf("===============================================================================================\n");
        printf("                        MODULO 2 - TESTES DE CONECTIVIDADE                                     \n");
        printf("===============================================================================================\n");

        printf("1. Selecionar um equipamento registado e executar um teste ping ao seu endereco IP.\n");
        printf("2. Guardar o resultado bruto do comando num ficheiro de texto.\n");
        printf("3. Ler o ficheiro de resultado e determinar se o equipamento respondeu.\n");
        printf("4. Atualizar automaticamente a data da ultima verificacao do equipamento.\n");
        printf("5. Alterar o estado do equipamento para EM_FALHA quando nao existir resposta.\n");
        printf("6. Registar cada teste realizado num ficheiro de texto denominado log_monitorizacao.txt.\n");
        printf("7. Criar automaticamente um incidente tecnico quando um equipamento nao responde.\n");
        printf("8. Permitir um teste geral da rede, executando ping a todos os equipamentos registados.\n");
        printf("9. Outras atividades que considere relevantes.\n");

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

            case 5:
                alterarEstadoFalha(s);
                break;

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
                mostrarFerramentasExtras(s);
                break;

            case 0:
                printf("A voltar...\n");
                break;

            default:
                printf("Opcao invalida!\n");
        }

    } while(opcao != 0);
}

/* ========================================================= */
/* EXECUTAR PING A EQUIPAMENTO                               */
/* ========================================================= */

void executarPingEquipamento(Sistema *s) {

    int codigo;

    NodeEquipamento *atual;

    if (s->equipamentos == NULL) {

        printf("Nao existem equipamentos registados!\n");

        return;
    }

    printf("\n=========== EQUIPAMENTOS ===========\n");

    atual = s->equipamentos;

    while (atual != NULL) {

        printf("Codigo: %d | Nome: %s | IP: %s\n",
               atual->dados.codigo,
               atual->dados.nome,
               atual->dados.ip);

        atual = atual->proximo;
    }

    printf("\nCodigo do equipamento: ");

    if (scanf("%d", &codigo) != 1) {

        printf("Codigo invalido!\n");

        limparBuffer();

        return;
    }

    limparBuffer();

    atual = s->equipamentos;

    while (atual != NULL) {

        if (atual->dados.codigo == codigo) {

            printf("\nA executar ping a %s (%s)\n",
                   atual->dados.nome,
                   atual->dados.ip);

            if (executarPingSeguro(atual->dados.ip,
                                   "resultado_ping.txt",
                                   4) == 0) {

                printf("Ping executado com sucesso!\n");

                escreverLog("Ping executado com sucesso.");

            } else {

                printf("Erro ao executar ping!\n");

                escreverLog("Erro ao executar ping.");
            }

            return;
        }

        atual = atual->proximo;
    }

    printf("Equipamento nao encontrado!\n");
}

/* ========================================================= */
/* GUARDAR RESULTADO                                         */
/* ========================================================= */

void guardarResultadoPing(Sistema *s) {

    FILE *origem;

    FILE *destino;

    char linha[256];

    char nome[100];

    time_t t;

    struct tm *tm_info;

    (void)s;

    origem = fopen("resultado_ping.txt", "r");

    if (origem == NULL) {

        printf("Nenhum resultado encontrado!\n");

        return;
    }

    t = time(NULL);

    tm_info = localtime(&t);

    snprintf(nome,
             sizeof(nome),
             "ping_%04d%02d%02d_%02d%02d%02d.txt",
             tm_info->tm_year + 1900,
             tm_info->tm_mon + 1,
             tm_info->tm_mday,
             tm_info->tm_hour,
             tm_info->tm_min,
             tm_info->tm_sec);

    destino = fopen(nome, "w");

    if (destino == NULL) {

        fclose(origem);

        printf("Erro ao criar ficheiro!\n");

        return;
    }

    while (fgets(linha, sizeof(linha), origem) != NULL) {

        fputs(linha, destino);
    }

    fclose(origem);

    fclose(destino);

    printf("Resultado guardado em %s\n", nome);
}

/* ========================================================= */
/* VERIFICAR RESPOSTA                                        */
/* ========================================================= */

void verificarRespostaPing(Sistema *s) {

    int respondeu;

    (void)s;

    respondeu = verificarRespostaFicheiro("resultado_ping.txt");

    if (respondeu) {

        printf("\n====================================\n");
        printf("Equipamento ONLINE\n");
        printf("====================================\n");

    } else {

        printf("\n====================================\n");
        printf("Equipamento OFFLINE\n");
        printf("====================================\n");
    }
}

/* ========================================================= */
/* ATUALIZAR DATA                                            */
/* ========================================================= */

void atualizarDataVerificacao(Sistema *s) {

    int codigo;

    NodeEquipamento *atual;

    if (s->equipamentos == NULL) {

        printf("Nao existem equipamentos!\n");

        return;
    }

    printf("Codigo do equipamento: ");

    if (scanf("%d", &codigo) != 1) {

        printf("Codigo invalido!\n");

        limparBuffer();

        return;
    }

    limparBuffer();

    atual = s->equipamentos;

    while (atual != NULL) {

        if (atual->dados.codigo == codigo) {

            obterDataAtual(atual->dados.dataUltimaVerificacao);

            guardarFicheiro(s);

            printf("Data atualizada: %s\n",
                   atual->dados.dataUltimaVerificacao);

            escreverLog("Data de verificacao atualizada.");

            return;
        }

        atual = atual->proximo;
    }

    printf("Equipamento nao encontrado!\n");
}

/* ========================================================= */
/* ALTERAR ESTADO                                            */
/* ========================================================= */

void alterarEstadoFalha(Sistema *s) {

    int codigo;

    NodeEquipamento *atual;

    printf("Codigo do equipamento: ");

    if (scanf("%d", &codigo) != 1) {

        printf("Codigo invalido!\n");

        limparBuffer();

        return;
    }

    limparBuffer();

    atual = s->equipamentos;

    while (atual != NULL) {

        if (atual->dados.codigo == codigo) {

            atual->dados.estado = EM_FALHA;

            guardarFicheiro(s);

            printf("Estado alterado para EM_FALHA\n");

            escreverLog("Equipamento alterado para EM_FALHA.");

            return;
        }

        atual = atual->proximo;
    }

    printf("Equipamento nao encontrado!\n");
}

/* ========================================================= */
/* REGISTAR TESTE                                            */
/* ========================================================= */

void registarTesteLog(Sistema *s) {

    (void)s;

    escreverLog("Teste de conectividade executado.");

    printf("Log registado com sucesso!\n");
}

/* ========================================================= */
/* CRIAR INCIDENTE                                           */
/* ========================================================= */

void criarIncidenteAutomatico(Sistema *s) {

    NodeIncidente *novo;

    int codigo;

    printf("Codigo do equipamento: ");

    if (scanf("%d", &codigo) != 1) {

        printf("Codigo invalido!\n");

        limparBuffer();

        return;
    }

    limparBuffer();

    novo = (NodeIncidente*) malloc(sizeof(NodeIncidente));

    if (novo == NULL) {

        printf("Erro de memoria!\n");

        return;
    }

    novo->dados.codigo = s->proximoCodigoInc++;

    novo->dados.codigoEquipamento = codigo;

    strcpy(novo->dados.descricao,
           "Equipamento nao respondeu ao ping");

    novo->dados.estado = PENDENTE;

    obterDataAtual(novo->dados.dataAbertura);

    strcpy(novo->dados.dataFecho, "-");

    novo->proximo = s->incidentes;

    s->incidentes = novo;

    s->totalIncidentes++;

    guardarFicheiro(s);

    escreverLog("Incidente criado automaticamente.");

    printf("Incidente criado com sucesso!\n");
}

/* ========================================================= */
/* PING GERAL                                                */
/* ========================================================= */

void pingGeral(Sistema *s) {

    NodeEquipamento *atual;

    char temp[100];

    int online = 0;

    int offline = 0;

    atual = s->equipamentos;

    if (atual == NULL) {

        printf("Nao existem equipamentos!\n");

        return;
    }

    printf("\n=========== TESTE GERAL ===========\n");

    while (atual != NULL) {

#ifdef _WIN32

        snprintf(temp,
         sizeof(temp),
         "temp_ping_%lu.txt",
         (unsigned long)GetCurrentProcessId());

#else

        snprintf(temp,
                 sizeof(temp),
                 "temp_ping_%d.txt",
                 getpid());

#endif

        printf("A testar %s (%s)...\n",
               atual->dados.nome,
               atual->dados.ip);

        executarPingSeguro(atual->dados.ip,
                           temp,
                           1);

        if (verificarRespostaFicheiro(temp)) {

            printf("ONLINE\n");

            online++;

        } else {

            printf("OFFLINE\n");

            offline++;

            atual->dados.estado = EM_FALHA;
        }

        remove(temp);

        atual = atual->proximo;
    }

    printf("\n=========== RESUMO ===========\n");

    printf("ONLINE : %d\n", online);

    printf("OFFLINE: %d\n", offline);

    guardarFicheiro(s);

    escreverLog("Teste geral da rede executado.");
}

/* ========================================================= */
/* FERRAMENTAS EXTRA                                         */
/* ========================================================= */

void mostrarFerramentasExtras(Sistema *s) {

    int opcao;

    char entrada[100];

    char comando[300];

    (void)s;

    do {

        printf("\n=========== FERRAMENTAS EXTRA ===========\n");
        printf("1. Mostrar configuracao IP\n");
        printf("2. Mostrar tabela ARP\n");
        printf("3. Executar NSLOOKUP\n");
        printf("4. Executar TRACERT/TRACEROUTE\n");
        printf("0. Voltar\n");

        printf("Opcao: ");

        if (scanf("%d", &opcao) != 1) {

            printf("Opcao invalida!\n");

            limparBuffer();

            continue;
        }

        limparBuffer();

        switch(opcao) {

            case 1:

#ifdef _WIN32
                system("ipconfig > resultado_rede_local.txt");
#else
                system("ip addr > resultado_rede_local.txt");
#endif

                printf("Resultado guardado em resultado_rede_local.txt\n");

                break;

            case 2:

                system("arp -a > resultado_arp.txt");

                printf("Resultado guardado em resultado_arp.txt\n");

                break;

            case 3:

                printf("Dominio/IP: ");

                fgets(entrada,
                      sizeof(entrada),
                      stdin);

                entrada[strcspn(entrada, "\n")] = '\0';

                if (!entradaValida(entrada)) {

                    printf("Entrada invalida!\n");

                    break;
                }

                snprintf(comando,
                         sizeof(comando),
                         "nslookup %s > resultado_dns.txt",
                         entrada);

                system(comando);

                printf("Resultado guardado em resultado_dns.txt\n");

                break;

            case 4:

                printf("Destino/IP: ");

                fgets(entrada,
                      sizeof(entrada),
                      stdin);

                entrada[strcspn(entrada, "\n")] = '\0';

                if (!entradaValida(entrada)) {

                    printf("Entrada invalida!\n");

                    break;
                }

#ifdef _WIN32

                snprintf(comando,
                         sizeof(comando),
                         "tracert %s > resultado_rota.txt",
                         entrada);

#else

                snprintf(comando,
                         sizeof(comando),
                         "traceroute %s > resultado_rota.txt",
                         entrada);

#endif

                system(comando);

                printf("Resultado guardado em resultado_rota.txt\n");

                break;

            case 0:

                printf("A voltar...\n");

                break;

            default:

                printf("Opcao invalida!\n");
        }

    } while(opcao != 0);
}
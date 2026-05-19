#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "noc.h"

// ================= MENU CONECTIVIDADE =================

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
        printf("5. Alterar o estado do equipamento para 'Em Falha' quando nao existir resposta.                \n");
        printf("6. Registar cada teste realizado num ficheiro de texto denominado <log_monitorizacao.txt>      \n");
        printf("7. Criar automaticamente um incidente tecnico quando um equipamento nao responde.              \n");
        printf("8. Permitir um teste geral da rede, executando ping a todos os equipamentos registados.        \n");
        printf("9. Outras atividades que considere relevantes.                                                 \n");
        printf("-----------------------------------------------------------------------------------------------\n");
        printf("0. Voltar                                                                                      \n");
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
/* FUNCAO: executarPingEquipamento                           */
/* OBJETIVO: executar ping a um equipamento registado        */
/* ========================================================= */

void executarPingEquipamento(Sistema *s) {

    int codigo;

    char comando[200];

    NodeEquipamento *atual;

    /* Verificar se existem equipamentos */

    if (s->equipamentos == NULL) {

        printf("Nao existem equipamentos registados!\n");

        return;
    }

    /* Mostrar equipamentos */

    printf("\n========== EQUIPAMENTOS REGISTADOS ==========\n");

    atual = s->equipamentos;

    while (atual != NULL) {

        printf("Codigo: %d | Nome: %s | IP: %s\n",
               atual->dados.codigo,
               atual->dados.nome,
               atual->dados.ip);

        atual = atual->proximo;
    }

    /* Ler codigo */

    printf("\nIntroduza o codigo do equipamento: ");

    scanf("%d", &codigo);

    limparBuffer();

    /* Procurar equipamento */

    atual = s->equipamentos;

    while (atual != NULL) {

        if (atual->dados.codigo == codigo) {

            printf("\nA executar ping ao equipamento %s (%s)...\n",
                   atual->dados.nome,
                   atual->dados.ip);

            /*
             * Compatibilidade Windows / Linux
             */

#ifdef _WIN32

            snprintf(comando,
                     sizeof(comando),
                     "ping -n 4 %s > resultado_ping.txt",
                     atual->dados.ip);

#else

            snprintf(comando,
                     sizeof(comando),
                     "ping -c 4 %s > resultado_ping.txt",
                     atual->dados.ip);

#endif

            /* Executar comando */

            system(comando);

            printf("Ping executado com sucesso!\n");

            printf("Resultado guardado em resultado_ping.txt\n");

            return;
        }

        atual = atual->proximo;
    }

    printf("Equipamento nao encontrado!\n");
}

/* ========================================================= */
/* FUNCAO: guardarResultadoPing                              */
/* OBJETIVO: guardar copia do resultado do ping              */
/* ========================================================= */

void guardarResultadoPing(Sistema *s) {

    FILE *origem;
    FILE *destino;

    char nomeFicheiro[100];
    char linha[256];

    time_t t;

    struct tm *tm_info;

    (void)s;

    /* Abrir ficheiro do ping */

    origem = fopen("resultado_ping.txt", "r");

    if (origem == NULL) {

        printf("Erro! Ainda nao existe nenhum resultado de ping.\n");

        printf("Execute primeiro a opcao 1.\n");

        return;
    }

    /* Gerar nome automatico */

    t = time(NULL);

    tm_info = localtime(&t);

    snprintf(nomeFicheiro,
             sizeof(nomeFicheiro),
             "ping_%02d%02d%04d_%02d%02d%02d.txt",
             tm_info->tm_mday,
             tm_info->tm_mon + 1,
             tm_info->tm_year + 1900,
             tm_info->tm_hour,
             tm_info->tm_min,
             tm_info->tm_sec);

    /* Criar ficheiro destino */

    destino = fopen(nomeFicheiro, "w");

    if (destino == NULL) {

        printf("Erro ao criar ficheiro!\n");

        fclose(origem);

        return;
    }

    /* Copiar conteudo */

    while (fgets(linha, sizeof(linha), origem) != NULL) {

        fputs(linha, destino);
    }

    fclose(origem);

    fclose(destino);

    printf("Resultado do ping guardado com sucesso!\n");

    printf("Ficheiro criado: %s\n", nomeFicheiro);
}

/* ========================================================= */
/* FUNCAO: verificarRespostaPing                             */
/* OBJETIVO: verificar se o equipamento respondeu            */
/* ========================================================= */

void verificarRespostaPing(Sistema *s) {

    FILE *ficheiro;

    char linha[300];

    int respondeu = 0;

    (void)s;

    /* Abrir ficheiro do resultado */

    ficheiro = fopen("resultado_ping.txt", "r");

    if (ficheiro == NULL) {

        printf("Erro ao abrir o ficheiro resultado_ping.txt\n");

        return;
    }

    /* Ler ficheiro linha a linha */

    while (fgets(linha, sizeof(linha), ficheiro) != NULL) {

        /*
         * WINDOWS -> TTL=
         * LINUX   -> ttl=
         */

        if (strstr(linha, "TTL=") != NULL ||
            strstr(linha, "ttl=") != NULL) {

            respondeu = 1;
        }

        /*
         * EXTRA:
         * Mostrar estatisticas do ping
         */

        if (strstr(linha, "%") != NULL ||
            strstr(linha, "packet loss") != NULL) {

            printf("%s", linha);
        }
    }

    fclose(ficheiro);

    /* Mostrar resultado */

    if (respondeu) {

        printf("\n====================================\n");

        printf("Equipamento RESPONDEU ao ping.\n");

        printf("Estado da ligacao: ONLINE\n");

        printf("====================================\n");

    } else {

        printf("\n====================================\n");

        printf("Equipamento NAO respondeu ao ping.\n");

        printf("Estado da ligacao: OFFLINE\n");

        printf("====================================\n");
    }
}

/* ========================================================= */
/* FUNCAO: atualizarDataVerificacao                          */
/* OBJETIVO: atualizar data da ultima verificacao            */
/* ========================================================= */

void atualizarDataVerificacao(Sistema *s) {

    int codigo;

    NodeEquipamento *atual;

    if (s->equipamentos == NULL) {

        printf("Nao existem equipamentos registados!\n");

        return;
    }

    printf("Codigo do equipamento: ");

    scanf("%d", &codigo);

    limparBuffer();

    atual = s->equipamentos;

    while (atual != NULL) {

        if (atual->dados.codigo == codigo) {

            obterDataAtual(atual->dados.dataUltimaVerificacao);

            guardarFicheiro(s);

            printf("Data atualizada com sucesso!\n");

            printf("Nova data: %s\n",
                   atual->dados.dataUltimaVerificacao);

            return;
        }

        atual = atual->proximo;
    }

    printf("Equipamento nao encontrado!\n");
}

/* ========================================================= */
/* FUNCAO: alterarEstadoFalha                                */
/* OBJETIVO: colocar equipamento em falha                    */
/* ========================================================= */

void alterarEstadoFalha(Sistema *s) {

    int codigo;

    NodeEquipamento *atual;

    if (s->equipamentos == NULL) {

        printf("Nao existem equipamentos registados!\n");

        return;
    }

    printf("Codigo do equipamento: ");

    scanf("%d", &codigo);

    limparBuffer();

    atual = s->equipamentos;

    while (atual != NULL) {

        if (atual->dados.codigo == codigo) {

            atual->dados.estado = EM_FALHA;

            guardarFicheiro(s);

            printf("Estado alterado para EM_FALHA.\n");

            return;
        }

        atual = atual->proximo;
    }

    printf("Equipamento nao encontrado!\n");
}

/* ========================================================= */
/* FUNCAO: registarTesteLog                                  */
/* OBJETIVO: registar testes em ficheiro log                 */
/* ========================================================= */

void registarTesteLog(Sistema *s) {

    FILE *log;

    time_t t;

    struct tm *tm_info;

    char data[50];

    (void)s;

    log = fopen("log_monitorizacao.txt", "a");

    if (log == NULL) {

        printf("Erro ao abrir log!\n");

        return;
    }

    t = time(NULL);

    tm_info = localtime(&t);

    strftime(data,
             sizeof(data),
             "%d/%m/%Y %H:%M:%S",
             tm_info);

    fprintf(log,
            "[%s] Teste de conectividade executado com sucesso.\n",
            data);

    fclose(log);

    printf("Teste registado em log_monitorizacao.txt\n");
}

/* ========================================================= */
/* FUNCAO: criarIncidenteAutomatico                          */
/* OBJETIVO: criar incidente automaticamente                 */
/* ========================================================= */

void criarIncidenteAutomatico(Sistema *s) {

    NodeIncidente *novo;

    int codigo;

    printf("Codigo do equipamento: ");

    scanf("%d", &codigo);

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

    printf("Incidente criado automaticamente!\n");
}

/* ========================================================= */
/* FUNCAO: pingGeral                                         */
/* OBJETIVO: testar todos os equipamentos da rede            */
/* ========================================================= */

void pingGeral(Sistema *s) {

    NodeEquipamento *atual;

    char comando[200];

    int online = 0;
    int offline = 0;

    atual = s->equipamentos;

    if (atual == NULL) {

        printf("Nao existem equipamentos.\n");

        return;
    }

    printf("\n=========== TESTE GERAL DA REDE ===========\n");

    while (atual != NULL) {

        printf("A testar %s (%s)...\n",
               atual->dados.nome,
               atual->dados.ip);

#ifdef _WIN32

        snprintf(comando,
                 sizeof(comando),
                 "ping -n 1 %s > temp_ping.txt",
                 atual->dados.ip);

#else

        snprintf(comando,
                 sizeof(comando),
                 "ping -c 1 %s > temp_ping.txt",
                 atual->dados.ip);

#endif

        system(comando);

        FILE *f = fopen("temp_ping.txt", "r");

        char linha[300];

        int respondeu = 0;

        if (f == NULL) {

            printf("Erro ao abrir temp_ping.txt\n");

            atual = atual->proximo;

            continue;
        }

        while (fgets(linha, sizeof(linha), f) != NULL) {

            if (strstr(linha, "TTL=") != NULL ||
                strstr(linha, "ttl=") != NULL) {

                respondeu = 1;
            }
        }

        fclose(f);

        if (respondeu) {

            printf("ONLINE\n");

            online++;

        } else {

            printf("OFFLINE\n");

            offline++;
        }

        atual = atual->proximo;
    }

    remove("temp_ping.txt");

    printf("\n=========== RESUMO DA REDE ===========\n");

    printf("Equipamentos ONLINE : %d\n", online);

    printf("Equipamentos OFFLINE: %d\n", offline);

    if (offline == 0) {

        printf("Estado global: NORMAL\n");

    } else if (offline <= 2) {

        printf("Estado global: AVISO\n");

    } else {

        printf("Estado global: CRITICO\n");
    }
}

/* ========================================================= */
/* FUNCAO: mostrarFerramentasExtras                          */
/* OBJETIVO: ferramentas extra de diagnostico                */
/* ========================================================= */

void mostrarFerramentasExtras(Sistema *s) {

    int opcao;

    (void)s;

    do {

        printf("\n=========== FERRAMENTAS EXTRA ===========\n");

        printf("1. Mostrar configuracao IP\n");
        printf("2. Executar ARP\n");
        printf("3. Executar NSLOOKUP\n");
        printf("4. Executar TRACERT/TRACEROUTE\n");
        printf("0. Voltar\n");

        printf("Opcao: ");

        scanf("%d", &opcao);

        limparBuffer();

        switch(opcao) {

            case 1:

#ifdef _WIN32
                system("ipconfig");
#else
                system("ip addr");
#endif
                break;

            case 2:

                system("arp -a");

                break;

            case 3: {

                char dominio[100];

                char comando[200];

                printf("Dominio/IP: ");

                fgets(dominio,
                      sizeof(dominio),
                      stdin);

                dominio[strcspn(dominio, "\n")] = '\0';

                snprintf(comando,
                         sizeof(comando),
                         "nslookup %s",
                         dominio);

                system(comando);

                break;
            }

            case 4: {

                char destino[100];

                char comando[200];

                printf("Destino/IP: ");

                fgets(destino,
                      sizeof(destino),
                      stdin);

                destino[strcspn(destino, "\n")] = '\0';

#ifdef _WIN32

                snprintf(comando,
                         sizeof(comando),
                         "tracert %s",
                         destino);

#else

                snprintf(comando,
                         sizeof(comando),
                         "traceroute %s",
                         destino);

#endif

                system(comando);

                break;
            }

            case 0:

                printf("A voltar...\n");

                break;

            default:

                printf("Opcao invalida!\n");
        }

    } while(opcao != 0);
}
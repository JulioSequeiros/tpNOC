#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "noc.h"

static void pausarLocal(void) {
    printf("\nPressione ENTER para continuar...");
    limparBufferLocal();
    getchar();
}

static void limparEcraLocal(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// ================= MENU INCIDENTES =================

void menuIncidente(Sistema *s) {

    int opcao;
    char entidadeId[50];
    int prioridade;

    do {
        limparEcraLocal();

        printf("===============================================================================\n");
        printf("        MODULO 4 - INCIDENTES TECNICOS                                         \n");
        printf("===============================================================================\n");

        printf(" 1. Criar um novo incidente manualmente.\n");
        printf(" 2. Criar incidente automaticamente (falha ping).\n");
        printf(" 3. Criar incidente automaticamente (sensor anomalia).\n");
        printf(" 4. Colocar incidentes em fila de atendimento.\n");
        printf(" 5. Processar proximo incidente (EM CURSO).\n");
        printf(" 6. Concluir incidente (data/hora).\n");
        printf(" 7. Listar incidentes pendentes.\n");
        printf(" 8. Listar incidentes em curso.\n");
        printf(" 9. Listar incidentes concluidos.\n");
        printf("10. Listar por equipamento/sensor.\n");
        printf("11. Listar por prioridade.\n");
        printf("12. Guardar/carregar ficheiro binario.\n");
        printf("13. Outras atividades.\n");
        printf("-------------------------------------------------------------------------------\n");
        printf(" 0. Voltar\n");
        printf("===============================================================================\n");

        printf("Opcao: ");
        scanf("%d", &opcao);
        limparBufferLocal();

        switch(opcao) {
            case 1: 
                criarIncidente(s); 
                break;
            case 2: 
                criarIncidenteAutoPorFalhaNoPing(s); 
                break;
            case 3: 
                criarIncidenteAutoPorLeituraAnomalaDoSensor(s); 
                break;
            case 4: 
                gerirIncidentesPendentesNaFilaDeAtendimento(s); 
                break;
            case 5: 
                processarProximoIncidenteNaFilaDeAtendimento(s); 
                break;
            case 6: 
                resolverEConcluirIncidenteDataHora(s); 
                break;
            case 7: 
                listarIncidentesPendentes(s); 
                break;
            case 8: 
                listarIncidentesEmCurso(s); 
                break;
            case 9:
                listarIncidentesConcluidos(s); 
                break;
            case 10:
                printf("Codigo equipamento/sensor: ");
                fgets(entidadeId, sizeof(entidadeId), stdin);
                entidadeId[strcspn(entidadeId, "\n")] = 0;
                listarIncidentesPorEntidade(s, entidadeId);
                break;

            case 11:
                printf("Prioridade (1=Baixa, 2=Media, 3=Alta): ");
                scanf("%d", &prioridade);
                limparBufferLocal();
                listarIncidentesPorPrioridadeComParam(s, prioridade);
                break;

            case 12:
                guardarCarregarIncidentesFicheiroBinario(s);
                break;

            case 13:
                outrasAtividadesRelevantes(s);   // 👈 FIX: nome correto do header
                break;

            case 0:
                printf("A voltar ao menu principal...\n");
                break;

            default:
                printf("Opcao invalida!\n");
        }

        if (opcao != 0) pausarLocal();

    } while(opcao != 0);
}

// ================= FUNÇÕES =================

void criarIncidente(Sistema *s){
    if (s->equipamentos == NULL) {
        printf("Nao existem equipamentos.\n");
        return;
    }

    int codigoEquip;
    char desc[MAX_DESC];

    printf("Codigo do equipamento: ");
    scanf("%d", &codigoEquip);
    limparBufferLocal();

    printf("Descricao: ");
    fgets(desc, MAX_DESC, stdin);
    desc[strcspn(desc, "\n")] = 0;

    NodeIncidente *novo = malloc(sizeof(NodeIncidente));
    if (!novo) {
        printf("Erro de memoria!\n");
        return;
    }

    novo->dados.codigo = s->proximoCodigoInc++;
    novo->dados.codigoEquipamento = codigoEquip;

    strcpy(novo->dados.descricao, desc);
    novo->dados.estado = PENDENTE;

    obterDataAtual(novo->dados.dataAbertura);
    strcpy(novo->dados.dataFecho, "-");

    novo->proximo = s->incidentes;
    s->incidentes = novo;

    s->totalIncidentes++;

    guardarFicheiro(s);

    printf("Incidente criado com sucesso!\n");
}

void criarIncidenteAutoPorFalhaNoPing(Sistema *s){
    
    int codigoEquip;

    printf("Codigo equipamento: ");
    scanf("%d", &codigoEquip);
    limparBufferLocal();

    NodeIncidente *novo = malloc(sizeof(NodeIncidente));
    if (!novo) return;

    novo->dados.codigo = s->proximoCodigoInc++;
    novo->dados.codigoEquipamento = codigoEquip;

    strcpy(novo->dados.descricao,
           "Falha de comunicacao (ping nao respondeu)");

    novo->dados.estado = PENDENTE;

    obterDataAtual(novo->dados.dataAbertura);
    strcpy(novo->dados.dataFecho, "-");

    novo->proximo = s->incidentes;
    s->incidentes = novo;

    s->totalIncidentes++;

    guardarFicheiro(s);

    printf("Incidente automatico criado (PING).\n");
}


void criarIncidenteAutoPorLeituraAnomalaDoSensor(Sistema *s){
    
    int codigoEquip;

    printf("Codigo sensor/equipamento: ");
    scanf("%d", &codigoEquip);
    limparBufferLocal();

    NodeIncidente *novo = malloc(sizeof(NodeIncidente));
    if (!novo) return;

    novo->dados.codigo = s->proximoCodigoInc++;
    novo->dados.codigoEquipamento = codigoEquip;

    strcpy(novo->dados.descricao,
           "Leitura anomala detetada em sensor");

    novo->dados.estado = PENDENTE;

    obterDataAtual(novo->dados.dataAbertura);
    strcpy(novo->dados.dataFecho, "-");

    novo->proximo = s->incidentes;
    s->incidentes = novo;

    s->totalIncidentes++;

    guardarFicheiro(s);

    printf("Incidente criado por sensor.\n");
}


void gerirIncidentesPendentesNaFilaDeAtendimento(Sistema *s) {

    if (s == NULL || s->incidentes == NULL) {

        printf("Nao existem incidentes.\n");

        return;
    }

    NodeIncidente *atual = s->incidentes;

    int encontrados = 0;
    int posicao = 1;

    printf("\n=========== FILA DE INCIDENTES PENDENTES ===========\n");

    while (atual != NULL) {

        if (atual->dados.estado == PENDENTE) {

            printf("\n[%d] INCIDENTE #%d\n",
                   posicao,
                   atual->dados.codigo);

            printf("Equipamento : %d\n",
                   atual->dados.codigoEquipamento);

            printf("Descricao   : %s\n",
                   atual->dados.descricao);

            printf("Data abertura: %s\n",
                   atual->dados.dataAbertura);

            printf("-------------------------------------------\n");

            encontrados++;
            posicao++;
        }

        atual = atual->proximo;
    }

    if (encontrados == 0) {

        printf("Nao existem incidentes pendentes.\n");

    } else {

        printf("\nTotal de incidentes pendentes: %d\n",
               encontrados);
    }
}


void processarProximoIncidenteNaFilaDeAtendimento(Sistema *s){

    if (s == NULL || s->incidentes == NULL) {

        printf("Nao existem incidentes.\n");

        return;
    }

    NodeIncidente *atual = s->incidentes;

    while (atual != NULL) {

        if (atual->dados.estado == PENDENTE) {

            printf("\n=========== PROCESSAMENTO DE INCIDENTE ===========\n");

            printf("Incidente selecionado : %d\n",
                   atual->dados.codigo);

            printf("Equipamento           : %d\n",
                   atual->dados.codigoEquipamento);

            printf("Descricao             : %s\n",
                   atual->dados.descricao);

            printf("Data abertura         : %s\n",
                   atual->dados.dataAbertura);

            /* ALTERAR ESTADO */

#ifdef EM_CURSO
            atual->dados.estado = EM_CURSO;
            printf("Novo estado           : EM CURSO\n");
#else
            printf("Estado                : EM CURSO (simulado)\n");
#endif

            printf("==================================================\n");

            guardarFicheiro(s);

            return;
        }

        atual = atual->proximo;
    }

    printf("Nao existem incidentes pendentes.\n");
}

void resolverEConcluirIncidenteDataHora(Sistema *s){
    
    if (s->incidentes == NULL) {
        printf("Nao existem incidentes.\n");
        return;
    }

    int codigo;

    printf("Codigo do incidente a resolver: ");
    scanf("%d", &codigo);
    limparBufferLocal();

    NodeIncidente *atual = s->incidentes;

    while (atual != NULL) {

        if (atual->dados.codigo == codigo) {

            if (atual->dados.estado == RESOLVIDO) {
                printf("Este incidente ja esta resolvido.\n");
                return;
            }

            atual->dados.estado = RESOLVIDO;

            obterDataAtual(atual->dados.dataFecho);

            guardarFicheiro(s);

            printf("\n=========== INCIDENTE RESOLVIDO ===========\n");
            printf("Codigo      : %d\n", atual->dados.codigo);
            printf("Equipamento : %d\n", atual->dados.codigoEquipamento);
            printf("Descricao   : %s\n", atual->dados.descricao);
            printf("Aberto em   : %s\n", atual->dados.dataAbertura);
            printf("Fechado em  : %s\n", atual->dados.dataFecho);
            printf("Estado      : RESOLVIDO\n");
            printf("===========================================\n");

            return;
        }

        atual = atual->proximo;
    }

    printf("Incidente nao encontrado.\n");
}

void listarIncidentesPendentes(const Sistema *s) {

    if (s == NULL || s->incidentes == NULL) {
        printf("Nao existem incidentes registados.\n");
        return;
    }

    NodeIncidente *atual = s->incidentes;
    int count = 0;

    printf("\n=========== INCIDENTES PENDENTES ===========\n");

    while (atual != NULL) {

        if (atual->dados.estado == PENDENTE) {

            printf("\nCodigo incidente : %d\n", atual->dados.codigo);
            printf("Equipamento      : %d\n", atual->dados.codigoEquipamento);
            printf("Descricao        : %s\n", atual->dados.descricao);
            printf("Data abertura    : %s\n", atual->dados.dataAbertura);
            printf("-------------------------------------------\n");

            count++;
        }

        atual = atual->proximo;
    }

    if (count == 0) {
        printf("Nenhum incidente pendente.\n");
    } else {
        printf("\nTotal de incidentes pendentes: %d\n", count);
    }
}

void listarIncidentesEmCurso(const Sistema *s) {

    if (s == NULL || s->incidentes == NULL) {
        printf("Nao existem incidentes registados.\n");
        return;
    }

    NodeIncidente *atual = s->incidentes;
    int count = 0;

    printf("\n=========== INCIDENTES EM CURSO (SIMULADO) ===========\n");

    while (atual != NULL) {

        /* Simulação: tratamos como "em curso" os pendentes */
        if (atual->dados.estado == PENDENTE) {

            printf("\nCodigo incidente : %d\n", atual->dados.codigo);
            printf("Equipamento      : %d\n", atual->dados.codigoEquipamento);
            printf("Descricao        : %s\n", atual->dados.descricao);
            printf("Data abertura    : %s\n", atual->dados.dataAbertura);
            printf("Estado           : EM CURSO (simulado)\n");
            printf("-------------------------------------------\n");

            count++;
        }

        atual = atual->proximo;
    }

    if (count == 0) {
        printf("Nenhum incidente em curso.\n");
    } else {
        printf("\nTotal em curso: %d\n", count);
    }
}

void listarIncidentesConcluidos(const Sistema *s) {

    if (s == NULL || s->incidentes == NULL) {
        printf("Nao existem incidentes registados.\n");
        return;
    }

    NodeIncidente *atual = s->incidentes;
    int count = 0;

    printf("\n=========== INCIDENTES CONCLUIDOS ===========\n");

    while (atual != NULL) {

        if (atual->dados.estado == RESOLVIDO) {

            printf("\nCodigo incidente : %d\n", atual->dados.codigo);
            printf("Equipamento      : %d\n", atual->dados.codigoEquipamento);
            printf("Descricao        : %s\n", atual->dados.descricao);
            printf("Data abertura    : %s\n", atual->dados.dataAbertura);
            printf("Data fecho       : %s\n", atual->dados.dataFecho);
            printf("Estado           : RESOLVIDO\n");
            printf("-------------------------------------------\n");

            count++;
        }

        atual = atual->proximo;
    }

    if (count == 0) {
        printf("Nenhum incidente concluido.\n");
    } else {
        printf("\nTotal concluidos: %d\n", count);
    }
}

void listarIncidentesPorEntidade(Sistema *s, char *entidadeId) {

    if (s == NULL || s->incidentes == NULL) {
        printf("Nao existem incidentes registados.\n");
        return;
    }

    int codigo = atoi(entidadeId);

    NodeIncidente *atual = s->incidentes;
    int count = 0;

    printf("\n=========== INCIDENTES DO EQUIPAMENTO %d ===========\n", codigo);

    while (atual != NULL) {

        if (atual->dados.codigoEquipamento == codigo) {

            printf("\nCodigo incidente : %d\n", atual->dados.codigo);
            printf("Descricao        : %s\n", atual->dados.descricao);
            printf("Estado           : %s\n",
                   (atual->dados.estado == PENDENTE) ? "PENDENTE" : "RESOLVIDO");
            printf("Data abertura    : %s\n", atual->dados.dataAbertura);
            printf("Data fecho       : %s\n", atual->dados.dataFecho);
            printf("-------------------------------------------\n");

            count++;
        }

        atual = atual->proximo;
    }

    if (count == 0) {
        printf("Nenhum incidente encontrado para este equipamento.\n");
    } else {
        printf("\nTotal encontrados: %d\n", count);
    }
}

void listarIncidentesPorPrioridadeComParam(Sistema *s, int prioridade) {

    if (s == NULL || s->incidentes == NULL) {
        printf("Nao existem incidentes registados.\n");
        return;
    }

    NodeIncidente *atual = s->incidentes;
    int count = 0;

    printf("\n=========== INCIDENTES POR PRIORIDADE (%d) ===========\n", prioridade);

    while (atual != NULL) {

        int mostrar = 0;

        if (prioridade == 1) {
            // Baixa prioridade = resolvidos
            if (atual->dados.estado == RESOLVIDO)
                mostrar = 1;
        }
        else if (prioridade == 2) {
            // Média = pendentes normais
            if (atual->dados.estado == PENDENTE)
                mostrar = 1;
        }
        else if (prioridade == 3) {
            // Alta = pendentes antigos (simulação simples)
            if (atual->dados.estado == PENDENTE)
                mostrar = 1;
        }

        if (mostrar) {

            printf("\nCodigo incidente : %d\n", atual->dados.codigo);
            printf("Equipamento      : %d\n", atual->dados.codigoEquipamento);
            printf("Descricao        : %s\n", atual->dados.descricao);
            printf("Estado           : %s\n",
                   (atual->dados.estado == PENDENTE) ? "PENDENTE" : "RESOLVIDO");
            printf("Data abertura    : %s\n", atual->dados.dataAbertura);
            printf("-------------------------------------------\n");

            count++;
        }

        atual = atual->proximo;
    }

    if (count == 0) {
        printf("Nenhum incidente encontrado para esta prioridade.\n");
    } else {
        printf("\nTotal encontrados: %d\n", count);
    }
}

void guardarCarregarIncidentesFicheiroBinario(Sistema *s) {

    int opcao;

    printf("\n1. Guardar\n2. Carregar\nOpcao: ");
    scanf("%d", &opcao);
    limparBufferLocal();

    /* ========================= */
    /*          GUARDAR         */
    /* ========================= */
    if (opcao == 1) {

        FILE *f = fopen("incidentes.dat", "wb");

        if (f == NULL) {
            printf("Erro ao abrir ficheiro para escrita.\n");
            return;
        }

        /* opcional: guardar quantidade */
        fwrite(&s->totalIncidentes, sizeof(int), 1, f);

        NodeIncidente *atual = s->incidentes;

        while (atual != NULL) {
            fwrite(&atual->dados, sizeof(Incidente), 1, f);
            atual = atual->proximo;
        }

        fclose(f);

        printf("Incidentes guardados com sucesso.\n");
    }

    /* ========================= */
    /*          CARREGAR        */
    /* ========================= */
    else if (opcao == 2) {

        FILE *f = fopen("incidentes.dat", "rb");

        if (f == NULL) {
            printf("Erro ao abrir ficheiro para leitura.\n");
            return;
        }

        /* limpar lista atual */
        NodeIncidente *atual = s->incidentes;

        while (atual != NULL) {
            NodeIncidente *tmp = atual;
            atual = atual->proximo;
            free(tmp);
        }

        s->incidentes = NULL;
        s->totalIncidentes = 0;

        int total = 0;

        /* ler quantidade (se existir) */
        if (fread(&total, sizeof(int), 1, f) != 1) {
            printf("Ficheiro vazio ou corrompido.\n");
            fclose(f);
            return;
        }

        for (int i = 0; i < total; i++) {

            Incidente temp;

            if (fread(&temp, sizeof(Incidente), 1, f) != 1) {
                break;
            }

            NodeIncidente *novo = malloc(sizeof(NodeIncidente));

            if (!novo) {
                printf("Erro de memoria.\n");
                fclose(f);
                return;
            }

            novo->dados = temp;
            novo->proximo = s->incidentes;
            s->incidentes = novo;

            s->totalIncidentes++;
        }

        fclose(f);

        printf("Incidentes carregados com sucesso (%d).\n", s->totalIncidentes);
    }

    else {
        printf("Opcao invalida.\n");
    }
}

void outrasAtividadesRelevantes(Sistema *s) {
      int opcao;

    do {

        printf("\n=========== MODULO 4 - FERRAMENTAS EXTRA ===========\n");
        printf("1. Resumo da fila de incidentes\n");
        printf("2. Detectar incidentes criticos (pendentes antigos)\n");
        printf("3. Estatisticas de atendimento\n");
        printf("4. Simular prioridade automatica\n");
        printf("5. Verificar backlog (fila acumulada)\n");
        printf("6. Limpar incidentes resolvidos antigos\n");
        printf("0. Voltar\n");
        printf("Opcao: ");

        scanf("%d", &opcao);
        limparBufferLocal();

        switch (opcao) {

        /* ========================================= */
        case 1: {

            int pend = 0;
            NodeIncidente *i = s->incidentes;

            while (i) {
                if (i->dados.estado == PENDENTE)
                    pend++;
                i = i->proximo;
            }

            printf("\n--- RESUMO FILA ---\n");
            printf("Total incidentes: %d\n", s->totalIncidentes);
            printf("Pendentes       : %d\n", pend);
            printf("Resolvidos      : %d\n",
                   s->totalIncidentes - pend);

            break;
        }

        /* ========================================= */
        case 2: {

            printf("\n--- INCIDENTES CRITICOS (SIMULADO) ---\n");

            NodeIncidente *i = s->incidentes;
            int count = 0;

            while (i) {

                if (i->dados.estado == PENDENTE) {

                    printf("CRITICO -> %d | %s\n",
                           i->dados.codigo,
                           i->dados.descricao);

                    count++;
                }

                i = i->proximo;
            }

            if (count == 0)
                printf("Nenhum incidente critico.\n");

            break;
        }

        /* ========================================= */
        case 3: {

            if (s->totalIncidentes == 0) {
                printf("Sem dados.\n");
                break;
            }

            NodeIncidente *i = s->incidentes;
            int resolvidos = 0;

            while (i) {
                if (i->dados.estado == RESOLVIDO)
                    resolvidos++;
                i = i->proximo;
            }

            float taxa = (resolvidos * 100.0) / s->totalIncidentes;

            printf("\n--- ESTATISTICAS ---\n");
            printf("Taxa de resolucao: %.2f%%\n", taxa);

            if (taxa > 80)
                printf("Desempenho: BOM\n");
            else if (taxa > 50)
                printf("Desempenho: MEDIO\n");
            else
                printf("Desempenho: CRITICO\n");

            break;
        }

        /* ========================================= */
        case 4: {

            printf("\n--- SIMULACAO PRIORIDADE ---\n");

            NodeIncidente *i = s->incidentes;

            while (i) {

                if (i->dados.estado == PENDENTE) {

                    printf("Incidente %d -> PRIORIDADE ALTA\n",
                           i->dados.codigo);
                }

                i = i->proximo;
            }

            break;
        }

        /* ========================================= */
        case 5: {

            printf("\n--- BACKLOG (FILA ACUMULADA) ---\n");

            int count = 0;
            NodeIncidente *i = s->incidentes;

            while (i) {

                if (i->dados.estado == PENDENTE)
                    count++;

                i = i->proximo;
            }

            printf("Incidentes em espera: %d\n", count);

            if (count > 10)
                printf("ALERTA: Backlog elevado!\n");

            break;
        }

        /* ========================================= */
        case 6: {

            printf("\n--- LIMPAR RESOLVIDOS ANTIGOS ---\n");

            NodeIncidente *i = s->incidentes;
            NodeIncidente *ant = NULL;

            while (i) {

                if (i->dados.estado == RESOLVIDO) {

                    NodeIncidente *tmp = i;

                    if (ant == NULL)
                        s->incidentes = i->proximo;
                    else
                        ant->proximo = i->proximo;

                    i = i->proximo;
                    free(tmp);
                    s->totalIncidentes--;

                } else {
                    ant = i;
                    i = i->proximo;
                }
            }

            printf("Limpeza concluida.\n");
            break;
        }

        case 0:
            printf("A voltar...\n");
            break;

        default:
            printf("Opcao invalida!\n");
        }

    } while (opcao != 0);
}







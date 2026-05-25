#include "noc.h"

// ============================================================
//  MENU PRINCIPAL
// ============================================================

void menuPrincipal(Sistema *s)
{
    int opcao;
    do
    {
        limparEcra();
        printf("\n  ╔══════════════════════════════════════════════════════════════════╗\n");
        printf("  ║   NOC - Network Operations Center                                ║\n");
        printf("  ╠══════════════════════════════════════════════════════════════════╣\n");
        printf("  ║  1. Inventario de Equipamentos  [M1]                             ║\n");
        printf("  ║  2. Conectividade               [M2]                             ║\n");
        printf("  ║  3. Sensores                    [M3]                             ║\n");
        printf("  ║  4. Incidentes                  [M4]                             ║\n");
        printf("  ║  5. Configuracoes               [M5]                             ║\n");
        printf("  ║  6. Relatorios                  [M6]                             ║\n");
        printf("  ║  7. Ficheiros                                                    ║\n");
        printf("  ║  0. Sair                                                         ║\n");
        printf("  ╠══════════════════════════════════════════════════════════════════╣\n");
        printf("  ║  Equipamentos: %-4d   Incidentes: %-4d   Configuracoes: %-4d     ║\n",
                    s->totalEquipamentos, s->totalIncidentes, s->totalConfiguracoes);
        printf("  ╚══════════════════════════════════════════════════════════════════╝\n");
        opcao = lerInteiro("  Opcao", 0, 7);
        switch (opcao)
        {
            case 1: menuEquipamento(s);    break;
            case 2: menuConectividade(s);  break;
            // case 3: menuSensores(s);       break;
            case 4: menuIncidente(s);     break;
            // case 5: menuConfiguracoes(s);  break;
            case 6: menuRelatorios(s);     break;
            // case 7: menuFicheiro(s);       break;
            case 0: break;
        }
    } while (opcao != 0);
}

// ============================================================
//  MAIN
// ============================================================

int main(void)
{
    Sistema s;
    memset(&s, 0, sizeof(Sistema));

    // Inicializar contadores
    s.proximoCodigoEquip = 1;
    s.proximoCodigoInc   = 1;
    s.proximoCodigoCfg   = 1;

    // Inicializar fila de atendimento (Módulo 4)
    inicializarFilaAtendimento(&s);

    menuPrincipal(&s);

    // Libertar memoria — equipamentos
    NodeEquipamento *eq = s.equipamentos;
    while (eq) { 
        NodeEquipamento *tmp = eq->proximo; 
        free(eq); 
        eq = tmp; 
    }

    // Libertar memoria — sensores
    NodeSensor *sen = s.sensores;
    while (sen) { 
        NodeSensor *tmp = sen->proximo; 
        free(sen); 
        sen = tmp; 
    }

    // Libertar memoria — incidentes
    NodeIncidente *inc = s.incidentes;
    while (inc) { 
        NodeIncidente *tmp = inc->proximo; 
        free(inc); 
        inc = tmp; 
    }

    // Libertar memoria — fila de atendimento
    // A fila contém ponteiros para os incidentes, mas nao deve liberta-los novamente
    // porque ja foram libertados acima. Apenas garantir que os ponteiros estao NULL
    s.filaAtendimento.inicio = NULL;
    s.filaAtendimento.fim = NULL;
    s.filaAtendimento.total = 0;

    // Libertar memoria — pilha de configuracoes
    NodeConfiguracao *cfg = s.pilhaConfiguracoes.topo;
    while (cfg) { 
        NodeConfiguracao *tmp = cfg->proximo; 
        free(cfg); 
        cfg = tmp; 
    }

    printf("\n  Ate logo!\n\n");
    return 0;
}
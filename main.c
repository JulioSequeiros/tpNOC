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
        printf("\n  ╔══════════════════════════════════════╗\n");
        printf("  ║   NOC — Network Operations Center   ║\n");
        printf("  ╠══════════════════════════════════════╣\n");
        printf("  ║  1. Inventário de Equipamentos  [M1] ║\n");
        printf("  ║  2. Conectividade               [M2] ║\n");
        printf("  ║  3. Sensores                    [M3] ║\n");
        printf("  ║  4. Incidentes                  [M4] ║\n");
        printf("  ║  5. Configurações               [M5] ║\n");
        printf("  ║  6. Relatórios                  [M6] ║\n");
        printf("  ║  7. Ficheiros                        ║\n");
        printf("  ║  0. Sair                             ║\n");
        printf("  ╚══════════════════════════════════════╝\n");
        printf("  Equipamentos: %-4d  Incidentes: %-4d  Configurações: %d\n",
               s->totalEquipamentos, s->totalIncidentes, s->totalConfiguracoes);

        opcao = lerInteiro("  Opção: ", 0, 7);
        switch (opcao)
        {
            case 1: menuEquipamento(s);    break;
            // case 2: menuConectividade(s);  break;
            // case 3: menuSensores(s);       break;
            // case 4: menuIncidentes(s);     break;
            // case 5: menuConfiguracoes(s);  break;
            // case 6: menuRelatorios(s);     break;
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

    menuPrincipal(&s);

    // Libertar memória — equipamentos
    NodeEquipamento *eq = s.equipamentos;
    while (eq) { NodeEquipamento *tmp = eq->proximo; free(eq); eq = tmp; }

    // Libertar memória — sensores
    NodeSensor *sen = s.sensores;
    while (sen) { NodeSensor *tmp = sen->proximo; free(sen); sen = tmp; }

    // Libertar memória — incidentes
    NodeIncidente *inc = s.incidentes;
    while (inc) { NodeIncidente *tmp = inc->proximo; free(inc); inc = tmp; }

    // Libertar memória — pilha de configurações
    NodeConfiguracao *cfg = s.pilhaConfiguracoes.topo;
    while (cfg) { NodeConfiguracao *tmp = cfg->proximo; free(cfg); cfg = tmp; }

    printf("  Até logo!\n\n");
    return 0;
}

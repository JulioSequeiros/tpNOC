#include "noc.h"
#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
//  MENU PRINCIPAL
// ============================================================

void menuPrincipal(Sistema *s)
{
    int opcao;
    do
    {
        limparEcra();
        printf("\n");
        printf("\n  ╔═══════════════════════════════════════════════════════════════════════╗\n");
        printf("  ║   NOC - Network Operations Center                                     ║\n");
        printf("  ╠═══════════════════════════════════════════════════════════════════════╣\n");
        printf("  ║  1. Inventario de Equipamentos  [M1]                                  ║\n");
        printf("  ║  2. Conectividade               [M2]                                  ║\n");
        printf("  ║  3. Sensores                    [M3]                                  ║\n");
        printf("  ║  4. Incidentes                  [M4]                                  ║\n");
        printf("  ║  5. Configuracoes               [M5]                                  ║\n");
        printf("  ║  6. Relatorios                  [M6]                                  ║\n");
        printf("  ║  7. Perfil / Conta                                                    ║\n");
        printf("  ║  0. Sair                                                              ║\n");
        printf("  ╠═══════════════════════════════════════════════════════════════════════╣\n");
        printf("  ║  Equip: %-4d  Incidentes: %-4d  Config: %-4d  Tecnico: %-16s       ║\n",
                    s->totalEquipamentos, s->totalIncidentes,
                    s->totalConfiguracoes, s->tecnicoLogado);
        printf("  ╚═══════════════════════════════════════════════════════════════════════╝\n");
        opcao = lerInteiro("  Opcao", 0, 7);
        switch (opcao)
        {
            case 1: menuEquipamento(s);    break;
            case 2: menuConectividade(s);  break;
            case 3: menuSensores(s);       break;
            case 4: menuIncidente(s);      break;
            case 5: menuConfiguracoes(s);  break;
            case 6: menuRelatorios(s);     break;
            case 7: menuGestaoPerfil(s);   break;
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

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Inicializar contadores
    s.proximoCodigoEquip = 1;
    s.proximoCodigoInc   = 1;
    s.proximoCodigoCfg   = 1;

    // Inicializar fila de atendimento (Módulo 4)
    inicializarFilaAtendimento(&s);

    // Autenticação — bloqueia até ‘login’ valido
    inicializarTecnicos();
    if (!autenticarTecnico(&s)) return 1;

    // Req 1 — carregar dados existentes dos ficheiros binários
    printf("\n  A carregar dados...\n");
    carregarEquipamento(&s);
    carregarSensoresFicheiro(&s);
    carregarConfiguracoesFicheiro(&s);

    _sleep(10);
    limparEcra();

    menuPrincipal(&s);

    // Req 14 — guardar dados antes de sair
    printf("\n  Guardar dados antes de sair? (1-Sim  2-Nao): ");
    int opcaoGuardar;
    char lixo;
    if (scanf("%d%c", &opcaoGuardar, &lixo) == 2 && opcaoGuardar == 1)
    {
        guardarEquipamento(&s);
        guardarSensoresFicheiro(&s);
        guardarConfiguracoesFicheiro(&s);
        printf("  [OK] Dados guardados.\n");
    }

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
    // A fila contém ponteiros para os incidentes, mas não deve libertá-los novamente
    // porque já foram libertados acima. Apenas garantir que os ponteiros estao NULL
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
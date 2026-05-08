#include <stdio.h>

int main() {
    int opcao;

    do {
        // Tela de boas-vindas
        printf("=================================================================================\n");
        printf("====== Bem-vindo ao Sistema Mini NOC para Monitorizacao da Rede de uma PME ======\n");
        printf("=================================================================================\n");
        // Menu do Sistema
        printf("================================ Menu de Opcoes =================================\n");
        printf("1 - Adicionar um Novo Equipamentona Rede\n");
        printf("2 - Remover um Equipamento de Rede.\n");
        printf("3 - Alterar os Dados do Equipamento.\n");
        printf("4 - Alterar o Status do Equipamento (Operacional/Em Falha /Em Manutencao ou Desativado).\n");
        printf("5 - Listar todos os Equipamentos.\n");
        printf("6 - Listar todos os Equipamentos por tipo.\n");
        printf("7 - Listar todos os Equipamentos por Estado.\n");
        printf("8 - Pesquisar um Equipamento por: (Código, Endereço IP ou Endereço MAC).\n");
        printf("9 - Guardar e Carregar o Inventário de Equipamentos através de Ficheiro Binário.\n");
        printf("10 - Outras Atividades que considere relevantes.\n");
        printf("0 - Sair\n");
        printf("=================================================================================\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);

        // Processar opção
        switch(opcao) {

            case 1:
                printf("1. Adicionar um Novo Equipamento de Rede.\n");
                break;

            case 2: {
                printf("2. Remover um Equipamento de Rede.\n");
                break;
            }
            case 3: {
                printf("3. Alterar os Dados do Equipamento.\n");
                break;
            }
            case 4: {
                printf("4. Alterar o Status do Equipamento (Operacional/Em Falha /Em Manutencao ou Desativado).\n");
                break;
            }
            case 5: {
                printf("5. Listar todos os Equipamentos.\n");
                break;
            }
            case 6: {
                printf("6. Listar todos os Equipamentos por tipo.\n");
                break;
            }
            case 7: {
                printf("7. Listar todos os Equipamentos por Estado.\n");
                break;
            }
            case 8: {
                printf("8. Pesquisar um Equipamento por: (Código, Endereço IP ou Endereço MAC).\n");
                break;
            }
            case 9:{
                printf("9. Guardar e Carregar o Inventário de Equipamentos através de Ficheiro Binário.\n");
                break;
            }
            case 10: {
                printf("10. Outras Atividades: (Gerar Relatórios, Alertas de Falhas, etc.).\n");
                break;
            }
            case 0:
                printf("\nA Encerrar o Sistema.\n");
                break;

            default:
                printf("\n[Erro] Opcao invalida!\n");
        }

    } while(opcao != 0);

    return 0;
}
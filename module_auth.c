// module_auth.c — Autenticacao simples de tecnico (sem base de dados externa)

#include "noc.h"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

/* ─── Helpers privados ──────────────────────────────────────────────────── */

/* djb2 — hash determinístico para senhas */
static unsigned int hashSenha(const char *senha)
{
    unsigned int h = 5381;
    unsigned char c;
    while ((c = (unsigned char)*senha++))
        h = ((h << 5) + h) + c;
    return h;
}

/* Lê senha mascarando cada caracter com '*'.
 * Quando stdin nao e um terminal (pipe/redirect) usa fgets sem mascara. */
static void lerSenha(const char *prompt, char *dest, int maxLen)
{
    printf("%s: ", prompt);
    fflush(stdout);
#ifdef _WIN32
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (GetFileType(hIn) == FILE_TYPE_CHAR) {
        /* Terminal interativo — mascara com '*' */
        int i = 0, c;
        while ((c = _getch()) != '\r' && i < maxLen - 1) {
            if (c == '\b' && i > 0) { i--; printf("\b \b"); }
            else if (c >= 32)       { dest[i++] = (char)c; putchar('*'); }
        }
        dest[i] = '\0';
        putchar('\n');
    } else {
        /* Stdin redirecionado (pipe/ficheiro) — leitura simples */
        if (fgets(dest, maxLen, stdin) != NULL)
            dest[strcspn(dest, "\r\n")] = '\0';
    }
#else
    if (isatty(STDIN_FILENO)) {
        struct termios old, novo;
        tcgetattr(STDIN_FILENO, &old);
        novo = old;
        novo.c_lflag &= ~(tcflag_t)ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &novo);
        int i = 0, c;
        while ((c = getchar()) != '\n' && c != EOF && i < maxLen - 1)
            dest[i++] = (char)c;
        dest[i] = '\0';
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        putchar('\n');
    } else {
        if (fgets(dest, maxLen, stdin) != NULL)
            dest[strcspn(dest, "\r\n")] = '\0';
    }
#endif
}

static int carregarCredenciais(Credencial **out, int *total)
{
    FILE *f = fopen(FICH_TECNICOS, "rb");
    if (f == NULL) { *out = NULL; *total = 0; return 0; }
    if (fread(total, sizeof(int), 1, f) != 1 || *total <= 0)
        { fclose(f); *out = NULL; *total = 0; return 0; }
    *out = malloc((size_t)*total * sizeof(Credencial));
    if (*out == NULL) { fclose(f); *total = 0; return 0; }
    if ((int)fread(*out, sizeof(Credencial), (size_t)*total, f) != *total)
        { free(*out); *out = NULL; *total = 0; fclose(f); return 0; }
    fclose(f);
    return 1;
}

static void guardarCredenciais(const Credencial *creds, int total)
{
    FILE *f = fopen(FICH_TECNICOS, "wb");
    if (f == NULL) return;
    fwrite(&total, sizeof(int), 1, f);
    fwrite(creds, sizeof(Credencial), (size_t)total, f);
    fclose(f);
}

/* Cria ficheiro de credenciais com conta padrao se ainda nao existir */
void inicializarTecnicos(void)
{
    FILE *f = fopen(FICH_TECNICOS, "rb");
    if (f != NULL) { fclose(f); return; }

    Credencial admin;
    memset(&admin, 0, sizeof(Credencial));
    strncpy(admin.username,     "admin",         MAX_NOME - 1);
    strncpy(admin.nomeCompleto, "Administrador", MAX_NOME - 1);
    admin.passwordHash = hashSenha("admin123");

    int total = 1;
    guardarCredenciais(&admin, total);
    printf("\n  [INFO] 1.a execucao — conta criada:  utilizador: admin  |  senha: admin123\n");
}

/* Bloqueia o arranque ate login valido.
 * Devolve 1 em caso de sucesso, 0 apos 3 tentativas falhadas. */
int autenticarTecnico(Sistema *s)
{
    Credencial *creds;
    int total;
    carregarCredenciais(&creds, &total);

    for (int tentativas = 3; tentativas > 0; tentativas--)
    {
        limparEcra();
        printf("\n  ╔══════════════════════════════════════════╗\n");
        printf("  ║   NOC — Autenticacao de Tecnico          ║\n");
        printf("  ╚══════════════════════════════════════════╝\n\n");
        printf("  Tentativas restantes: %d\n\n", tentativas);

        char username[MAX_NOME] = {0};
        char senha[MAX_NOME]    = {0};
        lerString("  Utilizador", username, MAX_NOME);
        lerSenha("  Senha",       senha,    MAX_NOME);

        unsigned int h = hashSenha(senha);
        for (int i = 0; i < total; i++) {
            if (strcmp(creds[i].username, username) == 0 &&
                creds[i].passwordHash == h)
            {
                strncpy(s->tecnicoLogado, creds[i].nomeCompleto, MAX_TECNICO - 1);
                s->tecnicoLogado[MAX_TECNICO - 1] = '\0';
                free(creds);
                limparEcra();
                printf("\n  Bem-vindo, %s!\n", s->tecnicoLogado);
                pausar();
                limparEcra();
                return 1;
            }
        }
        printf("\n  [!] Utilizador ou senha incorretos.\n");
        if (tentativas > 1) pausar();
    }

    free(creds);
    printf("\n  [!] Numero maximo de tentativas excedido. A encerrar.\n");
    return 0;
}

/* Menu de gestao de perfil acessivel a partir do menu principal */
void menuGestaoPerfil(Sistema *s)
{
    int opcao;
    do {
        limparEcra();
        printf("\n  ╔══════════════════════════════════════════╗\n");
        printf("  ║   Perfil / Conta                         ║\n");
        printf("  ╠══════════════════════════════════════════╣\n");
        printf("  ║  Tecnico : %-30s║\n", s->tecnicoLogado);
        printf("  ╠══════════════════════════════════════════╣\n");
        printf("  ║  1. Alterar senha                        ║\n");
        printf("  ║  2. Adicionar tecnico                    ║\n");
        printf("  ║  3. Listar tecnicos                      ║\n");
        printf("  ║  0. Voltar                               ║\n");
        printf("  ╚══════════════════════════════════════════╝\n");

        opcao = lerInteiro("  Opcao", 0, 3);

        if (opcao == 1)
        {
            Credencial *creds; int total;
            carregarCredenciais(&creds, &total);

            char username[MAX_NOME]  = {0};
            char senhaAtual[MAX_NOME]= {0};
            char senhaNova[MAX_NOME] = {0};
            char senhaConf[MAX_NOME] = {0};

            lerString("  Utilizador",  username,   MAX_NOME);
            lerSenha("  Senha atual",  senhaAtual, MAX_NOME);

            int idx = -1;
            unsigned int h = hashSenha(senhaAtual);
            for (int i = 0; i < total; i++)
                if (strcmp(creds[i].username, username) == 0 &&
                    creds[i].passwordHash == h) { idx = i; break; }

            if (idx == -1) {
                printf("\n  [!] Credenciais incorretas.\n");
            } else {
                lerSenha("  Nova senha", senhaNova, MAX_NOME);
                lerSenha("  Confirmar",  senhaConf, MAX_NOME);
                if (strcmp(senhaNova, senhaConf) != 0)
                    printf("\n  [!] As senhas nao coincidem.\n");
                else if (strlen(senhaNova) < 4)
                    printf("\n  [!] Senha demasiado curta (minimo 4 caracteres).\n");
                else {
                    creds[idx].passwordHash = hashSenha(senhaNova);
                    guardarCredenciais(creds, total);
                    printf("\n  [OK] Senha alterada com sucesso.\n");
                }
            }
            free(creds);
            pausar();
        }
        else if (opcao == 2)
        {
            char novoUser[MAX_NOME] = {0};
            lerString("  Novo utilizador", novoUser, MAX_NOME);

            Credencial *creds; int total;
            carregarCredenciais(&creds, &total);

            int dup = 0;
            for (int i = 0; i < total; i++)
                if (strcmp(creds[i].username, novoUser) == 0) { dup = 1; break; }

            if (dup) {
                printf("\n  [!] Utilizador ja existe.\n");
                free(creds);
            } else {
                Credencial novo;
                memset(&novo, 0, sizeof(Credencial));
                strncpy(novo.username, novoUser, MAX_NOME - 1);
                lerString("  Nome completo", novo.nomeCompleto, MAX_NOME);
                char senha[MAX_NOME] = {0};
                lerSenha("  Senha", senha, MAX_NOME);
                novo.passwordHash = hashSenha(senha);

                Credencial *novos = realloc(creds, (size_t)(total + 1) * sizeof(Credencial));
                if (novos == NULL) {
                    printf("\n  [!] Erro de memoria.\n");
                    free(creds);
                } else {
                    novos[total] = novo;
                    guardarCredenciais(novos, total + 1);
                    free(novos);
                    printf("\n  [OK] Tecnico '%s' adicionado.\n", novoUser);
                }
            }
            pausar();
        }
        else if (opcao == 3)
        {
            Credencial *creds; int total;
            carregarCredenciais(&creds, &total);
            printf("\n  %-20s %s\n", "Utilizador", "Nome Completo");
            printf("  ");
            for (int i = 0; i < 52; i++) printf("-");
            printf("\n");
            for (int i = 0; i < total; i++)
                printf("  %-20s %s\n", creds[i].username, creds[i].nomeCompleto);
            free(creds);
            pausar();
        }
    } while (opcao != 0);
}

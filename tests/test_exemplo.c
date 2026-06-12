#include "unity/unity.h"
#include "../noc.h"

/* setUp e tearDown são obrigatórias no Unity — correm antes/depois de cada teste */
void setUp(void) {}
void tearDown(void) {}

/* ── tipoToString ─────────────────────────────────────────────────────────── */

void test_tipoToString_router(void) {
    TEST_ASSERT_EQUAL_STRING("Router", tipoToString(ROUTER));
}

void test_tipoToString_switch(void) {
    TEST_ASSERT_EQUAL_STRING("Switch L2", tipoToString(SWITCH_L2));
}

void test_tipoToString_invalido(void) {
    TEST_ASSERT_EQUAL_STRING("Desconhecido", tipoToString((TipoEquipamento)99));
}

/* ── estadoEquipamentoToString ────────────────────────────────────────────── */

void test_estadoEquipamento_operacional(void) {
    TEST_ASSERT_EQUAL_STRING("Operacional", estadoEquipamentoToString(OPERACIONAL));
}

void test_estadoEquipamento_falha(void) {
    TEST_ASSERT_EQUAL_STRING("Em Falha", estadoEquipamentoToString(EM_FALHA));
}

/* ── encontrarPorCodigo ───────────────────────────────────────────────────── */

void test_encontrarPorCodigo_encontra(void) {
    /* Monta uma lista ligada manualmente, sem chamar adicionarEquipamento */
    Sistema s = {0};

    NodeEquipamento no1 = {0};
    no1.dados.codigo = 10;
    no1.proximo = NULL;

    NodeEquipamento no2 = {0};
    no2.dados.codigo = 20;
    no2.proximo = &no1;

    s.equipamentos = &no2;

    NodeEquipamento *resultado = encontrarPorCodigo(&s, 10);
    TEST_ASSERT_NOT_NULL(resultado);
    TEST_ASSERT_EQUAL_INT(10, resultado->dados.codigo);
}

void test_encontrarPorCodigo_nao_encontra(void) {
    Sistema s = {0};
    s.equipamentos = NULL;

    TEST_ASSERT_NULL(encontrarPorCodigo(&s, 99));
}

/* ── temIncidentePendente ─────────────────────────────────────────────────── */

void test_temIncidentePendente_com_pendente(void) {
    Sistema s = {0};

    NodeIncidente inc = {0};
    inc.dados.codigoEquipamento = 5;
    inc.dados.estado = PENDENTE;
    inc.proximo = NULL;

    s.incidentes = &inc;

    TEST_ASSERT_EQUAL_INT(1, temIncidentePendente(&s, 5));
}

void test_temIncidentePendente_concluido_nao_conta(void) {
    Sistema s = {0};

    NodeIncidente inc = {0};
    inc.dados.codigoEquipamento = 5;
    inc.dados.estado = CONCLUIDO;
    inc.proximo = NULL;

    s.incidentes = &inc;

    TEST_ASSERT_EQUAL_INT(0, temIncidentePendente(&s, 5));
}

void test_temIncidentePendente_lista_vazia(void) {
    Sistema s = {0};
    s.incidentes = NULL;

    TEST_ASSERT_EQUAL_INT(0, temIncidentePendente(&s, 1));
}

/* ── main ─────────────────────────────────────────────────────────────────── */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_tipoToString_router);
    RUN_TEST(test_tipoToString_switch);
    RUN_TEST(test_tipoToString_invalido);

    RUN_TEST(test_estadoEquipamento_operacional);
    RUN_TEST(test_estadoEquipamento_falha);

    RUN_TEST(test_encontrarPorCodigo_encontra);
    RUN_TEST(test_encontrarPorCodigo_nao_encontra);

    RUN_TEST(test_temIncidentePendente_com_pendente);
    RUN_TEST(test_temIncidentePendente_concluido_nao_conta);
    RUN_TEST(test_temIncidentePendente_lista_vazia);

    return UNITY_END();
}

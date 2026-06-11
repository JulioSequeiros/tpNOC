#include "noc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void limparBufferLocal(void) {

    int c;

    while ((c = getchar()) != '\n' && c != EOF);
}

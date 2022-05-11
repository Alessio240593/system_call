/** @file err_exit.h
 *  @brief Contiene la definizione della funzione di stampa degli errori.
*/

#ifndef ERR_EXIT_H
#define ERR_EXIT_H

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

void errExit(const char *msg);

#endif
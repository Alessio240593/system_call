/** @file err_exit.h
 *  @brief Contiene la definizione della funzione di stampa degli errori.
*/

#ifndef ERR_EXIT_H
#define ERR_EXIT_H

/// @brief Prints the error message of the last failed
///         system call and terminates the calling process.
void errExit(const char *msg);

#endif
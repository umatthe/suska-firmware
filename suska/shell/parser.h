/////////////////////////////////////
//                                 //
// Kommandozeilen-Parser           //
//                                 //
// Udo Matthe   08.12.2012         //
//                                 //
/////////////////////////////////////

#ifndef __PARSER_H__
#define __PARSER_H__
#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#ifdef __HAVE_FILESYSTEM__
#include "../tff/ff.h"
#endif

#define MAXPARAMETER 3

struct befehle
{
	uint8_t parameter;
	const __flash char   *command;
	void    *function;
	const __flash char    *help;
};



uint8_t parse_line(uint8_t *line);
#ifdef __HAVE_FILESYSTEM__
FRESULT parse_file(char *filename);
#endif

void shell_reset( void );
void shell_trace( uint8_t *level );
void shell_delay( uint8_t *deltime );
void shell_help( void );
#ifdef __HAVE_FILESYSTEM__
void shell_ls( void );
void shell_do( uint8_t *filename );
void shell_hex( uint8_t *filename );
void shell_cat( uint8_t *filename );
#endif

#endif

/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes paul.mattes@case.edu), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <config.h>
 #include "private.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

static const struct
{
	unsigned char chr;
	const gchar * utf;
} cg_to_utf[] =
{
	{ 0x8c, "≤" }, // CG 0xf7, less or equal "≤"
	{ 0xae, "≥" }, // CG 0xd9, greater or equal "≥"
	{ 0xbe, "≠" }, // CG 0xbe, not equal "≠"
	{ 0xad, "[" }, // "["
	{ 0xbd, "]" }, // "]"
	{ 0xb8, "÷"	}, // Division Sign ÷
	{ 0x90, "⎕"	}, // APL FUNCTIONAL SYMBOL QUAD
};

const gchar * v3270_translate_cg_to_utf(unsigned char chr)
{
	size_t ix;

	if(!chr)
		return " ";

	for(ix = 0; ix < G_N_ELEMENTS(cg_to_utf); ix++)
	{
		if(chr == cg_to_utf[ix].chr)
			return cg_to_utf[ix].utf;

	}

	return NULL;
}

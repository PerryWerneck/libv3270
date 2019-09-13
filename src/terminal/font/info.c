/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
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

void v3270_font_info_init(v3270FontInfo *info)
{
	memset(info,0,sizeof(v3270FontInfo));

	info->family	= g_strdup(v3270_get_default_font_name());
	info->step		= 1;
	info->size		= 0.1;
}

void v3270_font_info_unset(v3270FontInfo *info)
{
	if(info->family)
	{
		g_free(info->family);
		info->family = NULL;
	}

	if(info->scaled)
	{
		cairo_scaled_font_destroy(info->scaled);
		info->scaled = NULL;
	}

	if(info->face) {
		cairo_font_face_destroy(info->face);
		info->face = NULL;
	}

}

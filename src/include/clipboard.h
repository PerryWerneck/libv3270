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
 * Este programa está nomeado como clipboard.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifndef V3270_CLIPBOARD_H_INCLUDED

	#define V3270_CLIPBOARD_H_INCLUDED

	#include <gtk/gtk.h>
	#include <lib3270.h>
	#include <v3270.h>
	#include <terminal.h>
	#include <internals.h>
	#include <lib3270/selection.h>
	#include <lib3270/log.h>
	#include <lib3270/actions.h>

	enum
	{
		CLIPBOARD_TYPE_TEXT,
	};

	struct selection
	{
		struct {
			unsigned int row;
			unsigned int col;
			unsigned int width;
			unsigned int height;

		} bounds;					///< @brief Clipboard rectangle.

		struct v3270_character contents[1];

	};

	G_GNUC_INTERNAL void		  v3270_update_system_clipboard(GtkWidget *widget);
	G_GNUC_INTERNAL const char	* v3270_update_selected_text(GtkWidget *widget, gboolean cut);



#endif // V3270_CLIPBOARD_H_INCLUDED


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
		CLIPBOARD_TYPE_CSV,
		CLIPBOARD_TYPE_HTML,
		CLIPBOARD_TYPE_V3270_UNPROTECTED
	};

	/// @brief Column from selection.
	struct ColumnDescription
	{
		unsigned int begin;
		unsigned int width;
	};

	/// @brief Selection data for sending to another application.
	struct SelectionHeader
	{
		unsigned int length;		///< @brief Length of the data block.
		unsigned int build;			///< @brief V3270 build id.
	};

	/// @brief Header of a list of fields.
	struct SelectionBlockHeader
	{
		/// @brief Cursor address.
		unsigned int cursor_address;

		/// @brief Number of records;
		unsigned int records;
	};

	/// @brief Header for a field prefix.
	struct SelectionFieldHeader
	{
		/// @brief Field address.
		unsigned short baddr;

		/// @brief Field length.
		unsigned short length;

	};

	G_GNUC_INTERNAL void		  v3270_update_system_clipboard(GtkWidget *widget);
	G_GNUC_INTERNAL const char	* v3270_update_selected_text(GtkWidget *widget, gboolean cut);
	G_GNUC_INTERNAL GList		* v3270_getColumns_from_selection(v3270 * terminal);


	/// @brief Get formatted contents as single text.
	G_GNUC_INTERNAL gchar		* v3270_get_copy_as_text(v3270 * terminal);
	G_GNUC_INTERNAL gchar		* v3270_get_copy_as_html(v3270 * terminal);
	G_GNUC_INTERNAL gchar		* v3270_get_copy_as_table(v3270 * terminal, const gchar *delimiter);
	G_GNUC_INTERNAL gchar		* v3270_get_copy_as_data_block(v3270 * terminal);

#endif // V3270_CLIPBOARD_H_INCLUDED


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
 * Este programa está nomeado como private.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifndef PRIVATE_H_INCLUDED

	#include <config.h>

	#include <gtk/gtk.h>

	#define ENABLE_NLS
	#define GETTEXT_PACKAGE PACKAGE_NAME

	#include <libintl.h>
	#include <glib/gi18n.h>

	#include <v3270.h>
	#include <v3270/trace.h>

	#include <lib3270.h>
	#include <lib3270/log.h>
	#include <lib3270/trace.h>

	/// @brief V3270 Trace Signal list
	enum
	{
		V3270_TRACE_SIGNAL_COMMAND,

		V3270_TRACE_SIGNAL_LAST
	};

	G_GNUC_INTERNAL guint v3270_trace_signal[V3270_TRACE_SIGNAL_LAST];

	G_BEGIN_DECLS

	struct _V3270TraceClass
	{
		GtkBoxClass parent_class;

	};

	struct _V3270Trace
	{

		GtkBox				  parent;
		H3270				* hSession;		///< @brief TN3270 Session.
		GtkWidget			* terminal;		///< @brief V3270 Widget.
		GtkScrolledWindow	* scroll;

		GtkTextView			* view;			///< @brief Text view;
		GtkTextBuffer		* text;			///< @brief Trace window contents.
		GtkEntry			* entry;		///< @brief Command line entry.
		GtkWidget 			* buttons;		///< @brief Button bar.

		gchar 				* filename;		///< @brief Selected file name.

		guint 				  log_handler;	///< @brief GTK Log Handler.

		/// @brief lib3270's saved trace handler.
		struct {
				void (*handler)(H3270 *session, void *userdata, const char *fmt, va_list args);
				void *userdata;
		} trace;

	};

	 G_END_DECLS



#endif // PRIVATE_H_INCLUDED


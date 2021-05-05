/* SPDX-License-Identifier: LGPL-3.0-or-later */

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

#ifndef V3270_SETTINGS_URL_H_INCLUDED

	#define V3270_SETTINGS_URL_H_INCLUDED

	#include <gtk/gtk.h>

	/*--[ URL Settings Widget ]--------------------------------------------------------------------------*/

	G_BEGIN_DECLS

	#define GTK_TYPE_V3270URLEdit				(V3270URLEdit_get_type ())
	#define GTK_V3270URLEdit(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270URLEdit, V3270URLEdit))
	#define GTK_V3270URLEdit_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270URLEdit, V3270URLEditClass))
	#define GTK_IS_V3270URLEdit(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270URLEdit))
	#define GTK_IS_V3270URLEdit_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270URLEdit))
	#define GTK_V3270URLEdit_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270URLEdit, V3270URLEditClass))

	typedef struct _V3270URLEdit		V3270URLEdit;
	typedef struct _V3270URLEditClass	V3270URLEditClass;

	GtkWidget * v3270_url_edit_new();
	void v3270_url_edit_set_url(GtkWidget *widget, const gchar *url);
	const gchar * v3270_url_edit_get_url(GtkWidget *widget);
	gboolean v3270_url_edit_is_valid(GtkWidget *widget);

	G_END_DECLS

#endif // V3270_SETTINGS_URL_H_INCLUDED

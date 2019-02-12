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

 #define ENABLE_NLS
 #define GETTEXT_PACKAGE PACKAGE_NAME

 #include <gtk/gtk.h>
 #include <libintl.h>
 #include <glib/gi18n.h>

 #include "private.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 const gchar * v3270_get_ssl_state_icon_name(GtkWidget *widget)
 {
 	g_return_val_if_fail(GTK_IS_V3270(widget),"");
	return lib3270_get_ssl_state_icon_name(v3270_get_session(widget));
 }

 const gchar * v3270_get_ssl_state_message(GtkWidget *widget)
 {
 	g_return_val_if_fail(GTK_IS_V3270(widget),"");
	return lib3270_get_ssl_state_message(v3270_get_session(widget));
 }

 const gchar * v3270_get_ssl_status_description(GtkWidget *widget)
 {
 	g_return_val_if_fail(GTK_IS_V3270(widget),"");
	return lib3270_get_ssl_state_description(v3270_get_session(widget));
 }


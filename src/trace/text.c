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

 #include "private.h"

 #include <lib3270/toggle.h>
 #include <lib3270/properties.h>
 #include <internals.h>
 #include <v3270/dialogs.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 struct _append_text
 {
 	GtkWidget	* widget;
 	gchar		  text[1];
 };

 static gboolean bg_append_text(struct _append_text *cfg)
 {
 	GtkTextBuffer * buffer = v3270_trace_get_text_buffer(cfg->widget);

	GtkTextIter	itr;
	gtk_text_buffer_get_end_iter(buffer,&itr);

	if(g_utf8_validate(cfg->text,strlen(cfg->text),NULL))
		gtk_text_buffer_insert(buffer,&itr,cfg->text,strlen(cfg->text));
	else
		gtk_text_buffer_insert(buffer,&itr,"** Invalid UTF8 String **",-1);

	// Move window
	GtkScrolledWindow	* scrolled = v3270_trace_get_scrolled_window(cfg->widget);
	GtkAdjustment		* vadj = gtk_scrolled_window_get_vadjustment(scrolled);

	gtk_adjustment_set_value(vadj,gtk_adjustment_get_upper(vadj));
	gtk_scrolled_window_set_vadjustment(scrolled, vadj);

	return FALSE;

 }

 LIB3270_EXPORT void v3270_trace_append_text(GtkWidget *widget, const gchar *text)
 {
 	g_return_if_fail(GTK_IS_V3270_TRACE(widget));

	// Enqueue update.
 	struct _append_text * cfg = g_malloc0(sizeof(struct _append_text)+strlen(text)+1);
 	cfg->widget = GTK_V3270_TRACE(widget);
 	strcpy(cfg->text,text);

	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,(GSourceFunc) bg_append_text, cfg, g_free);

 }

 LIB3270_EXPORT void v3270_trace_vprintf(GtkWidget *widget, const char *fmt, va_list args)
 {
 	g_autofree gchar * text = g_strdup_vprintf(fmt,args);
 	v3270_trace_append_text(widget,text);
 }

 LIB3270_EXPORT void v3270_trace_printf(GtkWidget *widget, const char *fmt, ... )
 {
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	v3270_trace_vprintf(widget,fmt,arg_ptr);
	va_end(arg_ptr);
 }


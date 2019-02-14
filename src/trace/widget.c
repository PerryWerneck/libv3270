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
 * Este programa está nomeado como trace.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

/**
 * SECTION:V3270Trace
 * @Short_description: A trace monitor widget.
 * @Title: V3270Trace
 *
 * The #V3270Trace shows a text area with the lib3270 trace output.
 *
 */

 #include <gtk/gtk.h>

 #define ENABLE_NLS
 #define GETTEXT_PACKAGE PACKAGE_NAME

 #include <libintl.h>
 #include <glib/gi18n.h>

 #include <v3270.h>
 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>
 #include <lib3270/properties.h>
 #include <v3270/trace.h>

#if defined( HAVE_SYSLOG )
 #include <syslog.h>
#endif // HAVE_SYSLOG

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 G_BEGIN_DECLS

 struct _V3270TraceClass
 {
 	GtkGridClass parent_class;

 };

 struct _V3270Trace
 {
 	GtkGrid			  parent;
 	H3270			* hSession;	/// @brief TN3270 Session.

	GtkTextBuffer	* text;		/// @brief Trace window contents.
	GtkWidget		* entry;	/// @brief Command line entry.
	GtkWidget		* run;		/// @brief "exec" button.

	/// @brief lib3270's saved trace handler.
	struct {
			void (*handler)(H3270 *session, void *userdata, const char *fmt, va_list args);
			void *userdata;
	} trace;

 };

 G_END_DECLS

 G_DEFINE_TYPE(V3270Trace, V3270Trace, GTK_TYPE_GRID);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void trace_handler(H3270 *hSession, void *userdata, const char *fmt, va_list args)
 {
	g_autofree gchar *ptr		= g_strdup_vprintf(fmt,args);
	g_autofree gchar * utftext	= g_convert_with_fallback(ptr,-1,"UTF-8",lib3270_get_display_charset(hSession),"?",NULL,NULL,NULL);

	v3270_trace_append_text(GTK_WIDGET(userdata),utftext);

 }

 static void set_session(V3270Trace *widget, H3270 *hSession)
 {
	if(widget->hSession) {
		lib3270_set_trace_handler(widget->hSession,widget->trace.handler,widget->trace.userdata);
	}

	widget->hSession = hSession;

	if(hSession) {
		lib3270_get_trace_handler(hSession,&widget->trace.handler,&widget->trace.userdata);
		lib3270_set_trace_handler(hSession,trace_handler,(void *) widget);
	}

 }

 static void finalize(GObject *object)
 {
	debug("V3270Trace::%s",__FUNCTION__);

	set_session(GTK_V3270_TRACE(object),NULL);

	G_OBJECT_CLASS(V3270Trace_parent_class)->finalize(object);
 }

 static void V3270Trace_class_init(G_GNUC_UNUSED V3270TraceClass *klass)
 {
	G_OBJECT_CLASS(klass)->finalize = finalize;
 }

 static void V3270Trace_init(V3270Trace *widget)
 {

	// Create text view
	{
		GtkWidget *scrolled = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_widget_set_vexpand(scrolled,TRUE);
		gtk_widget_set_hexpand(scrolled,TRUE);
		gtk_grid_attach(GTK_GRID(widget),scrolled,0,0,10,1);

		GtkWidget *view = gtk_text_view_new();
		gtk_text_view_set_monospace(GTK_TEXT_VIEW(view),TRUE);

		widget->text = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
		gtk_text_view_set_editable(GTK_TEXT_VIEW(view), TRUE);

		gtk_container_add(GTK_CONTAINER(scrolled),view);

	}

	// Create command line
	{
		widget->entry = gtk_entry_new();
		gtk_grid_attach(GTK_GRID(widget),widget->entry,0,1,8,1);
		gtk_widget_set_sensitive(widget->entry,FALSE);

		widget->run = gtk_button_new_from_icon_name("system-run",GTK_ICON_SIZE_BUTTON);
		gtk_grid_attach(GTK_GRID(widget),widget->run,9,1,1,1);
		gtk_widget_set_sensitive(widget->run,FALSE);

	}


 }

 LIB3270_EXPORT	GtkWidget * v3270_trace_new(GtkWidget *terminal)
 {
	g_return_val_if_fail(GTK_IS_V3270(terminal),NULL);

	V3270Trace * widget = GTK_V3270_TRACE(g_object_new(GTK_TYPE_V3270_TRACE, NULL));

	set_session(widget, v3270_get_session(terminal));

	return GTK_WIDGET(widget);
 }

 struct _append_text
 {
 	V3270Trace *widget;
 	gchar text[1];
 };

 static void bg_append_text(struct _append_text *cfg)
 {
	GtkTextIter	itr;
	gtk_text_buffer_get_end_iter(cfg->widget->text,&itr);

	if(g_utf8_validate(cfg->text,strlen(cfg->text),NULL))
	{
		gtk_text_buffer_insert(cfg->widget->text,&itr,cfg->text,strlen(cfg->text));
	}
	else
	{
		gtk_text_buffer_insert(cfg->widget->text,&itr,"** Invalid UTF8 String **",-1);
	}

	// gtk_text_buffer_get_end_iter(hwnd->text,&itr);
	// gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW(hwnd->view), &itr, 0.0, FALSE, 0.0, 0.0);

	//GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(hwnd->scroll));
	//gtk_adjustment_set_value(vadj,gtk_adjustment_get_upper(vadj));
	//gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(hwnd->scroll), vadj);
 }

 LIB3270_EXPORT void v3270_trace_append_text(GtkWidget *widget, const gchar *text)
 {
	g_return_if_fail(GTK_IS_V3270_TRACE(widget));

	// Enqueue update.
 	struct _append_text * cfg = g_malloc0(sizeof(struct _append_text)+strlen(text)+1);
	gdk_threads_add_idle_full(G_PRIORITY_LOW,(GSourceFunc) bg_append_text,cfg,g_free);

 }

 LIB3270_EXPORT void v3270_trace_vprintf(GtkWidget *widget, const char *fmt, va_list args)
 {
 	g_autofree * text = g_strdup_vprintf(fmt,args);
 	v3270_trace_append_text(widget,text);
 }

 LIB3270_EXPORT void v3270_trace_printf(GtkWidget *widget, const char *fmt, ... )
 {
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	v3270_trace_vprintf(widget,fmt,arg_ptr);
	va_end(arg_ptr);
 }


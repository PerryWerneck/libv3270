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

 #include "private.h"

 #include <terminal.h>
 #include <lib3270/toggle.h>
 #include <lib3270/properties.h>
 #include <internals.h>
 #include <v3270/dialogs.h>
 #include "marshal.h"

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static const LIB3270_TOGGLE_ID toggles[] = {
	LIB3270_TOGGLE_DS_TRACE,
	LIB3270_TOGGLE_NETWORK_TRACE,
	LIB3270_TOGGLE_EVENT_TRACE,
	LIB3270_TOGGLE_SSL_TRACE,
	LIB3270_TOGGLE_SCREEN_TRACE
 };

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

	struct
	{
		GtkWidget 		* box;			///< @brief Button box.
		GtkWidget		* widgets[G_N_ELEMENTS(toggles)];
	} buttons;

	gchar 				* filename;		///< @brief Selected file name.

	guint 				  log_handler;	///< @brief GTK Log Handler.

	/// @brief lib3270's saved trace handler.
	struct {
			void (*handler)(H3270 *session, void *userdata, const char *fmt, va_list args);
			void *userdata;
	} trace;

 };

 guint v3270_trace_signal[V3270_TRACE_SIGNAL_LAST]	= { 0 };

 G_DEFINE_TYPE(V3270Trace, V3270Trace, GTK_TYPE_BOX);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void trace_handler(H3270 *hSession, void *userdata, const char *fmt, va_list args)
 {
	g_autofree gchar *ptr		= g_strdup_vprintf(fmt,args);
	g_autofree gchar * utftext	= g_convert_with_fallback(ptr,-1,"UTF-8",lib3270_get_display_charset(hSession),"?",NULL,NULL,NULL);

	v3270_trace_append_text(GTK_WIDGET(userdata),utftext);

 }

 static void set_session(V3270Trace *widget, H3270 *hSession)
 {
 	// Return if it's the same session.
 	if(widget->hSession == hSession)
		return;

	debug("%s: Session changes %p -> %p", __FUNCTION__, widget->hSession, hSession);

	if(widget->hSession) {

		debug("Disconnecting from session %p",widget->hSession);

		lib3270_set_trace_handler(widget->hSession,widget->trace.handler,widget->trace.userdata);

		size_t ix;
		for(ix=0;ix < G_N_ELEMENTS(toggles); ix++)
			lib3270_set_toggle(widget->hSession, toggles[ix],0);
	}

	widget->hSession = hSession;

	if(hSession) {
		lib3270_get_trace_handler(hSession,&widget->trace.handler,&widget->trace.userdata);
		lib3270_set_trace_handler(hSession,trace_handler,(void *) widget);
	}

	// v3270_toggle_button_set_session
	size_t ix;
	for(ix = 0; ix < G_N_ELEMENTS(toggles); ix++)
	{
		if(widget->buttons.widgets[ix])
			v3270_toggle_button_set_session(widget->buttons.widgets[ix],hSession);
	}


 }

 static void finalize(GObject *object) {

	debug("V3270Trace::%s",__FUNCTION__);

 	V3270Trace *trace = GTK_V3270_TRACE(object);

 	size_t ix;
	for(ix = 0; ix < G_N_ELEMENTS(toggles); ix++)
	{
		// TODO: Use button "destroy" signal to cleanup.
		trace->buttons.widgets[ix] = NULL;
	}

 	if(trace->filename) {
		g_free(trace->filename);
		trace->filename = NULL;
	}

	if(trace->log_handler) {

		// Remove glib log handler

		g_log_remove_handler(NULL,trace->log_handler);
		trace->log_handler = 0;
	}

	if(trace->terminal && GTK_V3270(trace->terminal)->trace == GTK_WIDGET(object))
	{
		debug("V3270Trace::%s - Removing trace widget association",__FUNCTION__);


		g_object_notify_by_pspec(
			G_OBJECT(trace->terminal),
			GTK_V3270_GET_CLASS(trace->terminal)->properties.trace
		);

		GTK_V3270(trace->terminal)->trace = NULL;
	}

	set_session(trace,NULL);

	g_clear_object(&trace->terminal);

	G_OBJECT_CLASS(V3270Trace_parent_class)->finalize(object);
 }

 static void V3270Trace_class_init(V3270TraceClass *klass)
 {
	GObjectClass * gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->finalize = finalize;

	v3270_trace_signal[V3270_TRACE_SIGNAL_COMMAND] =
		g_signal_new(	I_("command"),
						G_OBJECT_CLASS_TYPE(klass),
						G_SIGNAL_RUN_LAST,
						0,
						NULL, NULL,
						v3270trace_BOOLEAN__POINTER_POINTER,
						G_TYPE_BOOLEAN, 2, G_TYPE_POINTER, G_TYPE_POINTER);



 }

 static void v3270_trace_execute(GtkWidget *widget, const gchar *cmd)
 {
 	if(!*cmd)
		return;

	v3270_trace_printf(widget, "%s\n",cmd);

	V3270Trace *trace = GTK_V3270_TRACE(widget);

	if(trace->terminal)
	{
		int rc = v3270_trace_exec_command(widget,cmd);
		if(rc)
			v3270_trace_printf(widget, "rc=%d (%s)\n",rc,strerror(rc));
	}
	else
	{
		v3270_trace_append_text(widget, "Can't execute command without an associated terminal");
	}

	gtk_entry_set_text(trace->entry, "");

 }

 static void execute_command(GtkEntry *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, GtkWidget *widget)
 {
 	v3270_trace_execute(widget, gtk_entry_get_text(entry));
 }

 static void entry_activated(GtkEntry *entry, GtkWidget *widget)
 {
 	v3270_trace_execute(widget, gtk_entry_get_text(entry));
 }

 static void log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, GtkWidget *widget)
 {
	#ifndef LOG_INFO
		#define LOG_INFO 0
	#endif // LOG_INFO

	#ifndef LOG_ERR
		#define LOG_ERR 0
	#endif // LOG_ERR

	#ifndef LOG_DEBUG
		#define LOG_DEBUG 0
	#endif // LOG_DEBUG

 	static const struct _logtype
 	{
 		GLogLevelFlags	  log_level;
 		const gchar		* msg;
 	} logtype[] =
 	{
		{ G_LOG_FLAG_RECURSION,			"recursion"			},
		{ G_LOG_FLAG_FATAL,				"fatal error"		},

		/* GLib log levels */
		{ G_LOG_LEVEL_ERROR,			"error"				},
		{ G_LOG_LEVEL_CRITICAL,			"critical error"	},
		{ G_LOG_LEVEL_WARNING,			"warning"			},
		{ G_LOG_LEVEL_MESSAGE,			"message"			},
		{ G_LOG_LEVEL_INFO,				"info"				},
		{ G_LOG_LEVEL_DEBUG,			"debug"				},
 	};

	size_t f;

	for(f=0;f<G_N_ELEMENTS(logtype);f++)
	{
		if(logtype[f].log_level == log_level)
		{
			g_autofree gchar *text = g_strdup_printf("%s: %s %s",logtype[f].msg,log_domain ? log_domain : "",message);

			gchar *ptr;
			for(ptr = text;*ptr;ptr++)
			{
				if(*ptr < ' ')
					*ptr = ' ';
			}

			v3270_trace_printf(widget,"%s\n",text);

			return;
		}
	}

	v3270_trace_printf(widget,"%s %s\n",log_domain ? log_domain : "", message);

 }

 static void V3270Trace_init(V3270Trace *widget)
 {

 	debug("%s(%p)",__FUNCTION__,widget);

 	gtk_orientable_set_orientation(GTK_ORIENTABLE(widget),GTK_ORIENTATION_VERTICAL);

 	// Create toolbar
 	GtkWidget *buttons = widget->buttons.box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
 	{
		// https://developer.gnome.org/hig/stable/visual-layout.html.en
		gtk_container_set_border_width(GTK_CONTAINER(buttons), 12);
		gtk_box_set_spacing(GTK_BOX(buttons),12);

		gtk_button_box_set_layout(GTK_BUTTON_BOX(buttons), GTK_BUTTONBOX_START);

		gtk_widget_set_valign(buttons,GTK_ALIGN_START);
		gtk_box_pack_start(GTK_BOX(widget),buttons,FALSE,FALSE,0);

 	}

	// Create text view
	{
		widget->scroll = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL,NULL));
		gtk_scrolled_window_set_policy(widget->scroll,GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_widget_set_vexpand(GTK_WIDGET(widget->scroll),TRUE);
		gtk_widget_set_hexpand(GTK_WIDGET(widget->scroll),TRUE);

		gtk_box_pack_start(GTK_BOX(widget),GTK_WIDGET(widget->scroll),TRUE,TRUE,4);

		widget->view = GTK_TEXT_VIEW(gtk_text_view_new());

#if GTK_CHECK_VERSION(3,16,0)
		gtk_text_view_set_monospace(widget->view,TRUE);
#endif // GTK_CHECK_VERSION

		widget->text = gtk_text_view_get_buffer(widget->view);
		gtk_text_view_set_editable(widget->view, TRUE);

		gtk_container_add(GTK_CONTAINER(widget->scroll),GTK_WIDGET(widget->view));
		gtk_widget_set_can_default(GTK_WIDGET(widget->view),FALSE);

	}

	// Create command line
	{
		widget->entry = GTK_ENTRY(gtk_entry_new());

		gtk_widget_set_can_default(GTK_WIDGET(widget->entry),TRUE);
		gtk_widget_grab_focus(GTK_WIDGET(widget->entry));

		gtk_entry_set_activates_default(widget->entry,TRUE);
		gtk_widget_set_vexpand(GTK_WIDGET(widget->entry),FALSE);
		gtk_widget_set_hexpand(GTK_WIDGET(widget->entry),TRUE);

		gtk_entry_set_icon_from_icon_name(widget->entry,GTK_ENTRY_ICON_SECONDARY,"system-run");
		gtk_entry_set_placeholder_text(widget->entry,_("Command to execute"));

		gtk_box_pack_end(GTK_BOX(widget),GTK_WIDGET(widget->entry),FALSE,FALSE,4);

		g_signal_connect(G_OBJECT(widget->entry),"icon-press",G_CALLBACK(execute_command),widget);
		g_signal_connect(G_OBJECT(widget->entry),"activate",G_CALLBACK(entry_activated),widget);

	}

	// Create toggle buttons
	{
 		size_t ix;

		for(ix = 0; ix < G_N_ELEMENTS(toggles); ix++)
		{
			GtkWidget * item = widget->buttons.widgets[ix] = v3270_toggle_button_new(toggles[ix]);

			gtk_widget_set_can_focus(item,FALSE);
			gtk_widget_set_can_default(item,FALSE);

#if GTK_CHECK_VERSION(3,20,0)
			gtk_widget_set_focus_on_click(item,FALSE);
#endif // GTK 3,20,0

			gtk_box_pack_start(GTK_BOX(buttons),item,FALSE,FALSE,0);

		}
	}

	// Grab GTK messages.
	widget->log_handler = g_log_set_handler(NULL,G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,(GLogFunc) log_handler, widget);

 }


 LIB3270_EXPORT GtkWidget * v3270_trace_get_button_box(GtkWidget *widget)
 {
	g_return_val_if_fail(GTK_IS_V3270_TRACE(widget),NULL);

	return GTK_V3270_TRACE(widget)->buttons.box;

 }

 LIB3270_EXPORT void v3270_trace_button_box_insert(GtkWidget *widget, GtkWidget *button)
 {
	g_return_if_fail(GTK_IS_V3270_TRACE(widget));

	gtk_widget_set_can_focus(button,FALSE);
	gtk_widget_set_can_default(button,FALSE);

#if GTK_CHECK_VERSION(3,20,0)
	gtk_widget_set_focus_on_click(button,FALSE);
#endif // GTK 3,20,0

	gtk_box_pack_start(GTK_BOX(GTK_V3270_TRACE(widget)->buttons.box),button,FALSE,FALSE,0);

 }

 LIB3270_EXPORT	GtkWidget * v3270_trace_new(GtkWidget *terminal)
 {
	g_return_val_if_fail(GTK_IS_V3270(terminal),NULL);
	V3270Trace * widget = GTK_V3270_TRACE(g_object_new(GTK_TYPE_V3270_TRACE, NULL));

	// Set terminal widget
	{
		widget->terminal = terminal;
		g_object_ref_sink(G_OBJECT(terminal));

		set_session(widget, v3270_get_session(widget->terminal));

		GTK_V3270(terminal)->trace = GTK_WIDGET(widget);

		g_object_notify_by_pspec(
			G_OBJECT(terminal),
			GTK_V3270_GET_CLASS(terminal)->properties.trace
		);

	}

	{
		// Set header
	 	GtkTextBuffer * buffer = v3270_trace_get_text_buffer(GTK_WIDGET(widget));

	 	const char * text[] = {
			G_STRINGIFY(PRODUCT_NAME),
			" Revisions ",
			lib3270_get_build_rpq_timestamp(),
			" ",
			G_STRINGIFY(RPQ_TIMESTAMP),
			"\n\n"
	 	};

	 	size_t ix;
		GtkTextIter	itr;
		gtk_text_buffer_get_end_iter(buffer,&itr);

	 	for(ix = 0; ix < G_N_ELEMENTS(text); ix++)
		{
			gtk_text_buffer_insert(buffer,&itr,text[ix],-1);
		}

	}

	return GTK_WIDGET(widget);
 }

 const gchar * v3270_trace_get_filename(GtkWidget *widget)
 {
 	g_return_val_if_fail(GTK_IS_V3270_TRACE(widget),NULL);
  	return GTK_V3270_TRACE(widget)->filename;
 }

 LIB3270_EXPORT void v3270_trace_save(GtkWidget *widget)
 {
 	const gchar *filename = v3270_trace_get_filename(widget);
 	V3270Trace * trace = GTK_V3270_TRACE(widget);

	if(filename)
	{
		GError		* error = NULL;
		gchar		* text;
		GtkTextIter	  start;
		GtkTextIter	  end;

		gtk_text_buffer_get_start_iter(trace->text,&start);
		gtk_text_buffer_get_end_iter(trace->text,&end);
		text = gtk_text_buffer_get_text(trace->text,&start,&end,FALSE);

		g_file_set_contents(trace->filename,text,-1,&error);

		g_free(text);

		if(error)
		{
			v3270_popup_gerror(
					widget,
					error,
					NULL,
					_(  "Can't save %s" ), filename
			);

			g_error_free(error);

		}

	}

 }

 LIB3270_EXPORT void v3270_trace_select_file(GtkWidget *widget)
 {
	V3270Trace * trace = GTK_V3270_TRACE(widget);

	gchar * filename =
		v3270_select_file(
			GTK_WIDGET(trace),
			_("Save trace to file"),
			_("Save"),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			trace->filename
		);

	if(filename) {
		g_free(trace->filename);
		trace->filename = filename;
		v3270_trace_save(widget);
	}

 }

 H3270 * v3270_trace_get_session(GtkWidget *widget)
 {
 	return GTK_V3270_TRACE(widget)->hSession;
 }

 GtkWidget * v3270_trace_get_terminal(GtkWidget *widget)
 {
 	return GTK_V3270_TRACE(widget)->terminal;
 }

 GtkTextBuffer * v3270_trace_get_text_buffer(GtkWidget *widget)
 {
 	return GTK_V3270_TRACE(widget)->text;
 }

 GtkScrolledWindow * v3270_trace_get_scrolled_window(GtkWidget *widget)
 {
 	return GTK_V3270_TRACE(widget)->scroll;
 }



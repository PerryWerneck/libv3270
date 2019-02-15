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
 #include <internals.h>

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
 	GtkGrid				  parent;
 	H3270				* hSession;		///< @brief TN3270 Session.
 	GtkWidget			* terminal;		///< @brief V3270 Widget.
 	GtkScrolledWindow	* scroll;

 	GtkTextView			* view;			///< @brief Text view;
	GtkTextBuffer		* text;			///< @brief Trace window contents.
	GtkEntry			* entry;		///< @brief Command line entry.

 	gchar 				* filename;		///< @brief Selected file name.

	guint 				  log_handler;	///< @brief GTK Log Handler.

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
 	// Return if it's the same session.
 	if(widget->hSession == hSession)
		return;

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

 	V3270Trace *trace = GTK_V3270_TRACE(object);

 	if(trace->filename)
	{
		g_free(trace->filename);
		trace->filename = NULL;
	}

	if(trace->log_handler)
	{
		g_log_remove_handler(NULL,trace->log_handler);
		trace->log_handler = 0;
	}

	set_session(trace,NULL);
	g_clear_object(&trace->terminal);

	G_OBJECT_CLASS(V3270Trace_parent_class)->finalize(object);
 }

 static void V3270Trace_class_init(G_GNUC_UNUSED V3270TraceClass *klass)
 {
	G_OBJECT_CLASS(klass)->finalize = finalize;
 }

 static void v3270_trace_execute(GtkWidget *widget, const gchar *cmd)
 {
 	if(!*cmd)
		return;

	v3270_trace_printf(widget, "%s\n",cmd);

	V3270Trace *trace = GTK_V3270_TRACE(widget);

	if(trace->terminal)
	{
		int rc = v3270_exec_command(trace->terminal,cmd);
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
	gtk_grid_set_row_spacing(GTK_GRID(widget),6);
	gtk_grid_set_column_spacing(GTK_GRID(widget),12);

	// Create text view
	{
		widget->scroll = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL,NULL));
		gtk_scrolled_window_set_policy(widget->scroll,GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_widget_set_vexpand(GTK_WIDGET(widget->scroll),TRUE);
		gtk_widget_set_hexpand(GTK_WIDGET(widget->scroll),TRUE);
		gtk_grid_attach(GTK_GRID(widget),GTK_WIDGET(widget->scroll),0,0,10,1);

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
		gtk_widget_set_sensitive(GTK_WIDGET(widget->entry),FALSE);
		gtk_widget_set_vexpand(GTK_WIDGET(widget->entry),FALSE);
		gtk_widget_set_hexpand(GTK_WIDGET(widget->entry),TRUE);

		gtk_entry_set_icon_from_icon_name(widget->entry,GTK_ENTRY_ICON_SECONDARY,"system-run");
		gtk_entry_set_placeholder_text(widget->entry,_("Command to execute"));

		gtk_grid_attach(GTK_GRID(widget),GTK_WIDGET(widget->entry),0,1,10,1);

		g_signal_connect(G_OBJECT(widget->entry),"icon-press",G_CALLBACK(execute_command),widget);
		g_signal_connect(G_OBJECT(widget->entry),"activate",G_CALLBACK(entry_activated),widget);

	}

	// Grab GTK messages.
	widget->log_handler = g_log_set_handler(NULL,G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,(GLogFunc) log_handler, widget);

 }

 LIB3270_EXPORT	void v3270_trace_set_terminal(GtkWidget *widget, GtkWidget *terminal)
 {
	V3270Trace * trace = GTK_V3270_TRACE(widget);

	if(trace->terminal == terminal)
		return;

	g_clear_object(&trace->terminal);

	if(terminal)
	{
		trace->terminal = terminal;
		g_object_ref_sink(G_OBJECT(terminal));
	}

	set_session(trace, v3270_get_session(trace->terminal));

	gtk_widget_set_sensitive(GTK_WIDGET(trace->entry),trace->terminal != NULL);

 }

 LIB3270_EXPORT	GtkWidget * v3270_trace_new(GtkWidget *terminal)
 {
	g_return_val_if_fail(GTK_IS_V3270(terminal),NULL);

	V3270Trace * widget = GTK_V3270_TRACE(g_object_new(GTK_TYPE_V3270_TRACE, NULL));

	v3270_trace_set_terminal(GTK_WIDGET(widget),terminal);

	return GTK_WIDGET(widget);
 }

 struct _append_text
 {
 	V3270Trace *widget;
 	gchar text[1];
 };

 static gboolean bg_append_text(struct _append_text *cfg)
 {
 	if(!GTK_IS_TEXT_BUFFER(cfg->widget->text))
		return FALSE;

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

	// gtk_text_buffer_get_end_iter(cfg->widget->text,&itr);
	// gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(cfg->widget->view), &itr, 0.0, FALSE, 0.0, 0.0);

	GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment(cfg->widget->scroll);
	gtk_adjustment_set_value(vadj,gtk_adjustment_get_upper(vadj));
	gtk_scrolled_window_set_vadjustment(cfg->widget->scroll, vadj);

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

 static void menu_item_new(GtkWidget *menu, const gchar *label, GCallback callback, gpointer data)
 {
	GtkWidget *widget = gtk_menu_item_new_with_mnemonic(label);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),widget);
	g_signal_connect(G_OBJECT(widget), "activate", callback, data);
 }

 static void menu_save(G_GNUC_UNUSED GtkWidget *button, GtkWidget *trace)
 {
	v3270_trace_save(trace);
 }

 static void menu_save_as(G_GNUC_UNUSED GtkWidget *button, GtkWidget *trace)
 {
	v3270_trace_select_file(trace);
 }

 static void menu_close(G_GNUC_UNUSED GtkWidget *button, GtkWidget *window)
 {
	debug("%s",__FUNCTION__);
	gtk_widget_destroy(window);
 }

 LIB3270_EXPORT GtkWidget * v3270_trace_window_new(GtkWidget *widget, const gchar *header)
 {
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

 	GtkWidget 	* window	= gtk_window_new(GTK_WINDOW_TOPLEVEL);
 	GtkWidget 	* vbox		= gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
 	GtkWidget	* trace		= v3270_trace_new(widget);

 	// Set window title and default size
 	{
		const gchar 		* url 	= lib3270_get_url(v3270_get_session(widget));
		g_autofree gchar 	* title = NULL;

		if(url)
			title = g_strdup_printf("%s - %s - Trace", v3270_get_session_name(widget), url);
		else
			title = g_strdup_printf("%s - Trace", v3270_get_session_name(widget));

		gtk_window_set_title(GTK_WINDOW(window), title);
		gtk_window_set_default_size(GTK_WINDOW(window),590,430);
 	}

	// Top menu
	{
		GtkWidget * menubar = gtk_menu_bar_new();
		GtkWidget * topitem	= gtk_menu_item_new_with_mnemonic(_("_File"));
		GtkWidget * submenu	= gtk_menu_new();

		gtk_menu_item_set_submenu(GTK_MENU_ITEM(topitem), submenu);
		gtk_menu_shell_append(GTK_MENU_SHELL(menubar), topitem);

		menu_item_new(submenu,_("_Save"),G_CALLBACK(menu_save),trace);
		menu_item_new(submenu,_("Save _As"),G_CALLBACK(menu_save_as),trace);
		menu_item_new(submenu,_("_Close"),G_CALLBACK(menu_close),window);

		gtk_box_pack_start(GTK_BOX(vbox),menubar,FALSE,TRUE,0);
	}

	// Trace window
	gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(trace),TRUE,TRUE,0);

	gtk_container_add(GTK_CONTAINER(window),vbox);
	gtk_widget_show_all(window);

	if(header)
		v3270_trace_append_text(trace,header);

 	return window;
 }

 LIB3270_EXPORT void v3270_trace_save(GtkWidget *widget)
 {
 	V3270Trace * trace = GTK_V3270_TRACE(widget);

	if(trace && trace->filename)
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
			GtkWidget *popup =
				gtk_message_dialog_new_with_markup(
						GTK_WINDOW(gtk_widget_get_toplevel(widget)),
						GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_CLOSE,
						_( "Can't save %s" ),
						trace->filename
				);

			gtk_window_set_title(GTK_WINDOW(popup),_("Can't save file"));

			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",error->message);
			g_error_free(error);

			gtk_dialog_run(GTK_DIALOG(popup));
			gtk_widget_destroy(popup);

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
			trace->filename,
			N_("Text file"), "*.txt",
			NULL
		);

	if(filename) {
		g_free(trace->filename);
		trace->filename = filename;
		v3270_trace_save(widget);
	}

 }


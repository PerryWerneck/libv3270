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

/**
 * SECTION:V3270FTSettingsDialog
 * @Short_description: A dialog for define/edit file transfer.
 * @Title: V3270FTSettingsDialog
 *
 * The #V3270FTSettingsDialog is a complete dialog box for managing a V3270
 * file transfer queue.
 *
 */

 #include <internals.h>
 #include <v3270/filetransfer.h>
 #include "private.h"

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 struct _V3270FTSettingsDialog
 {
 	GtkDialog parent;

 	GtkWidget	* settings;

 	H3270		* hSession;
 	const void	* stHandle;

 	struct
 	{
 		GtkWidget * insert;
 		GtkWidget * update;
 		GtkWidget * remove;
 		GtkWidget * reset;
 		GtkWidget * begin;
 	} button;

 	struct
 	{
		GtkWidget * view;
		GtkWidget * load;
		GtkWidget * save;
		GtkWidget * saveAs;
 	} queue;

 };

 struct _V3270FTSettingsDialogClass
 {
	GtkDialogClass parent_class;
 };

 G_DEFINE_TYPE(V3270FTSettingsDialog, V3270FTSettingsDialog, GTK_TYPE_DIALOG);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void finalize(GObject *object)
{
	debug("%s",__FUNCTION__);

	V3270FTSettingsDialog * dialog = GTK_V3270_FT_DIALOG(object);

	if(dialog->hSession && dialog->stHandle)
	{
		lib3270_unregister_schange(dialog->hSession,LIB3270_STATE_CONNECT,dialog->stHandle);
		dialog->stHandle = NULL;
	}

	G_OBJECT_CLASS(V3270FTSettingsDialog_parent_class)->finalize(object);
}

static void V3270FTSettingsDialog_class_init(G_GNUC_UNUSED V3270FTSettingsDialogClass *klass)
{
	G_OBJECT_CLASS(klass)->finalize = finalize;
}

void activity_selected(GtkWidget G_GNUC_UNUSED(*view), GObject * activity, V3270FTSettingsDialog *widget)
{
	debug("%s(widget=%p activity=%p",__FUNCTION__,widget,activity);

	v3270_ft_settings_set_activity(widget->settings,activity);

	if(activity)
	{
		gtk_widget_set_sensitive(widget->button.update,TRUE);
		gtk_widget_set_sensitive(widget->button.remove,TRUE);
		gtk_widget_set_sensitive(widget->button.reset,TRUE);
	}
	else
	{
		gtk_widget_set_sensitive(widget->button.update,FALSE);
		gtk_widget_set_sensitive(widget->button.remove,FALSE);
		gtk_widget_set_sensitive(widget->button.reset,FALSE);
	}


}

static void validity_changed(GtkWidget G_GNUC_UNUSED(*settings), gboolean valid, V3270FTSettingsDialog *widget)
{
	debug("The file transfer settings are now %s",valid ? "valid" : "invalid");
	gtk_widget_set_sensitive(widget->button.insert,valid);
	gtk_widget_set_sensitive(widget->button.update,valid);
}

static void has_activity_changed(GtkWidget G_GNUC_UNUSED(*settings), gboolean have_activity, V3270FTSettingsDialog *widget)
{
	gtk_widget_set_sensitive(widget->button.reset,have_activity);
	gtk_widget_set_sensitive(widget->button.update,have_activity);
	gtk_widget_set_sensitive(widget->button.remove,have_activity);
}

static void reset_clicked(GtkButton G_GNUC_UNUSED(*button), V3270FTSettingsDialog *widget)
{
	v3270_ft_settings_reset(widget->settings);
}

static void update_clicked(GtkButton G_GNUC_UNUSED(*button), V3270FTSettingsDialog *widget)
{
	v3270_ft_settings_update(widget->settings);
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(widget->queue.view));
}

static void load_queue_clicked(GtkButton G_GNUC_UNUSED(*button), V3270FTSettingsDialog *widget)
{
	v3270_activity_list_load(widget->queue.view);
}

static void save_queue_clicked(GtkButton G_GNUC_UNUSED(*button), V3270FTSettingsDialog *widget)
{
	debug("%s",__FUNCTION__);
	v3270_activity_list_save(widget->queue.view);
}

static void save_queue_as_clicked(GtkButton G_GNUC_UNUSED(*button), V3270FTSettingsDialog *widget)
{
	v3270_activity_list_save_as(widget->queue.view);
}

static void begin_clicked(GtkButton G_GNUC_UNUSED(*button), V3270FTSettingsDialog *widget)
{
	gtk_dialog_response(GTK_DIALOG(widget),GTK_RESPONSE_ACCEPT);
}

static gboolean v3270_activity_compare_filenames(const GObject *a, const GObject *b)
{
 	if(strcmp(v3270_ft_activity_get_local_filename(a),v3270_ft_activity_get_local_filename(b)))
		return FALSE;

 	if(strcmp(v3270_ft_activity_get_remote_filename(a),v3270_ft_activity_get_remote_filename(b)))
		return FALSE;

	return TRUE;
}

int v3270_ft_settings_dialog_append_activity(GtkWidget *widget, GObject *activity, GError **error)
{
	V3270FTSettingsDialog *dialog = GTK_V3270_FT_DIALOG(widget);

	GtkTreeIter		  iter;
	GtkTreeModel	* model	= gtk_tree_view_get_model(GTK_TREE_VIEW(dialog->queue.view));

	if(gtk_tree_model_get_iter_first(model,&iter))
	{
		do
		{
			GObject * a = NULL;
			gtk_tree_model_get(model, &iter, 0, &a, -1);

			if(a && v3270_activity_compare_filenames(activity, a))
			{
				// Activity already exist
				if(error && !*error)
					*error = g_error_new_literal(g_quark_from_static_string(PACKAGE_NAME),EPERM,_("Activity already on the queue"));

				return -1;
			}

		}
		while(gtk_tree_model_iter_next(model,&iter));
	}

	// Not found, insert it.
	v3270_activity_list_append(dialog->queue.view,activity,FALSE);

	return 0;
}

static void insert_clicked(GtkWidget *button, V3270FTSettingsDialog *widget)
{
	GtkTreeIter		  iter;
	GtkTreeModel	* model	= gtk_tree_view_get_model(GTK_TREE_VIEW(widget->queue.view));

	if(gtk_tree_model_get_iter_first(model,&iter))
	{
		do
		{
			GObject * activity = NULL;
			gtk_tree_model_get(model, &iter, 0, &activity, -1);

			if(activity && v3270_ft_settings_equals(widget->settings,activity))
			{
				// Activity already exist
				GtkWidget * dialog =
					gtk_message_dialog_new(
						GTK_WINDOW(gtk_widget_get_toplevel(button)),
						GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_CLOSE,
						_("Activity already on the queue")
					);

				gtk_message_dialog_format_secondary_text(
					GTK_MESSAGE_DIALOG(dialog),
					_( "You can't add more than one acitivity with the same files.")
				);

				gtk_window_set_title(GTK_WINDOW(dialog),_("Can't add activity"));

				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);

				return;
			}

		}
		while(gtk_tree_model_iter_next(model,&iter));
	}

	// Not found, insert it.
	v3270_activity_list_append(widget->queue.view,v3270_ft_settings_create_activity(widget->settings),FALSE);

}

static void remove_clicked(GtkWidget G_GNUC_UNUSED(*button), V3270FTSettingsDialog *widget)
{
	v3270_activity_list_remove(widget->queue.view,v3270_ft_settings_get_activity(widget->settings));
	v3270_ft_settings_set_activity(widget->settings,NULL);
}

static void enable_queue_save(GtkWidget G_GNUC_UNUSED(*save), gboolean enabled, V3270FTSettingsDialog *widget)
{
	debug("%s: %s",__FUNCTION__,(enabled ? "Enable" : "Disable"));
	gtk_widget_set_sensitive(widget->queue.save,enabled);
}

static void V3270FTSettingsDialog_init(V3270FTSettingsDialog *widget)
{
	widget->settings = v3270_ft_settings_new();
	g_signal_connect(G_OBJECT(widget->settings),"validity",G_CALLBACK(validity_changed),widget);
	g_signal_connect(G_OBJECT(widget->settings),"has-activity",G_CALLBACK(has_activity_changed),widget);

#if GTK_CHECK_VERSION(3,12,0)
	// Does the dialog have header bar?
	GtkHeaderBar * header = v3270_dialog_get_header_bar(GTK_WIDGET(widget));

	if(header)
		gtk_header_bar_set_title(header,_( "3270 File transfer"));
	else
		gtk_window_set_title(GTK_WINDOW(widget),_( "3270 File transfer"));
#else
	gtk_window_set_title(GTK_WINDOW(widget),_( "3270 File transfer"));
#endif // GTK 3.12

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(widget))),18);

	// Create box
	GtkWidget * container = gtk_box_new(GTK_ORIENTATION_VERTICAL,6);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(widget))),container,TRUE,TRUE,0);

	// Pack settings widget
	gtk_widget_set_halign(GTK_WIDGET(widget->settings),GTK_ALIGN_START);
	gtk_widget_set_hexpand(GTK_WIDGET(widget->settings),FALSE);
	gtk_widget_set_vexpand(GTK_WIDGET(widget->settings),FALSE);
	gtk_box_pack_start(GTK_BOX(container),widget->settings,FALSE,FALSE,0);

	// Create action buttons
	{
		GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);

		g_object_set(G_OBJECT(box),"margin-top",6,NULL);
		g_object_set(G_OBJECT(box),"margin-end",6,NULL);

		widget->button.reset = v3270_box_pack_end(box,gtk_button_new_with_mnemonic("_Reset"),FALSE,FALSE,0);
		g_signal_connect(widget->button.reset,"clicked",G_CALLBACK(reset_clicked),widget);

		widget->button.update = v3270_box_pack_end(box,gtk_button_new_with_mnemonic("_Update"),FALSE,FALSE,0);
		g_signal_connect(widget->button.update,"clicked",G_CALLBACK(update_clicked),widget);

		widget->button.remove = v3270_box_pack_end(box,gtk_button_new_with_mnemonic("_Delete"),FALSE,FALSE,0);
		g_signal_connect(widget->button.remove,"clicked",G_CALLBACK(remove_clicked),widget);

		widget->button.insert = v3270_box_pack_end(box,gtk_button_new_with_mnemonic("_Insert"),FALSE,FALSE,0);
		g_signal_connect(widget->button.insert,"clicked",G_CALLBACK(insert_clicked),widget);

		gtk_widget_set_sensitive(widget->button.update,FALSE);
		gtk_widget_set_sensitive(widget->button.remove,FALSE);
		gtk_widget_set_sensitive(widget->button.insert,FALSE);
		gtk_widget_set_sensitive(widget->button.reset,FALSE);

		gtk_box_pack_start(GTK_BOX(container),box,FALSE,FALSE,0);

	}

	// Create Transfer queue buttons
	{
#if GTK_CHECK_VERSION(3,10,0)
		// https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
		widget->queue.load = gtk_button_new_from_icon_name("document-open",GTK_ICON_SIZE_SMALL_TOOLBAR);
		widget->queue.save = gtk_button_new_from_icon_name("document-save",GTK_ICON_SIZE_SMALL_TOOLBAR);
		widget->queue.saveAs = gtk_button_new_from_icon_name("document-save-as",GTK_ICON_SIZE_SMALL_TOOLBAR);
#else
		// https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
		widget->queue.load = gtk_button_new_from_stock(GTK_STOCK_OPEN);
		widget->queue.save = gtk_button_new_from_stock(GTK_STOCK_SAVE);
		widget->queue.saveAs = gtk_button_new_from_stock(GTK_STOCK_SAVE_AS);
#endif // GTK_CHECK_VERSION

		gtk_widget_set_tooltip_markup(widget->queue.load,_("Get transfer queue from file"));
		g_signal_connect(widget->queue.load,"clicked",G_CALLBACK(load_queue_clicked),widget);

		gtk_widget_set_tooltip_markup(widget->queue.save,_("Save transfer queue"));
		g_signal_connect(widget->queue.save,"clicked",G_CALLBACK(save_queue_clicked),widget);
		gtk_widget_set_sensitive(widget->queue.save,FALSE);

		gtk_widget_set_tooltip_markup(widget->queue.saveAs,_("Save transfer queue to file"));
		g_signal_connect(widget->queue.saveAs,"clicked",G_CALLBACK(save_queue_as_clicked),widget);

	}

	// Create file list view
	{
		widget->queue.view = v3270_activity_list_new();
		gtk_widget_set_tooltip_markup(widget->queue.view,_("Files to transfer"));
		g_signal_connect(G_OBJECT(widget->queue.view),"has-file",G_CALLBACK(enable_queue_save),widget);
		g_signal_connect(G_OBJECT(widget->queue.view),"changed",G_CALLBACK(activity_selected),widget);

		// Put the view inside a scrolled window.
		GtkWidget * scrolled = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(scrolled),widget->queue.view);
		gtk_widget_set_size_request(scrolled,50,150);

		gtk_widget_set_vexpand(scrolled,TRUE);
		gtk_widget_set_hexpand(scrolled,TRUE);

#if GTK_CHECK_VERSION(3,12,0)
		if(header)
		{
			debug("Dialog %s header bar","have");

			GtkWidget * frame = v3270_dialog_create_frame(scrolled,_("Transfer queue"));
			gtk_box_pack_start(GTK_BOX(container),frame,TRUE,TRUE,0);

			widget->button.begin = gtk_button_new_from_icon_name("network-transmit",GTK_ICON_SIZE_SMALL_TOOLBAR);

			g_object_set(G_OBJECT(widget->button.begin),"margin-end",12,NULL);
			g_signal_connect(widget->button.begin,"clicked",G_CALLBACK(begin_clicked),widget);

			gtk_header_bar_pack_start(header,widget->button.begin);
			gtk_header_bar_pack_start(header,widget->queue.load);
			gtk_header_bar_pack_start(header,widget->queue.save);
			gtk_header_bar_pack_start(header,widget->queue.saveAs);

		}
		else
		{
			debug("Dialog %s header bar","don't have");

			GtkWidget * hBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
			GtkWidget * frame = v3270_dialog_create_frame(hBox,_("Transfer queue"));
			gtk_box_pack_start(GTK_BOX(container),frame,TRUE,TRUE,0);

			GtkBox * box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL,6));
			gtk_box_pack_start(GTK_BOX(hBox),GTK_WIDGET(box),FALSE,FALSE,0);

			gtk_box_pack_start(box,widget->queue.load,FALSE,FALSE,0);
			gtk_box_pack_start(box,widget->queue.save,FALSE,FALSE,0);
			gtk_box_pack_start(box,widget->queue.saveAs,FALSE,FALSE,0);

			gtk_box_pack_start(GTK_BOX(hBox),GTK_WIDGET(scrolled),TRUE,TRUE,0);

			gtk_dialog_add_button(GTK_DIALOG (widget),_("_Cancel"),GTK_RESPONSE_CANCEL);
			widget->button.begin = gtk_dialog_add_button(GTK_DIALOG (widget),_("B_egin transfer"),GTK_RESPONSE_ACCEPT);

		}
#else

		GtkWidget * hBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
		GtkWidget * frame = v3270_dialog_create_frame(hBox,_("Transfer queue"));
		gtk_box_pack_start(GTK_BOX(container),frame,TRUE,TRUE,0);

		GtkBox * box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL,6));
		gtk_box_pack_start(GTK_BOX(hBox),GTK_WIDGET(box),FALSE,FALSE,0);

		gtk_box_pack_start(box,widget->queue.load,FALSE,FALSE,0);
		gtk_box_pack_start(box,widget->queue.save,FALSE,FALSE,0);
		gtk_box_pack_start(box,widget->queue.saveAs,FALSE,FALSE,0);

		gtk_box_pack_start(GTK_BOX(hBox),GTK_WIDGET(scrolled),TRUE,TRUE,0);

		gtk_dialog_add_button(GTK_DIALOG (widget),_("_Cancel"),GTK_RESPONSE_CANCEL);
		widget->button.begin = gtk_dialog_add_button(GTK_DIALOG (widget),_("B_egin transfer"),GTK_RESPONSE_ACCEPT);

#endif // GTK_CHECK_VERSION(3,12,0)

		// gtk_widget_set_sensitive(widget->button.begin,FALSE);
		gtk_widget_set_tooltip_markup(widget->button.begin,_("Start transfer"));
		gtk_widget_set_sensitive(widget->button.begin,FALSE);

/*
#ifdef DEBUG
		GObject * activity = v3270_ft_activity_new();

		v3270_ft_activity_set_local_filename(activity,"local---");
		v3270_ft_activity_set_remote_filename(activity,"remote---");
		v3270_ft_activity_set_options(activity,LIB3270_FT_OPTION_SEND|LIB3270_FT_OPTION_ASCII|LIB3270_FT_OPTION_CRLF|LIB3270_FT_OPTION_REMAP|LIB3270_FT_OPTION_APPEND|LIB3270_FT_RECORD_FORMAT_VARIABLE);

		v3270_activity_list_append(widget->queue.view,activity,FALSE);
#endif // DEBUG
*/

	}


}

LIB3270_EXPORT GtkWidget * v3270_ft_settings_dialog_new(GtkWidget *parent)
{
	gboolean use_header;
	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &use_header, NULL);

	GtkWidget * dialog =
		GTK_WIDGET(g_object_new(
			GTK_TYPE_V3270_FT_DIALOG,
			"use-header-bar", (use_header ? 1 : 0),
			NULL
		));

	if(parent)
	{
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gtk_widget_get_toplevel(parent)));
		gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
		gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
	}

	if(GTK_IS_V3270(parent))
		v3270_ft_settings_dialog_set_session(dialog,v3270_get_session(parent));

	return dialog;
}

static void connect_changed(H3270 G_GNUC_UNUSED(*hSession), int state, void *widget)
{
	V3270FTSettingsDialog * dialog = GTK_V3270_FT_DIALOG( ((GtkWidget *) widget) );
	gtk_widget_set_sensitive(dialog->button.begin,state ? TRUE : FALSE);
}

LIB3270_EXPORT void v3270_ft_settings_dialog_set_session(GtkWidget *widget, H3270 *hSession)
{
	V3270FTSettingsDialog * dialog = GTK_V3270_FT_DIALOG(widget);

	if(dialog->hSession && dialog->stHandle)
	{
		lib3270_unregister_schange(dialog->hSession,LIB3270_STATE_CONNECT,dialog->stHandle);
		dialog->stHandle = NULL;
	}

	dialog->hSession = hSession;
	dialog->stHandle = lib3270_register_schange(hSession,LIB3270_STATE_CONNECT,connect_changed,(void *) widget);

	gtk_widget_set_sensitive(dialog->button.begin,lib3270_is_connected(hSession) ? TRUE : FALSE);

	v3270_ft_settings_set_tso(dialog->settings,lib3270_is_tso(hSession));

}

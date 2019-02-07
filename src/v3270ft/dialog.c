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

 #include <internals.h>
 #include <v3270/filetransfer.h>
 #include "private.h"

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 struct _V3270FTDialog
 {
 	GtkDialog parent;

 	GtkWidget * settings;
 	// GtkWidget * buttons[FT_BUTTON_COUNT];

 };

 struct _V3270FTDialogClass
 {
	GtkDialogClass parent_class;
 };

 G_DEFINE_TYPE(V3270FTDialog, V3270FTDialog, GTK_TYPE_DIALOG);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void V3270FTDialog_class_init(G_GNUC_UNUSED V3270FTDialogClass *klass)
{
}

/*
static void apply_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *dialog)
{
	gtk_dialog_response(GTK_DIALOG(dialog),GTK_RESPONSE_APPLY);
}

static void cancel_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *dialog)
{
	gtk_dialog_response(GTK_DIALOG(dialog),GTK_RESPONSE_CANCEL);
}
*/

/*
static GtkWidget * create_button(V3270FTDialog *widget, FT_BUTTON id, const gchar *icon, const gchar *tooltip, GCallback callback)
{
	widget->buttons[id] = gtk_button_new_from_icon_name(icon,GTK_ICON_SIZE_BUTTON);
	gtk_widget_set_tooltip_markup(widget->buttons[id],tooltip);

	// g_signal_connect(widget->buttons[id],"clicked",callback,widget);

	return widget->buttons[id];
}
*/

static void V3270FTDialog_init(V3270FTDialog *widget)
{
	widget->settings = v3270_ft_settings_new();

	gtk_window_set_title(GTK_WINDOW(widget),_( "3270 File transfer"));

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

	// Create file list view
	{
		GtkWidget * files = v3270_activity_list_new();
		gtk_widget_set_tooltip_markup(files,_("Files to transfer"));

		// Put the view inside a scrolled window.
		GtkWidget * scrolled = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(scrolled),files);

		gtk_widget_set_vexpand(scrolled,TRUE);
		gtk_widget_set_hexpand(scrolled,TRUE);

		GtkWidget * frame = v3270_dialog_create_frame(scrolled,_("Transfer queue"));

		gtk_box_pack_start(GTK_BOX(container),frame,TRUE,TRUE,0);

#ifdef DEBUG
		GObject * activity = v3270_ft_activity_new();

		v3270_ft_activity_set_local_filename(activity,"local---");
		v3270_ft_activity_set_remote_filename(activity,"remote---");

		v3270_activity_list_append(files,activity);
		v3270_ft_settings_set_activity(widget->settings,activity);
#endif // DEBUG

	}


}

LIB3270_EXPORT GtkWidget * v3270_ft_dialog_new(GtkWidget *parent)
{
	gboolean use_header;
	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &use_header, NULL);

	GtkWidget * dialog =
		GTK_WIDGET(g_object_new(
			GTK_TYPE_V3270_FT_DIALOG,
			"use-header-bar", 0, // (use_header ? 1 : 0),
			NULL
		));

	if(parent)
	{
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gtk_widget_get_toplevel(parent)));
		gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
		gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
	}

	return dialog;
}


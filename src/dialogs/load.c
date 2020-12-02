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
 #include <internals.h>
 #include <lib3270.h>
 #include <lib3270/selection.h>
 #include <clipboard.h>
 #include <limits.h>
 #include <v3270/dialogs.h>

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 typedef struct _V3270LoadDialog
 {
	GtkDialog parent;

	GtkWidget				* terminal;
	GtkWidget				* filename;
	GtkWidget				* charset;

 } V3270LoadDialog;

 typedef struct _V3270LoadDialogClass
 {
	GtkDialogClass parent_class;
	int dummy;

 } V3270LoadDialogClass;

 #define GTK_TYPE_V3270LoadDialog			(V3270LoadDialog_get_type ())
 #define V3270_LOAD_DIALOG(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270LoadDialog, V3270LoadDialog))
 #define V3270LoadDialog_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270Loadialog, V3270LoadDialogClass))
 #define IS_V3270LoadDialog(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270LoadDialog))
 #define IS_V3270LoadDialog_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270LoadDialog))
 #define V3270LoadDialog_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270LoadDialog, V3270LoadDialogClass))

 G_DEFINE_TYPE(V3270LoadDialog, V3270LoadDialog, GTK_TYPE_DIALOG);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void V3270LoadDialog_class_init(V3270LoadDialogClass G_GNUC_UNUSED(*klass))
 {

	debug("%s",__FUNCTION__);

 }

 static void cancel_operation(GtkButton G_GNUC_UNUSED(*button), GtkDialog *dialog)
 {
	gtk_dialog_response(dialog,GTK_RESPONSE_CANCEL);
 }

 static void apply_operation(GtkButton G_GNUC_UNUSED(*button), GtkDialog *dialog)
 {
	gtk_dialog_response(dialog,GTK_RESPONSE_APPLY);
 }

#ifdef WIN32
static void icon_press(GtkEntry G_GNUC_UNUSED(*entry), G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, V3270LoadDialog *widget)
{
	g_autofree gchar *filename =
						v3270_select_file(
								GTK_WIDGET(widget),
								_( "Select file"),
								_("Open"),
								GTK_FILE_CHOOSER_ACTION_OPEN,
								gtk_entry_get_text(GTK_ENTRY(widget->filename))
						);

	if(filename && *filename) {
		gtk_entry_set_text(GTK_ENTRY(widget->filename),filename);
	}

}
#else
static void icon_press(GtkEntry *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, V3270LoadDialog *widget)
{
	GtkWidget * dialog =
		gtk_file_chooser_dialog_new(
				_( "Select file"),
				GTK_WINDOW(widget),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				_("Cancel"),	GTK_RESPONSE_CANCEL,
				_("Open"),		GTK_RESPONSE_ACCEPT,
				NULL );


	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);
	g_signal_connect(G_OBJECT(dialog),"close",G_CALLBACK(v3270_dialog_close),NULL);

	const gchar *filename = gtk_entry_get_text(entry);

	if(filename && *filename)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),filename);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		gtk_entry_set_text(entry,gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));

	gtk_widget_destroy(dialog);

 }
#endif // _WIN32

 static void V3270LoadDialog_init(V3270LoadDialog *dialog)
 {
 	//     0--------1---------------------2-------3--------------------4
 	// 0 - Filename xxxxxxxxx.xxxxxxxxx.xxxxxxxxx.xxxxxxxxx.xxxxxxxxx. x
 	// 1 - Charset  xxxxxxxxx.xxxxxxxxx.  Format: xxxxxxxxx.xxxxxxxxx.


//	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

	// Setup visual elements
	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	GtkWidget *widget;
	GtkWidget *button;

	GtkBox * box = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
	gtk_window_set_resizable(GTK_WINDOW(dialog),FALSE);

#ifdef G_OS_UNIX
	gtk_container_set_border_width(GTK_CONTAINER(box),18);
#endif // UNIX

	GtkGrid * grid = GTK_GRID(gtk_grid_new());
	gtk_grid_set_row_spacing(GTK_GRID(grid),6);
 	gtk_grid_set_column_spacing(GTK_GRID(grid),12);
	gtk_box_pack_start(box,GTK_WIDGET(grid),TRUE,TRUE,2);

	// Filename entry
	{
		dialog->filename = gtk_entry_new();
		gtk_widget_set_hexpand(GTK_WIDGET(dialog->filename),TRUE);

		widget = gtk_label_new_with_mnemonic(_("_Filename"));
		gtk_widget_set_halign(widget,GTK_ALIGN_END);
		gtk_widget_set_valign(widget,GTK_ALIGN_CENTER);
		gtk_grid_attach(grid,widget,0,0,1,1);
		gtk_label_set_mnemonic_widget(GTK_LABEL(widget),dialog->filename);

		gtk_entry_set_icon_from_icon_name(GTK_ENTRY(dialog->filename),GTK_ENTRY_ICON_SECONDARY,"document-open");
		gtk_entry_set_icon_activatable(GTK_ENTRY(dialog->filename),GTK_ENTRY_ICON_SECONDARY,TRUE);
		gtk_entry_set_icon_tooltip_text(GTK_ENTRY(dialog->filename),GTK_ENTRY_ICON_SECONDARY,_("Select file"));
		g_signal_connect(G_OBJECT(dialog->filename),"icon-press",G_CALLBACK(icon_press),dialog);

		gtk_entry_set_width_chars(GTK_ENTRY(dialog->filename),60);
		gtk_entry_set_max_length(GTK_ENTRY(dialog->filename),PATH_MAX);
		gtk_grid_attach(grid,GTK_WIDGET(dialog->filename),1,0,3,1);
	}

	// Charset drop-down
	{
		widget = gtk_label_new_with_mnemonic (_("C_haracter Coding"));
		gtk_widget_set_halign(widget,GTK_ALIGN_END);
		gtk_widget_set_valign(widget,GTK_ALIGN_CENTER);
		gtk_grid_attach(grid,widget,0,1,1,1);

		dialog->charset = v3270_charset_combo_box_new();
		gtk_grid_attach(grid,dialog->charset,1,1,1,1);

	}


	// Buttons
	// https://developer.gnome.org/icon-naming-spec/
#ifdef _WIN32
	widget = NULL;
#elif GTK_CHECK_VERSION(3,14,0)
	widget = gtk_dialog_get_header_bar(GTK_DIALOG(dialog));
#else
	widget = NULL;
#endif // GTK(3,14,0)

	if(widget)
	{
		// Have header bar
		button = gtk_button_new_with_mnemonic(_("_Cancel"));
		gtk_widget_set_tooltip_markup(button,_("Click to cancel operation"));
		gtk_header_bar_pack_start(GTK_HEADER_BAR(widget),button);
		g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(cancel_operation),dialog);

		button = gtk_button_new_with_mnemonic(_("_Load"));
		gtk_widget_set_tooltip_markup(button,_("Click to load file"));
		gtk_header_bar_pack_end(GTK_HEADER_BAR(widget),button);
		g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(apply_operation),dialog);
	}
	else
	{
		gtk_box_set_spacing(
			GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
			18
		);

		gtk_dialog_add_buttons(
			GTK_DIALOG (dialog),
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_Load"), GTK_RESPONSE_APPLY,
			NULL
		);
	}

 }

 GtkWidget * v3270_load_dialog_new(GtkWidget *widget, const gchar *filename)
 {
 	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	// Create dialog
	V3270LoadDialog * dialog = V3270_LOAD_DIALOG(
									g_object_new(
										GTK_TYPE_V3270LoadDialog,
										"use-header-bar", (v3270_dialog_get_use_header() ? 1 : 0),
										NULL)
									);

	dialog->terminal	= widget;

	gtk_window_set_title(GTK_WINDOW(dialog),_("Paste from file"));
	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

	if(filename && *filename)
		gtk_entry_set_text(GTK_ENTRY(dialog->filename),filename);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gtk_widget_get_toplevel(widget)));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

	return GTK_WIDGET(dialog);
 }

 void v3270_load_dialog_apply(GtkWidget *widget, GError **error)
 {
 	V3270LoadDialog * dialog = V3270_LOAD_DIALOG(widget);

 	debug("%s error=%p",__FUNCTION__,*error);

 	if(!v3270_is_connected(dialog->terminal))
	{
		*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),ENOTCONN,"%s",strerror(ENOTCONN));
		return;
	}

	// Load file
	g_autofree gchar * contents = NULL;
	g_file_get_contents(
		gtk_entry_get_text(GTK_ENTRY(dialog->filename)),
        &contents,
		NULL,
		error
	);

	// Got contents, check for charset conversion.
	g_autofree gchar * converted =
						v3270_convert_to_3270_charset(
								dialog->terminal,
								contents,
								gtk_combo_box_get_active_id(GTK_COMBO_BOX(dialog->charset)),
								error
						);

	if(*error)
		return;

	int remains = lib3270_paste_text(
						v3270_get_session(dialog->terminal),
						(unsigned char *) converted
					) ? TRUE : FALSE;

	debug("next=%s",remains > 0 ? "YES" : "NO");

	v3270_signal_emit(
		dialog->terminal,
		V3270_SIGNAL_PASTENEXT,
		remains > 0
	);

 }

 void v3270_load_dialog_run(GtkWidget *widget)
 {
 	debug("%s",__FUNCTION__);
	if(gtk_dialog_run(GTK_DIALOG(widget)) == GTK_RESPONSE_APPLY)
	{
		GError * error 	= NULL;
		v3270_load_dialog_apply(widget,&error);

		// The operation has failed? If yes notify user and free error object.
		v3270_popup_gerror(
			widget,
			&error,
			NULL,
			_("Can't open %s"),gtk_entry_get_text(GTK_ENTRY(V3270_LOAD_DIALOG(widget)->filename))
		);

	}
 	debug("%s",__FUNCTION__);

 }



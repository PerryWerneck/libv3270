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
 #include <limits.h>

/*--[ GTK Requires ]---------------------------------------------------------------------------------*/

 G_DEFINE_TYPE(V3270SaveDialog, V3270SaveDialog, GTK_TYPE_DIALOG);

/*--[ Implement ]------------------------------------------------------------------------------------*/

/*
 static void V3270SaveDialog_finalize(V3270SaveDialog *object)
 {
	V3270SaveDialog *dialog = V3270_SAVE_DIALOG(object);
 }
*/

 static void V3270SaveDialog_class_init(V3270SaveDialogClass *klass)
 {

	debug("%s",__FUNCTION__);
//	G_OBJECT_CLASS(klass)->finalize = V3270SaveDialog_finalize;

 }

 static void cancel_operation(GtkButton G_GNUC_UNUSED(*button), GtkDialog *dialog)
 {
	gtk_dialog_response(dialog,GTK_RESPONSE_CANCEL);
 }

 static void apply_operation(GtkButton G_GNUC_UNUSED(*button), GtkDialog *dialog)
 {
	gtk_dialog_response(dialog,GTK_RESPONSE_APPLY);
 }

 static void V3270SaveDialog_init(V3270SaveDialog *dialog)
 {
	dialog->mode = LIB3270_CONTENT_ALL;

	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

	// Setup visual elements
	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	GtkWidget *widget;
	GtkWidget *button;

	GtkBox * box = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
	gtk_window_set_resizable(GTK_WINDOW(dialog),FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(box),18);

	GtkGrid * grid = GTK_GRID(gtk_grid_new());
	gtk_grid_set_row_spacing(GTK_GRID(grid),6);
 	gtk_grid_set_column_spacing(GTK_GRID(grid),12);
	gtk_box_pack_start(box,GTK_WIDGET(grid),TRUE,TRUE,2);

	dialog->filename = gtk_entry_new();
	gtk_widget_set_hexpand(GTK_WIDGET(dialog->filename),TRUE);

	widget = gtk_label_new_with_mnemonic("_Filename:");
	gtk_widget_set_halign(widget,GTK_ALIGN_END);
	gtk_widget_set_valign(widget,GTK_ALIGN_CENTER);
	gtk_grid_attach(grid,GTK_WIDGET(widget),0,0,1,1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(widget),dialog->filename);

#ifdef WIN32
	widget = gtk_button_new_from_icon_name("document-open",GTK_ICON_SIZE_BUTTON);
	//g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(select_local_file),dialog);
	gtk_grid_attach(grid,widget,2,0,1,1);
#else
	gtk_entry_set_icon_from_icon_name(GTK_ENTRY(dialog->filename),GTK_ENTRY_ICON_SECONDARY,"document-open");
	gtk_entry_set_icon_activatable(GTK_ENTRY(dialog->filename),GTK_ENTRY_ICON_SECONDARY,TRUE);
	gtk_entry_set_icon_tooltip_text(GTK_ENTRY(dialog->filename),GTK_ENTRY_ICON_SECONDARY,_("Select file"));
	// g_signal_connect(G_OBJECT(dialog->filename),"icon-press",G_CALLBACK(icon_press),dialog);
#endif // WIN32

	gtk_entry_set_width_chars(GTK_ENTRY(dialog->filename),60);
	gtk_entry_set_max_length(GTK_ENTRY(dialog->filename),PATH_MAX);
	gtk_grid_attach(grid,GTK_WIDGET(dialog->filename),1,0,1,1);

	// Buttons
	// https://developer.gnome.org/icon-naming-spec/
#if GTK_CHECK_VERSION(3,14,0)

	widget = gtk_dialog_get_header_bar(GTK_DIALOG(dialog));

	button = gtk_button_new_with_mnemonic(_("_Cancel"));
	gtk_widget_set_tooltip_markup(button,_("Click to cancel operation"));
	gtk_header_bar_pack_start(GTK_HEADER_BAR(widget),button);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(cancel_operation),dialog);

	button = gtk_button_new_with_mnemonic(_("_Save"));
	gtk_widget_set_tooltip_markup(button,_("Click to save file"));
	gtk_header_bar_pack_end(GTK_HEADER_BAR(widget),button);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(apply_operation),dialog);

#else

	#error TODO!

#endif // GTK(3,14,0)

 }

 GtkWidget * v3270_save_dialog_new(GtkWidget *widget, LIB3270_CONTENT_OPTION mode, const gchar *filename)
 {
	V3270SaveDialog * dialog = V3270_SAVE_DIALOG(g_object_new(GTK_TYPE_V3270SaveDialog,"use-header-bar", (gint) 1, NULL));
	dialog->mode = mode;

	if(filename)
		gtk_entry_set_text(GTK_ENTRY(dialog->filename),filename);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gtk_widget_get_toplevel(widget)));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

	return GTK_WIDGET(dialog);
 }

 void v3270_save_dialog_run(GtkWidget *widget)
 {
	gtk_dialog_run(GTK_DIALOG(widget));
 }



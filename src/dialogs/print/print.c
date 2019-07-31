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
 #include <sys/param.h>
 #include <lib3270/selection.h>
 #include <clipboard.h>

 G_DEFINE_TYPE(V3270PrintOperation, V3270PrintOperation, GTK_TYPE_PRINT_OPERATION);

/*--[ Implement ]------------------------------------------------------------------------------------*/


 static void done(GtkPrintOperation *prt, GtkPrintOperationResult result)
 {
 	V3270PrintOperation * operation = GTK_V3270_PRINT_OPERATION(prt);

	debug("%s",__FUNCTION__);

	if(result == GTK_PRINT_OPERATION_RESULT_ERROR)
	{
		GError * err		= NULL;

		gtk_print_operation_get_error(prt,&err);

		GtkWidget *dialog = gtk_message_dialog_new_with_markup(
										GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(operation->widget))),
										GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
										"%s",_( "Print operation failed" )
								);

		g_warning("%s",err->message);

		gtk_window_set_title(GTK_WINDOW(dialog),_("Error"));

		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),"%s",err->message);

		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		g_error_free(err);

	}

	if(operation->widget)
		g_signal_emit(GTK_WIDGET(operation->widget), v3270_widget_signal[V3270_SIGNAL_PRINT_DONE], 0, prt, (guint) result);

 }

 static void dispose(GObject *object)
 {
	debug("%s",__FUNCTION__);

	V3270PrintOperation * operation = GTK_V3270_PRINT_OPERATION(object);

	if(operation->font.info.scaled)
		cairo_scaled_font_destroy(operation->font.info.scaled);

	if(operation->font.name)
		g_free(operation->font.name);

	if(operation->contents.dynamic)
	{
		g_list_free_full(operation->contents.dynamic,(GDestroyNotify) lib3270_free);
		operation->contents.dynamic = NULL;
	}
	operation->contents.selection = NULL;

	G_OBJECT_CLASS(V3270PrintOperation_parent_class)->dispose(object);


 }

#ifndef _WIN32
 static GtkWidget * custom_widget_new(GtkPrintOperation *prt)
 {
 	GtkWidget * widget		= gtk_frame_new("");
 	GtkWidget * settings	= V3270_print_settings_new(GTK_WIDGET(GTK_V3270_PRINT_OPERATION(prt)->widget));

	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label),_("<b>Text options</b>"));
	gtk_frame_set_label_widget(GTK_FRAME(widget),label);

 	gtk_container_set_border_width(GTK_CONTAINER(widget),12);

	// The print dialog doesn't follow the guidelines from https://developer.gnome.org/hig/stable/visual-layout.html.en )-:
	gtk_frame_set_shadow_type(GTK_FRAME(widget),GTK_SHADOW_NONE);

 	gtk_container_set_border_width(GTK_CONTAINER(settings),6);
 	g_object_set(G_OBJECT(settings),"margin-start",8,NULL);

	gtk_container_add(GTK_CONTAINER(widget),settings);

	gtk_widget_show_all(widget);

    return widget;
 }

 static void custom_widget_apply(GtkPrintOperation *prt, GtkWidget *widget)
 {
	V3270PrintOperation	* operation	= GTK_V3270_PRINT_OPERATION(prt);
 	GtkWidget			* settings	= gtk_bin_get_child(GTK_BIN(widget));

	// Setup options.
	operation->show_selection	= v3270_print_settings_get_show_selection(settings);
	operation->font.name		= g_strdup(v3270_font_selection_get_family(settings));

	v3270_print_settings_get_rgba(settings, operation->settings.colors, V3270_COLOR_COUNT);

 }
#endif // _WIN32

 static void V3270PrintOperation_class_init(V3270PrintOperationClass *klass)
 {
	GtkPrintOperationClass	* operation	= GTK_PRINT_OPERATION_CLASS(klass);

	G_OBJECT_CLASS(klass)->dispose = dispose;
	operation->done 		= done;
	operation->begin_print	= V3270PrintOperation_begin_print;
	operation->draw_page	= V3270PrintOperation_draw_page;

#ifndef _WIN32
	operation->create_custom_widget = custom_widget_new;
	operation->custom_widget_apply  = custom_widget_apply;
#endif // _WIN32

 }

 static void V3270PrintOperation_init(V3270PrintOperation *widget)
 {
	// Setup print operation.
	gtk_print_operation_set_custom_tab_label(GTK_PRINT_OPERATION(widget), _( "Options" ) );
	gtk_print_operation_set_show_progress(GTK_PRINT_OPERATION(widget),TRUE);
	gtk_print_operation_set_print_settings(GTK_PRINT_OPERATION(widget),gtk_print_settings_new());
	gtk_print_operation_set_default_page_setup(GTK_PRINT_OPERATION(widget),gtk_page_setup_new());

 	// Setup defaults
    widget->mode 			= LIB3270_CONTENT_ALL;
//    widget->show_selection	= FALSE;
//    widget->font.name		= NULL; // g_strdup(v3270_default_font);

 }

V3270PrintOperation	* v3270_print_operation_new(GtkWidget *widget, LIB3270_CONTENT_OPTION mode)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	V3270PrintOperation	* operation	= GTK_V3270_PRINT_OPERATION(g_object_new(GTK_TYPE_V3270_PRINT_OPERATION, NULL));

	operation->mode			= mode;
	operation->widget		= GTK_V3270(widget);
	operation->session		= v3270_get_session(widget);

	v3270_set_mono_color_table(operation->settings.colors,"#000000","#FFFFFF");

	// Get contents.
	switch(operation->mode)
	{
	case LIB3270_CONTENT_ALL:
		debug("%s","LIB3270_CONTENT_ALL");
		operation->contents.dynamic = g_new0(GList,1);
		operation->contents.dynamic->data = (gpointer) lib3270_get_selection(operation->session,0,1);
		operation->contents.selection = operation->contents.dynamic;
		break;

	case LIB3270_CONTENT_COPY:
		debug("%s","LIB3270_CONTENT_COPY");
		operation->contents.selection = v3270_get_selection_blocks(GTK_WIDGET(operation->widget));
		break;

	case LIB3270_CONTENT_SELECTED:
		debug("%s","LIB3270_CONTENT_SELECTED");
		operation->contents.dynamic = g_new0(GList,1);
		operation->contents.dynamic->data = (gpointer) lib3270_get_selection(operation->session,0,0);
		operation->contents.selection = operation->contents.dynamic;
		break;
	}

	// Get metrics
	const GList * element;
	for(element = operation->contents.selection; element; element = element->next)
	{
		const lib3270_selection * selection = (const lib3270_selection *) element->data;

		if(!selection)
			break;

		if(selection->bounds.width > operation->contents.width)
			operation->contents.width = selection->bounds.width;

		operation->contents.height += selection->bounds.height;
	}

	return operation;
}


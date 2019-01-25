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

 #pragma GCC diagnostic ignored "-Wdeprecated-declarations"

 #include "private.h"
 #include <v3270.h>
 #include <v3270/colorscheme.h>
 #include <v3270/print.h>
 #include <lib3270/selection.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 struct _V3270PrintOperationClass
 {
 	GtkPrintOperationClass parent_class;

 };

 struct _V3270PrintOperation
 {
 	GtkPrintOperation	  parent;
	GdkRGBA				  color[V3270_COLOR_COUNT];
    LIB3270_PRINT_MODE	  mode;
    v3270				* widget;
    H3270				* session;

    struct {
		gchar			* name;
		v3270FontInfo	  info;
    } font;

    gboolean			  show_selection;


 };

 G_DEFINE_TYPE(V3270PrintOperation, V3270PrintOperation, GTK_TYPE_PRINT_OPERATION);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void done(GtkPrintOperation *prt, GtkPrintOperationResult result)
 {
 	V3270PrintOperation * operation = GTK_V3270_PRINT_OPERATION(prt);

	debug("%s",__FUNCTION__);

	if(operation->widget)
		g_signal_emit(GTK_WIDGET(operation->widget), v3270_widget_signal[SIGNAL_PRINT_DONE], 0, prt, (guint) result);

 }

#ifndef _WIN32
 static GtkWidget * create_custom_widget(GtkPrintOperation *prt)
 {
	static const gchar * text[] =
	{
			N_( "_Font:" ),
			N_( "C_olor scheme:" )
	};

	size_t 		  f;

 	V3270PrintOperation	* operation = GTK_V3270_PRINT_OPERATION(prt);

	if(operation->widget)
		g_signal_emit(operation->widget, v3270_widget_signal[SIGNAL_PRINT_SETUP], 0, prt);

	// Create dialog

 	GtkGrid 			* grid = GTK_GRID(gtk_grid_new());
 	GtkWidget			* font = v3270_font_selection_new(operation->font.name);
 	GtkWidget			* color = v3270_color_scheme_new();

 	gtk_container_set_border_width(GTK_CONTAINER(grid),10);
 	gtk_grid_set_row_spacing(grid,5);
 	gtk_grid_set_column_spacing(grid,5);

 	v3270_color_scheme_set_rgba(color,operation->color);

	for(f=0;f<G_N_ELEMENTS(text);f++)
	{
		GtkWidget *label = gtk_label_new_with_mnemonic(gettext(text[f]));
		gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
		gtk_grid_attach(grid,label,0,f,1,1);
	}

	gtk_grid_attach(grid,font,1,0,1,1);
	gtk_grid_attach(grid,color,1,1,1,1);

	gtk_widget_show_all(GTK_WIDGET(grid));
	return GTK_WIDGET(grid);
 }

 static void custom_widget_apply(GtkPrintOperation *prt, GtkWidget *widget)
 {
 	V3270PrintOperation	* operation = GTK_V3270_PRINT_OPERATION(prt);

	debug("%s",__FUNCTION__);

	if(operation->widget)
		g_signal_emit(operation->widget, v3270_widget_signal[SIGNAL_PRINT_APPLY], 0, prt);

 }
#endif // _WIN32

 static void dispose(GObject *object)
 {
	debug("%s",__FUNCTION__);

	V3270PrintOperation * operation = GTK_V3270_PRINT_OPERATION(object);

	g_free(operation->font.name);

 }

 static void V3270PrintOperation_class_init(V3270PrintOperationClass *klass)
 {
	GtkPrintOperationClass	* operation	= GTK_PRINT_OPERATION_CLASS(klass);

	G_OBJECT_CLASS(klass)->dispose = dispose;
	operation->done = done;

#ifndef _WIN32
	operation->create_custom_widget = create_custom_widget;
	operation->custom_widget_apply  = custom_widget_apply;
#endif // _WIN32
 /*
       // Common signals
    g_signal_connect(print,"done",G_CALLBACK(done),*info);

#if GTK_CHECK_VERSION(3,0,0) && !defined(WIN32)
        g_signal_connect(print,"create-custom-widget",G_CALLBACK(create_custom_widget), *info);
        g_signal_connect(print,"custom-widget-apply",G_CALLBACK(custom_widget_apply), *info);
#endif // !WIN32
*/

 }

 static void V3270PrintOperation_init(V3270PrintOperation *widget)
 {
	// Setup print operation.
	gtk_print_operation_set_custom_tab_label(GTK_PRINT_OPERATION(widget), _( "Options" ) );
	gtk_print_operation_set_show_progress(GTK_PRINT_OPERATION(widget),TRUE);
	gtk_print_operation_set_print_settings(GTK_PRINT_OPERATION(widget),gtk_print_settings_new());
	gtk_print_operation_set_default_page_setup(GTK_PRINT_OPERATION(widget),gtk_page_setup_new());


 	// Setup defaults
    widget->mode 			= LIB3270_PRINT_ALL;
    widget->show_selection	= FALSE;
    widget->font.name		= g_strdup(v3270_default_font);

	v3270_set_mono_color_table(widget->color,"#000000","#FFFFFF");

 }

V3270PrintOperation	* v3270_print_operation_new(GtkWidget *widget, LIB3270_PRINT_MODE mode)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	V3270PrintOperation	* operation	= GTK_V3270_PRINT_OPERATION(g_object_new(GTK_TYPE_V3270_PRINT_OPERATION, NULL));

	operation->mode	= mode;
	operation->widget = GTK_V3270(widget);
	operation->session = v3270_get_session(widget);

	return operation;
}

/*--[ Convenience ]----------------------------------------------------------------------------------*/

 void v3270_print(GtkWidget *widget, LIB3270_PRINT_MODE mode)
 {
 	g_autoptr(GError) err = NULL;

	V3270PrintOperation * operation = v3270_print_operation_new(widget, mode);
	gtk_print_operation_run(GTK_PRINT_OPERATION(operation),GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,GTK_WINDOW(gtk_widget_get_toplevel(widget)),&err);
	g_object_unref(operation);

 }

 void v3270_print_all(GtkWidget *widget)
 {
	v3270_print(widget,LIB3270_PRINT_ALL);
 }

 void v3270_print_selected(GtkWidget *widget)
 {
	v3270_print(widget,LIB3270_PRINT_SELECTED);
 }

 void v3270_print_copy(GtkWidget *widget)
 {
	v3270_print(widget,LIB3270_PRINT_COPY);
 }

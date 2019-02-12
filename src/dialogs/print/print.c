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
 #include "../../terminal/private.h"	// Required for v3270 signal.
 #include <v3270/colorscheme.h>
 #include <lib3270/selection.h>

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
 static void color_scheme_changed(GtkWidget G_GNUC_UNUSED(*widget), const GdkRGBA *colors, V3270PrintOperation *operation) {

	debug("%s=%p",__FUNCTION__,colors);

	int f;
	for(f=0;f<V3270_COLOR_COUNT;f++)
		operation->colors[f] = colors[f];

 }

 static void font_name_changed(GtkComboBox *widget, V3270PrintOperation *operation)
 {
	GValue value = { 0, };
	GtkTreeIter iter;

	if(!gtk_combo_box_get_active_iter(widget,&iter))
		return;

	gtk_tree_model_get_value(gtk_combo_box_get_model(widget),&iter,0,&value);

	g_free(operation->font.name);
	operation->font.name = g_value_dup_string(&value);

	debug("%s=%s",__FUNCTION__,operation->font.name);
 }

 static void toggle_show_selection(GtkToggleButton *widget, V3270PrintOperation *operation)
 {
 	operation->show_selection = gtk_toggle_button_get_active(widget);
 }

 static GtkWidget * create_custom_widget(GtkPrintOperation *prt)
 {
	static const gchar * text[] =
	{
			N_( "_Font:" ),
			N_( "C_olor scheme:" )
	};

	size_t f;

 	V3270PrintOperation	* operation = GTK_V3270_PRINT_OPERATION(prt);

	if(operation->widget)
		g_signal_emit(operation->widget, v3270_widget_signal[SIGNAL_PRINT_SETUP], 0, prt);

	// Create dialog

	GtkWidget			* frame = gtk_frame_new("");
 	GtkGrid 			* grid = GTK_GRID(gtk_grid_new());
 	GtkWidget			* font = v3270_font_selection_new(operation->font.name);
 	GtkWidget			* color = v3270_color_scheme_new();
	GtkWidget 			* selected = gtk_check_button_new_with_label( _("Print selection box") );

	// The print dialog doesn't follow the guidelines from https://developer.gnome.org/hig/stable/visual-layout.html.en )-:

	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_NONE);

	GtkWidget *label = gtk_label_new(NULL);

	gtk_label_set_markup(GTK_LABEL(label),_("<b>Text options</b>"));
	gtk_frame_set_label_widget(GTK_FRAME(frame),label);

 	gtk_container_set_border_width(GTK_CONTAINER(frame),12);

 	gtk_container_set_border_width(GTK_CONTAINER(grid),6);

 	g_object_set(G_OBJECT(grid),"margin-start",8,NULL);

 	gtk_grid_set_row_spacing(grid,6);
 	gtk_grid_set_column_spacing(grid,12);

 	v3270_color_scheme_set_rgba(color,operation->colors);
	g_signal_connect(G_OBJECT(color),"update-colors",G_CALLBACK(color_scheme_changed),operation);
	g_signal_connect(G_OBJECT(font),"changed",G_CALLBACK(font_name_changed),operation);
	g_signal_connect(G_OBJECT(selected),"toggled",G_CALLBACK(toggle_show_selection),operation);


	for(f=0;f<G_N_ELEMENTS(text);f++)
	{
		GtkWidget *label = gtk_label_new_with_mnemonic(gettext(text[f]));
		gtk_widget_set_halign(label,GTK_ALIGN_START);
		gtk_grid_attach(grid,label,0,f,1,1);
	}

	gtk_grid_attach(grid,font,1,0,1,1);
	gtk_grid_attach(grid,color,1,1,1,1);
	gtk_grid_attach(grid,selected,1,2,1,1);

	gtk_container_add(GTK_CONTAINER(frame),GTK_WIDGET(grid));

	gtk_widget_show_all(GTK_WIDGET(frame));
	return frame;
 }

 static void custom_widget_apply(GtkPrintOperation *prt, GtkWidget G_GNUC_UNUSED(*widget))
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

	if(operation->font.info.scaled)
		cairo_scaled_font_destroy(operation->font.info.scaled);

	g_free(operation->font.name);

	if(operation->contents.text)
	{
		size_t row;

		for(row = 0; operation->contents.text[row]; row++)
		{
			g_free(operation->contents.text[row]);
		}
		g_free(operation->contents.text);

	}

	G_OBJECT_CLASS(V3270PrintOperation_parent_class)->dispose(object);

 }

 static void V3270PrintOperation_class_init(V3270PrintOperationClass *klass)
 {
	GtkPrintOperationClass	* operation	= GTK_PRINT_OPERATION_CLASS(klass);

	G_OBJECT_CLASS(klass)->dispose = dispose;
	operation->done = done;
	operation->begin_print = V3270PrintOperation_begin_print;
	operation->draw_page = V3270PrintOperation_draw_page;

#ifndef _WIN32
	operation->create_custom_widget = create_custom_widget;
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
    widget->mode 			= LIB3270_PRINT_ALL;
    widget->show_selection	= FALSE;
    widget->font.name		= NULL; // g_strdup(v3270_default_font);
    widget->contents.width	= 80;

	v3270_set_mono_color_table(widget->colors,"#000000","#FFFFFF");

 }

V3270PrintOperation	* v3270_print_operation_new(GtkWidget *widget, LIB3270_PRINT_MODE mode)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	V3270PrintOperation	* operation	= GTK_V3270_PRINT_OPERATION(g_object_new(GTK_TYPE_V3270_PRINT_OPERATION, NULL));

	operation->mode	= mode;
	operation->widget = GTK_V3270(widget);
	operation->session = v3270_get_session(widget);
	operation->font.name = g_strdup(v3270_get_font_family(widget));

	V3270PrintOperation_set_text_by_mode(operation, mode);

	return operation;
}

/*--[ Convenience ]----------------------------------------------------------------------------------*/

 int v3270_print(GtkWidget *widget, LIB3270_PRINT_MODE mode, GError **error)
 {
 	if(*error)
	{
		return -1;
	}

	lib3270_trace_event(v3270_get_session(widget),"print action activated (type=%d)",(int) mode);

 	if(v3270_is_connected(widget))
	{
		V3270PrintOperation * operation = v3270_print_operation_new(widget, mode);
		gtk_print_operation_run(GTK_PRINT_OPERATION(operation),GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,GTK_WINDOW(gtk_widget_get_toplevel(widget)),error);
		g_object_unref(operation);
		return (*error != NULL);
	}

	*error = g_error_new(g_quark_from_static_string(PACKAGE_NAME),ENOTCONN,"%s",strerror(ENOTCONN));

	return -1;

 }

 int v3270_print_all(GtkWidget *widget, GError **error)
 {
	return v3270_print(widget,LIB3270_PRINT_ALL,error);
 }

 int v3270_print_selected(GtkWidget *widget, GError **error)
 {
	return v3270_print(widget,LIB3270_PRINT_SELECTED,error);
 }

 int v3270_print_copy(GtkWidget *widget, GError **error)
 {
	return v3270_print(widget,LIB3270_PRINT_COPY,error);
 }

 void V3270PrintOperation_set_text_by_mode(V3270PrintOperation * operation, LIB3270_PRINT_MODE mode)
 {
 	operation->mode = mode;

	switch(mode)
	{
	case LIB3270_PRINT_ALL:
		{
			size_t row, col;
			int baddr = 0;
			column * text;

			operation->contents.height = lib3270_get_height(operation->session);
			operation->contents.width = lib3270_get_width(operation->session);

			operation->contents.text = g_new0(column *, operation->contents.height+1);

			for(row = 0; row < operation->contents.height; row++)
			{
				operation->contents.text[row] = text = g_new0(column, operation->contents.width);
				for(col = 0; col < operation->contents.width; col++)
				{
					lib3270_get_element(operation->session,baddr++,&text[col].c,&text[col].attr);
				}
			}

		}
		break;

	case LIB3270_PRINT_SELECTED:
		{
			int row, col;
			int baddr = 0;

			GdkRectangle rect;
			memset(&rect,0,sizeof(rect));
			rect.x = lib3270_get_width(operation->session);
			rect.y = lib3270_get_height(operation->session);

			// Get regions

			for(row = 0; row < lib3270_get_height(operation->session); row++)
			{
				for(col = 0; col < lib3270_get_width(operation->session); col++)
				{
					if(lib3270_is_selected(operation->session,baddr++))
					{
						rect.x = MIN(rect.x,col);
						rect.width = MAX(rect.width,col);

						rect.y = MIN(rect.y,row);
						rect.height = MAX(rect.height,row);
					}
				}
			}

			operation->contents.height = rect.height - rect.y;
			operation->contents.width = rect.width - rect.x;

			// Get contents
			int r = 0;
			column * text;

			operation->contents.text = g_new0(column *, operation->contents.height+1);

			for(row = rect.y; row < rect.width; row++)
			{
				int c = 0;
				operation->contents.text[r++] = text = g_new0(column, operation->contents.width);

				for(col = rect.x; col < rect.height; col++)
				{
					lib3270_get_element(operation->session,lib3270_translate_to_address(operation->session,row,col),&text[c].c,&text[c].attr);
					if(!(text[c].attr & LIB3270_ATTR_SELECTED))
					{
						text[c].c = 0;
						text[c].attr = 0;
					}
                    c++;
				}

			}

		}
		break;

	case LIB3270_PRINT_COPY:
		{
			lib3270_autoptr(char) copy = v3270_get_copy(GTK_WIDGET(operation->widget));
			if(copy)
			{
				size_t r;
				gchar ** text = g_strsplit(copy,"\n",-1);
				operation->contents.height = g_strv_length(text);
				operation->contents.width = 0;

				for(r=0;r < operation->contents.height;r++)
				{
					operation->contents.width = MAX(operation->contents.width,strlen(text[r]));
				}

				g_strfreev(text);

			}
		}
		break;

	}

	debug("%s: width=%u height=%u",__FUNCTION__,(unsigned int) operation->contents.width, (unsigned int) operation->contents.height);

 }

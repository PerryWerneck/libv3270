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
 #include <terminal.h>
 #include <v3270/colorscheme.h>
 #include <lib3270/selection.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

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

 GtkWidget * V3270PrintOperation_custom_widget_new(GtkPrintOperation *prt)
 {
	static const gchar * text[] =
	{
			N_( "_Font:" ),
			N_( "C_olor scheme:" )
	};

	size_t f;

 	V3270PrintOperation	* operation = GTK_V3270_PRINT_OPERATION(prt);

	if(operation->widget)
		g_signal_emit(operation->widget, v3270_widget_signal[V3270_SIGNAL_PRINT_SETUP], 0, prt);

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

 void V3270PrintOperation_custom_widget_apply(GtkPrintOperation *prt, GtkWidget G_GNUC_UNUSED(*widget))
 {
 	V3270PrintOperation	* operation = GTK_V3270_PRINT_OPERATION(prt);

	debug("%s",__FUNCTION__);

	if(operation->widget)
		g_signal_emit(operation->widget, v3270_widget_signal[V3270_SIGNAL_PRINT_APPLY], 0, prt);

 }

#endif // _WIN32

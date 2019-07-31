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

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 G_DEFINE_TYPE(V3270PrintSettings, V3270PrintSettings, GTK_TYPE_GRID);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void finalize(GObject *object)
 {

//	V3270PrintSettings * settings = GTK_V3270_PRINT_SETTINGS(object);


	G_OBJECT_CLASS(V3270PrintSettings_parent_class)->finalize(object);
 }

static void V3270PrintSettings_class_init(V3270PrintSettingsClass *klass)
{
	GObjectClass * gobject_class = G_OBJECT_CLASS(klass);

	// Object methods
	gobject_class->finalize	= finalize;

}

/*
static void color_scheme_changed(GtkWidget G_GNUC_UNUSED(*widget), const GdkRGBA *colors, V3270PrintSettings *settings)
{

	debug("%s=%p",__FUNCTION__,colors);

	int f;
	for(f=0;f<V3270_COLOR_COUNT;f++)
		settings->colors[f] = colors[f];

}
*/

static void V3270PrintSettings_init(V3270PrintSettings *widget)
{
	static const gchar * text[] =
	{
			N_( "_Font:" ),
			N_( "C_olor scheme:" )
	};

	size_t f;
	widget->font 		= v3270_font_selection_new("monospace");
	widget->color 		= v3270_color_scheme_new();
	widget->selected 	= gtk_check_button_new_with_label( _("Print selection box") );

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
 	gtk_grid_set_row_spacing(GTK_GRID(widget),6);
 	gtk_grid_set_column_spacing(GTK_GRID(widget),12);

	// g_signal_connect(G_OBJECT(widget->color),"update-colors",G_CALLBACK(color_scheme_changed),widget);
	// g_signal_connect(G_OBJECT(widget->selected),"toggled",G_CALLBACK(toggle_show_selection),widget);

	for(f=0;f<G_N_ELEMENTS(text);f++)
	{
		GtkWidget *label = gtk_label_new_with_mnemonic(gettext(text[f]));
		gtk_widget_set_halign(label,GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(widget),label,0,f,1,1);
	}

	gtk_grid_attach(GTK_GRID(widget),widget->font,1,0,1,1);
	gtk_grid_attach(GTK_GRID(widget),widget->color,1,1,1,1);
	gtk_grid_attach(GTK_GRID(widget),widget->selected,1,2,1,1);

	// Set color scheme
	GdkRGBA colors[V3270_COLOR_COUNT];
	v3270_set_mono_color_table(colors,"#000000","#FFFFFF");
 	v3270_color_scheme_set_rgba(widget->color,colors);

	gtk_widget_show_all(GTK_WIDGET(widget));

}

LIB3270_EXPORT GtkWidget * V3270_print_settings_new(GtkWidget *widget)
{
	V3270PrintSettings * settings = GTK_V3270_PRINT_SETTINGS(g_object_new(GTK_TYPE_V3270_PRINT_SETTINGS, NULL));

	v3270_font_selection_set_family(settings->font, v3270_get_font_family(widget));

	return GTK_WIDGET(settings);

}

LIB3270_EXPORT GtkWidget * V3270_print_settings_new_from_operation(GtkPrintOperation *operation)
{
    g_return_val_if_fail(GTK_IS_V3270_PRINT_OPERATION(operation),NULL);

    V3270PrintOperation	* opr		= GTK_V3270_PRINT_OPERATION(operation);
	V3270PrintSettings	* settings	= GTK_V3270_PRINT_SETTINGS(g_object_new(GTK_TYPE_V3270_PRINT_SETTINGS, NULL));

	v3270_font_selection_set_family(settings->font, opr->font.name);
 	v3270_color_scheme_set_rgba(settings->color,opr->settings.colors);
 	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(settings->selected),opr->settings.show_selection);

	return GTK_WIDGET(settings);

}

LIB3270_EXPORT int v3270_print_settings_get_rgba(GtkWidget *widget, GdkRGBA *colors, size_t num_colors)
{
	V3270PrintSettings * settings = GTK_V3270_PRINT_SETTINGS(widget);
	return v3270_color_scheme_get_rgba(settings->color, colors, num_colors);
}

LIB3270_EXPORT gboolean v3270_print_settings_get_show_selection(GtkWidget *widget)
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_V3270_PRINT_SETTINGS(widget)->selected));
}

LIB3270_EXPORT void v3270_print_settings_set_show_selection(GtkWidget *widget, gboolean is_active)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_V3270_PRINT_SETTINGS(widget)->selected),is_active);
}

LIB3270_EXPORT gchar * v3270_print_settings_get_color_scheme(GtkWidget *widget)
{
	return v3270_color_scheme_get_text((GTK_V3270_PRINT_SETTINGS(widget)->color));
}

LIB3270_EXPORT void v3270_print_settings_set_color_scheme(GtkWidget *widget, const gchar *colors)
{
	v3270_color_scheme_set_text(GTK_V3270_PRINT_SETTINGS(widget)->color, colors);
}

LIB3270_EXPORT gchar * v3270_print_settings_get_font_family(GtkWidget *widget)
{
	return v3270_font_selection_get_family(GTK_V3270_PRINT_SETTINGS(widget)->font);
}

LIB3270_EXPORT gboolean v3270_print_settings_set_font_family(GtkWidget *widget, const gchar *fontname)
{
	return v3270_font_selection_set_family(GTK_V3270_PRINT_SETTINGS(widget)->font,fontname);
}

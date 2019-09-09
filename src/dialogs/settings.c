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
 #include <v3270/settings.h>
 #include <lib3270/log.h>

 G_DEFINE_TYPE(V3270Settings, V3270Settings, GTK_TYPE_GRID);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void apply(GtkWidget G_GNUC_UNUSED(*widget), GtkWidget G_GNUC_UNUSED(*terminal))
{
	debug("V3270Settings::%s",__FUNCTION__);
}

static void cancel(GtkWidget G_GNUC_UNUSED(*widget), GtkWidget G_GNUC_UNUSED(*terminal))
{
	debug("V3270Settings::%s",__FUNCTION__);
}

static void load(GtkWidget G_GNUC_UNUSED(*widget), GtkWidget G_GNUC_UNUSED(*terminal))
{
	debug("V3270Settings::%s",__FUNCTION__);
}

static void V3270Settings_class_init(V3270SettingsClass *klass)
{
    klass->apply = apply;
    klass->cancel = cancel;
    klass->load = load;
}

static void V3270Settings_init(V3270Settings *widget)
{
    widget->terminal = NULL;

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
 	gtk_grid_set_row_spacing(GTK_GRID(widget),6);
 	gtk_grid_set_column_spacing(GTK_GRID(widget),12);

}

LIB3270_EXPORT void v3270_settings_set_terminal_widget(GtkWidget *widget, GtkWidget *terminal)
{
	g_return_if_fail(GTK_IS_V3270(terminal));
	g_return_if_fail(GTK_IS_V3270_SETTINGS(widget));

    GTK_V3270_SETTINGS(widget)->terminal = terminal;
    GTK_V3270_SETTINGS_GET_CLASS(widget)->load(widget,terminal);
}

 LIB3270_EXPORT void v3270_settings_apply(GtkWidget *widget)
 {
    g_return_if_fail(GTK_IS_V3270_SETTINGS(widget));
    GTK_V3270_SETTINGS_GET_CLASS(widget)->apply(widget,GTK_V3270_SETTINGS(widget)->terminal);
 }

 LIB3270_EXPORT void v3270_settings_cancel(GtkWidget *widget)
 {
    g_return_if_fail(GTK_IS_V3270_SETTINGS(widget));
    GTK_V3270_SETTINGS_GET_CLASS(widget)->cancel(widget,GTK_V3270_SETTINGS(widget)->terminal);
 }

 LIB3270_EXPORT void v3270_settings_on_dialog_response(GtkDialog G_GNUC_UNUSED(*dialog), gint response_id, GtkWidget *settings)
 {
    switch(response_id)
    {
    case GTK_RESPONSE_APPLY:
        v3270_settings_apply(settings);
        break;

    case GTK_RESPONSE_CANCEL:
        v3270_settings_cancel(settings);
        break;

    }
 }

 LIB3270_EXPORT GtkWidget * v3270_settings_dialog_new(GtkWidget *terminal, GtkWidget *settings)
 {
#if GTK_CHECK_VERSION(3,12,0)

	gboolean use_header;
	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &use_header, NULL);

	GtkWidget * dialog =
		GTK_WIDGET(g_object_new(
			GTK_TYPE_DIALOG,
			"use-header-bar", (use_header ? 1 : 0),
			NULL
		));

#else

	GtkWidget * dialog = gtk_dialog_new();

#endif	// GTK 3.12

	GtkWidget * content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	gtk_box_set_spacing(
		GTK_BOX(content_area),
		18
	);

	gtk_box_pack_start(GTK_BOX(content_area),settings,FALSE,FALSE,2);
    v3270_settings_set_terminal_widget(settings,terminal);

    g_signal_connect(G_OBJECT(dialog),"response",G_CALLBACK(v3270_settings_on_dialog_response),settings);

	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),18);

    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gtk_widget_get_toplevel(terminal)));
    //gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

    return dialog;
 }

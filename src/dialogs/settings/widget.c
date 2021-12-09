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

 #include "../private.h"
 #include <marshal/terminal.h>
 #include <internals.h>
 #include <v3270/settings.h>
 #include <lib3270/log.h>

 static void signal_update_message(GtkWidget *terminal, LIB3270_MESSAGE G_GNUC_UNUSED(id), GtkWidget *settings);

 G_DEFINE_TYPE(V3270Settings, V3270Settings, GTK_TYPE_GRID);

 enum
 {
 	VALIDITY_SIGNAL,	///< @brief Signal for valid state.
 	SIGNAL_LAST
 };

 static guint signals[SIGNAL_LAST]	= { 0 };


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

static void update_message(GtkWidget G_GNUC_UNUSED(*widget), GtkWidget G_GNUC_UNUSED(*terminal))
{
	debug("V3270Settings::%s",__FUNCTION__);
}

static void finalize(GObject *object)
{
	V3270Settings * settings = GTK_V3270_SETTINGS(object);

	if(settings->terminal)
	{
		// Disconnect terminal widget
		//
		// (Can't use v3270_settings_set_terminal_widget here)
		//
		gulong handler = g_signal_handler_find(
								settings->terminal,
								G_SIGNAL_MATCH_FUNC|G_SIGNAL_MATCH_DATA,
								0,
								0,
								NULL,
								G_CALLBACK(signal_update_message),
								object
							);

		if(handler)
			g_signal_handler_disconnect(settings->terminal, handler);

		settings->terminal = NULL;
	}

	G_OBJECT_CLASS(V3270Settings_parent_class)->finalize(object);
}

static void validity(GtkWidget G_GNUC_UNUSED(*widget), gboolean G_GNUC_UNUSED(state))
{

}

static void V3270Settings_class_init(V3270SettingsClass *klass)
{
	GObjectClass * gobject_class = G_OBJECT_CLASS(klass);

    klass->apply = apply;
    klass->revert = cancel;
    klass->load = load;
    klass->validity = validity;
    klass->update_message = update_message;

    gobject_class->finalize = finalize;

	signals[VALIDITY_SIGNAL] =
		g_signal_new(	I_("validity"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (V3270SettingsClass, validity),
						NULL, NULL,
						v3270_VOID__VOID_BOOLEAN,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

}

static void V3270Settings_init(V3270Settings *widget)
{
    widget->terminal	= NULL;
    widget->valid		= FALSE;

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
 	gtk_grid_set_row_spacing(GTK_GRID(widget),6);
 	gtk_grid_set_column_spacing(GTK_GRID(widget),12);

}

static void signal_update_message(GtkWidget *terminal, LIB3270_MESSAGE G_GNUC_UNUSED(id), GtkWidget *settings)
{
	GTK_V3270_SETTINGS_GET_CLASS(settings)->update_message(settings,terminal);
}

LIB3270_EXPORT void v3270_settings_set_terminal_widget(GtkWidget *widget, GtkWidget *terminal)
{
	g_return_if_fail(GTK_IS_V3270_SETTINGS(widget));

	V3270Settings * settings = GTK_V3270_SETTINGS(widget);

	debug("%s(%p,%p)",__FUNCTION__,widget,terminal);

	// Return if there's nothing to do.
	if(settings->terminal == terminal)
		return;

	if(settings->terminal)
	{
		// Disconnect old terminal widget
		gulong handler = g_signal_handler_find(
								settings->terminal,
								G_SIGNAL_MATCH_FUNC|G_SIGNAL_MATCH_DATA,
								0,
								0,
								NULL,
								G_CALLBACK(signal_update_message),
								widget
							);

		debug("handler=%u",(unsigned long) handler);

		if(handler)
			g_signal_handler_disconnect(settings->terminal, handler);

	}

	// Update terminal
	settings->terminal = terminal;

	if(settings->terminal)
	{
		// Connect the new widget.
		g_signal_connect(G_OBJECT(terminal),I_("message_changed"), G_CALLBACK(signal_update_message), widget);

		// Set as valid before update message and load contents.
		v3270_settings_set_valid(widget,TRUE);

		// Update dialog state.
		GTK_V3270_SETTINGS_GET_CLASS(widget)->update_message(widget,terminal);

		// Load the dialog contents.
		GTK_V3270_SETTINGS_GET_CLASS(widget)->load(widget,terminal);
	}
	else
	{
		v3270_settings_set_valid(widget,FALSE);
	}

 }

 LIB3270_EXPORT GtkWidget * v3270_settings_get_terminal_widget(GtkWidget *widget)
 {
	g_return_val_if_fail(GTK_IS_V3270_SETTINGS(widget),NULL);
    return GTK_V3270_SETTINGS(widget)->terminal;
 }

 LIB3270_EXPORT void v3270_settings_apply(GtkWidget *widget)
 {
    g_return_if_fail(GTK_IS_V3270_SETTINGS(widget));
    GTK_V3270_SETTINGS_GET_CLASS(widget)->apply(widget,GTK_V3270_SETTINGS(widget)->terminal);
 }

 LIB3270_EXPORT void v3270_settings_revert(GtkWidget *widget)
 {
    g_return_if_fail(GTK_IS_V3270_SETTINGS(widget));
    GTK_V3270_SETTINGS_GET_CLASS(widget)->revert(widget,GTK_V3270_SETTINGS(widget)->terminal);
 }

 LIB3270_EXPORT const gchar * v3270_settings_get_title(GtkWidget *widget)
 {
 	g_return_val_if_fail(GTK_IS_V3270_SETTINGS(widget),NULL);
 	return GTK_V3270_SETTINGS(widget)->title;
 }

 LIB3270_EXPORT const gchar * v3270_settings_get_label(GtkWidget *widget)
 {
 	g_return_val_if_fail(GTK_IS_V3270_SETTINGS(widget),NULL);
 	return GTK_V3270_SETTINGS(widget)->label;
 }

 LIB3270_EXPORT const gchar * v3270_settings_get_tooltip(GtkWidget *widget)
 {
 	g_return_val_if_fail(GTK_IS_V3270_SETTINGS(widget),NULL);
 	return GTK_V3270_SETTINGS(widget)->tooltip;
 }

 LIB3270_EXPORT gboolean v3270_settings_get_valid(GtkWidget *widget)
 {
 	// Non settings widget is always valid.
 	g_return_val_if_fail(GTK_IS_V3270_SETTINGS(widget),TRUE);
 	return GTK_V3270_SETTINGS(widget)->valid;
 }

 LIB3270_EXPORT void v3270_settings_set_valid(GtkWidget *widget, gboolean is_valid)
 {
	V3270Settings * settings = GTK_V3270_SETTINGS(widget);

	if(is_valid != settings->valid)
	{
		settings->valid = is_valid;
		g_signal_emit(widget, signals[VALIDITY_SIGNAL], 0, settings->valid);
	}

 }

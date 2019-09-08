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

static void apply(GtkWidget *widget, GtkWidget *terminal)
{

}

static void revert(GtkWidget *widget, GtkWidget *terminal)
{

}

static void V3270Settings_class_init(V3270SettingsClass *klass)
{
    klass->apply = apply;
    klass->revert = revert;
}

static void V3270Settings_init(V3270Settings *widget)
{
    widget->terminal = NULL;
}

LIB3270_EXPORT void v3270_settings_set_terminal_widget(GtkWidget *widget, GtkWidget *terminal)
{
	g_return_if_fail(GTK_IS_V3270(terminal));
	g_return_if_fail(GTK_IS_V3270_SETTINGS(widget));

    GTK_V3270_SETTINGS(widget)->terminal = terminal;
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

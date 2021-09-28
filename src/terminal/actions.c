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

 #include <config.h>
 #include <terminal.h>
 #include <lib3270/actions.h>
 #include <v3270/settings.h>
 #include <v3270/tools.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

LIB3270_EXPORT void v3270_disconnect(GtkWidget *widget)
{
	g_return_if_fail(GTK_IS_V3270(widget));
	v3270_disable_updates(widget);
	debug("%s",__FUNCTION__);
	lib3270_unselect(GTK_V3270(widget)->host);
	debug("%s",__FUNCTION__);
	lib3270_disconnect(GTK_V3270(widget)->host);
	debug("%s",__FUNCTION__);
	v3270_enable_updates(widget);
	debug("%s",__FUNCTION__);
}

LIB3270_EXPORT int v3270_reconnect(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),EINVAL);

	return lib3270_reconnect(GTK_V3270(widget)->host,0);
}

void v3270_activate(GtkWidget *widget) {

	v3270 * terminal = GTK_V3270(widget);

	terminal->activity.timestamp = time(0);

	if(lib3270_is_connected(terminal->host)) {
		lib3270_enter(terminal->host);
	} else if(lib3270_get_url(terminal->host)) {
		v3270_reconnect(widget);
	} else {
		g_message("Terminal widget activated without connection or valid url");

        GtkWidget * dialog = v3270_settings_dialog_new();
		gtk_container_add(GTK_CONTAINER(dialog), v3270_host_settings_new());
        gtk_window_set_title(GTK_WINDOW(dialog),_("Setup host properties"));

		gtk_dialog_set_toplevel(dialog,widget);

        v3270_settings_dialog_set_terminal_widget(dialog, widget);
        g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
        g_signal_connect(dialog,"response",G_CALLBACK(v3270_setttings_dialog_response),NULL);

        gtk_widget_show_all(dialog);
	}
}


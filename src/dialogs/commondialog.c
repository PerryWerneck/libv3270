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

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void apply_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *dialog)
{
	gtk_dialog_response(GTK_DIALOG(dialog),GTK_RESPONSE_APPLY);
}

static void cancel_clicked(GtkButton G_GNUC_UNUSED(*button), GtkWidget *dialog)
{
	gtk_dialog_response(GTK_DIALOG(dialog),GTK_RESPONSE_CANCEL);
}

static GtkWidget * create_button(GtkWidget *dialog, const gchar *mnemonic, GCallback callback)
{
	GtkWidget * button = gtk_button_new_with_mnemonic(mnemonic);
	gtk_button_set_use_underline(GTK_BUTTON (button), TRUE);
	g_signal_connect(button,"clicked",callback,dialog);
	gtk_widget_show(button);
	return button;
}

LIB3270_EXPORT GtkWidget * v3270_dialog_new(const gchar *title, GtkWindow *parent, const gchar *apply)
{
	gboolean use_header;
	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &use_header, NULL);

	GtkWidget * dialog =
		GTK_WIDGET(g_object_new(
			GTK_TYPE_DIALOG,
			"use-header-bar", (use_header ? 1 : 0),
			NULL
		));

	gtk_window_set_title(GTK_WINDOW(dialog), title);
	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),18);

	if(parent)
	{
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
		gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
		gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
	}

	GtkWidget * header = gtk_dialog_get_header_bar(GTK_DIALOG(dialog));

	if(header)
	{
		gtk_header_bar_pack_start(GTK_HEADER_BAR(header),create_button(dialog,_("_Cancel"),G_CALLBACK(cancel_clicked)));
		gtk_header_bar_pack_end(GTK_HEADER_BAR(header),create_button(dialog,apply,G_CALLBACK(apply_clicked)));
	}
	else
	{
		gtk_dialog_add_buttons(
			GTK_DIALOG (dialog),
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			apply, GTK_RESPONSE_APPLY,
			NULL
		);
	}

	return dialog;
}

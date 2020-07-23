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
 #include <internals.h>
 #include <terminal.h>
 #include <v3270/dialogs.h>
 #include <v3270/settings.h>
 #include <lib3270/popup.h>

 /*--[ Implement ]------------------------------------------------------------------------------------*/

 GtkResponseType v3270_show_popup(GtkWidget *widget, const LIB3270_POPUP *popup, gboolean wait) {

	g_return_val_if_fail(GTK_IS_WIDGET(widget),GTK_RESPONSE_NONE);

	// Popup settings.
	static const struct _settings {
		GtkMessageType type;
		const gchar *button;
		const gchar *title;
	} settings[LIB3270_NOTIFY_USER] = {

		// LIB3270_NOTIFY_INFO - Simple information dialog.
		{
			.type = GTK_MESSAGE_INFO,
			.button = N_("_Ok"),
			.title = N_("Information")

		},

		// LIB3270_NOTIFY_WARNING - Warning message.
		{
			.type = GTK_MESSAGE_WARNING,
			.button = N_("_Ok"),
			.title = N_("Warning")

		},

		// LIB3270_NOTIFY_ERROR - Error message.
		{
			.type = GTK_MESSAGE_ERROR,
			.button = N_("_Ok"),
			.title = N_("Error")

		},

		// LIB3270_NOTIFY_CRITICAL - Critical error, user can abort application.
		{
			.type = GTK_MESSAGE_ERROR,
			.button = N_("_Close"),
			.title = N_("Critical Error")

		},

		// LIB3270_NOTIFY_SECURE - Secure host dialog.
		{
			.type = GTK_MESSAGE_OTHER,
			.button = N_("_Ok"),
			.title = N_("Security alert")

		}

	};

	// Create dialog
	GtkWidget * dialog =
			gtk_message_dialog_new_with_markup(
				GTK_WINDOW(gtk_widget_get_toplevel(widget)),
				GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
				settings[popup->type].type,
				GTK_BUTTONS_NONE,
				"%s",popup->summary
			);

	if(popup->body) {
		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),"%s",popup->body);
	}

	if(popup->title) {
		gtk_window_set_title(GTK_WINDOW(dialog),popup->title);
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog),g_dgettext(GETTEXT_PACKAGE,settings[popup->type].title));
	}


	if(wait) {

		// Wait for response.
		if(popup->label) {

			gtk_dialog_add_button(GTK_DIALOG(dialog), _("_Cancel"), GTK_RESPONSE_CANCEL);
			gtk_dialog_add_button(GTK_DIALOG(dialog), popup->label, GTK_RESPONSE_APPLY);
			gtk_dialog_set_default_response(
				GTK_DIALOG(dialog),
				(popup->type == LIB3270_NOTIFY_SECURE ? GTK_RESPONSE_CANCEL : GTK_RESPONSE_APPLY)
			);

		} else {

			gtk_dialog_add_button(GTK_DIALOG(dialog), g_dgettext(GETTEXT_PACKAGE,settings[popup->type].button), GTK_RESPONSE_OK);

		}

		gtk_widget_show_all(dialog);
		gint rc = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return rc;

	}

	// Unnamed dialog, no need for wait.
	g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
	g_signal_connect(dialog,"response",G_CALLBACK(gtk_widget_destroy),NULL);

	gtk_dialog_add_button(GTK_DIALOG(dialog), settings[popup->type].button, GTK_RESPONSE_OK);

	gtk_widget_show_all(dialog);

	return GTK_RESPONSE_NONE;

 }

 /*
 void v3270_popup_message(GtkWidget *widget, LIB3270_NOTIFY type , const gchar *title, const gchar *message, const gchar *text) {

	LIB3270_POPUP popup = {
		.type = type,
		.body = text
	};

	if(message) {
		popup.summary = message;
		popup.title = title;
	} else {
		popup.summary = title;
	}

	v3270_show_popup(widget, &popup, FALSE);

 }
 */

 void v3270_error_popup(GtkWidget *widget, const gchar *title, const gchar *summary, const gchar *body) {

	LIB3270_POPUP popup = {
		.type = LIB3270_NOTIFY_ERROR,
		.title = title,
		.summary = summary,
		.body = body
	};

	v3270_show_popup(widget, &popup, FALSE);

 }

 void v3270_popup_gerror(GtkWidget *widget, GError *error, const gchar *title, const gchar *fmt, ...) {

	// Format message.
 	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	g_autofree gchar *summary = g_strdup_vprintf(fmt,arg_ptr);
	va_end(arg_ptr);

	LIB3270_POPUP popup = {
		.type = LIB3270_NOTIFY_ERROR,
		.title = title,
		.summary = summary,
		.body = error->message
	};

	v3270_show_popup(widget, &popup, FALSE);

 }

 GtkResponseType v3270_popup_toggleable_dialog(GtkWidget *widget, V3270_TOGGLEABLE_DIALOG id, const gchar *title, const gchar *summary, const gchar *body, const gchar *first_button_text, ...) {

	GtkResponseType response = GTK_V3270(widget)->responses[id];

	if(response == GTK_RESPONSE_NONE) {
		GtkWidget * dialog =
			gtk_message_dialog_new(
				GTK_WINDOW(gtk_widget_get_toplevel(widget)),
				GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_INFO,
				GTK_BUTTONS_NONE,
				"%s",
				summary
			);

		if(body)
			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),"%s",body);

		gtk_window_set_title(GTK_WINDOW(dialog),title);

		// Set check button
		GtkWidget * dont_ask = gtk_check_button_new_with_label(_("Don't ask again"));

		gtk_widget_set_can_focus(dont_ask,FALSE);
		gtk_widget_set_can_default(dont_ask,FALSE);
#if GTK_CHECK_VERSION(3,20,0)
		gtk_widget_set_focus_on_click(dont_ask,FALSE);
#endif // GTK 3,20,0

		gtk_widget_set_valign(dont_ask, GTK_ALIGN_BASELINE);
		gtk_widget_set_halign(dont_ask, GTK_ALIGN_START);

		gtk_box_pack_start(
			GTK_BOX(gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog))),
			dont_ask,
			TRUE,
			TRUE,
			0
		);

		if(first_button_text) {
            // From https://github.com/GNOME/gtk/blob/master/gtk/gtkdialog.c
			va_list args;
			const gchar* text;
			gint response_id;

			va_start(args, first_button_text);

			text = first_button_text;
			while(text) {
				response_id = va_arg(args, gint);

				gtk_dialog_add_button(GTK_DIALOG(dialog), text, response_id);

				if(response_id == GTK_RESPONSE_APPLY || response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_YES)
					gtk_dialog_set_default_response(GTK_DIALOG(dialog),response_id);

				text = va_arg(args, gchar*);
			}

			va_end (args);
		}

		gtk_widget_show_all(dialog);
		response = gtk_dialog_run(GTK_DIALOG(dialog));

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dont_ask))) {

			GTK_V3270(widget)->responses[id] = response;
			g_object_notify_by_pspec(G_OBJECT(widget), GTK_V3270_GET_CLASS(widget)->responses[id]);
			v3270_emit_save_settings(widget);

			debug(
				"Property %s is now %d",
					g_param_spec_get_name(GTK_V3270_GET_CLASS(widget)->responses[id]),
					GTK_V3270(widget)->responses[id]
			);

		}

		gtk_widget_destroy(dialog);
	}

	return response;

 }

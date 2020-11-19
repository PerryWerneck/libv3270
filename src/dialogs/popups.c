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

 static void v3270_dialog_add_class_for_response(GtkWidget *dialog, gint response_id, const char *className) {
	GtkWidget * widget = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),response_id);
	gtk_widget_add_class(widget,className);
 }

 GtkResponseType v3270_popup_dialog_show(GtkWidget *widget, const LIB3270_POPUP *popup, gboolean wait) {

	// https://developer.gnome.org/hig/stable/dialogs.html.en
	// https://developer.gnome.org/hig/stable/visual-layout.html.en

	g_return_val_if_fail(GTK_IS_WIDGET(widget),GTK_RESPONSE_NONE);

	// Check if the dialog is enabled
	gboolean allow_disabling = (popup->name && GTK_IS_V3270(widget));

	debug("%s: name=%s allow-disabling: %s", __FUNCTION__, popup->name, allow_disabling ? "Yes" : "No");

	if(allow_disabling) {

			GtkResponseType response = 0;

			v3270_signal_emit(
				widget,
				V3270_SIGNAL_LOAD_POPUP_RESPONSE,
				popup->name,
				&response
			);

			if(response && response != GTK_RESPONSE_NONE)
				return response;

			allow_disabling = (response == GTK_RESPONSE_NONE);
	}

	// Popup settings.
	static const struct _settings {
		GtkMessageType type;
		const gchar *button;
	} settings[LIB3270_NOTIFY_USER] = {

		// LIB3270_NOTIFY_INFO - Simple information dialog.
		{
			.type = GTK_MESSAGE_INFO,
			.button = N_("_Ok"),
		},

		// LIB3270_NOTIFY_WARNING - Warning message.
		{
			.type = GTK_MESSAGE_WARNING,
			.button = N_("_Ok"),
		},

		// LIB3270_NOTIFY_ERROR - Error message.
		{
			.type = GTK_MESSAGE_ERROR,
			.button = N_("_Ok"),
		},

		// LIB3270_NOTIFY_CRITICAL - Critical error, user can abort application.
		{
			.type = GTK_MESSAGE_ERROR,
			.button = N_("_Close"),
		},

		// LIB3270_NOTIFY_SECURE - Secure host dialog.
		{
			.type = GTK_MESSAGE_OTHER,
			.button = N_("_Ok"),
		}

	};

	// Create dialog
	GtkWidget * dialog =
			gtk_message_dialog_new_with_markup(
				GTK_WINDOW(gtk_widget_get_toplevel(widget)),
				GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
				settings[popup->type].type,
				GTK_BUTTONS_NONE,
				(popup->body ? "<b><big>%s</big></b>" : "%s"),
					popup->summary
			);

	if(popup->body) {
		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),"<small>%s</small>",popup->body);
	}

#ifdef _WIN32
	gtk_container_set_border_width(GTK_CONTAINER(dialog),12);
#endif // _WIN32

	if(popup->title) {

		gtk_window_set_title(GTK_WINDOW(dialog),popup->title);

	} else if(GTK_IS_V3270(widget)) {

		const gchar *url = v3270_get_url(widget);
		g_autofree gchar * title = g_strconcat(
										v3270_get_session_name(widget),
										" - ",
										url ? url : _("No host"),
										NULL
									);

		gtk_window_set_title(GTK_WINDOW(dialog),title);

	}

	if(wait) {

		GtkWidget * dont_ask = NULL;

		if(allow_disabling) {
			// Set check button
			dont_ask = gtk_check_button_new_with_label(_("Don't ask again"));

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
		}

		// Wait for response.
		if(popup->label) {

			gtk_dialog_add_button(GTK_DIALOG(dialog), _("_Cancel"), GTK_RESPONSE_CANCEL);
			gtk_dialog_add_button(GTK_DIALOG(dialog), popup->label, GTK_RESPONSE_APPLY);

			switch(popup->type) {
			case LIB3270_NOTIFY_SECURE:
			case LIB3270_NOTIFY_CRITICAL:
				gtk_dialog_set_default_response(
					GTK_DIALOG(dialog),
					GTK_RESPONSE_CANCEL
				);

				// https://developer.gnome.org/Buttons/
				v3270_dialog_add_class_for_response(dialog,GTK_RESPONSE_APPLY,"destructive-action");
				v3270_dialog_add_class_for_response(dialog,GTK_RESPONSE_CANCEL,"suggested-action");
				break;

			case LIB3270_NOTIFY_ERROR:
				gtk_dialog_set_default_response(
					GTK_DIALOG(dialog),
					GTK_RESPONSE_CANCEL
				);

				// https://developer.gnome.org/Buttons/
				v3270_dialog_add_class_for_response(dialog,GTK_RESPONSE_CANCEL,"suggested-action");
				break;

			case LIB3270_NOTIFY_INFO:
				gtk_dialog_set_default_response(
					GTK_DIALOG(dialog),
					GTK_RESPONSE_APPLY
				);

				// https://developer.gnome.org/Buttons/
				v3270_dialog_add_class_for_response(dialog,GTK_RESPONSE_APPLY,"suggested-action");
				break;

			default:
				gtk_dialog_set_default_response(
					GTK_DIALOG(dialog),
					GTK_RESPONSE_APPLY
				);
			}

		} else {

			gtk_dialog_add_button(GTK_DIALOG(dialog), g_dgettext(GETTEXT_PACKAGE,settings[popup->type].button), GTK_RESPONSE_OK);

		}

		gtk_widget_show_all(dialog);
		GtkResponseType rc = gtk_dialog_run(GTK_DIALOG(dialog));

		if(dont_ask && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dont_ask)) && rc != GTK_RESPONSE_DELETE_EVENT && rc != GTK_RESPONSE_NONE) {

			gboolean saved = FALSE;

			v3270_signal_emit(
				widget,
				V3270_SIGNAL_SAVE_POPUP_RESPONSE,
				popup->name,
				rc,
				&saved
			);

			debug("response was %s",saved ? "saved" : "not saved");

		}

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

 void v3270_error_popup(GtkWidget *widget, const gchar *title, const gchar *summary, const gchar *body) {

	LIB3270_POPUP popup = {
		.type = LIB3270_NOTIFY_ERROR,
		.title = title,
		.summary = summary,
		.body = body
	};

	v3270_popup_dialog_show(widget, &popup, FALSE);

 }

 gboolean v3270_popup_gerror(GtkWidget *widget, GError **error, const gchar *title, const gchar *fmt, ...) {

	if(!(error && *error))
		return FALSE;

	// Format message.
 	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	g_autofree gchar *summary = g_strdup_vprintf(fmt,arg_ptr);
	va_end(arg_ptr);

	LIB3270_POPUP popup = {
		.type = LIB3270_NOTIFY_ERROR,
		.title = title,
		.summary = summary,
		.body = (*error)->message
	};

	v3270_popup_dialog_show(widget, &popup, FALSE);

	g_error_free(*error);
	*error = NULL;

	return TRUE;

 }


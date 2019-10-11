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

 #include <internals.h>
 #include <lib3270.h>
 #include <lib3270/toggle.h>

 /*--[ Widget definition ]----------------------------------------------------------------------------*/

 struct _V3270ToggleButtonClass
 {
 	GtkToggleButtonClass parent_class;

 };

 struct _V3270ToggleButton
 {
 	GtkToggleButton parent;

 	H3270				* hSession;
 	LIB3270_TOGGLE_ID	  id;
 	const void			* hListener;

 };

 G_DEFINE_TYPE(V3270ToggleButton, V3270ToggleButton, GTK_TYPE_TOGGLE_BUTTON);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void dispose(GObject *object)
 {
	debug("%s",__FUNCTION__);

	V3270ToggleButton * widget = GTK_V3270_TOGGLE_BUTTON(object);

	if(widget->hListener)
	{
		lib3270_unregister_toggle_listener(widget->hSession,widget->id,widget->hListener);
		widget->hListener = NULL;
	}

	G_OBJECT_CLASS(V3270ToggleButton_parent_class)->dispose(object);

 }

 static void toggled(GtkToggleButton *toggle_button)
 {
	V3270ToggleButton * widget = GTK_V3270_TOGGLE_BUTTON(toggle_button);

	int rc = lib3270_set_toggle(widget->hSession, widget->id, gtk_toggle_button_get_active(toggle_button) ? 1 : 0);

	if(rc < 0)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(toggle_button),FALSE);
		g_warning("Can't set toggle button: %s",strerror(errno));
		return;
	}

	if(rc)
		gtk_toggle_button_toggled(toggle_button);

 }

 static void V3270ToggleButton_class_init(G_GNUC_UNUSED V3270ToggleButtonClass *klass)
 {
	GTK_TOGGLE_BUTTON_CLASS(klass)->toggled	= toggled;
	G_OBJECT_CLASS(klass)->dispose			= dispose;

 }

 static void V3270ToggleButton_init(V3270ToggleButton G_GNUC_UNUSED(*widget))
 {
 }

 static void toggle_listener(H3270 G_GNUC_UNUSED(*hSession), LIB3270_TOGGLE_ID toggle, char state, void *button)
 {
 	g_message("%s is %s\n", lib3270_get_toggle_name(toggle),(state ? "ON" : "OFF"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),(state == 0 ? FALSE : TRUE));
 }

 GtkWidget * v3270_toggle_button_new(GtkWidget *terminal, LIB3270_TOGGLE_ID toggle)
 {
 	g_return_val_if_fail(GTK_IS_V3270(terminal),NULL);

	V3270ToggleButton * widget = GTK_V3270_TOGGLE_BUTTON(g_object_new(GTK_TYPE_V3270_TOGGLE_BUTTON, NULL));

 	widget->hSession = v3270_get_session(terminal);
	widget->id = toggle;

	widget->hListener = lib3270_register_toggle_listener(widget->hSession, widget->id,toggle_listener,widget);

	gtk_widget_set_name(GTK_WIDGET(widget),lib3270_get_toggle_name(toggle));
	gtk_button_set_label(GTK_BUTTON(widget),gettext(lib3270_get_toggle_label(toggle)));
	gtk_widget_set_tooltip_text(GTK_WIDGET(widget),gettext(lib3270_get_toggle_description(toggle)));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),lib3270_get_toggle(widget->hSession, widget->id));

 	return GTK_WIDGET(widget);
 }

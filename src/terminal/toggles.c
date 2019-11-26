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
 #include <internals.h>

 #include <lib3270/toggle.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

LIB3270_EXPORT gboolean v3270_get_toggle(GtkWidget *widget, LIB3270_TOGGLE_ID ix)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);

	if(ix < LIB3270_TOGGLE_COUNT)
		return lib3270_get_toggle(GTK_V3270(widget)->host,ix) ? TRUE : FALSE;

	return FALSE;
}

LIB3270_EXPORT gboolean	v3270_set_toggle(GtkWidget *widget, LIB3270_TOGGLE_ID ix, gboolean state)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);

	if(ix < LIB3270_TOGGLE_COUNT)
		return lib3270_set_toggle(GTK_V3270(widget)->host,ix,state ? 1 : 0) ? TRUE : FALSE;

	return FALSE;

}

void v3270_update_toggle(GtkWidget *widget, LIB3270_TOGGLE_ID id, unsigned char value, const char *name)
{
 	v3270Class	* klass = GTK_V3270_GET_CLASS(widget);

 	trace("%s(%s,%d)",__FUNCTION__,name,(int) value);

	switch(id)
	{
	case LIB3270_TOGGLE_CURSOR_POS:
	case LIB3270_TOGGLE_MONOCASE:
	case LIB3270_TOGGLE_LINE_WRAP:
	case LIB3270_TOGGLE_CROSSHAIR:
	case LIB3270_TOGGLE_BLANK_FILL:
	case LIB3270_TOGGLE_MARGINED_PASTE:
	case LIB3270_TOGGLE_SHOW_TIMING:
	case LIB3270_TOGGLE_RECTANGLE_SELECT:
	case LIB3270_TOGGLE_UNDERLINE:
	case LIB3270_TOGGLE_VIEW_FIELD:
	case LIB3270_TOGGLE_ALTSCREEN:
		v3270_reload(widget);
		gtk_widget_queue_draw(widget);
		break;

	case LIB3270_TOGGLE_CURSOR_BLINK:
		GTK_V3270(widget)->cursor.show |= 1;
		break;

	case LIB3270_TOGGLE_INSERT:
		v3270_draw_ins_status(GTK_V3270(widget));
		v3270_cursor_draw(GTK_V3270(widget));
		break;

	case LIB3270_TOGGLE_BOLD:
		v3270_reconfigure(GTK_V3270(widget));
		gtk_widget_queue_draw(widget);
		break;

	case LIB3270_TOGGLE_FULL_SCREEN:
		if(value)
			gtk_window_fullscreen(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
		else
			gtk_window_unfullscreen(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
		break;

	case LIB3270_TOGGLE_DS_TRACE:
	case LIB3270_TOGGLE_SSL_TRACE:
	case LIB3270_TOGGLE_SCREEN_TRACE:
	case LIB3270_TOGGLE_EVENT_TRACE:
	case LIB3270_TOGGLE_RECONNECT:
	case LIB3270_TOGGLE_SMART_PASTE:
	case LIB3270_TOGGLE_KEEP_SELECTED:
	case LIB3270_TOGGLE_CONNECT_ON_STARTUP:
	case LIB3270_TOGGLE_KP_ALTERNATIVE:
	case LIB3270_TOGGLE_NETWORK_TRACE:
	case LIB3270_TOGGLE_BEEP:
	case LIB3270_TOGGLE_KEEP_ALIVE:
	case LIB3270_TOGGLE_COUNT:
		break;

	}

	debug("%s: pspec=%p",__FUNCTION__,klass->properties.toggle[id]);
	g_object_notify_by_pspec(G_OBJECT(widget), klass->properties.toggle[id]);
	g_signal_emit(widget, v3270_widget_signal[V3270_SIGNAL_TOGGLE_CHANGED], 0, (guint) id, (gboolean) (value != 0), (gchar *) name);

	v3270_emit_save_settings(widget);

}

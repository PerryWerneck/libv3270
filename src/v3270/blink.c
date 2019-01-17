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
 #include "private.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static gboolean blink_timer_tick(v3270 *widget)
 {
 	gboolean rc = FALSE;

	widget->blink.show 	= !widget->blink.show;

 	if(widget->script)
	{
		v3270_oia_update_text_field(widget,1,V3270_OIA_SCRIPT,widget->blink.show ? widget->script : ' ');

		rc = TRUE;
	}

	if(lib3270_get_secure(widget->host) == LIB3270_SSL_NEGOTIATING)
	{
		GdkRectangle	* r;
		cairo_t			* cr = v3270_oia_set_update_region(widget,&r,V3270_OIA_SSL);

		v3270_draw_ssl_status(widget,cr,r);
		v3270_queue_draw_area(GTK_WIDGET(widget),r->x,r->y,r->width,r->height);
		cairo_destroy(cr);

		rc = TRUE;
	}

 	/*
	if(!widget->script.id)
		return FALSE;

	update_text_field(widget,1,V3270_OIA_SCRIPT,widget->script.blink ? 'S' : ' ');
	widget->script.blink = !widget->script.blink;

	return TRUE;
	*/

 	return rc;

 }

 static void blink_timer_removed(v3270 *widget)
 {
	widget->blink.timer = NULL;
	widget->blink.show = 1;
 }

 void v3270_start_blinking(GtkWidget *widget)
 {
	g_return_if_fail(GTK_IS_V3270(widget));

	v3270 * terminal = GTK_V3270(widget);

	if(!terminal->blink.timer)
	{
		terminal->blink.show = 1;
		terminal->blink.timer = g_timeout_source_new(500);
		g_source_set_callback(terminal->blink.timer,(GSourceFunc) blink_timer_tick, terminal, (GDestroyNotify) blink_timer_removed);

		g_source_attach(terminal->blink.timer, NULL);
		g_source_unref(terminal->blink.timer);
	}

 }


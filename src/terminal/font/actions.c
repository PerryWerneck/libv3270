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

/*
static void update_font_size(v3270 * terminal, cairo_t *cr, gint width, gint height, int step)
{


	cairo_set_font_face(cr,terminal->font.face);

	cairo_set_font_size(cr,terminal->font.size);

	v3270_update_font_metrics(terminal, width, height);

	if(terminal->font.scaled)
		cairo_scaled_font_destroy(terminal->font.scaled);

	terminal->font.scaled = cairo_get_scaled_font(cr);
	cairo_scaled_font_reference(terminal->font.scaled);

	v3270_redraw(terminal, cr, width, height);

	gtk_widget_queue_draw(GTK_WIDGET(terminal));

}
*/

void v3270_zoom_best(GtkWidget *widget)
{
	debug("%s",__FUNCTION__);

	g_return_if_fail(GTK_IS_V3270(widget));
	v3270 * terminal = GTK_V3270(widget);
	if(!(gtk_widget_get_realized(widget) && terminal->drawing && lib3270_is_connected(terminal->host)))
	{
		gdk_display_beep(gdk_display_get_default());
		return;
	}

	gint width	= gtk_widget_get_allocated_width(widget);
	gint height	= gtk_widget_get_allocated_height(widget);

	cairo_t *cr = cairo_create(terminal->surface);

	v3270_compute_font_size(terminal, cr, width, height);
	v3270_update_font_metrics(terminal, width, height);

	v3270_redraw(terminal, cr, width, height);

    cairo_destroy(cr);

	gtk_widget_queue_draw(widget);

}

static void zoom(GtkWidget *widget, double step)
{
	debug("%s",__FUNCTION__);

	g_return_if_fail(GTK_IS_V3270(widget));
	v3270 * terminal = GTK_V3270(widget);
	if(!(gtk_widget_get_realized(widget) && terminal->drawing && lib3270_is_connected(terminal->host)))
	{
		gtk_widget_error_bell(widget);
		return;
	}

	terminal->font.size += step;

	// Redraw window

	gint width	= gtk_widget_get_allocated_width(widget);
	gint height	= gtk_widget_get_allocated_height(widget);

	cairo_t *cr = cairo_create(terminal->surface);

	cairo_set_font_face(cr,terminal->font.face);
	cairo_set_font_size(cr,terminal->font.size);

	v3270_update_font_metrics(terminal, width, height);

	if(terminal->font.scaled)
		cairo_scaled_font_destroy(terminal->font.scaled);

	terminal->font.scaled = cairo_get_scaled_font(cr);
	cairo_scaled_font_reference(terminal->font.scaled);

	v3270_redraw(terminal, cr, width, height);

    cairo_destroy(cr);

	gtk_widget_queue_draw(GTK_WIDGET(terminal));

}

void v3270_zoom_in(GtkWidget *widget)
{
	zoom(widget,1);
}

void v3270_zoom_out(GtkWidget *widget)
{
	debug("%s",__FUNCTION__);
	zoom(widget,-1);
}


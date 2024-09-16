/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes paul.mattes@case.edu), de emulação de terminal 3270 para acesso a
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

/*--[ Implement ]------------------------------------------------------------------------------------*/

/**
 * @brief Reconfigure widget surface.
 *
 * Called when the widget is resized or moved to regenerate the cairo surface:
 *
 * * Recreate the surface.
 * * Compute the new font size & metrics.
 * * Redraw contents.
 * * Emit "GDK_CONFIGURE" event.
 *
 */
void v3270_reconfigure(v3270 * terminal)
{
	GtkWidget *	widget = GTK_WIDGET(terminal);
	GdkWindow * window = gtk_widget_get_window(widget);

	// If the widget isn't realized just return.
	if(!window)
		return;

	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);

	GdkEvent *event = gdk_event_new(GDK_CONFIGURE);
	event->configure.window = g_object_ref(window);
	event->configure.send_event = TRUE;
	event->configure.x = allocation.x;
	event->configure.y = allocation.y;
	event->configure.width = allocation.width;
	event->configure.height = allocation.height;

	if(terminal->surface)
		cairo_surface_destroy(terminal->surface);

	terminal->surface = (cairo_surface_t *) gdk_window_create_similar_surface(window,CAIRO_CONTENT_COLOR,allocation.width,allocation.height);

	// Update the created image
	cairo_t * cr = cairo_create(terminal->surface);
	v3270_compute_font_size(terminal, cr, allocation.width, allocation.height);
	v3270_update_font_metrics(terminal, allocation.width, allocation.height);

	v3270_redraw(terminal, cr, allocation.width, allocation.height);

    cairo_destroy(cr);

#if( !GTK_CHECK_VERSION(3,0,0))
	terminal->width  = allocation.width;
	terminal->height = allocation.height;
#endif

	gtk_widget_event(widget, event);

	gdk_event_free(event);
}




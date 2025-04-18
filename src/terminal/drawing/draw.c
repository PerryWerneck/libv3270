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
 * Este programa está nomeado como draw.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifdef WIN32
	#include <winsock2.h>
	#include <windows.h>
#endif // WIN32

 #include <gtk/gtk.h>
 #include <math.h>
 #include <ctype.h>
 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <lib3270/session.h>
 #include <lib3270/toggle.h>
 #include <internals.h>
 #include <lib3270/trace.h>

 #include <v3270.h>
 #include <terminal.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

void v3270_cursor_draw(v3270 *widget)
{
	int 			pos = lib3270_get_cursor_address(widget->host);
	unsigned char	c;
	unsigned short	attr;

	lib3270_get_contents(widget->host,pos,pos,&c,&attr);
	v3270_update_cursor_surface(widget,c,attr);
	v3270_queue_draw_area(	GTK_WIDGET(widget),
							widget->cursor.rect.x,widget->cursor.rect.y,
							widget->cursor.rect.width,widget->cursor.rect.height);

}

gboolean v3270_draw(GtkWidget * widget, cairo_t * cr)
{
	v3270 * terminal = GTK_V3270(widget);

	cairo_set_source_surface(cr,terminal->surface,0,0);
	cairo_paint(cr);

	if(lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_CROSSHAIR) && (terminal->cursor.show&2))
	{
		GtkAllocation allocation;
		gtk_widget_get_allocation(widget, &allocation);

		gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_CROSS_HAIR);

		cairo_rectangle(cr,	0,terminal->cursor.rect.y+terminal->font.height,allocation.width,1);
		cairo_fill(cr);

		cairo_rectangle(cr,	terminal->cursor.rect.x,0,1,terminal->oia.rect->y-3);
		cairo_fill(cr);
	}

	if(terminal->cursor.show == 3)
	{
		cairo_set_source_surface(cr,terminal->cursor.surface,terminal->cursor.rect.x,terminal->cursor.rect.y);

		if(lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_INSERT))
		{
			cairo_rectangle(cr,	terminal->cursor.rect.x,
								terminal->cursor.rect.y,
								terminal->cursor.rect.width,
								terminal->cursor.rect.height );
		}
		else
		{
			cairo_rectangle(cr,	terminal->cursor.rect.x,
								terminal->cursor.rect.y+terminal->font.height,
								terminal->cursor.rect.width,
								terminal->font.descent );
		}

		cairo_fill(cr);
	}

	return FALSE;
}

#if( !GTK_CHECK_VERSION(3,0,0))
gboolean v3270_expose(GtkWidget *widget, GdkEventExpose *event)
{
	cairo_t *cr = gdk_cairo_create(widget->window);
	v3270_draw(widget,cr);
    cairo_destroy(cr);
    return FALSE;
}
#endif // GTk3


static void get_element_colors(unsigned short attr, GdkRGBA **fg, GdkRGBA **bg, GdkRGBA *color) {

	int index[2];

	if(attr & LIB3270_ATTR_SELECTED) {

		index[0] = V3270_COLOR_SELECTED_FG;
		index[1] = V3270_COLOR_SELECTED_BG;

//		*fg = color+V3270_COLOR_SELECTED_FG;
//		*bg = color+V3270_COLOR_SELECTED_BG;

	} else {

		if(attr & LIB3270_ATTR_FIELD)
			index[0] = (attr & 0x0003)+V3270_COLOR_FIELD;
		else
			index[0] = (attr & 0x000F);

		index[1] = ((attr & 0x00F0) >> 4);

		/*
		*bg = color+((attr & 0x00F0) >> 4);

		if(attr & LIB3270_ATTR_FIELD)
			*fg = color+(attr & 0x0003)+V3270_COLOR_FIELD;
		else
			*fg = color+(attr & 0x000F);
		*/
	}

	*fg = color+index[0];
	*bg = color+index[1];

}

void v3270_draw_element(cairo_t *cr, unsigned char chr, unsigned short attr, H3270 *session, v3270FontInfo *fontInfo, GdkRectangle *rect, GdkRGBA *color)
{
	GdkRGBA *fg;
	GdkRGBA *bg;

	get_element_colors(attr,&fg,&bg,color);
	v3270_draw_char(cr,chr,attr,session,fontInfo,rect,fg,bg);

	if(attr & LIB3270_ATTR_UNDERLINE)
	{
		cairo_scaled_font_t		* font	= cairo_get_scaled_font(cr);
		cairo_font_extents_t	  extents;
		double					  sl;

		cairo_scaled_font_extents(font,&extents);

		sl = extents.descent/3;
		if(sl < 1)
			sl = 1;

		gdk_cairo_set_source_rgba(cr,fg);

		cairo_rectangle(cr,rect->x,rect->y+sl+extents.ascent+(extents.descent/2),rect->width,sl);
		cairo_fill(cr);

		cairo_stroke(cr);
	}

}

void v3270_draw_text_at(cairo_t *cr, int x, int y, v3270FontInfo *font, const char *str) {

	size_t szText = strlen(str);

	if(szText == 1 && isspace(*str)) {
		return;
	}

	// Tem string, desenha
	cairo_status_t		 		  status;
	cairo_glyph_t				* glyphs			= NULL;
	int							  num_glyphs		= 0;
	cairo_text_cluster_t		* clusters			= NULL;
	int							  num_clusters		= 0;
	cairo_text_cluster_flags_t	  cluster_flags;
	cairo_scaled_font_t			* scaled_font		= cairo_get_scaled_font(cr);

	status = cairo_scaled_font_text_to_glyphs(
					scaled_font,
					(double) x, (double) (y+font->height),
					str, szText,
					&glyphs, &num_glyphs,
					&clusters, &num_clusters, &cluster_flags );

	if (status == CAIRO_STATUS_SUCCESS) {
		cairo_show_text_glyphs(cr,str,szText,glyphs, num_glyphs,clusters, num_clusters, cluster_flags);
	}

	if(glyphs)
		cairo_glyph_free(glyphs);

	if(clusters)
		cairo_text_cluster_free(clusters);

}

void v3270_draw_text(cairo_t *cr, const GdkRectangle *rect, v3270FontInfo *font, const char *str) {
	v3270_draw_text_at(cr,rect->x,rect->y,font,str);
}

static void draw_small_text(cairo_t *cr, const GdkRectangle *rect, v3270FontInfo *font, const char *str, int mode)
{
		cairo_status_t		 		  status;
		cairo_glyph_t				* glyphs			= NULL;
		int							  num_glyphs		= 0;
		cairo_text_cluster_t		* clusters			= NULL;
		int							  num_clusters		= 0;
		double						  y					= (double) rect->y;

		cairo_text_cluster_flags_t	  cluster_flags;
		cairo_scaled_font_t			* scaled_font		= cairo_get_scaled_font(cr);
		cairo_font_extents_t		  extents;

		cairo_save(cr);

		cairo_set_font_face(cr,font->face);
		cairo_set_font_size(cr,font->size/ 1.6);
		cairo_font_extents(cr,&extents);

		if(mode == 0)
		{
            y += ((double) extents.height);
		}
		else
		{
			y += font->height;
		}

		status = cairo_scaled_font_text_to_glyphs(
						scaled_font,
						(double) rect->x, y,
						str, 1,
						&glyphs, &num_glyphs,
						&clusters, &num_clusters, &cluster_flags );

		if (status == CAIRO_STATUS_SUCCESS) {
			cairo_show_text_glyphs(cr,str,1,glyphs, num_glyphs,clusters, num_clusters, cluster_flags);
		}

		if(glyphs)
			cairo_glyph_free(glyphs);

		if(clusters)
			cairo_text_cluster_free(clusters);

		cairo_restore(cr);

}

static gboolean draw_cg(cairo_t *cr, unsigned char chr, v3270FontInfo *font, GdkRectangle *rect)
{
	// 0x00 & 0x20 are both blank.
	if(!chr || chr == 0x20)
		return TRUE;

	if(chr >= 0xf0 && chr <= 0xf9)
	{
		char str[] = { '0' + (chr-0xF0), 0 };
		draw_small_text(cr, rect, font, str, 0);
		return TRUE;
	}

	if(chr >= 0xe1 && chr <= 0xe3)
	{
		char str[] = { '1' + (chr-0xe1), 0 };
		draw_small_text(cr, rect, font, str, 1);
		return TRUE;
	}

	const gchar * utf =  v3270_translate_cg_to_utf(chr);

	if(utf)
	{
		v3270_draw_text(cr,rect,font,utf);
		return TRUE;
	}

	debug("%s: Unknown char 0x%02x",__FUNCTION__,(int) chr);

	return FALSE;
}

void v3270_draw_char(cairo_t *cr, unsigned char chr, unsigned short attr, H3270 *session, v3270FontInfo *font, GdkRectangle *rect, GdkRGBA *fg, GdkRGBA *bg)
{
	// Clear element area
	gdk_cairo_set_source_rgba(cr,bg);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);

	// Set foreground color
	gdk_cairo_set_source_rgba(cr,fg);

	// Draw char
	if( (attr & LIB3270_ATTR_MARKER) && lib3270_get_toggle(session,LIB3270_TOGGLE_VIEW_FIELD) )
	{
		double sz = (double) rect->width;
		if(rect->height < rect->width)
			sz = (double) rect->height;

		cairo_save(cr);

		sz /= 10;

		cairo_translate(cr, rect->x + (rect->width / 2), rect->y + (rect->height / 2));
		cairo_scale(cr, sz, sz);
		cairo_arc(cr, 0., 0., 1., 0., 2 * M_PI);

		cairo_restore(cr);
	}
	else if(attr & LIB3270_ATTR_CG)
	{

		//http://www.prycroft6.com.au/misc/3270eds.html
		switch(chr)
		{
		case 0xd3: // CG 0xab, plus
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y);
			cairo_rel_line_to(cr,0,rect->height);
			cairo_move_to(cr,rect->x,rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width,0);
			break;

		case 0xa2: // CG 0x92, horizontal line
			cairo_move_to(cr,rect->x,rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width,0);
			break;

		case 0x85: // CG 0x184, vertical line
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y);
			cairo_rel_line_to(cr,0,rect->height);
			break;

		case 0xd4: // CG 0xac, LR corner ⌟
			cairo_move_to(cr,rect->x, rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width/2,0);
			cairo_rel_line_to(cr,0,-(rect->height/2));
			break;

		case 0xd5: // CG 0xad, UR corner ⌝
			cairo_move_to(cr,rect->x, rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width/2,0);
			cairo_rel_line_to(cr,0,rect->height/2);
			break;

		case 0xc5: // CG 0xa4, UL corner ⌜
			cairo_move_to(cr,rect->x+rect->width,rect->y+(rect->height/2));
			cairo_rel_line_to(cr,-(rect->width/2),0);
			cairo_rel_line_to(cr,0,(rect->height/2));
			break;

		case 0xc4: // CG 0xa3, LL corner ⌞
			cairo_move_to(cr,rect->x+rect->width,rect->y+(rect->height/2));
			cairo_rel_line_to(cr,-(rect->width/2),0);
			cairo_rel_line_to(cr,0,-(rect->height/2));
			break;

		case 0xc6: // CG 0xa5, left tee
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width/2,0);
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y);
			cairo_rel_line_to(cr,0,rect->height);
			break;

		case 0xd6: // CG 0xae, right tee
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y+(rect->height/2));
			cairo_rel_line_to(cr,-(rect->width/2),0);
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y);
			cairo_rel_line_to(cr,0,rect->height);
			break;

		case 0xc7: // CG 0xa6, bottom tee
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y+(rect->height/2));
			cairo_rel_line_to(cr,0,-(rect->height/2));
			cairo_move_to(cr,rect->x,rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width,0);
			break;

		case 0xd7: // CG 0xaf, top tee
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y+(rect->height/2));
			cairo_rel_line_to(cr,0,rect->height/2);
			cairo_move_to(cr,rect->x,rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width,0);
			break;

		case 0x90: // APL FUNCTIONAL SYMBOL QUAD
			cairo_rectangle(cr, rect->x+1, rect->y+1, rect->width-2, rect->height-2);
			break;

		default:

			if(!draw_cg(cr, chr, font, rect))
			{
				lib3270_write_screen_trace(session,"I don't known how to draw CG character %02x\n",(int) chr);
				cairo_rectangle(cr, rect->x+1, rect->y+1, rect->width-2, rect->height-2);

			}

		}
	}
	else if(chr)
	{
		gchar *utf = g_convert((char *) &chr, 1, "UTF-8", lib3270_get_display_charset(session), NULL, NULL, NULL);

		if(utf)
		{
			v3270_draw_text(cr,rect,font,utf);
			g_free(utf);
		}
	}

	cairo_stroke(cr);
}

#if !GTK_CHECK_VERSION(2, 22, 0)
cairo_surface_t *gdk_window_create_similar_surface(GdkWindow *window, cairo_content_t content, int width, int height)
{
	cairo_t *cairoContext = gdk_cairo_create(window);
	cairo_surface_t *cairoSurface = cairo_get_target(cairoContext);
	cairo_surface_t *newSurface = cairo_surface_create_similar(cairoSurface, content, width, height);
	cairo_destroy(cairoContext);
	return newSurface;
}
#endif // GTK_CHECK_VERSION(2, 22, 0)

/// @brief Draw terminal contents.
///
/// @param terminal	Terminal widget.
/// @param cr		a cairo context.
/// @param width	the width of the rectangle.
/// @param height	the height of the rectangle.
///
void v3270_redraw(v3270 *terminal, cairo_t * cr, gint width, gint height)
{
	unsigned int rows, cols, r;
	GdkRectangle rect;
	int addr, cursor;

	gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_BACKGROUND);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);
	cairo_stroke(cr);

	lib3270_get_screen_size(terminal->host,&rows,&cols);

	memset(&rect,0,sizeof(rect));
	rect.y		= terminal->font.margin.top;
	rect.width	= terminal->font.width;
	rect.height	= terminal->font.spacing.value;
	addr 		= 0;
	cursor		= lib3270_get_cursor_address(terminal->host);

	cairo_set_scaled_font(cr,terminal->font.scaled);

	for(r = 0; r < rows; r++)
	{
		unsigned int c;

		rect.x = terminal->font.margin.left;

		for(c=0;c < cols;c++)
		{
			struct v3270_character element = { 0, 0 };

			lib3270_get_contents(terminal->host,addr,addr,&element.chr,&element.attr);

			if(addr == cursor)
				v3270_update_cursor_rect(terminal,&rect,&element);

			v3270_draw_element(cr,element.chr,element.attr,terminal->host,&terminal->font,&rect,terminal->color);

			addr++;
			rect.x += rect.width;
		}

		rect.y += terminal->font.spacing.value;

	}

	v3270_draw_oia(terminal, cr, rect.y, cols);

}

LIB3270_EXPORT void v3270_reload(GtkWidget *widget)
{
	v3270 * terminal = GTK_V3270(widget);

	if(!(gtk_widget_get_realized(widget) && terminal->drawing))
		return;

	gint width	= gtk_widget_get_allocated_width(widget);
	gint height	= gtk_widget_get_allocated_height(widget);

	cairo_t * cr = cairo_create(terminal->surface);

	v3270_redraw(terminal, cr, width, height);

    cairo_destroy(cr);

}

GdkPixbuf * v3270_get_as_pixbuf(GtkWidget *widget)
{
	v3270 * terminal = GTK_V3270(widget);

	if(!(gtk_widget_get_realized(widget) && terminal->drawing))
		return NULL;

	gint width	= gtk_widget_get_allocated_width(widget);
	gint height	= gtk_widget_get_allocated_height(widget);

	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

	cairo_t *cr = cairo_create(surface);
	v3270_redraw(terminal, cr, width, height);
	cairo_destroy (cr);

	GdkPixbuf * pixbuf = gdk_pixbuf_get_from_surface(surface,0,0,width,height);

	cairo_surface_destroy (surface);

	return pixbuf;
}

void v3270_update_char(H3270 *session, int addr, unsigned char chr, unsigned short attr, unsigned char cursor)
{
	v3270					* terminal = GTK_V3270(lib3270_get_user_data(session));
	cairo_t					* cr;
	GdkRectangle			  rect;
	unsigned int	  		  rows,cols;
	struct v3270_character	  element;

	element.chr		= chr;
	element.attr	= attr;

	if(!(gtk_widget_get_realized(GTK_WIDGET(terminal)) && terminal->drawing))
		return;

	if(!terminal->surface)
	{
		v3270_reconfigure(terminal);
		gtk_widget_queue_draw(GTK_WIDGET(terminal));
		return;
	}

	lib3270_get_screen_size(terminal->host,&rows,&cols);

	memset(&rect,0,sizeof(rect));
	rect.x          = terminal->font.margin.left + ((addr % cols) * terminal->font.width);
	rect.y          = terminal->font.margin.top  + ((addr / cols) * terminal->font.spacing.value);
	rect.width      = terminal->font.width;
	rect.height     = terminal->font.spacing.value;

	cr = cairo_create(terminal->surface);
	cairo_set_scaled_font(cr,terminal->font.scaled);
	v3270_draw_element(cr, chr, attr, terminal->host, &terminal->font, &rect,terminal->color);
    cairo_destroy(cr);

	if(cursor)
		v3270_update_cursor_rect(terminal,&rect,&element);

	v3270_queue_draw_area(GTK_WIDGET(terminal),rect.x,rect.y,rect.width,rect.height);

}

void v3270_update_cursor_surface(v3270 *widget,unsigned char chr,unsigned short attr)
{
	if(widget->cursor.surface && widget->drawing)
	{
		GdkRectangle	  rect	= widget->cursor.rect;
		cairo_t			* cr 	= cairo_create(widget->cursor.surface);
		GdkRGBA		* fg;
		GdkRGBA 		* bg;

		get_element_colors(attr,&fg,&bg,widget->color);

		cairo_set_scaled_font(cr,widget->font.scaled);

		rect.x = 0;
		rect.y = 0;
		v3270_draw_char(cr,chr,attr,widget->host,&widget->font,&rect,bg,fg);

		cairo_destroy(cr);
	}


}

void v3270_update_cursor_rect(v3270 *widget, GdkRectangle *rect, const struct v3270_character *element)
{
	widget->cursor.chr  = element->chr;
	widget->cursor.rect = *rect;
	widget->cursor.attr = element->attr;
	widget->cursor.rect.height = widget->font.height + widget->font.descent;
	v3270_update_cursor_surface(widget,element->chr,element->attr);
}

void v3270_queue_draw_area(GtkWidget *widget, gint x, gint y, gint width, gint height)
{

	if(GTK_V3270(widget)->drawing && gtk_widget_get_realized(widget))
	{
		gtk_widget_queue_draw_area(widget,x,y,width,height);
	}

}

void v3270_disable_updates(GtkWidget *widget)
{
	GTK_V3270(widget)->drawing = 0;
}

void v3270_enable_updates(GtkWidget *widget)
{
	if(gtk_widget_get_realized(widget))
	{
		GTK_V3270(widget)->drawing = 1;
		v3270_reload(widget);
		gtk_widget_queue_draw(widget);
	}
}

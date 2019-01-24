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
 * Este programa está nomeado como oia.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <config.h>

 #define ENABLE_NLS
 #define GETTEXT_PACKAGE PACKAGE_NAME

 #include <gtk/gtk.h>
 #include <libintl.h>
 #include <glib/gi18n.h>

 #ifdef WIN32
	#include <winsock2.h>
	#include <windows.h>
	#include <ws2tcpip.h>
 #endif // WIN32

 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/log.h>
 #include <config.h>
 #include <string.h>
 #include <errno.h>
 #include <ctype.h>

 #ifdef HAVE_LIBM
	#include <math.h>
 #endif // HAVE_LIBM

 #include <v3270.h>
 #include "private.h"
 #include <v3270/accessible.h>

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

static void draw_cursor_position(cairo_t *cr, GdkRectangle *rect, v3270FontInfo *metrics, int row, int col);

/*--[ Statics ]--------------------------------------------------------------------------------------*/

 #include "xbm/locked.xbm"
 #include "xbm/unlocked.xbm"
 #include "xbm/negotiated.xbm"
 #include "xbm/warning.xbm"

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void short2string(char *ptr, unsigned short vlr, size_t sz)
{
	int f;

	for(f=sz-1;f>=0;f--)
	{
		ptr[f] = '0'+(vlr%10);
		vlr /= 10;
	}
}


#ifdef HAVE_LIBM
static gint draw_spinner(cairo_t *cr, GdkRectangle *r, GdkRGBA *color, gint step)
{
	static const guint num_steps	= 10;

	gdouble dx = r->width/2;
	gdouble dy = r->height/2;
	gdouble radius = MIN (r->width / 2, r->height / 2);
	gdouble half = num_steps / 2;
	gint i;

	cairo_save(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	cairo_rectangle(cr, r->x, r->y, r->width, r->height);
	cairo_clip(cr);
	cairo_translate(cr, r->x, r->y);

	step++;
	step %= num_steps;

	debug("%s step=%d",__FUNCTION__,step);

	for (i = 0; i < ((gint) num_steps); i++)
	{
		gint inset = 0.7 * radius;

		/* transparency is a function of time and intial value */
		gdouble t = (gdouble) ((i + num_steps - step) % num_steps) / num_steps;

		cairo_save(cr);

		cairo_set_source_rgba (cr,
							 color[V3270_COLOR_OIA_SPINNER].red,
							 color[V3270_COLOR_OIA_SPINNER].green,
							 color[V3270_COLOR_OIA_SPINNER].blue,
							 t);

		cairo_set_line_width (cr, 2.0);
		cairo_move_to (cr,
					 dx + (radius - inset) * cos (i * G_PI / half),
					 dy + (radius - inset) * sin (i * G_PI / half));
		cairo_line_to (cr,
					 dx + radius * cos (i * G_PI / half),
					 dy + radius * sin (i * G_PI / half));
		cairo_stroke (cr);

		cairo_restore (cr);
	}

	cairo_restore(cr);

 	return step;
}
#endif // HAVE_LIBM

static void setup_cursor_position(GdkRectangle *rect, v3270FontInfo *metrics, cairo_t *cr, H3270 *host, int cols, G_GNUC_UNUSED GdkRGBA *color)
{
	rect->width = metrics->width * 8;
	rect->x -= rect->width;

	if(lib3270_get_toggle(host,LIB3270_TOGGLE_CURSOR_POS))
	{
		int addr = lib3270_get_cursor_address(host);
		draw_cursor_position(cr,rect,metrics,addr/cols,addr%cols);
	}
}

static void setup_ticking_position(GdkRectangle *rect, G_GNUC_UNUSED v3270FontInfo *metrics, cairo_t *cr, G_GNUC_UNUSED H3270 *host, G_GNUC_UNUSED int cols, G_GNUC_UNUSED GdkRGBA *color)
{
	char buffer[7];
	cairo_text_extents_t extents;

	short2string(buffer,0,2);
	buffer[2] = ':';
	short2string(buffer+3,0,2);
	buffer[5] = 0;

	cairo_text_extents(cr,buffer,&extents);
	rect->width = ((int) extents.width + 2);
	rect->x -= rect->width;
}

static void setup_spinner_position(GdkRectangle *rect, G_GNUC_UNUSED v3270FontInfo *metrics, G_GNUC_UNUSED cairo_t *cr, G_GNUC_UNUSED H3270 *host, G_GNUC_UNUSED int cols, G_GNUC_UNUSED GdkRGBA *color)
{
	rect->width = rect->height;
	rect->x -= rect->width;
//	draw_spinner(cr,rect,color,0);
}

static void setup_luname_position(GdkRectangle *rect, v3270FontInfo *font, cairo_t *cr, H3270 *host, G_GNUC_UNUSED int cols, GdkRGBA *color)
{
	const char *luname = lib3270_get_luname(host);

	rect->width *= 16;
	rect->x -= rect->width;

	cairo_save(cr);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_clip(cr);

#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
#else
	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_BACKGROUND);
#endif

	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);

	if(luname)
	{
		gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_LUNAME);
		v3270_draw_text(cr,rect,font,luname);
	}

	cairo_restore(cr);

}

static void setup_single_char_right(GdkRectangle *rect, G_GNUC_UNUSED v3270FontInfo *metrics, G_GNUC_UNUSED cairo_t *cr, G_GNUC_UNUSED H3270 *host, G_GNUC_UNUSED int cols, G_GNUC_UNUSED GdkRGBA *color)
{
	rect->x -= rect->width;

	/*
#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);
#endif
*/

}

static void setup_insert_position(GdkRectangle *rect, G_GNUC_UNUSED  v3270FontInfo *metrics, cairo_t *cr, G_GNUC_UNUSED H3270 *host, G_GNUC_UNUSED int cols, G_GNUC_UNUSED GdkRGBA *color)
{
	if(rect->width > rect->height)
	{
		rect->width = rect->height;
	}
	else if(rect->height > rect->width)
	{
		rect->y += (rect->height - rect->width)/2;
		rect->height = rect->width;
	}

	rect->x -= rect->width;

#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);
#endif

}



static void setup_double_char_position(GdkRectangle *rect, G_GNUC_UNUSED v3270FontInfo *metrics, cairo_t *cr, G_GNUC_UNUSED H3270 *host, G_GNUC_UNUSED int cols, G_GNUC_UNUSED GdkRGBA *color)
{
	rect->width <<= 1;
	rect->x -= rect->width;

#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);
#endif

}

static int draw_centered_char(cairo_t *cr, v3270FontInfo *metrics, int x, int y, const gchar chr)
{
	char str[2] = { chr, 0 };
//	cairo_text_extents_t extents;

	cairo_set_scaled_font(cr,metrics->scaled);
//	cairo_text_extents(cr,str,&extents);

	v3270_draw_text_at(cr, x, y, metrics, str);

	return y+metrics->height+2;

}

static void draw_undera(cairo_t *cr, H3270 *host, v3270FontInfo *metrics, GdkRGBA *color, GdkRectangle *rect)
{
	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_BACKGROUND);

	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);

	if(lib3270_get_undera(host))
	{
		int y;

		gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_FOREGROUND);

		y = draw_centered_char(cr, metrics,rect->x,rect->y, lib3270_in_e(host) ? 'B' : 'A');

		cairo_move_to(cr,rect->x,y);
		cairo_rel_line_to(cr,10,0);
		cairo_stroke(cr);

	}
	debug("%s",__FUNCTION__);

}

void v3270_draw_connection(cairo_t *cr, H3270 *host, v3270FontInfo *metrics, GdkRGBA *color, const GdkRectangle *rect)
{
 	gchar str = ' ';

	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_BACKGROUND);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);

	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_FOREGROUND);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_stroke(cr);

	if(lib3270_get_oia_box_solid(host))
	{
		cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
		cairo_fill(cr);
		return;
	}

	if(lib3270_in_ansi(host))
		str = 'N';
	else if(lib3270_in_sscp(host))
		str = 'S';
	else
		str = '?';

	draw_centered_char(cr,metrics,rect->x,rect->y,str);

}

static void draw_xbm(cairo_t *cr, GdkRectangle *rect, int width, int height, unsigned char *bits)
{
	double			  sz	= rect->width < rect->height ? rect->width : rect->height;
	cairo_surface_t	* icon	= cairo_image_surface_create_for_data(
									bits,
									CAIRO_FORMAT_A1,
									width,height,
									cairo_format_stride_for_width(CAIRO_FORMAT_A1,width));

	cairo_save(cr);

	cairo_scale(cr,	sz / ((double) width),
					sz / ((double) height));

	cairo_mask_surface(cr,icon,(rect->width-sz)/2,(rect->height-sz)/2);

	cairo_surface_destroy(icon);

	cairo_restore(cr);
}

void v3270_draw_ssl_status(v3270 *widget, cairo_t *cr, GdkRectangle *rect)
{
	// v3270_draw_ssl_status(cr,terminal->host,&terminal->font,terminal->color,r);

	gdk_cairo_set_source_rgba(cr,widget->color+V3270_COLOR_OIA_BACKGROUND);

	cairo_translate(cr, rect->x, rect->y);
	cairo_rectangle(cr, 0, 0, rect->width, rect->height);
	cairo_fill(cr);

	switch(lib3270_get_secure(widget->host))
	{
	case LIB3270_SSL_UNDEFINED:			// Undefined.
		break;

	case LIB3270_SSL_UNSECURE:			// No secure connection
		gdk_cairo_set_source_rgba(cr,widget->color+V3270_COLOR_OIA_FOREGROUND);
		draw_xbm(cr,rect,unlocked_width,unlocked_height,unlocked_bits);
		break;

	case LIB3270_SSL_NEGOTIATING:		// Negotiating SSL
		if(widget->blink.show)
		{
			gdk_cairo_set_source_rgba(cr,widget->color+V3270_COLOR_OIA_STATUS_WARNING);
			draw_xbm(cr,rect,negotiated_width,negotiated_height,negotiated_bits);
		}
		break;

	case LIB3270_SSL_NEGOTIATED:		// Connection secure, no CA, self-signed or expired CRL
		gdk_cairo_set_source_rgba(cr,widget->color+V3270_COLOR_OIA_STATUS_OK);
		draw_xbm(cr,rect,locked_width,locked_height,locked_bits);
		gdk_cairo_set_source_rgba(cr,widget->color+V3270_COLOR_OIA_STATUS_WARNING);
		draw_xbm(cr,rect,warning_width,warning_height,warning_bits);
		break;

	case LIB3270_SSL_SECURE:			// Connection secure with CA check
		gdk_cairo_set_source_rgba(cr,widget->color+V3270_COLOR_OIA_STATUS_OK);
		draw_xbm(cr,rect,locked_width,locked_height,locked_bits);
		break;

	}


}

static void draw_status_message(cairo_t *cr, LIB3270_MESSAGE id, v3270FontInfo *font, GdkRGBA *color, const GdkRectangle *r)
{
	#ifdef DEBUG
		#define OIA_MESSAGE(x,c,y) { #x, c, y }
	#else
		#define OIA_MESSAGE(x,c,y) { c, y }
	#endif

	static const struct _message
	{
	#ifdef DEBUG
		const gchar			* dbg;
	#endif
		enum V3270_COLOR
			  color;
		const gchar			* msg;
	} message[] =
 	{
		OIA_MESSAGE(	LIB3270_MESSAGE_NONE,
						V3270_COLOR_OIA_STATUS_OK,
						NULL ),

		OIA_MESSAGE(	LIB3270_MESSAGE_SYSWAIT,
						V3270_COLOR_OIA_STATUS_OK,
						N_( "X System" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_TWAIT,
						V3270_COLOR_OIA_STATUS_OK,
						N_( "X Wait" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_CONNECTED,
						V3270_COLOR_OIA_STATUS_OK,
						NULL ),

		OIA_MESSAGE(	LIB3270_MESSAGE_DISCONNECTED,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X Not Connected" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_AWAITING_FIRST,
						V3270_COLOR_OIA_STATUS_OK,
						N_( "X" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_MINUS,
						V3270_COLOR_OIA_STATUS_OK,
						N_( "X -f" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_PROTECTED,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X Protected" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_NUMERIC,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X Numeric" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_OVERFLOW,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X Overflow" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_INHIBIT,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X Inhibit" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_KYBDLOCK,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X") ),

		OIA_MESSAGE(	LIB3270_MESSAGE_X,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_RESOLVING,
						V3270_COLOR_OIA_STATUS_WARNING,
						N_( "X Resolving" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_CONNECTING,
						V3270_COLOR_OIA_STATUS_WARNING,
						N_( "X Connecting" ) ),


	};

	GdkRectangle	  rect;
	const gchar		* msg = message[0].msg;

	memcpy(&rect,r,sizeof(GdkRectangle));

	if(id >= 0 && id < G_N_ELEMENTS(message))
	{
		msg = message[id].msg;
#ifdef DEBUG
		if(!msg)
			msg = message[id].dbg;
#endif // DEBUG
	}

	// Limpa o bloco
	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_BACKGROUND);
	cairo_rectangle(cr, rect.x, rect.y, rect.width, rect.height);
	cairo_fill(cr);

	if(msg && *msg)
	{
		msg = gettext(msg);
	}

	if(msg && *msg)
	{
		rect.x++;

		debug("%s(%s)",__FUNCTION__,msg);

		gdk_cairo_set_source_rgba(cr,color+message[id].color);

		if(*msg == 'X')
		{
			cairo_save(cr);

			cairo_move_to(cr,rect.x+1,rect.y+(font->height)-(font->ascent));
			cairo_rel_line_to(cr,font->width,font->ascent);
			cairo_rel_move_to(cr,-font->width,0);
			cairo_rel_line_to(cr,font->width,-font->ascent);

			cairo_stroke(cr);
			rect.x += font->width;
			msg++;

			cairo_restore(cr);
		}

		while(isspace(*msg))
		{
			msg++;
			rect.x += font->width;
		}

		if(*msg)
		{
			v3270_draw_text(cr,&rect, font, msg);
//			cairo_move_to(cr,x,rect->y+metrics->height);
//			cairo_show_text(cr,msg);
		}

	}

}

static void draw_insert(cairo_t *cr, H3270 *host, GdkRGBA *color, GdkRectangle *rect)
{
	if(lib3270_get_toggle(host,LIB3270_TOGGLE_INSERT))
	{
		double y = rect->y+(rect->height-2);

		cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
		cairo_clip(cr);

		gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_FOREGROUND);

		cairo_move_to(cr,rect->x,y);
		cairo_rel_line_to(cr,rect->width/2,-(rect->height/1.7));
		cairo_line_to(cr,rect->x+rect->width,y);
		cairo_stroke(cr);
	}

}

// v3270_draw_oia(cr, terminal->host, rect.y, cols, &terminal->font, terminal->color,terminal->oia_rect);
// void v3270_draw_oia(cairo_t *cr, H3270 *host, int row, int cols, v3270FontInfo *metrics, GdkRGBA *color, GdkRectangle *rect)

void v3270_draw_oia(v3270 *terminal, cairo_t *cr, int row, int cols)
{
	static const struct _right_fields
	{
		V3270_OIA_FIELD id;
		void (*draw)(GdkRectangle *rect, v3270FontInfo *metrics, cairo_t *cr, H3270 *host, int cols, GdkRGBA *color);
	} right[] =
	{
		{ V3270_OIA_CURSOR_POSITION,	setup_cursor_position 		},
		{ V3270_OIA_TIMER,				setup_ticking_position		},
		{ V3270_OIA_SPINNER, 			setup_spinner_position		},
		{ V3270_OIA_LUNAME, 			setup_luname_position		},
#ifdef HAVE_PRINTER
		{ V3270_OIA_PRINTER,			setup_single_char_right		},
#endif // HAVE_PRINTER
		{ V3270_OIA_SCRIPT,				setup_single_char_right		},
		{ V3270_OIA_INSERT,				setup_insert_position		},
		{ V3270_OIA_TYPEAHEAD,			setup_single_char_right		},
		{ V3270_OIA_SHIFT,				setup_double_char_position	},
//		{ V3270_OIA_CAPS,				setup_single_char_right		},
		{ V3270_OIA_ALT,				setup_single_char_right		},
		{ V3270_OIA_SSL,				setup_double_char_position	},
	};

	int f;
	int rCol = terminal->font.left+(cols*terminal->font.width);
	int lCol = terminal->font.left+1;

	row += OIA_TOP_MARGIN;
	gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_OIA_SEPARATOR);
	cairo_rectangle(cr, terminal->font.left, row, cols*terminal->font.width, 1);
	cairo_fill(cr);

	row += 2;

	gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_OIA_BACKGROUND);
	cairo_rectangle(cr, terminal->font.left, row, cols*terminal->font.width, terminal->font.spacing);
	cairo_fill(cr);

	for(f=0;f< (int) G_N_ELEMENTS(right);f++)
	{
		GdkRectangle *r = terminal->oia.rect+right[f].id;

		memset(r,0,sizeof(GdkRectangle));
		r->x = rCol;
		r->y = row;
		r->width  = terminal->font.width;
		r->height = terminal->font.spacing;
		gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_OIA_FOREGROUND);
		right[f].draw(r,&terminal->font,cr,terminal->host,cols,terminal->color);
		rCol = r->x - (terminal->font.width/3);
	}

	gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_OIA_FOREGROUND);

	draw_centered_char(cr,&terminal->font,lCol,row,'4');

	cairo_stroke(cr);
	cairo_rectangle(cr, lCol, row, terminal->font.width+2, terminal->font.spacing);
	cairo_stroke(cr);

	lCol += (terminal->font.width+5);

	// Undera indicator
	terminal->oia.rect[V3270_OIA_UNDERA].x = lCol;
	terminal->oia.rect[V3270_OIA_UNDERA].y = row;
	terminal->oia.rect[V3270_OIA_UNDERA].width  = terminal->font.width+3;
	terminal->oia.rect[V3270_OIA_UNDERA].height = terminal->font.spacing;
	draw_undera(cr,terminal->host,&terminal->font,terminal->color,terminal->oia.rect+V3270_OIA_UNDERA);

	lCol += (3 + terminal->oia.rect[V3270_OIA_UNDERA].width);

	// Connection indicator
	terminal->oia.rect[V3270_OIA_CONNECTION].x = lCol;
	terminal->oia.rect[V3270_OIA_CONNECTION].y = row;
	terminal->oia.rect[V3270_OIA_CONNECTION].width  = terminal->font.width+3;
	terminal->oia.rect[V3270_OIA_CONNECTION].height = terminal->font.spacing;
	v3270_draw_connection(cr,terminal->host,&terminal->font,terminal->color,terminal->oia.rect+V3270_OIA_CONNECTION);

	lCol += (4 + terminal->oia.rect[V3270_OIA_CONNECTION].width);

	memset(terminal->oia.rect+V3270_OIA_MESSAGE,0,sizeof(GdkRectangle));

	if(lCol < rCol)
	{
		GdkRectangle *r = terminal->oia.rect+V3270_OIA_MESSAGE;
		r->x = lCol;
		r->y = row;
		r->width  = rCol - lCol;
		r->height = terminal->font.spacing;
		draw_status_message(cr,lib3270_get_program_message(terminal->host),&terminal->font,terminal->color,r);
	}

	cairo_save(cr);
//	v3270_draw_ssl_status(cr,terminal->host,&terminal->font,terminal->color,terminal->oia.rect+V3270_OIA_SSL);
	v3270_draw_ssl_status(terminal,cr,terminal->oia.rect+V3270_OIA_SSL);
	cairo_restore(cr);

	cairo_save(cr);
	draw_insert(cr,terminal->host,terminal->color,terminal->oia.rect+V3270_OIA_INSERT);
	cairo_restore(cr);
}

/**
 * Begin update of a specific OIA field.
 *
 * @param terminal	3270 terminal widget.
 * @param r			Rectangle to receive updated region.
 * @param id		Field id.
 *
 * @return cairo object for drawing.
 *
 */
cairo_t * v3270_oia_set_update_region(v3270 * terminal, GdkRectangle **r, V3270_OIA_FIELD id)
{
	GdkRectangle	* rect		= terminal->oia.rect + id;
	cairo_t 		* cr		= cairo_create(terminal->surface);

	cairo_set_scaled_font(cr,terminal->font.scaled);

	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_clip(cr);

	*r = rect;

#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
#else
	gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_OIA_BACKGROUND);
#endif

	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);

	gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_OIA_FOREGROUND);

	return cr;
}

void v3270_update_luname(GtkWidget *widget,const gchar *name)
{
	cairo_t 		* cr;
	GdkRectangle	* rect;
	v3270			* terminal = GTK_V3270(widget);

	if(terminal->surface)
	{
		cr = v3270_oia_set_update_region(terminal,&rect,V3270_OIA_LUNAME);

		if(name)
		{
//			cairo_move_to(cr,rect->x,rect->y+terminal->font.height);
//			cairo_show_text(cr,name);
//			cairo_stroke(cr);
			gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_OIA_LUNAME);
			v3270_draw_text_at(cr, rect->x, rect->y, &terminal->font, name);

		}

		cairo_destroy(cr);

		v3270_queue_draw_area(GTK_WIDGET(terminal),rect->x,rect->y,rect->width,rect->height);
	}

	g_object_notify_by_pspec(G_OBJECT(widget), v3270_properties.luname);

}

void v3270_update_message(v3270 *widget, LIB3270_MESSAGE id)
{
	cairo_t 		* cr;
	GdkRectangle	* rect;

	if(!widget->surface)
		return;

	cr = v3270_oia_set_update_region(widget,&rect,V3270_OIA_MESSAGE);

	draw_status_message(cr,id,&widget->font,widget->color,rect);

    cairo_destroy(cr);

	v3270_queue_draw_area(GTK_WIDGET(widget),rect->x,rect->y,rect->width,rect->height);

	if(widget->accessible)
		v3270_acessible_set_state(widget->accessible,id);

}

static void draw_cursor_position(cairo_t *cr, GdkRectangle *rect, v3270FontInfo *metrics, int row, int col)
{
	cairo_text_extents_t extents;
	char buffer[10];

	short2string(buffer,row+1,3);
	buffer[3] = '/';
	short2string(buffer+4,col+1,3);
	buffer[7] = 0;

	cairo_text_extents(cr,buffer,&extents);
	v3270_draw_text_at(cr,(rect->x+rect->width)-(extents.width+2),rect->y,metrics,buffer);
}

void v3270_update_cursor(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr)
{
	v3270				* terminal = GTK_V3270(lib3270_get_user_data(session));
	GdkRectangle		  saved;

	if(!terminal->surface)
		return;

	// Update cursor rectangle
	saved = terminal->cursor.rect;

	terminal->cursor.rect.x          = terminal->font.left + (col * terminal->cursor.rect.width);
	terminal->cursor.rect.y          = terminal->font.top  + (row * terminal->font.spacing);
	terminal->cursor.rect.width      = terminal->font.width;
	terminal->cursor.rect.height     = terminal->font.height+terminal->font.descent;
	terminal->cursor.show |= 1;

	v3270_queue_draw_area( GTK_WIDGET(terminal),	saved.x,
													saved.y,
													saved.width,
													saved.height);


	v3270_update_cursor_surface(terminal,c,attr);

	v3270_queue_draw_area(	GTK_WIDGET(terminal),
							terminal->cursor.rect.x,terminal->cursor.rect.y,
							terminal->cursor.rect.width,terminal->cursor.rect.height);

	if(lib3270_get_toggle(session,LIB3270_TOGGLE_CROSSHAIR))
	{
		GtkAllocation allocation;
		gtk_widget_get_allocation(GTK_WIDGET(terminal), &allocation);

		v3270_queue_draw_area(GTK_WIDGET(terminal),0,saved.y+terminal->font.height,allocation.width,1);
		v3270_queue_draw_area(GTK_WIDGET(terminal),saved.x,0,1,terminal->oia.rect->y-3);

		v3270_queue_draw_area(GTK_WIDGET(terminal),0,terminal->cursor.rect.y+terminal->font.height,allocation.width,1);
		v3270_queue_draw_area(GTK_WIDGET(terminal),terminal->cursor.rect.x,0,1,terminal->oia.rect->y-3);
	}

	if(lib3270_get_toggle(session,LIB3270_TOGGLE_CURSOR_POS))
	{
		// Update OIA
		GdkRectangle	* rect;
		cairo_t 		* cr;

		cr = v3270_oia_set_update_region(terminal,&rect,V3270_OIA_CURSOR_POSITION);

		draw_cursor_position(cr,rect,&terminal->font,row,col);

		cairo_destroy(cr);

		v3270_queue_draw_area(GTK_WIDGET(terminal),rect->x,rect->y,rect->width,rect->height);
	}

	if(terminal->accessible)
		g_signal_emit_by_name(ATK_TEXT(terminal->accessible),"text-caret-moved",lib3270_get_cursor_address(session));

}

struct timer_info
{
	time_t	  start;
	time_t	  last;
#ifdef HAVE_LIBM
	gint	  step;
#endif // HAVE_LIBM
	v3270	* terminal;
};

static void release_timer(struct timer_info *info)
{
	info->terminal->timer = NULL;

	if(info->terminal->surface)
	{
		// Erase timer info
		static const int id[] = {	V3270_OIA_TIMER,
#ifdef HAVE_LIBM
									V3270_OIA_SPINNER
#endif // HAVE_LIBM
								};
		int f;

		cairo_t *cr = cairo_create(info->terminal->surface);

#ifdef DEBUG
		cairo_set_source_rgb(cr,0.1,0.1,0.1);
#else
		gdk_cairo_set_source_rgba(cr,info->terminal->color+V3270_COLOR_OIA_BACKGROUND);
#endif

		for(f=0;f< (int) G_N_ELEMENTS(id);f++)
		{
			GdkRectangle *rect = info->terminal->oia.rect + id[f];
			cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
			cairo_fill(cr);
			v3270_queue_draw_area(GTK_WIDGET(info->terminal),rect->x,rect->y,rect->width,rect->height);
		}
		cairo_destroy(cr);
	}

	g_free(info);
}

void v3270_draw_shift_status(v3270 *terminal)
{
	GdkRectangle *r;
	cairo_t *cr;

	if(!terminal->surface)
		return;

	cr = v3270_oia_set_update_region(terminal,&r,V3270_OIA_SHIFT);
	cairo_translate(cr, r->x, r->y+1);

	if(r->width > 2 && r->height > 7 && (terminal->keyflags & KEY_FLAG_SHIFT))
	{
		int b,x,y,w,h,l;
		int height = r->height-6;

		if(height > r->width)
		{
			w = r->width;
			h = w*1.5;
		}
		else // width > height
		{
			h = height;
			w = h/1.5;
		}

		// Set image position
		x = (r->width - w)/2;
		y = (height - h)/2;
		l = (w/3);
		b = y+(w/1.5);

		cairo_move_to(cr,x+(w/2),y);
		cairo_line_to(cr,x+w,b);
		cairo_line_to(cr,(x+w)-l,b);
		cairo_line_to(cr,(x+w)-l,y+h);
		cairo_line_to(cr,x+l,y+h);
		cairo_line_to(cr,x+l,b);
		cairo_line_to(cr,x,b);
		cairo_close_path(cr);

		cairo_stroke(cr);

	}

    cairo_destroy(cr);
	v3270_queue_draw_area(GTK_WIDGET(terminal),r->x,r->y,r->width,r->height);

}

void v3270_oia_update_text_field(v3270 *terminal, gboolean flag, V3270_OIA_FIELD id, const gchar chr)
{
	GdkRectangle	* r;
	cairo_t 		* cr;
	gchar			  text[] = { chr, 0 };

	if(!terminal->surface)
		return;

	cr = v3270_oia_set_update_region(terminal,&r,id);
	cairo_translate(cr, r->x, r->y);

	if(flag)
	{
		v3270_draw_text_at(cr,0,0,&terminal->font,text);
//		cairo_move_to(cr,0,terminal->font.height);
//		cairo_show_text(cr, text);
//		cairo_stroke(cr);
	}

    cairo_destroy(cr);
	v3270_queue_draw_area(GTK_WIDGET(terminal),r->x,r->y,r->width,r->height);
}

#ifdef KEY_FLAG_ALT
void v3270_draw_alt_status(v3270 *terminal)
{
	v3270_oia_update_text_field(terminal,terminal->keyflags & KEY_FLAG_ALT,V3270_OIA_ALT,'A');
}
#else
void v3270_draw_alt_status(v3270 G_GNUC_UNUSED(*terminal))
{
}
#endif // KEY_FLAG_ALT

void v3270_draw_ins_status(v3270 *terminal)
{
	GdkRectangle *r;
	cairo_t *cr;

	if(!terminal->surface)
		return;

	cr = v3270_oia_set_update_region(terminal,&r,V3270_OIA_INSERT);

	draw_insert(cr,terminal->host,terminal->color,r);

    cairo_destroy(cr);
	v3270_queue_draw_area(GTK_WIDGET(terminal),r->x,r->y,r->width,r->height);

}

static gboolean update_timer(struct timer_info *info)
{
	cairo_t			* cr;
	time_t			  now = time(0);
	GdkRectangle	* rect;

//	debug("%s %p",__FUNCTION__,info->terminal->surface);
	if(!info->terminal->surface)
		return TRUE;

	cr = cairo_create(info->terminal->surface);

	if(now != info->last)
	{
		time_t seconds = now - info->start;
		char buffer[7];

		rect = info->terminal->oia.rect + V3270_OIA_TIMER;

		gdk_cairo_set_source_rgba(cr,info->terminal->color+V3270_COLOR_OIA_BACKGROUND);

		cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
		cairo_fill(cr);

		gdk_cairo_set_source_rgba(cr,info->terminal->color+V3270_COLOR_OIA_FOREGROUND);

		short2string(buffer,seconds/60,2);
		buffer[2] = ':';
		short2string(buffer+3,seconds%60,2);
		buffer[5] = 0;

		cairo_set_scaled_font(cr,info->terminal->font.scaled);
//		cairo_move_to(cr,rect->x,rect->y+info->terminal->font.height);
//		cairo_show_text(cr, buffer);

		v3270_draw_text(cr, rect, &info->terminal->font, buffer);

		cairo_stroke(cr);

		info->last = now;
		v3270_queue_draw_area(GTK_WIDGET(info->terminal),rect->x,rect->y,rect->width,rect->height);
	}

#ifdef HAVE_LIBM

    rect = info->terminal->oia.rect + V3270_OIA_SPINNER;

#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
#else
	gdk_cairo_set_source_rgba(cr,info->terminal->color+V3270_COLOR_OIA_BACKGROUND);
#endif

    cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
    cairo_fill(cr);

	gdk_cairo_set_source_rgba(cr,info->terminal->color+V3270_COLOR_OIA_FOREGROUND);

    info->step = draw_spinner(cr, rect, info->terminal->color, info->step);

    v3270_queue_draw_area(GTK_WIDGET(info->terminal),rect->x,rect->y,rect->width,rect->height);

#endif // HAVE_LIBM

    cairo_destroy(cr);

	return TRUE;
}

void v3270_start_timer(GtkWidget *widget)
{
	struct timer_info *info;
	v3270 *terminal = GTK_V3270(widget);

	if(terminal->timer)
	{
		g_source_ref(terminal->timer);
		return;
	}

	info = g_new0(struct timer_info,1);
	info->terminal	= terminal;
	info->start		= time(0);

	update_timer(info);

	terminal->timer = g_timeout_source_new(100);
	g_source_set_callback(terminal->timer,(GSourceFunc) update_timer, info, (GDestroyNotify) release_timer);

	g_source_attach(terminal->timer,NULL);
	g_source_unref(terminal->timer);

}

void v3270_stop_timer(GtkWidget *widget)
{
	v3270 *terminal = GTK_V3270(widget);

	if(terminal->timer)
	{
		if(terminal->timer->ref_count < 2)
		{
			g_source_destroy(terminal->timer);
		}
		else
		{
			g_source_unref(terminal->timer);
		}
	}

}

void v3270_blink_ssl(v3270 *terminal)
{
	if(terminal->surface)
	{
		GdkRectangle	* r;
		cairo_t			* cr = v3270_oia_set_update_region(terminal,&r,V3270_OIA_SSL);

		v3270_draw_ssl_status(terminal,cr,r);
		v3270_queue_draw_area(GTK_WIDGET(terminal),r->x,r->y,r->width,r->height);
		cairo_destroy(cr);

	}

}

void v3270_update_oia(v3270 *terminal, LIB3270_FLAG id, unsigned char on)
{
	cairo_t *cr;
	GdkRectangle *r;

	if(!(terminal->surface && terminal->drawing))
		return;

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wswitch"
	switch(id)
	{
	case LIB3270_FLAG_BOXSOLID:
		debug("%s LIB3270_FLAG_BOXSOLID",__FUNCTION__);
		cr = v3270_oia_set_update_region(terminal,&r,V3270_OIA_CONNECTION);
		v3270_draw_connection(cr,terminal->host,&terminal->font,terminal->color,r);
		cairo_destroy(cr);
		v3270_queue_draw_area(GTK_WIDGET(terminal),r->x,r->y,r->width,r->height);
		break;

	case LIB3270_FLAG_UNDERA:
		debug("%s LIB3270_FLAG_UNDERA",__FUNCTION__);
		cr = v3270_oia_set_update_region(terminal,&r,V3270_OIA_UNDERA);
		debug("%s LIB3270_FLAG_UNDERA",__FUNCTION__);
		draw_undera(cr,terminal->host,&terminal->font,terminal->color,r);
		debug("%s LIB3270_FLAG_UNDERA",__FUNCTION__);
		cairo_destroy(cr);
		debug("%s LIB3270_FLAG_UNDERA",__FUNCTION__);
		v3270_queue_draw_area(GTK_WIDGET(terminal),r->x,r->y,r->width,r->height);
		debug("%s LIB3270_FLAG_UNDERA",__FUNCTION__);
		break;

	case LIB3270_FLAG_TYPEAHEAD:
		debug("%s LIB3270_FLAG_TYPEAHEAD",__FUNCTION__);
		v3270_oia_update_text_field(terminal,on,V3270_OIA_TYPEAHEAD,'T');
		break;

#ifdef HAVE_PRINTER
	case LIB3270_FLAG_PRINTER:
		debug("%s LIB3270_FLAG_PRINTER",__FUNCTION__);
		v3270_oia_update_text_field(terminal,on,V3270_OIA_PRINTER,'P');
		break;
#endif // HAVE_PRINTER

/*
	case LIB3270_FLAG_SCRIPT:
		v3270_oia_update_text_field(terminal,on,V3270_OIA_SCRIPT,terminal->script_id);
		break;
*/

	}
	#pragma GCC diagnostic pop

}

int v3270_set_script(GtkWidget *widget, const gchar id)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),EINVAL);

	v3270 * terminal = GTK_V3270(widget);

	if(id && terminal->script)
		return EBUSY;

	terminal->script = id;

	if(terminal->script)
		v3270_start_blinking(widget);

	return 0;
}

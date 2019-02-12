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

 #include <lib3270.h>
 #include <lib3270/log.h>

/*--[ Globals ]--------------------------------------------------------------------------------------*/

const gchar * v3270_default_colors =
		"#000000,"			// V3270_COLOR_BACKGROUND
		"#7890F0,"			// V3270_COLOR_BLUE
		"#FF0000,"			// V3270_COLOR_RED
		"#FF00FF,"			// V3270_COLOR_PINK
		"#00FF00,"			// V3270_COLOR_GREEN
		"#00FFFF,"			// V3270_COLOR_TURQUOISE
		"#FFFF00,"			// V3270_COLOR_YELLOW
		"#FFFFFF,"			// V3270_COLOR_WHITE
		"#000000,"			// V3270_COLOR_BLACK
		"#000080,"			// V3270_COLOR_DARK_BLUE
		"#FFA200,"			// V3270_COLOR_ORANGE
		"#800080,"			// V3270_COLOR_PURPLE
		"#008000,"			// V3270_COLOR_DARK_GREEN
		"#008080,"			// V3270_COLOR_DARK_TURQUOISE
		"#A0A000,"			// V3270_COLOR_MUSTARD
		"#C0C0C0,"			// V3270_COLOR_GRAY

		"#00FF00,"			// V3270_COLOR_FIELD_DEFAULT
		"#FF0000,"			// V3270_COLOR_FIELD_INTENSIFIED
		"#00FFFF,"			// V3270_COLOR_FIELD_PROTECTED
		"#FFFFFF,"			// V3270_COLOR_FIELD_PROTECTED_INTENSIFIED

		"#404040,"			// V3270_COLOR_SELECTED_BG
		"#FFFFFF,"			// V3270_COLOR_SELECTED_FG,

		"#00FF00," 			// V3270_COLOR_CROSS_HAIR

		"#000000,"	 		// V3270_COLOR_OIA_BACKGROUND
		"#00FF00,"			// V3270_COLOR_OIA
		"#7890F0,"			// V3270_COLOR_OIA_SEPARATOR
		"#FFFFFF,"			// V3270_COLOR_OIA_STATUS_OK
		"#FFFF00,"			// V3270_COLOR_OIA_STATUS_WARNING
		"#FFFF00";			// V3270_COLOR_OIA_STATUS_INVALID


/*--[ Implement ]------------------------------------------------------------------------------------*/

LIB3270_EXPORT void v3270_set_colors(GtkWidget *widget, const gchar *colors)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	if(!colors)
	{
		colors = v3270_default_colors;
	}

	v3270_set_color_table(GTK_V3270(widget)->color,colors);
	g_signal_emit(widget,v3270_widget_signal[SIGNAL_UPDATE_CONFIG], 0, "colors", colors);
	v3270_reload(widget);

}

LIB3270_EXPORT void v3270_set_color(GtkWidget *widget, enum V3270_COLOR id, const GdkRGBA *color)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	GTK_V3270(widget)->color[id] = *color;

#if !GTK_CHECK_VERSION(3,0,0)
	gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),color,TRUE,TRUE);
#endif // !GTK(3,0,0)

}

LIB3270_EXPORT GdkRGBA * v3270_get_color(GtkWidget *widget, enum V3270_COLOR id)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);
 	return GTK_V3270(widget)->color+id;
}

LIB3270_EXPORT const GdkRGBA * v3270_get_color_table(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);
 	return GTK_V3270(widget)->color;
}

LIB3270_EXPORT void v3270_set_mono_color_table(GdkRGBA *clr, const gchar *fg, const gchar *bg)
{
	int f;

	gdk_rgba_parse(clr,bg);
	gdk_rgba_parse(clr+1,fg);

	for(f=2;f<V3270_COLOR_COUNT;f++)
		clr[f] = clr[1];

	clr[V3270_COLOR_BLACK]			= *clr;
	clr[V3270_COLOR_OIA_BACKGROUND]	= *clr;
	clr[V3270_COLOR_SELECTED_BG]	= clr[V3270_COLOR_WHITE];
	clr[V3270_COLOR_SELECTED_FG]	= clr[V3270_COLOR_BLACK];


}

LIB3270_EXPORT void v3270_set_color_table(GdkRGBA *table, const gchar *colors)
{
 	gchar	**clr;
 	guint	  cnt;
 	guint	  f;

	if(strchr(colors,':'))
		clr = g_strsplit(colors,":",V3270_COLOR_COUNT+1);
	else if(strchr(colors,';'))
		clr = g_strsplit(colors,";",V3270_COLOR_COUNT+1);
	else
		clr = g_strsplit(colors,",",V3270_COLOR_COUNT+1);

 	cnt = g_strv_length(clr);

 	switch(cnt)
 	{
 	case 28:				// Version 4 string
		for(f=0;f < 28;f++)
			gdk_rgba_parse(table+f,clr[f]);
		table[V3270_COLOR_OIA_STATUS_INVALID] = table[V3270_COLOR_OIA_STATUS_WARNING];
		break;

	case V3270_COLOR_COUNT:	// Complete string
		for(f=0;f < V3270_COLOR_COUNT;f++)
			gdk_rgba_parse(table+f,clr[f]);
		break;

	default:

		g_warning("Color table has %d elements; should be %d.",cnt,V3270_COLOR_COUNT);

		if(cnt < V3270_COLOR_COUNT)
		{
			// Less than the required
			for(f=0;f < cnt;f++)
				gdk_rgba_parse(table+f,clr[f]);

			for(f=cnt; f < V3270_COLOR_COUNT;f++)
				gdk_rgba_parse(table+f,clr[cnt-1]);

			clr[V3270_COLOR_OIA_BACKGROUND] = clr[0];
			clr[V3270_COLOR_SELECTED_BG] 	= clr[0];
		}
		else
		{
			// More than required
			for(f=0;f < V3270_COLOR_COUNT;f++)
				gdk_rgba_parse(table+f,clr[f]);
		}
 	}

	g_strfreev(clr);

}

LIB3270_EXPORT int v3270_set_session_color_type(GtkWidget *widget, unsigned short colortype)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),EINVAL);
	return lib3270_set_color_type(GTK_V3270(widget)->host,colortype);
}

LIB3270_EXPORT unsigned short v3270_get_session_color_type(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),-1);
	return lib3270_get_color_type(GTK_V3270(widget)->host);
}


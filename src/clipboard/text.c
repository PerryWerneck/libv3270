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

 #include <clipboard.h>


const char * v3270_update_selected_text(GtkWidget *widget, gboolean cut)
{
	/*
	char	* text;
	v3270	* terminal = GTK_V3270(widget);

    v3270_clear_clipboard(terminal);

	if(cut)
		text = lib3270_cut_selected(terminal->host);
	else
		text = lib3270_get_selected(terminal->host);

	if(!text)
	{
		g_signal_emit(widget,v3270_widget_signal[V3270_SIGNAL_CLIPBOARD], 0, FALSE);
		lib3270_ring_bell(terminal->host);
		return NULL;
	}

	if(terminal->selection.format == V3270_SELECT_TABLE)
	{
		// Convert text to table
		gchar 		**ln = g_strsplit(text,"\n",-1);
		int			  width = lib3270_get_width(terminal->host);
		gboolean	  cols[width];
		int 		  l;
		GString		* buffer;

		memset(cols,0,sizeof(gboolean)*width);

		// Find column delimiters
		for(l=0;ln[l];l++)
		{
			int		  c;
			gchar 	* ptr = ln[l];

			for(c=0;c<width && *ptr;c++)
			{
				if(!g_ascii_isspace(*ptr))
					cols[c] = TRUE;

				ptr++;
			}

		}

		// Read screen contents
		buffer = g_string_sized_new(strlen(text));
		for(l=0;ln[l];l++)
		{
			int 	  col	= 0;
			gchar	* src	= ln[l];

			while(col < width && *src)
			{
				if(col)
					g_string_append_c(buffer,'\t');

				// Find column start
				while(!cols[col] && col < width && *src)
				{
					col++;
					src++;
				}

				if(col < width && *src)
				{
					gchar	  tmp[width+1];
					gchar	* dst = tmp;

					// Copy column content
					while(cols[col] && col < width && *src)
					{
						*dst = *src;
						col++;
						dst++;
						src++;
					}
					*dst = 0;
					g_string_append(buffer,g_strstrip(tmp));
				}

			}
			g_string_append_c(buffer,'\n');

		}

		g_strfreev(ln);
		g_free(text);

		text = g_string_free(buffer,FALSE);
	}

    return terminal->selection.text = text;
    */

    return NULL;

}

LIB3270_EXPORT void v3270_paste_text(GtkWidget *widget, const gchar *text, const gchar *encoding)
{
 	gchar 		* buffer 	= NULL;
 	H3270		* session 	= v3270_get_session(widget);
	const gchar * charset 	= lib3270_get_display_charset(session);
 	gboolean	  next;

 	if(!text)
		return;
	else if(g_ascii_strcasecmp(encoding,charset))
		buffer = g_convert(text, -1, charset, encoding, NULL, NULL, NULL);
	else
		buffer = g_strdup(text);

    if(!buffer)
    {
    	/* Conversion failed, update special chars and try again */
    	size_t f;

    	static const struct _xlat
    	{
    		const gchar *from;
    		const gchar *to;
    	} xlat[] =
    	{
    		{ "–",		"-"		},
    		{ "→",		"->"	},
    		{ "←",		"<-" 	},
    		{ "©",		"(c)"	},
    		{ "↔",		"<->"	},
    		{ "™",		"(TM)"	},
    		{ "®",		"(R)"	},
    		{ "“",		"\""	},
    		{ "”",		"\""	},
    		{ "…",		"..."	},
    		{ "•",		"*"		},
    		{ "․",		"."		},
    		{ "·",		"*"		},

    	};

		gchar *string = g_strdup(text);

		// FIXME (perry#1#): Is there any better way for a "sed" here?
		for(f=0;f<G_N_ELEMENTS(xlat);f++)
		{
			gchar *ptr = g_strstr_len(string,-1,xlat[f].from);

			if(ptr)
			{
				gchar *old = string;
				gchar **tmp = g_strsplit(old,xlat[f].from,-1);
				string = g_strjoinv(xlat[f].to,tmp);
				g_strfreev(tmp);
				g_free(old);
			}
		}

		buffer = g_convert(string, -1, charset, encoding, NULL, NULL, NULL);

		if(!buffer)
		{
			// Still failing, convert line by line
			gchar **ln = g_strsplit(string,"\n",-1);

			for(f=0;ln[f];f++)
			{
				GError	*error	= NULL;
				gchar	*str	= g_convert(ln[f], -1, charset, encoding, NULL, NULL, &error);

				if(!str)
				{
					GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW( gtk_widget_get_toplevel(widget)),
																GTK_DIALOG_DESTROY_WITH_PARENT,
																GTK_MESSAGE_ERROR,
																GTK_BUTTONS_OK,
																_(  "Can't convert line %lu from %s to %s" ),(unsigned long) (f+1), encoding, charset);

					gtk_window_set_title(GTK_WINDOW(dialog), _( "Charset error" ) );
					gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s\n%s",error->message, ln[f]);

					gtk_dialog_run(GTK_DIALOG (dialog));
					gtk_widget_destroy(dialog);

					break;
				}
				else
				{
					g_free(str);
				}

			}
			g_strfreev(ln);

		}

		g_free(string);
    }

	if(buffer)
	{
		/* Remove TABS */
		gchar *ptr;

		for(ptr = buffer;*ptr;ptr++)
		{
			if(*ptr == '\t')
				*ptr = ' ';
		}
	}
    else
	{
		g_signal_emit(widget,v3270_widget_signal[V3270_SIGNAL_PASTENEXT], 0, FALSE);
		return;
	}

	next = lib3270_paste(session,(unsigned char *) buffer) ? TRUE : FALSE;

	g_free(buffer);

	g_signal_emit(widget,v3270_widget_signal[V3270_SIGNAL_PASTENEXT], 0, next);

}

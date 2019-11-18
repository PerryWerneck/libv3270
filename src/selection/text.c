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
 #include <lib3270/selection.h>
 #include <v3270/dialogs.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

/// @brief Get formatted contents as single text.
gchar * v3270_get_selection_as_text(v3270 * terminal, const GList *selection, const gchar *encoding, gboolean all)
{
	GString	* string = g_string_new("");

	while(selection)
	{
		lib3270_selection * block = ((lib3270_selection *) selection->data);
		unsigned int row, col, src = 0;

		for(row=0; row < block->bounds.height; row++)
		{
			for(col=0; col<block->bounds.width; col++)
			{
				if( (block->contents[src].attribute.visual & LIB3270_ATTR_SELECTED) || all )
					g_string_append_c(string,block->contents[src].chr);

				src++;

			}
		}

		selection = g_list_next(selection);
	}

	g_autofree char * text = g_string_free(string,FALSE);

	return g_convert(text, -1, (encoding ? encoding : "UTF-8"), lib3270_get_display_charset(terminal->host), NULL, NULL, NULL);

}

gchar * v3270_get_copy_as_text(v3270 * terminal, const gchar *encoding)
{
	if(terminal->selection.format == V3270_COPY_TABLE)
		return v3270_get_copy_as_table(terminal,"\t",encoding);
	return v3270_get_selection_as_text(terminal, terminal->selection.blocks, encoding, FALSE);
}

gchar * v3270_convert_to_3270_charset(GtkWidget *widget, const gchar *text, const gchar *encoding, GError **error)
{
	if(*error || !text)
		return NULL;

	const gchar * charset = lib3270_get_display_charset(v3270_get_session(widget));
	if(!encoding || (g_ascii_strcasecmp(encoding,charset) == 0))
	{
		// No conversion needed.
		return g_strdup(text);
	}

	gchar *converted = g_convert(text, -1, charset, encoding, NULL, NULL, NULL);
	if(converted)
		return converted;

	// Conversion failed, update UTF-8 special chars and try again
	if(!g_ascii_strcasecmp(encoding,"UTF-8"))
	{
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

		g_autofree gchar *string = g_strdup(text);

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

		converted = g_convert(string, -1, charset, encoding, NULL, NULL, NULL);

		if(converted)
			return converted;

	}

	/*
	// Still failing, convert line by line
	{
		size_t f;
		gchar **ln = g_strsplit(text,"\n",-1);

		for(f=0;ln[f];f++)
		{
			GError	*error	= NULL;
			gchar	*str	= g_convert(ln[f], -1, charset, encoding, NULL, NULL, error);

			if(!str)
			{
				g_strfreev(ln);
				return NULL;
			}

			g_free(str);

		}
		g_strfreev(ln);

	}
	*/

	// Can't convert, use fallbacks
	return g_convert_with_fallback(
				text,
				-1,
				charset,
				encoding,
				" ",
				NULL,
				NULL,
				error
			);
}

LIB3270_EXPORT void v3270_input_text(GtkWidget *widget, const gchar *text, const gchar *encoding)
{
	GError * error	= NULL;
	g_autofree gchar * buffer = v3270_convert_to_3270_charset(widget,text,encoding,&error);

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

	if(error)
	{
		g_signal_emit(
                widget,
                v3270_widget_signal[V3270_SIGNAL_PASTENEXT],
                0,
                FALSE
        );
		v3270_popup_gerror(widget,error,NULL,"%s",_("Can't paste text"));
		g_error_free(error);
		return;
	}

	// Do paste.
	int remains = lib3270_paste_text(
						v3270_get_session(widget),
						(unsigned char *) buffer
					) ? TRUE : FALSE;

	g_signal_emit(
		widget,
		v3270_widget_signal[V3270_SIGNAL_PASTENEXT],
		0,
		remains > 0
	);

}

LIB3270_EXPORT gchar * v3270_get_copy(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);
	return v3270_get_copy_as_text(GTK_V3270(widget),NULL);
}


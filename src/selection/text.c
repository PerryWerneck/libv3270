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
			g_string_append_c(string,'\n');
		}

		selection = g_list_next(selection);
	}

	g_autofree char * text = g_string_free(string,FALSE);

	// Remove ending CR.
	{
		size_t length = strlen(text);
		if(length > 1 && text[length-1] == '\n') // The '\n' isn't really necessary but...
			text[length-1] = 0;
	}

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

		// Is there any better way for a "sed" here?
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
		v3270_signal_emit(
                widget,
                V3270_SIGNAL_PASTENEXT,
                FALSE
        );
		v3270_popup_gerror(widget,&error,NULL,"%s",_("Can't paste text"));
		return;
	}

	// Do paste.
	int remains = lib3270_paste_text(
						v3270_get_session(widget),
						(unsigned char *) buffer
					) ? TRUE : FALSE;

	v3270_signal_emit(
		widget,
		V3270_SIGNAL_PASTENEXT,
		remains > 0
	);

}

LIB3270_EXPORT gchar * v3270_get_copy(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);
	return v3270_get_copy_as_text(GTK_V3270(widget),NULL);
}


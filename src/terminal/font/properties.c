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
 #include <gdk/gdk.h>
 #include <v3270/settings.h>

/*--[ Globals ]--------------------------------------------------------------------------------------*/

static const gchar * invalid_font_messages[] = {

	N_( "Font \"%s\" is valid" ),
	N_( "Font \"%s\" is not monospace" ),
	N_( "Can't find a valid font with the name \"%s\"")

};

/*--[ Implement ]------------------------------------------------------------------------------------*/

const gchar * v3270_get_default_font_name()
{
#if defined(_WIN32)
	{
		HKEY hKey;
		DWORD disp = 0;
		LSTATUS	rc = RegCreateKeyEx(
						HKEY_LOCAL_MACHINE,
						"Software\\" LIB3270_STRINGIZE_VALUE_OF(PRODUCT_NAME),
						0,
						NULL,
						REG_OPTION_NON_VOLATILE,
						KEY_QUERY_VALUE|KEY_READ,
						NULL,
						&hKey,
						&disp);

		debug("%s=%d","Software\\" LIB3270_STRINGIZE_VALUE_OF(PRODUCT_NAME),rc);

		if(rc == ERROR_SUCCESS)
		{
			static char * default_font_name = NULL;
			DWORD cbData = 4096;

			if(!default_font_name)
			{
				default_font_name = (char *) malloc(cbData+1);
			}
			else
			{
				default_font_name = (char *) realloc(default_font_name,cbData+1);
			}

			DWORD dwRet = RegQueryValueEx(hKey,"font-family",NULL,NULL,(LPBYTE) default_font_name, &cbData);

			debug("dwRet=%d",dwRet);

			RegCloseKey(hKey);

			if(dwRet == ERROR_SUCCESS)
			{
				default_font_name = (char *) realloc(default_font_name,cbData+1);
                default_font_name[cbData] = 0;
                return default_font_name;
			}

			free(default_font_name);
			default_font_name = NULL;
		}
	}

	// TODO: Search for a valid font-family
	return "Courier New";

#elif defined(__APPLE__)

	return "Courier New";

#else

	return "monospace";

#endif // _WIN32
}

static guint check_font_family(const gchar *family_name)
{
	int rc = 2;

	gint n_families, i;
	PangoFontFamily **families;

	PangoContext * context = gdk_pango_context_get_for_screen(gdk_screen_get_default());

	pango_context_list_families(context,&families, &n_families);

	for(i=0; i < n_families; i++)
	{
		if(!g_ascii_strcasecmp(pango_font_family_get_name(families[i]),family_name))
		{
			rc = pango_font_family_is_monospace(families[i]) ? 0 : 1;
			break;
		}

	}

	g_object_unref(G_OBJECT(context));
	g_free(families);

	if(rc)
		g_warning(invalid_font_messages[rc],family_name);
	else
		g_message(invalid_font_messages[rc],family_name);

	return rc;
}

LIB3270_EXPORT void v3270_set_font_family(GtkWidget *widget, const gchar *name)
{
	v3270 * terminal;

	g_return_if_fail(GTK_IS_V3270(widget));

	terminal = GTK_V3270(widget);

	if(!name)
		name = v3270_get_default_font_name();

	if(g_ascii_strcasecmp(terminal->font.family,name))
	{
		check_font_family(name);

		// Font has changed, update it
		g_free(terminal->font.family);

		terminal->font.family = g_strdup(name);
		terminal->font.weight = lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_BOLD) ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL;

		v3270_emit_save_settings(widget);

		debug("%s: %p",__FUNCTION__,GTK_V3270_GET_CLASS(widget)->properties.settings[V3270_SETTING_FONT_FAMILY]);
		v3270_notify_setting(widget,V3270_SETTING_FONT_FAMILY);

		if(gtk_widget_get_realized(widget) && gtk_widget_get_has_window(widget))
		{
			v3270_reconfigure(terminal);
			gtk_widget_queue_draw(widget);
		}

	}

}

void v3270_notify_setting(GtkWidget *widget, V3270_SETTING id)
{
	debug("%s(%u)",__FUNCTION__,(unsigned int) id);
	g_object_notify_by_pspec(G_OBJECT(widget), GTK_V3270_GET_CLASS(widget)->properties.settings[id]);
	v3270_emit_save_settings(widget);
}

LIB3270_EXPORT const gchar * v3270_get_font_family(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);
	return GTK_V3270(widget)->font.family;
}

LIB3270_EXPORT void v3270_set_scaled_fonts(GtkWidget *widget, gboolean on)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	GTK_V3270(widget)->scaled_fonts = on ? 1 : 0;

}

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

 #include <internals.h>
 #include <terminal.h>
 #include <lib3270/selection.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

/**
 * Get text at informed area.
 *
 * @param widget    Widget.
 * @param offset    Offset of the desired text.
 * @param len       Number of characters to get.
 *
 */
LIB3270_EXPORT gchar * v3270_get_text(GtkWidget *widget, int offset, int len)
{
	v3270	* terminal;
	gchar	* text;
	char	* str;

	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	terminal = GTK_V3270(widget);

	str = lib3270_get_string_at_address(terminal->host, offset, len, '\n');

	if(!str)
		return NULL;

	text = g_convert(str, -1, "UTF-8", lib3270_get_display_charset(terminal->host), NULL, NULL, NULL);

	lib3270_free(str);
	return text;
}

gchar * v3270_get_region(GtkWidget *widget, gint start_pos, gint end_pos, gboolean all)
{
	char	* str;
	gchar	* utftext;

	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	str = lib3270_get_region(GTK_V3270(widget)->host,start_pos,end_pos,all);
	if(!str)
		return NULL;

	utftext = g_convert(str, -1, "UTF-8", lib3270_get_display_charset(GTK_V3270(widget)->host), NULL, NULL, NULL);

	lib3270_free(str);

	return utftext;
}

gboolean v3270_get_dynamic_font_spacing(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);
	return GTK_V3270(widget)->font.spacing.dynamic;
}

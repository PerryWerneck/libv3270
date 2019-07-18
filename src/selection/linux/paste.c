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
 * Este programa está nomeado como selection.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <clipboard.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void text_received(G_GNUC_UNUSED  GtkClipboard *clipboard, const gchar *text, GtkWidget *widget)
{
	v3270_input_text(widget,text,"UTF-8");
}

static gboolean has_target(const gchar *name, const GdkAtom *atoms, const gint n_atoms)
{
	gint ix;

	for(ix = 0; ix < n_atoms; ix++)
	{
		if(!g_ascii_strcasecmp(name,gdk_atom_name(atoms[ix])))
			return TRUE;

	}

    return FALSE;
}

static void targets_received(GtkClipboard *clipboard, GdkAtom *atoms, gint n_atoms, GtkWidget *widget)
{
	if(has_target("application/x-" PACKAGE_NAME, atoms, n_atoms))
	{
		debug("Clipboard as TN3270 \"%s\" data",PACKAGE_NAME);
		return;
	}

	// No special format available, request it as text.
	gtk_clipboard_request_text(clipboard, (GtkClipboardTextReceivedFunc) text_received, (gpointer) widget);

}

LIB3270_EXPORT void v3270_paste(GtkWidget *widget)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	GtkClipboard * clipboard = gtk_widget_get_clipboard(widget,GTK_V3270(widget)->selection.target);

	gtk_clipboard_request_targets(clipboard, (GtkClipboardTargetsReceivedFunc) targets_received, (gpointer) widget);

}


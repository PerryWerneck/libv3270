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
 * Este programa está nomeado como trace.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

/**
 * SECTION:V3270Trace
 * @Short_description: A trace monitor widget.
 * @Title: V3270Trace
 *
 * The #V3270Trace shows a text area with the lib3270 trace output.
 *
 */

 #include "private.h"

 #include <terminal.h>
 #include <internals.h>
 #include <v3270/dialogs.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 gboolean v3270_get_trace(GtkWidget *terminal)
 {
	return GTK_V3270(terminal)->trace != NULL;
 }

 static gboolean bg_remove_trace(v3270 *terminal)
 {
	if(!terminal->trace)
		return FALSE;

	// Destroy trace window.
	GtkWidget * parent = gtk_widget_get_parent(terminal->trace);

	if(GTK_IS_NOTEBOOK(parent))
	{
		debug("%s: Parent window %s",__FUNCTION__,"is a notebook");
		gtk_notebook_remove_page(
			GTK_NOTEBOOK(parent),
			gtk_notebook_page_num(GTK_NOTEBOOK(parent),terminal->trace)
		);
	}
	else
	{
		debug("%s: Parent window %s",__FUNCTION__,"is not a notebook");
//		gtk_widget_destroy(terminal->trace);
	}

	return FALSE;

 }

 static gboolean bg_append_trace(GtkWidget *terminal)
 {
	if(GTK_V3270(terminal)->trace)
		return FALSE;

	// Destroy trace window.
	GtkWidget * parent	= gtk_widget_get_parent(terminal);

	if(GTK_IS_NOTEBOOK(parent))
	{
		debug("%s: Parent window %s",__FUNCTION__,"is a notebook");
		GtkWidget * trace = v3270_trace_new(terminal);
		gtk_widget_show_all(trace);

		gtk_notebook_insert_page(
			GTK_NOTEBOOK(parent),
			trace,
			gtk_label_new(_("Trace")),
			gtk_notebook_page_num(GTK_NOTEBOOK(parent),terminal)+1
		);

	}
	else
	{
		debug("%s: Parent window %s",__FUNCTION__,"is not a notebook, creating trace window");
		gtk_widget_show_all(v3270_trace_window_new(terminal,NULL));

	}

	return FALSE;

 }

 void v3270_set_trace(GtkWidget *widget, gboolean trace)
 {
 	g_return_if_fail(GTK_IS_V3270(widget));

 	if(trace)
	{
		debug("%s: trace is %s",__FUNCTION__,"ON");
		g_idle_add((GSourceFunc) bg_append_trace, GTK_V3270(widget));
	}
	else
	{
		debug("%s: trace is %s",__FUNCTION__,"OFF");
		g_idle_add((GSourceFunc) bg_remove_trace, GTK_V3270(widget));
	}

 }

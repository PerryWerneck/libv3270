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

 static void do_copy(v3270 *terminal, V3270_COPY_MODE mode, gboolean cut) {

	if(mode == V3270_COPY_SMART) {
		mode = (terminal->append ? V3270_COPY_APPEND : V3270_COPY_FORMATTED);
	}

	if(mode != V3270_COPY_APPEND) {
		// It's not append, clear current contents ...
		v3270_clear_selection(terminal);
		terminal->selection.format = mode;
	}

 	lib3270_selection * selection = lib3270_selection_new(terminal->host,cut,0);

 	if(selection) {
		terminal->selection.blocks = g_list_append(terminal->selection.blocks,selection);
		terminal->append = ((terminal->selection.options & V3270_SELECTION_SMART_COPY) != 0) ? 1 : 0 ; // Define next smart option.
	}

 }

 LIB3270_EXPORT void v3270_clipboard_set(GtkWidget *widget, V3270_COPY_MODE mode, gboolean cut) {

	g_return_if_fail(GTK_IS_V3270(widget));
	do_copy(GTK_V3270(widget),mode,cut);
	v3270_update_system_clipboard(widget);

 }

 LIB3270_EXPORT void v3270_copy_selection(GtkWidget *widget, V3270_COPY_MODE mode, gboolean cut) {
	v3270_clipboard_set(widget,mode,cut);
 }

 LIB3270_EXPORT void v3270_append_selection(GtkWidget *widget, gboolean cut) {
	v3270_clipboard_set(widget,V3270_COPY_APPEND,cut);
 }

 LIB3270_EXPORT void v3270_copy_as_html(GtkWidget *widget) {

	g_return_if_fail(GTK_IS_V3270(widget));
	v3270 * terminal = GTK_V3270(widget);

	debug("%s",__FUNCTION__);

	do_copy(terminal,V3270_COPY_FORMATTED,0);

	//
	// Export only in HTML format
	//
	GtkClipboard * clipboard = gtk_widget_get_clipboard(widget,terminal->selection.target);

	GtkTargetList * list = gtk_target_list_new(NULL,0);

	static const GtkTargetEntry entry = {
		.target = "text/html",
		.flags = 0,
		.info = CLIPBOARD_TYPE_HTML
	};

	gtk_target_list_add_table(list, &entry, 1);

	int				  n_targets;
	GtkTargetEntry	* targets = gtk_target_table_new_from_list(list, &n_targets);

	if(gtk_clipboard_set_with_owner(
			clipboard,
			targets,
			n_targets,
			(GtkClipboardGetFunc)	v3270_clipboard_get,
			(GtkClipboardClearFunc) v3270_clipboard_clear,
			G_OBJECT(widget)
		))
	{
		gtk_clipboard_set_can_store(clipboard,targets,1);
	}

	gtk_target_table_free(targets, n_targets);
	gtk_target_list_unref(list);

   	v3270_emit_copy_state(widget);
 }

 LIB3270_EXPORT void v3270_copy_as_pixbuff(GtkWidget *widget) {

	g_return_if_fail(GTK_IS_V3270(widget));
	v3270 * terminal = GTK_V3270(widget);

	debug("%s",__FUNCTION__);

	do_copy(terminal,V3270_COPY_FORMATTED,0);

	//
	// Export only in HTML format
	//
	GtkClipboard * clipboard = gtk_widget_get_clipboard(widget,terminal->selection.target);

	GtkTargetList * list = gtk_target_list_new(NULL,0);

	gtk_target_list_add_image_targets(list,CLIPBOARD_TYPE_PIXBUFF,TRUE);

	int				  n_targets;
	GtkTargetEntry	* targets = gtk_target_table_new_from_list(list, &n_targets);

	if(gtk_clipboard_set_with_owner(
			clipboard,
			targets,
			n_targets,
			(GtkClipboardGetFunc)	v3270_clipboard_get,
			(GtkClipboardClearFunc) v3270_clipboard_clear,
			G_OBJECT(widget)
		))
	{
		gtk_clipboard_set_can_store(clipboard,targets,1);
	}

	gtk_target_table_free(targets, n_targets);
	gtk_target_list_unref(list);

   	v3270_emit_copy_state(widget);

  }

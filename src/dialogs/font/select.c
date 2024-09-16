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

 #include <string.h>
 #include <v3270.h>
 #include <v3270/print.h>
 #include <v3270/dialogs.h>
 #include <lib3270/log.h>

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 LIB3270_EXPORT GtkWidget * v3270_font_selection_new(const gchar *selected)
 {
	GtkWidget		* widget = gtk_combo_box_new();
	GtkTreeIter		  active;
	GtkTreeModel	* model = v3270_font_family_model_new(widget,selected,&active);

	GtkCellRenderer	* renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), renderer, "text", 0, NULL);

	gtk_combo_box_set_model(GTK_COMBO_BOX(widget),model);
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&active);

	return widget;
 }

 LIB3270_EXPORT gboolean v3270_font_selection_set_family(GtkWidget *widget, const gchar *fontname)
 {
	GtkTreeModel	* model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	GtkTreeIter		  iter;

	if(!fontname)
		fontname = "monospace";

	if(gtk_tree_model_get_iter_first(model,&iter))
	{
		do
		{
			GValue value = { 0, };

			gtk_tree_model_get_value(model,&iter,0,&value);

			if(!g_ascii_strcasecmp(fontname,g_value_get_string(&value)))
			{
				gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
				g_value_unset(&value);
				return TRUE;
			}

			g_value_unset(&value);

		} while(gtk_tree_model_iter_next(model,&iter));
	}

	return FALSE;

 }

 LIB3270_EXPORT gchar * v3270_font_selection_get_family(GtkWidget *widget)
 {
	GtkTreeIter	iter;

	if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget),&iter))
	{
		GValue value = { 0, };

		gtk_tree_model_get_value(gtk_combo_box_get_model(GTK_COMBO_BOX(widget)),&iter,0,&value);

		gchar * rc = g_value_dup_string(&value);

		g_value_unset(&value);

		return rc;

	}

	return "monospace";

 }


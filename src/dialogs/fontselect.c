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

 #include <string.h>
 #include <v3270.h>
 #include <v3270/print.h>
 #include <lib3270/log.h>

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 LIB3270_EXPORT GtkTreeModel * v3270_font_family_model_new(GtkWidget *widget, const gchar *selected, GtkTreeIter * active)
 {
	GtkTreeModel * model = (GtkTreeModel *) gtk_list_store_new(1,G_TYPE_STRING);

	GtkCellRenderer	* renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), renderer, "text", 0, NULL);

	gint n_families, i;
	PangoFontFamily **families;
	pango_context_list_families(gtk_widget_get_pango_context(widget),&families, &n_families);

	if(!(selected && *selected))
		selected = v3270_get_default_font_name();

	memset(active,0,sizeof(GtkTreeIter));

	for(i=0; i < n_families; i++)
	{
		if(!pango_font_family_is_monospace(families[i]))
			continue;

		const gchar *name = pango_font_family_get_name (families[i]);
		GtkTreeIter iter;

		gtk_list_store_append((GtkListStore *) model,&iter);
		gtk_list_store_set((GtkListStore *) model, &iter,0, name, -1);

		if(!g_ascii_strcasecmp(name,selected))
			*active = iter;

	}

	g_free(families);

	return model;
 }

 LIB3270_EXPORT GtkWidget * v3270_font_selection_new(const gchar *selected)
 {
	GtkWidget		* widget = gtk_combo_box_new();
	GtkTreeIter		  active;
	GtkTreeModel	* model = v3270_font_family_model_new(widget,selected,&active);

	gtk_combo_box_set_model(GTK_COMBO_BOX(widget),model);
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&active);

	return widget;
 }


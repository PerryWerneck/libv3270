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

 #include "private.h"
 #include <hostselect.h>

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 static const struct _colortable
 {
	unsigned short 	  colors;
	const gchar		* description;
 } colortable[] =
 {
	{ 16,	N_( "16 colors"  )	},
	{ 8,	N_( "8 colors"	 )	},
	{ 2,	N_( "Monochrome" )	},
 };

 enum _entry
 {
 	ENTRY_HOSTNAME,
 	ENTRY_SRVCNAME,

 	ENTRY_COUNT
 };

 static const gchar *comboLabel[] = { N_("System _type"), N_("_Color table")  };

 struct _V3270HostSelectWidget
 {
 	GtkGrid parent;

	LIB3270_HOST_TYPE	type;									/**< @brief Connect option */

	struct
	{
		GtkEntry			* entry[ENTRY_COUNT];				/**< @brief Entry fields for host & service name */
		GtkToggleButton		* ssl;								/**< @brief SSL Connection? */
		GtkComboBox			* combo[G_N_ELEMENTS(comboLabel)];	/**< @brief Model & Color combobox */

	} input;

	unsigned short		  colors;								/**< @brief Number of colors */
	H3270				* hSession;								/**< @brief lib3270's session handle */

 };

 struct _V3270HostSelectWidgetClass
 {
	GtkGridClass parent_class;
 };


 G_DEFINE_TYPE(V3270HostSelectWidget, V3270HostSelectWidget, GTK_TYPE_GRID);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void V3270HostSelectWidget_class_init(G_GNUC_UNUSED V3270HostSelectWidgetClass *klass)
{
}

static void systype_changed(GtkComboBox *widget, V3270HostSelectWidget *dialog)
{
	GValue		value   = { 0, };
	GtkTreeIter iter;

	if(!gtk_combo_box_get_active_iter(widget,&iter))
		return;

	gtk_tree_model_get_value(gtk_combo_box_get_model(widget),&iter,1,&value);

	dialog->type = g_value_get_int(&value);

}

static void colortable_changed(GtkComboBox *widget, V3270HostSelectWidget *dialog)
{
	GValue		value   = { 0, };
	GtkTreeIter iter;

	if(!gtk_combo_box_get_active_iter(widget,&iter))
		return;

	gtk_tree_model_get_value(gtk_combo_box_get_model(widget),&iter,1,&value);

	dialog->colors = g_value_get_int(&value);

}

static void V3270HostSelectWidget_init(V3270HostSelectWidget *widget)
{
	int f;

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
 	gtk_grid_set_row_spacing(GTK_GRID(widget),6);
 	gtk_grid_set_column_spacing(GTK_GRID(widget),12);

 	// Entry fields
	GtkWidget * label[ENTRY_COUNT] =
	{
		gtk_label_new_with_mnemonic( _( "_Host" ) ),
		gtk_label_new_with_mnemonic( _( "_Service" ) )
	};

 	for(f=0;f<ENTRY_COUNT;f++)
	{
		widget->input.entry[f] = GTK_ENTRY(gtk_entry_new());
		gtk_widget_set_halign(label[f],GTK_ALIGN_END);
		gtk_label_set_mnemonic_widget(GTK_LABEL(label[f]),GTK_WIDGET(widget->input.entry[f]));
	}

	gtk_widget_set_tooltip_text(GTK_WIDGET(widget->input.entry[ENTRY_HOSTNAME]),_("Address or name of the host to connect.") );
	gtk_widget_set_tooltip_text(GTK_WIDGET(widget->input.entry[ENTRY_SRVCNAME]),_("Port or service name (empty for \"telnet\").") );

	gtk_entry_set_max_length(widget->input.entry[ENTRY_HOSTNAME],0xFF);
	gtk_entry_set_width_chars(widget->input.entry[ENTRY_HOSTNAME],50);

	gtk_entry_set_max_length(widget->input.entry[ENTRY_SRVCNAME],6);
	gtk_entry_set_width_chars(widget->input.entry[ENTRY_SRVCNAME],7);

	gtk_entry_set_placeholder_text(widget->input.entry[ENTRY_SRVCNAME],"telnet");

	gtk_widget_set_hexpand(GTK_WIDGET(widget->input.entry[ENTRY_HOSTNAME]),TRUE);

	gtk_grid_attach(GTK_GRID(widget),label[0],0,0,1,1);
	gtk_grid_attach(GTK_GRID(widget),GTK_WIDGET(widget->input.entry[ENTRY_HOSTNAME]),1,0,5,1);

	gtk_grid_attach(GTK_GRID(widget),label[1],0,1,1,1);
	gtk_grid_attach(GTK_GRID(widget),GTK_WIDGET(widget->input.entry[ENTRY_SRVCNAME]),1,1,1,1);

	// gtk_widget_set_hexpand(GTK_WIDGET(widget->input.ssl),TRUE);

	// SSL checkbox
	{
		widget->input.ssl = GTK_TOGGLE_BUTTON(gtk_check_button_new_with_mnemonic(_( "_Secure connection." )));
		gtk_widget_set_tooltip_text(GTK_WIDGET(widget->input.ssl),_( "Check for SSL secure connection." ));

		gtk_grid_attach(GTK_GRID(widget),GTK_WIDGET(widget->input.ssl),3,1,1,1);
	}

	// Host type
	{
		GtkTreeModel    * model		= (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_INT);
		GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();

		widget->input.combo[0] = GTK_COMBO_BOX(gtk_combo_box_new_with_model(model));

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget->input.combo[0]), renderer, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget->input.combo[0]), renderer, "text", 0, NULL);

		const LIB3270_HOST_TYPE_ENTRY *entry = lib3270_get_option_list();
		for(f=0;entry[f].name != NULL;f++)
		{
			GtkTreeIter iter;
			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter, 0, gettext(entry[f].description), 1, entry[f].type, -1);
		}

		g_signal_connect(G_OBJECT(widget->input.combo[0]),"changed",G_CALLBACK(systype_changed),widget);

	}

	// Color table
	{
		GtkTreeModel    * model		= (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_INT);
		GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();

		widget->input.combo[1] = GTK_COMBO_BOX(gtk_combo_box_new_with_model(model));

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget->input.combo[1]), renderer, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget->input.combo[1]), renderer, "text", 0, NULL);

		for(f=0;f< (int) G_N_ELEMENTS(colortable);f++)
		{
			GtkTreeIter iter;
			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter, 0, gettext(colortable[f].description), 1, colortable[f].colors, -1);
		}

		g_signal_connect(G_OBJECT(widget->input.combo[1]),"changed",G_CALLBACK(colortable_changed),widget);

	}

	// Host options
	for(f=0;f< (int) G_N_ELEMENTS(comboLabel);f++)
	{
		GtkWidget *label = gtk_label_new_with_mnemonic(gettext(comboLabel[f]));
		gtk_widget_set_halign(label,GTK_ALIGN_END);
		gtk_grid_attach(GTK_GRID(widget),label,0,f+2,1,1);
		gtk_grid_attach(GTK_GRID(widget),GTK_WIDGET(widget->input.combo[f]),1,f+2,2,1);
	}

	gtk_widget_show_all(GTK_WIDGET(widget));

}

LIB3270_EXPORT GtkWidget * v3270_host_select_new(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	GtkWidget * selector = GTK_WIDGET(g_object_new(GTK_TYPE_V3270HostSelectWidget, NULL));

	v3270_host_select_set_session(selector,widget);

	return selector;
}

LIB3270_EXPORT void v3270_host_select_set_session(GtkWidget *w, GtkWidget *session)
{
	g_return_if_fail(GTK_IS_V3270(session));
	g_return_if_fail(GTK_IS_V3270HostSelectWidget(w));

	V3270HostSelectWidget *widget = GTK_V3270HostSelectWidget(w);
	widget->hSession = v3270_get_session(session);

	gtk_entry_set_text(widget->input.entry[ENTRY_HOSTNAME],lib3270_get_hostname(widget->hSession));
	gtk_entry_set_text(widget->input.entry[ENTRY_SRVCNAME],lib3270_get_srvcname(widget->hSession));

	LIB3270_HOST_TYPE type = lib3270_get_host_type(widget->hSession);

	gtk_toggle_button_set_active(widget->input.ssl,lib3270_get_secure_host(widget->hSession) != 0);

	// Set host type
	{
		GtkTreeModel	* model = gtk_combo_box_get_model(widget->input.combo[0]);
		GtkTreeIter		  iter;

		if(gtk_tree_model_get_iter_first(model,&iter))
		{
			do
			{
				GValue		value   = { 0, };

				gtk_tree_model_get_value(model,&iter,1,&value);

				if(g_value_get_int(&value) == (int) type)
				{
					gtk_combo_box_set_active_iter(widget->input.combo[0],&iter);
					break;
				}

			} while(gtk_tree_model_iter_next(model,&iter));
		}
	}

	// Set color type
	{
		GtkTreeModel	* model = gtk_combo_box_get_model(widget->input.combo[1]);
		GtkTreeIter		  iter;
		int				  colors = (int) lib3270_get_color_type(widget->hSession);

		if(gtk_tree_model_get_iter_first(model,&iter))
		{
			do
			{
				GValue		value   = { 0, };

				gtk_tree_model_get_value(model,&iter,1,&value);

				if(g_value_get_int(&value) == colors)
				{
					gtk_combo_box_set_active_iter(widget->input.combo[1],&iter);
					break;
				}

			} while(gtk_tree_model_iter_next(model,&iter));
		}
	}

	// Just in case
	widget->type = type;

}

LIB3270_EXPORT void v3270_select_host(GtkWidget *widget)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	if(v3270_is_connected(widget))
	{
		gdk_display_beep(gdk_display_get_default());
		return;
	}

	GtkWidget * dialog	= v3270_host_select_new(widget);
	GtkWidget * win	= v3270_dialog_new(widget, _("Setup host"), _("C_onnect"));

	gtk_window_set_default_size(GTK_WINDOW(win), 700, 150);

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(win))),dialog,FALSE,FALSE,2);
	gtk_widget_show_all(dialog);

	gboolean again = TRUE;
 	while(again)
 	{
 		gtk_widget_set_sensitive(win,TRUE);

		gtk_widget_set_visible(win,TRUE);

 		switch(gtk_dialog_run(GTK_DIALOG(win)))
 		{
		case GTK_RESPONSE_APPLY:
			gtk_widget_set_visible(win,FALSE);
			gtk_widget_set_sensitive(win,FALSE);
			again = v3270_host_select_apply(dialog) != 0;
			break;

		case GTK_RESPONSE_CANCEL:
			again = FALSE;
			break;
 		}
 	}

	gtk_widget_destroy(win);

}

int v3270_host_select_apply(GtkWidget *w)
{
	g_return_val_if_fail(GTK_IS_V3270HostSelectWidget(w),EINVAL);

	V3270HostSelectWidget *widget = GTK_V3270HostSelectWidget(w);

	lib3270_set_hostname(widget->hSession,gtk_entry_get_text(widget->input.entry[ENTRY_HOSTNAME]));
	lib3270_set_srvcname(widget->hSession,gtk_entry_get_text(widget->input.entry[ENTRY_SRVCNAME]));
	lib3270_set_host_type(widget->hSession,widget->type);

	return lib3270_reconnect(widget->hSession,0);
}


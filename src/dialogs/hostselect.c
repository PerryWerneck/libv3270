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
 #include <v3270/dialogs.h>
 #include <v3270/settings.h>
 #include <lib3270/log.h>

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
 	V3270Settings parent;

	struct
	{
		GtkEntry			* entry[ENTRY_COUNT];				/**< @brief Entry fields for host & service name */
		GtkToggleButton		* ssl;								/**< @brief SSL Connection? */
		GtkComboBox			* combo[G_N_ELEMENTS(comboLabel)];	/**< @brief Model & Color combobox */

	} input;

 };

 struct _V3270HostSelectWidgetClass
 {
	V3270SettingsClass parent_class;
 };


 G_DEFINE_TYPE(V3270HostSelectWidget, V3270HostSelectWidget, GTK_TYPE_V3270_SETTINGS);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void apply(GtkWidget *w, GtkWidget *terminal)
{
	debug("V3270HostSelectWidget::%s",__FUNCTION__);

	V3270HostSelectWidget *widget = GTK_V3270HostSelectWidget(w);
	H3270 *hSession = v3270_get_session(terminal);

	// Apply URL
	{
		g_autofree gchar * url =
			g_strconcat(
							(gtk_toggle_button_get_active(widget->input.ssl) ? "tn3270s://" : "tn3270://"),
							gtk_entry_get_text(widget->input.entry[ENTRY_HOSTNAME]),
							":",
							gtk_entry_get_text(widget->input.entry[ENTRY_SRVCNAME]),
							NULL
						);

		debug("URL=[%s]",url);
		lib3270_set_url(hSession,url);

	}

	// Apply Host type
	{
		GtkTreeIter	iter;

		if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget->input.combo[0]), &iter))
		{
			GValue value = { 0, };
			gtk_tree_model_get_value(gtk_combo_box_get_model(GTK_COMBO_BOX(widget->input.combo[0])),&iter,1,&value);

			lib3270_set_host_type(hSession,g_value_get_int(&value));

			g_value_unset(&value);

		}

	}

	// Apply color type
	{
		GtkTreeIter	iter;

		if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget->input.combo[1]), &iter))
		{
			GValue value   = { 0, };

			gtk_tree_model_get_value(gtk_combo_box_get_model(GTK_COMBO_BOX(widget->input.combo[1])),&iter,1,&value);

			lib3270_set_color_type(hSession,g_value_get_int(&value));

			g_value_unset(&value);

		}

	}

}

static void load(GtkWidget *w, GtkWidget *terminal)
{
	debug("V3270HostSelectWidget::%s",__FUNCTION__);

	H3270 *hSession = v3270_get_session(terminal);
	V3270HostSelectWidget *widget = GTK_V3270HostSelectWidget(w);

	const gchar * u = lib3270_get_url(hSession);

	if(u)
    {

        g_autofree gchar * url = g_strdup(u);
        debug("URL=[%s]",url);

		gtk_toggle_button_set_active(widget->input.ssl,g_str_has_prefix(u,"tn3270s"));

        gchar *hostname = strstr(url,"://");
        if(!hostname)
        {
            g_message("Invalid URL: \"%s\" (no scheme)",url);
        }
        else
        {
            hostname += 3;

            gchar *srvcname = strchr(hostname,':');

            if(srvcname)
            {
                *(srvcname++) = 0;
            }
            else
            {
                srvcname = "telnet";
            }

            gtk_entry_set_text(widget->input.entry[ENTRY_HOSTNAME],hostname);
            gtk_entry_set_text(widget->input.entry[ENTRY_SRVCNAME],srvcname);

        }

    }

	LIB3270_HOST_TYPE type = lib3270_get_host_type(hSession);

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
		int				  colors = (int) lib3270_get_color_type(hSession);

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

}


static void V3270HostSelectWidget_class_init(G_GNUC_UNUSED V3270HostSelectWidgetClass *klass)
{
	GTK_V3270_SETTINGS_CLASS(klass)->apply = apply;
	GTK_V3270_SETTINGS_CLASS(klass)->load = load;
}

static void V3270HostSelectWidget_init(V3270HostSelectWidget *widget)
{
	int f;

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

LIB3270_EXPORT GtkWidget * v3270_host_select_new()
{
	return GTK_WIDGET(g_object_new(GTK_TYPE_V3270HostSelectWidget, NULL));
}

LIB3270_EXPORT void v3270_select_host(GtkWidget *widget)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	if(v3270_is_connected(widget))
	{
		gdk_display_beep(gdk_display_get_default());
		return;
	}

	debug("V3270HostSelectWidget::%s",__FUNCTION__);

	GtkWidget * dialog = v3270_settings_dialog_new(widget, v3270_host_select_new());

	v3270_dialog_setup(dialog,_("Setup host"),_("C_onnect"));

	gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 150);
	gtk_widget_show_all(dialog);

	gboolean again = TRUE;
 	while(again)
 	{
 		gtk_widget_set_sensitive(dialog,TRUE);
		gtk_widget_set_visible(dialog,TRUE);

 		switch(gtk_dialog_run(GTK_DIALOG(dialog)))
 		{
		case GTK_RESPONSE_APPLY:
			debug("V3270HostSelectWidget::%s=%s",__FUNCTION__,"GTK_RESPONSE_APPLY");
			again = lib3270_reconnect(v3270_get_session(widget),0);
			break;

		case GTK_RESPONSE_CANCEL:
			again = FALSE;
			debug("V3270HostSelectWidget::%s=%s",__FUNCTION__,"GTK_RESPONSE_CANCEL");
			break;
 		}
 	}

 	debug("%s end",__FUNCTION__);
	gtk_widget_destroy(dialog);

}

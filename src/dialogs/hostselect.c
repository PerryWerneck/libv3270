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

/*
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
 */

 enum _entry
 {
 	ENTRY_HOSTNAME,
 	ENTRY_SRVCNAME,

 	ENTRY_COUNT
 };

 static const struct Combos
 {
    gint top;
 	gint left;
 	const gchar * label;
 	const gchar * tooltip;
	const unsigned int * values;
 	const gchar **labels;

 	unsigned int (*get)(const H3270 *);
 	int (*set)(H3270 *, unsigned int);

 } combos[] =
 {
 	{
 		.top = 2,
		.left = 0,

		.label = N_("System _type"),
		.get = (unsigned int (*)(const H3270 *)) lib3270_get_host_type,
		.set = (int (*)(H3270 *, unsigned int)) lib3270_set_host_type,

		.values = (const unsigned int [])
		{
			LIB3270_HOST_S390,
			LIB3270_HOST_AS400,
			LIB3270_HOST_TSO,
			0,
		},

		.labels = (const gchar *[])
		{
			N_( "IBM S/390"			),
			N_( "IBM AS/400"		),
			N_( "Other (TSO)" 		),
			N_( "Other (VM/CMS)"    ),
			NULL
		}

 	},
 	{
 		.top = 2,
		.left = 3,

		.label = N_("_Color table"),
		.get = lib3270_get_color_type,
		.set = lib3270_set_color_type,

		.values = (const unsigned int [])
		{
			0,
			2,
			8,
			16,
		},

		.labels = (const gchar *[])
		{
			N_( "System default"),
			N_( "Monochrome" ),
			N_( "8 colors"	 ),
			N_( "16 colors"  ),
			NULL
		}

 	},
 	{
 		.top = 3,
		.left = 0,

		.label = N_("_Model"),
		.tooltip = N_("The model of 3270 display to be emulated"),

		.get = lib3270_get_model_number,
		.set = lib3270_set_model_number,

		.values = (const unsigned int [])
		{
			2,
			3,
			4,
			5
		},

		.labels = (const gchar *[])
		{
			N_( "Model 2 - 80x24"		),
			N_( "Model 3 - 80x32"		),
			N_( "Model 4 - 80x43"		),
			N_( "Model 5 - 132x27"	),
			NULL
		}

 	}
 };

 struct _V3270HostSelectWidget
 {
 	V3270Settings parent;

	struct
	{
		GtkEntry			* entry[ENTRY_COUNT];				/**< @brief Entry fields for host & service name */
		GtkToggleButton		* ssl;								/**< @brief SSL Connection? */
		GtkComboBox			* combos[G_N_ELEMENTS(combos)];		/**< @brief Combo-boxes */

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

	// Apply combos.
	size_t combo;
	for(combo = 0; combo < G_N_ELEMENTS(combos); combo++)
	{
		GtkTreeIter	iter;

		if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget->input.combos[combo]), &iter))
		{
			GValue value = { 0, };
			gtk_tree_model_get_value(gtk_combo_box_get_model(GTK_COMBO_BOX(widget->input.combos[combo])),&iter,1,&value);

			combos[combo].set(hSession,g_value_get_uint(&value));

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

	size_t combo;
	for(combo = 0; combo < G_N_ELEMENTS(combos); combo++)
	{

		GtkTreeModel	* model = gtk_combo_box_get_model(widget->input.combos[combo]);
		GtkTreeIter		  iter;
		unsigned int	  value = combos[combo].get(hSession);

		if(gtk_tree_model_get_iter_first(model,&iter))
		{
			GValue gVal = { 0, };

			do
			{
				gtk_tree_model_get_value(model,&iter,1,&gVal);

				if(g_value_get_uint(&gVal) == value)
				{
					gtk_combo_box_set_active_iter(widget->input.combos[combo],&iter);
					break;
				}

			} while(gtk_tree_model_iter_next(model,&iter));

			g_value_unset(&gVal);

		}

	}

}

static void update_message(GtkWidget *widget, GtkWidget *terminal)
{
	gtk_widget_set_sensitive(widget, lib3270_is_disconnected(v3270_get_session(terminal)));
}

static void V3270HostSelectWidget_class_init(G_GNUC_UNUSED V3270HostSelectWidgetClass *klass)
{
	V3270SettingsClass * widget = GTK_V3270_SETTINGS_CLASS(klass);

	widget->apply = apply;
	widget->load = load;
	widget->update_message = update_message;

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
		gtk_widget_set_halign(GTK_WIDGET(widget->input.ssl),GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(widget),GTK_WIDGET(widget->input.ssl),3,1,1,1);
	}

	// Create combo boxes
	{
		GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();

		size_t combo, item;

		for(combo = 0; combo < G_N_ELEMENTS(combos); combo++) {

			GtkTreeModel * model = (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_UINT);

			widget->input.combos[combo] = GTK_COMBO_BOX(gtk_combo_box_new_with_model(model));

			if(combos[combo].tooltip)
				gtk_widget_set_tooltip_markup(GTK_WIDGET(widget->input.combos[combo]),combos[combo].tooltip);

			gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget->input.combos[combo]), renderer, TRUE);
			gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget->input.combos[combo]), renderer, "text", 0, NULL);

			for(item = 0; combos[combo].labels[item]; item++)
			{
				GtkTreeIter iter;
				gtk_list_store_append((GtkListStore *) model, &iter);
				gtk_list_store_set((GtkListStore *) model, &iter, 0, gettext(combos[combo].labels[item]), 1, combos[combo].values[item], -1);
			}

			GtkWidget *label = gtk_label_new_with_mnemonic(gettext(combos[combo].label));
			gtk_widget_set_halign(label,GTK_ALIGN_END);
			gtk_grid_attach(GTK_GRID(widget),label,combos[combo].left,combos[combo].top,1,1);
			gtk_grid_attach(GTK_GRID(widget),GTK_WIDGET(widget->input.combos[combo]),combos[combo].left+1,combos[combo].top,2,1);

		}

	}

	gtk_widget_show_all(GTK_WIDGET(widget));

}

LIB3270_EXPORT GtkWidget * v3270_host_select_new()
{
 	V3270Settings * settings = GTK_V3270_SETTINGS(g_object_new(GTK_TYPE_V3270HostSelectWidget, NULL));

 	settings->title = _("Host definition");
 	settings->label = _("Host");

 	return GTK_WIDGET(settings);
}

LIB3270_EXPORT void v3270_select_host(GtkWidget *widget)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	GtkWidget * dialog = v3270_settings_dialog_new();

	gtk_window_set_title(GTK_WINDOW(dialog), _("Host definition"));
	gtk_container_add(GTK_CONTAINER(dialog), v3270_host_select_new());

	gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(gtk_widget_get_toplevel(widget)));
	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

	v3270_settings_dialog_set_terminal_widget(dialog, widget);

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
			v3270_settings_dialog_apply(dialog);
			again = lib3270_reconnect(v3270_get_session(widget),0);
			break;

		case GTK_RESPONSE_CANCEL:
			again = FALSE;
			debug("V3270HostSelectWidget::%s=%s",__FUNCTION__,"GTK_RESPONSE_CANCEL");
			v3270_settings_dialog_revert(dialog);
			break;
 		}
 	}

	gtk_widget_destroy(dialog);

}

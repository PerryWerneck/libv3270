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

 /**
  * @brief Implements the accelerator settings widget.
  *
  */

 #include <internals.h>
 #include <v3270/dialogs.h>
 #include <v3270/settings.h>
 #include <lib3270/log.h>
 #include <v3270/actions.h>
 #include <terminal.h>

/*--[ Widget Definition ]----------------------------------------------------------------------------*/

 #define GTK_TYPE_V3270_ACCELERATOR_SETTINGS			(V3270AcceleratorSettings_get_type ())
 #define GTK_V3270_ACCELERATOR_SETTINGS(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270_ACCELERATOR_SETTINGS, V3270AcceleratorSettings))
 #define GTK_V3270_ACCELERATOR_SETTINGS_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270_ACCELERATOR_SETTINGS, V3270AcceleratorSettingsClass))
 #define GTK_IS_V3270_ACCELERATOR_SETTINGS(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270_ACCELERATOR_SETTINGS))
 #define GTK_IS_V3270_ACCELERATOR_SETTINGS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270_ACCELERATOR_SETTINGS))
 #define GTK_V3270_ACCELERATOR_SETTINGS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270_ACCELERATOR_SETTINGS, V3270AcceleratorSettingsClass))

 typedef struct _V3270AcceleratorSettings
 {
 	V3270Settings parent;

 	GtkListStore * store;

 } V3270AcceleratorSettings;

 typedef struct _V3270AcceleratorSettingsClass
 {
	V3270SettingsClass parent;

 } V3270AcceleratorSettingsClass;

 G_DEFINE_TYPE(V3270AcceleratorSettings, V3270AcceleratorSettings, GTK_TYPE_V3270_SETTINGS);

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static void load(GtkWidget *w, GtkWidget *terminal);
 static void apply(GtkWidget *w, GtkWidget *terminal);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void V3270AcceleratorSettings_class_init(V3270AcceleratorSettingsClass *klass)
 {
 	klass->parent.apply = apply;
	klass->parent.load = load;

 }

 enum
 {
 	ACTION,				///< @brief The action object.
	DESCRIPTION,		///< @brief The action description.
	MAIN_MASK,			///< @brief The mask for the main accelerator.
	MAIN_VALUE,			///< @brief The value for the main accelerator.
	ALTERNATIVE_MASK,	///< @brief The mask for the alternative accelerator.
	ALTERNATIVE_VALUE,	///< @brief The value for the alternative accelerator.

	COLUMNS				///< @brief Number of view columns.
 };

 static void V3270AcceleratorSettings_init(V3270AcceleratorSettings *widget)
 {
 	// Create Accelerator list
 	GtkCellRenderer * text_renderer = gtk_cell_renderer_text_new();

 	GtkCellRenderer * accel_renderer = gtk_cell_renderer_accel_new();
	g_object_set(
		accel_renderer,
			"accel-mode", GTK_CELL_RENDERER_ACCEL_MODE_OTHER,
			"editable", TRUE,
		NULL);

	widget->store = GTK_LIST_STORE(gtk_list_store_new(COLUMNS, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_INT, G_TYPE_UINT, G_TYPE_INT, G_TYPE_UINT));

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(widget->store),1,GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID);

	GtkWidget * view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(widget->store));

	gtk_widget_set_tooltip_markup(view,_("Keyboard accelerators"));

	gtk_tree_view_insert_column_with_attributes(
		GTK_TREE_VIEW(view),
		-1,
		_("Action"),
		text_renderer,
		"text",
		DESCRIPTION,
		NULL
	);

	gtk_tree_view_insert_column_with_attributes(
		GTK_TREE_VIEW(view),
		-1,
		_("Accelerator"),
		accel_renderer,
		"accel-mods", MAIN_MASK,
		"accel-key", MAIN_VALUE,
		NULL
	);

	gtk_tree_view_insert_column_with_attributes(
		GTK_TREE_VIEW(view),
		-1,
		_("Alternative"),
		accel_renderer,
		"accel-mods", ALTERNATIVE_MASK,
		"accel-key", ALTERNATIVE_VALUE,
		NULL
	);

	// Create scroller view
	{
		GtkWidget * box	= gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(box),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(box),view);

		gtk_widget_set_vexpand(view,TRUE);
		gtk_widget_set_hexpand(view,TRUE);

		gtk_widget_set_vexpand(box,TRUE);
		gtk_widget_set_hexpand(box,TRUE);

		gtk_grid_attach(GTK_GRID(widget),box,0,0,4,4);
	}

 }

LIB3270_EXPORT GtkWidget * v3270_accelerator_settings_new()
{
 	V3270Settings * settings = GTK_V3270_SETTINGS(g_object_new(GTK_TYPE_V3270_ACCELERATOR_SETTINGS, NULL));

 	settings->title = _("Accelerator properties");
 	settings->label = _("Accelerators");

 	return GTK_WIDGET(settings);
}


void load(GtkWidget *widget, GtkWidget *terminal)
{
	struct KeyMap
	{
		guint           	  key;
		GdkModifierType 	  mods;
	};

	debug("%s::%s","V3270AcceleratorSettings",__FUNCTION__);

	GtkListStore * store = GTK_V3270_ACCELERATOR_SETTINGS(widget)->store;
	GSList		 * accelerator = GTK_V3270(terminal)->accelerators;

	while(accelerator)
	{
		struct KeyMap keymaps[2];
		size_t ix = 0;
		V3270Accelerator * current = (V3270Accelerator *) accelerator->data;

		memset(keymaps,0,sizeof(keymaps));

		while(accelerator && (((V3270Accelerator *) accelerator->data)->activate == current->activate) && (((V3270Accelerator *) accelerator->data)->arg == current->arg))
		{
			V3270Accelerator *accel = (V3270Accelerator *) accelerator->data;

			if(ix < G_N_ELEMENTS(keymaps))
			{
				keymaps[ix].key = accel->key;
				keymaps[ix].mods = accel->mods;
				ix++;
			}

			accelerator = g_slist_next(accelerator);
		}

		// Add entry
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(
			store,
			&iter,
			ACTION,				current,
			DESCRIPTION,		v3270_accelerator_get_description(current),
			MAIN_MASK,			keymaps[0].mods,
			MAIN_VALUE,			keymaps[0].key,
			ALTERNATIVE_MASK,	keymaps[1].mods,
			ALTERNATIVE_VALUE,	keymaps[1].key,
			-1
		);

	}

}

void apply(GtkWidget *widget, GtkWidget *terminal)
{
	debug("%s::%s","V3270AcceleratorSettings",__FUNCTION__);
}

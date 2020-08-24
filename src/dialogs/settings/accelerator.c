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
  * References:
  *
  * <https://github.com/Apress/foundations-of-gtk-dev/blob/master/chapter_8/accelerators.c>
  *
  */

 #include <internals.h>
 #include <v3270/dialogs.h>
 #include <v3270/settings.h>
 #include <lib3270/log.h>
 #include <v3270/actions.h>
 #include <terminal.h>
 #include <gdk/gdkkeysyms-compat.h>

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

 struct KeyMap
 {
	guint           	  key;
	GdkModifierType 	  mods;
 };

 static void load(GtkWidget *w, GtkWidget *terminal);
 static void apply(GtkWidget *w, GtkWidget *terminal);

 static void accel_edited(GtkCellRendererAccel*, gchar*, guint, GdkModifierType, guint, V3270AcceleratorSettings*);
 static void alternative_edited(GtkCellRendererAccel*, gchar*, guint, GdkModifierType, guint, V3270AcceleratorSettings*);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void V3270AcceleratorSettings_class_init(V3270AcceleratorSettingsClass *klass)
 {
 	klass->parent.apply = apply;
	klass->parent.load = load;

 }

 enum
 {
 	ACTION,				///< @brief The action object.
 	NAME,				///< @brief The action name.
	SUMMARY,			///< @brief The action summary.
	MAIN_MASK,			///< @brief The mask for the main accelerator.
	MAIN_VALUE,			///< @brief The value for the main accelerator.
	ALTERNATIVE_MASK,	///< @brief The mask for the alternative accelerator.
	ALTERNATIVE_VALUE,	///< @brief The value for the alternative accelerator.

	COLUMNS				///< @brief Number of view columns.
 };

 static void realize(GtkWidget G_GNUC_UNUSED(*widget), GtkTreeView *view)
 {
 	gtk_tree_view_columns_autosize(view);
 }

 static void V3270AcceleratorSettings_init(V3270AcceleratorSettings *widget)
 {
 	size_t ix;

 	// Create description list
 	GtkCellRenderer * text_renderer = gtk_cell_renderer_text_new();

 	// Create accelerator render
 	GtkCellRenderer * accel_renderer[] = { gtk_cell_renderer_accel_new(), gtk_cell_renderer_accel_new() };

	g_object_set(
		accel_renderer[0],
			"accel-mode", GTK_CELL_RENDERER_ACCEL_MODE_OTHER,
			"editable", TRUE,
		NULL);

	g_object_set(
		accel_renderer[1],
			"accel-mode", GTK_CELL_RENDERER_ACCEL_MODE_OTHER,
			"editable", TRUE,
		NULL);

	g_signal_connect (G_OBJECT(accel_renderer[0]), "accel_edited", G_CALLBACK (accel_edited), widget);
	g_signal_connect (G_OBJECT(accel_renderer[1]), "accel_edited", G_CALLBACK (alternative_edited), widget);

	widget->store = GTK_LIST_STORE(gtk_list_store_new(COLUMNS, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_UINT, G_TYPE_INT, G_TYPE_UINT));

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(widget->store),1,GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID);

	GtkWidget * view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(widget->store));

	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(view),TRUE);
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(view),1);


	g_signal_connect(G_OBJECT(widget),"realize",G_CALLBACK(realize),view);

	gtk_widget_set_tooltip_markup(view,_("Keyboard accelerators"));
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(view),FALSE);

	GtkTreeViewColumn * column;

	// Name columns
	column =
		gtk_tree_view_column_new_with_attributes(
			_("Name"),
			text_renderer,
			"text",
			NAME,
			NULL
		);

	gtk_tree_view_column_set_resizable(column, TRUE);

	gtk_tree_view_insert_column(
		GTK_TREE_VIEW(view),
		column,
		-1
	);

	// Summary column
	column =
		gtk_tree_view_column_new_with_attributes(
			_("Summary"),
			text_renderer,
			"text",
			SUMMARY,
			NULL
		);

	gtk_tree_view_column_set_min_width(column, 500);
	gtk_tree_view_column_set_resizable(column, TRUE);

	gtk_tree_view_insert_column(
		GTK_TREE_VIEW(view),
		column,
		-1
	);

	// Accelerator columns
	gtk_tree_view_insert_column_with_attributes(
		GTK_TREE_VIEW(view),
		-1,
		_("Accelerator"),
		accel_renderer[0],
		"accel-mods", MAIN_MASK,
		"accel-key", MAIN_VALUE,
		NULL
	);

	gtk_tree_view_insert_column_with_attributes(
		GTK_TREE_VIEW(view),
		-1,
		_("Alternative"),
		accel_renderer[1],
		"accel-mods", ALTERNATIVE_MASK,
		"accel-key", ALTERNATIVE_VALUE,
		NULL
	);

	for(ix = 1; ix < 3; ix++)
	{
		column = gtk_tree_view_get_column(GTK_TREE_VIEW(view), ix);
		gtk_tree_view_column_set_min_width(column, 200);
		gtk_tree_view_column_set_resizable(column, TRUE);
	}

	// Create scroller view
	{
		GtkWidget * box	= gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(box),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(box),view);

		/*
		gtk_widget_set_vexpand(view,TRUE);
		gtk_widget_set_hexpand(view,TRUE);
		*/

		gtk_widget_set_vexpand(box,TRUE);
		gtk_widget_set_hexpand(box,TRUE);

		gtk_grid_attach(GTK_GRID(widget),box,0,0,10,10);
	}

}


LIB3270_EXPORT GtkWidget * v3270_accelerator_settings_new()
{
 	V3270Settings * settings = GTK_V3270_SETTINGS(g_object_new(GTK_TYPE_V3270_ACCELERATOR_SETTINGS, NULL));

 	settings->title = _("Keyboard accelerators");
 	settings->label = _("Accelerators");

 	return GTK_WIDGET(settings);
}

struct AccelEditInfo
{
	GtkWidget * widget;
	gint response;
	V3270Accelerator * origin;
	V3270Accelerator * accel;
	GtkTreePath *path;
	guint accel_key;
	GdkModifierType mask;
	int id_key;
	int id_mask;
};



static gboolean check_accel(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, struct AccelEditInfo * info)
{
	static const gint columns[] = { MAIN_MASK, MAIN_VALUE, ALTERNATIVE_MASK, ALTERNATIVE_VALUE };
	size_t ix;
	GValue value;

//	debug("%s",__FUNCTION__);

	for(ix = 0; ix < 2; ix++)
	{
		guint          	  key;
		GdkModifierType	  mask;

		memset(&value,0,sizeof(value));
		gtk_tree_model_get_value(model, iter, columns[(ix * 2)], &value);
		mask = (GdkModifierType) g_value_get_int(&value);
		g_value_unset(&value);

		memset(&value,0,sizeof(value));
		gtk_tree_model_get_value(model, iter, columns[(ix * 2)+1], &value);
		key = g_value_get_uint(&value);
		g_value_unset(&value);

		if(key == info->accel_key && mask == info->mask) {

			debug("Index %d cmp=%d",(unsigned int) ix, gtk_tree_path_compare(path, info->path));
            GtkWidget * dialog;

			if(gtk_tree_path_compare(path, info->path))
			{
				// Another entry, rejects.
				memset(&value,0,sizeof(value));
				gtk_tree_model_get_value(model, iter, ACTION, &value);
				info->accel = (V3270Accelerator *) g_value_get_pointer(&value);
				g_value_unset(&value);

				debug("Key is already set on accel %s",v3270_accelerator_get_name(info->accel));

				// Ask for what the user wants to do.
				dialog = gtk_message_dialog_new_with_markup(
								GTK_WINDOW(gtk_widget_get_toplevel(info->widget)),
								GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_QUESTION,
								GTK_BUTTONS_YES_NO,
								_( "The selected accelerator is in use by action <b>%s</b> (<b>%s</b>)" ),
								v3270_accelerator_get_description(info->accel),
								v3270_accelerator_get_name(info->accel)
							);

				gtk_message_dialog_format_secondary_markup(
					GTK_MESSAGE_DIALOG(dialog),
					_( "Assign it to action (<b>%s</b>)?"),
					v3270_accelerator_get_name(info->origin)
				);

			}
			else
			{
				// It's on the same entry.
				dialog = gtk_message_dialog_new_with_markup(
								GTK_WINDOW(gtk_widget_get_toplevel(info->widget)),
								GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_INFO,
								GTK_BUTTONS_CANCEL,
								_( "The selected accelerator is in use by the same action" )
							);

			}

			gtk_window_set_title(GTK_WINDOW(dialog),_("Accelerator is in use"));
			gtk_widget_show_all(dialog);

			info->response = gtk_dialog_run(GTK_DIALOG(dialog));
			if(info->response == GTK_RESPONSE_YES)
			{
				debug("%s: Removing accelerator from the other action",__FUNCTION__);
				gtk_list_store_set(
					GTK_LIST_STORE(model),
					iter,
					columns[(ix * 2)],		0,
					columns[(ix * 2)+1],	0,
					-1
				);

			}

			gtk_widget_destroy(dialog);

			return TRUE;

		}

	}

	return FALSE;

}

static void change_accel(V3270AcceleratorSettings *widget, gchar *path, guint accel_key, GdkModifierType mask, int id_key, int id_mask)
{
	struct AccelEditInfo info = {
		.response = GTK_RESPONSE_YES,
		.widget = GTK_WIDGET(widget),
		.accel = NULL,
		.accel_key = accel_key,
		.mask = mask,
		.id_key = id_key,
		.id_mask = id_mask,
		.path = gtk_tree_path_new_from_string(path)
	};

	GtkTreeIter iter;
	if(gtk_tree_model_get_iter(GTK_TREE_MODEL(widget->store),&iter,info.path))
	{
		GValue value;
		memset(&value,0,sizeof(value));
		gtk_tree_model_get_value(GTK_TREE_MODEL(widget->store), &iter, ACTION, &value);
		info.origin = (V3270Accelerator *) g_value_get_pointer(&value);
		g_value_unset(&value);

		if(!info.origin)
		{
			g_warning("Can't identify the origin accelerator, aborting action");
			return;
		}

	}

	gtk_tree_model_foreach(GTK_TREE_MODEL(widget->store), (GtkTreeModelForeachFunc) check_accel, &info);

	if(info.response == GTK_RESPONSE_YES && gtk_tree_model_get_iter(GTK_TREE_MODEL(widget->store),&iter,info.path))
	{
		debug("%s: Setting the new accelerator to %u/%d",__FUNCTION__,accel_key,mask);

		gtk_list_store_set(
			widget->store,
			&iter,
			id_key,		accel_key,
			id_mask,	mask,
			-1
		);

	}

	gtk_tree_path_free(info.path);

}

static void accel_edited(GtkCellRendererAccel G_GNUC_UNUSED(*accel), gchar *path, guint accel_key, GdkModifierType mask, guint G_GNUC_UNUSED(hardware_keycode), V3270AcceleratorSettings *widget)
{
#ifdef DEBUG
	{
		g_autofree gchar * keyname = gtk_accelerator_name(accel_key,mask);
		debug("%s(%s) = %u/%d (%s)",__FUNCTION__,path,accel_key,mask,keyname);
	}
#endif // DEBUG
	change_accel(widget, path, accel_key, mask, MAIN_VALUE, MAIN_MASK);
}

static void alternative_edited(GtkCellRendererAccel G_GNUC_UNUSED(*renderer), gchar *path, guint accel_key, GdkModifierType mask, guint G_GNUC_UNUSED(hardware_keycode), V3270AcceleratorSettings *widget)
{
#ifdef DEBUG
	{
		g_autofree gchar * keyname = gtk_accelerator_name(accel_key,mask);
		debug("%s(%s) = %u/%d (%s)",__FUNCTION__,path,accel_key,mask,keyname);
	}
#endif // DEBUG

	// Check for "single-accel" actions
	V3270Accelerator *accel = NULL;
	GtkTreePath * tree_path = gtk_tree_path_new_from_string(path);
	GtkTreeIter iter;
	if(gtk_tree_model_get_iter(GTK_TREE_MODEL(widget->store),&iter,tree_path))
	{
		GValue value;
		memset(&value,0,sizeof(value));
		gtk_tree_model_get_value(GTK_TREE_MODEL(widget->store), &iter, ACTION, &value);
		accel = (V3270Accelerator *) g_value_get_pointer(&value);
		g_value_unset(&value);
	}
	gtk_tree_path_free(tree_path);

	// Call the common validation.
	change_accel(widget, path, accel_key, mask, ALTERNATIVE_VALUE, ALTERNATIVE_MASK);
}

void load(GtkWidget *widget, GtkWidget *terminal)
{
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
			NAME,				v3270_accelerator_get_name(current),
			SUMMARY,			v3270_accelerator_get_summary(current),
			MAIN_MASK,			keymaps[0].mods,
			MAIN_VALUE,			keymaps[0].key,
			ALTERNATIVE_MASK,	keymaps[1].mods,
			ALTERNATIVE_VALUE,	keymaps[1].key,
			-1
		);


	}

}

static gboolean add_accel(GtkTreeModel *model, GtkTreePath G_GNUC_UNUSED(*path), GtkTreeIter *iter, GSList **accelerators)
{
	static const gint columns[] = { MAIN_MASK, MAIN_VALUE, ALTERNATIVE_MASK, ALTERNATIVE_VALUE };
	size_t ix;
	GValue value;
	struct KeyMap keymap[2];

	V3270Accelerator * accel;

	memset(&value,0,sizeof(value));
	gtk_tree_model_get_value(model, iter, ACTION, &value);
	accel = (V3270Accelerator *) g_value_get_pointer(&value);
	g_value_unset(&value);

	for(ix = 0; ix < 2; ix++)
	{
		memset(&value,0,sizeof(value));
		gtk_tree_model_get_value(model, iter, columns[(ix * 2)], &value);
		keymap[ix].mods = (GdkModifierType) g_value_get_int(&value);
		g_value_unset(&value);

		memset(&value,0,sizeof(value));
		gtk_tree_model_get_value(model, iter, columns[(ix * 2)+1], &value);
		keymap[ix].key = g_value_get_uint(&value);
		g_value_unset(&value);

	}

	// Allways create the "main" accelerator to keep the action active.
	V3270Accelerator * acc = v3270_accelerator_clone(accel);

	acc->key	= keymap[0].key;
	acc->mods	= keymap[0].mods;
	*accelerators = g_slist_prepend(*accelerators,acc);

	// The alternative one is created only when set.
	if(keymap[1].key)
	{
		acc = v3270_accelerator_clone(accel);
		acc->key	= keymap[1].key;
		acc->mods	= keymap[1].mods;
		*accelerators = g_slist_prepend(*accelerators,acc);
	}

	return FALSE;
}

void apply(GtkWidget *s, GtkWidget *t)
{
	v3270 						* terminal = GTK_V3270(t);
	V3270AcceleratorSettings	* settings = GTK_V3270_ACCELERATOR_SETTINGS(s);

	debug("%s::%s","V3270AcceleratorSettings",__FUNCTION__);

    // Create a new accelerator table.
	GSList * accelerators = NULL;

	gtk_tree_model_foreach(GTK_TREE_MODEL(settings->store), (GtkTreeModelForeachFunc) add_accel, &accelerators);

    // Replace the accelerator table.
	if(terminal->accelerators)
		g_slist_free_full(terminal->accelerators,g_free);

	terminal->accelerators = v3270_accelerator_map_sort(accelerators);

}


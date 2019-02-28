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

 #include <internals.h>
 #include "private.h"
 #include "marshal.h"

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 enum _SIGNALS
 {
 	V3270_ACTIVITY_LIST_HAS_FILE_SIGNAL,	///< @brief Indicates if the list has a file name set.
 	V3270_ACTIVITY_LIST_SELECTED_SIGNAL,	///< @brief Indicates if the list has a file name set.

 	V3270_ACTIVITY_LIST_LAST_SIGNAL
 };

 typedef struct _activities
 {

 }  Activities;

 struct _V3270FTActivityListClass
 {
 	GtkTreeViewClass parent_class;

 	struct
 	{
		void (*has_file)(GtkWidget *, gboolean);
 	} signal;

 };

 struct _V3270FTActivityList
 {
 	GtkTreeView parent;
 	gchar * filename;
 };

 G_DEFINE_TYPE(V3270FTActivityList, V3270FTActivityList, GTK_TYPE_TREE_VIEW);

 static guint v3270_activity_list_signals[V3270_ACTIVITY_LIST_LAST_SIGNAL] = { 0 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void dispose(GObject *object)
 {
	debug("activityList::%s(%p)",__FUNCTION__,object);

	V3270FTActivityList * list = GTK_V3270_FT_ACTIVITY_LIST(object);

	// Release filename
	debug("Freeing %s",list->filename);
	g_free(list->filename);
	list->filename = NULL;

	GtkTreeIter		  iter;
	GtkTreeModel	* model	= gtk_tree_view_get_model(GTK_TREE_VIEW(object));

	while(model && gtk_tree_model_get_iter_first(model,&iter))
	{
		GObject * activity = NULL;
		gtk_tree_model_get(model, &iter, 0, &activity, -1);

		gtk_list_store_remove(GTK_LIST_STORE(model),&iter);
		debug("Disposing activity %p",activity);

		g_clear_object(&activity);

	}


	G_OBJECT_CLASS(V3270FTActivityList_parent_class)->dispose(object);

 }

 static void V3270FTActivityList_has_file(GtkWidget G_GNUC_UNUSED(*widget), gboolean G_GNUC_UNUSED(has_file))
 {
 	debug("%s",__FUNCTION__);
 }

 static void row_activated(GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn G_GNUC_UNUSED(*column))
 {
 	debug("%s",__FUNCTION__);

	GtkTreeIter iter;
	GtkTreeModel * model = gtk_tree_view_get_model(view);

	if(gtk_tree_model_get_iter(model, &iter, path))
	{
		GObject * activity = NULL;
		gtk_tree_model_get(model, &iter, 0, &activity, -1);

		if(activity)
		{
			debug("%s: activity is %p",__FUNCTION__,activity);
			g_signal_emit(view, v3270_activity_list_signals[V3270_ACTIVITY_LIST_SELECTED_SIGNAL], 0, activity);
		}

	}
 }

 static void V3270FTActivityList_class_init(V3270FTActivityListClass *klass)
 {
	GObjectClass 		* gobject_class 	= G_OBJECT_CLASS(klass);
	GtkTreeViewClass	* treeview_class	= GTK_TREE_VIEW_CLASS(klass);

	gobject_class->dispose = dispose;

	treeview_class->row_activated = row_activated;

	klass->signal.has_file = V3270FTActivityList_has_file;

	v3270_activity_list_signals[V3270_ACTIVITY_LIST_HAS_FILE_SIGNAL] =
		g_signal_new(	"has-file",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (V3270FTActivityListClass, signal.has_file),
						NULL, NULL,
						v3270ft_VOID__VOID_BOOLEAN,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	v3270_activity_list_signals[V3270_ACTIVITY_LIST_SELECTED_SIGNAL] =
		g_signal_new(	"changed",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						v3270ft_VOID__VOID_POINTER,
						G_TYPE_NONE, 1, G_TYPE_POINTER);


 }

 static void render_local(GtkTreeViewColumn G_GNUC_UNUSED(*tree_column), GtkCellRenderer *cell, GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer G_GNUC_UNUSED(data))
 {
	GObject * activity;
	gtk_tree_model_get(tree_model, iter, 0, &activity, -1);
	g_object_set(G_OBJECT(cell),"text",v3270_ft_activity_get_local_filename(activity),NULL);
 }

 static void render_remote(GtkTreeViewColumn G_GNUC_UNUSED(*tree_column), GtkCellRenderer *cell, GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer G_GNUC_UNUSED(data))
 {
	GObject * activity;
	gtk_tree_model_get(tree_model, iter, 0, &activity, -1);
	g_object_set(G_OBJECT(cell),"text",v3270_ft_activity_get_remote_filename(activity),NULL);
 }

 gboolean v3270_activity_list_append_filename(GtkWidget *widget, const gchar *filename, gboolean select)
 {
	debug("%s(%s,%s)",__FUNCTION__,filename,select ? "SELECT" : "NO-SELECT");

 	GtkTreeModel * model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	GtkTreeIter iter;

	if(gtk_tree_model_get_iter_first(model,&iter))
	{
		do
		{
			GObject * activity = NULL;
			gtk_tree_model_get(model, &iter, 0, &activity, -1);

			if(activity && !strcmp(filename,v3270_ft_activity_get_local_filename(activity)))
			{
				debug("%s already in the list",filename);
				if(select)
				{
					gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(widget)),&iter);
				}
				return FALSE;
			}


		}
		while(gtk_tree_model_iter_next(model,&iter));
	}

	// Append filename
	v3270_activity_list_append(widget,v3270_ft_activity_new_from_filename(filename),select);

	return TRUE;
 }

 guint v3270_activity_list_set_from_selection(GtkWidget *widget, GtkSelectionData *data)
 {
	gchar	**uris 	= g_strsplit((const gchar *) gtk_selection_data_get_text(data),"\n",-1);
	guint	  rc	= 0;
	size_t	  ix;

	for(ix = 0; uris[ix]; ix++)
	{
		if(!g_ascii_strncasecmp("file:///",uris[ix],8)) {
			if(v3270_activity_list_append_filename(widget,uris[ix]+7,TRUE))
				rc++;
		}
	}

	g_strfreev(uris);

	debug("%s exits with rc=%u",__FUNCTION__,rc);
	return rc;
}


 static void drag_data_received(GtkWidget *widget, GdkDragContext *context, G_GNUC_UNUSED gint x, G_GNUC_UNUSED gint y, GtkSelectionData *data, G_GNUC_UNUSED guint info, guint time)
 {
	debug("activitylist::%s",__FUNCTION__);
	gtk_drag_finish(context, v3270_activity_list_set_from_selection(widget, data) > 0, FALSE, time);

 }

 static void V3270FTActivityList_init(V3270FTActivityList *widget)
 {
	GtkTreeModel * model = GTK_TREE_MODEL(gtk_list_store_new(1,G_TYPE_POINTER));	// Using pointer type because I take care of refcounts.

	widget->filename = NULL;

	gtk_tree_view_set_model(GTK_TREE_VIEW(widget),model);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget),TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(widget),TRUE);

	gtk_tree_view_insert_column_with_data_func(
		GTK_TREE_VIEW(widget),
		-1,
		_( "Local file" ),
		gtk_cell_renderer_text_new(),
		render_local,
		0, NULL
	);

	gtk_tree_view_insert_column_with_data_func(
		GTK_TREE_VIEW(widget),
		-1,
		_( "Remote file" ),
		gtk_cell_renderer_text_new(),
		render_remote,
		0, NULL
	);

	gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(widget),TRUE);
	v3270_drag_dest_set(GTK_WIDGET(widget), G_CALLBACK(drag_data_received));

 }

 GtkWidget * v3270_activity_list_new()
 {
	return g_object_new(GTK_TYPE_V3270_FT_ACTIVITY_LIST, NULL);
 }

 void v3270_activity_list_append(GtkWidget *widget, GObject *activity, gboolean select)
 {
 	GtkTreeModel * model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	GtkTreeIter iter;
	gtk_list_store_append((GtkListStore *) model,&iter);
	gtk_list_store_set((GtkListStore *) model, &iter, 0, activity, -1);
	g_object_ref_sink(activity);

	if(select)
	{
		debug("%s: Selecting inserted activity",__FUNCTION__);
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(widget)),&iter);
		g_signal_emit(widget, v3270_activity_list_signals[V3270_ACTIVITY_LIST_SELECTED_SIGNAL], 0, activity);
	}

 }

 void v3270_activity_list_remove(GtkWidget *widget, GObject *activity)
 {
 	if(!activity)
		return;

 	GtkTreeModel * model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	GtkTreeIter iter;

	if(gtk_tree_model_get_iter_first(model,&iter))
	{
		do
		{
			GObject * stored = NULL;
			gtk_tree_model_get(model, &iter, 0, &stored, -1);

			if(stored == activity)
			{
				debug("Removing activity %p",activity);
				gtk_list_store_remove(GTK_LIST_STORE(model),&iter);
				g_object_unref(stored);
				return;
			}


		}
		while(gtk_tree_model_iter_next(model,&iter));
	}

 }


 static void element_start(GMarkupParseContext *context, const gchar *element_name, const gchar G_GNUC_UNUSED(**names),const gchar G_GNUC_UNUSED(**values), V3270FTActivityList *widget, GError G_GNUC_UNUSED(**error))
 {
 	debug("%s(%s)",__FUNCTION__, element_name);
	if(!g_ascii_strcasecmp(element_name,"entry"))
	{
		// Create new activity
		GObject * activity = v3270_ft_activity_new();
		v3270_ft_activity_set_from_context(activity,context);
		v3270_activity_list_append(GTK_WIDGET(widget), activity, FALSE);
	}

 }

 static void element_end(GMarkupParseContext *context, const gchar *element_name, G_GNUC_UNUSED void *info,G_GNUC_UNUSED  GError **error)
 {
 	debug("%s(%s)",__FUNCTION__, element_name);
	if(!g_ascii_strcasecmp(element_name,"entry"))
	{
		g_markup_parse_context_pop(context);
	}
 }

 static void reload(GtkWidget *widget)
 {
	static const GMarkupParser parser = {
		(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **)) element_start,
		(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **)) element_end,
		(void (*)(GMarkupParseContext *, const gchar *, gsize, gpointer, GError **)) NULL,
		(void (*)(GMarkupParseContext *, const gchar *, gsize,  gpointer, GError **)) NULL,
		(void (*)(GMarkupParseContext *, GError *, gpointer)) NULL
	};

	GError				* error	= NULL;
	g_autofree gchar	* text	= NULL;

	if(g_file_get_contents(GTK_V3270_FT_ACTIVITY_LIST(widget)->filename,&text,NULL,&error)) {

		GMarkupParseContext	* context =
			g_markup_parse_context_new(
				&parser,
				G_MARKUP_TREAT_CDATA_AS_TEXT|G_MARKUP_PREFIX_ERROR_POSITION,
				widget,
				NULL
			);

		g_markup_parse_context_parse(context,text,strlen(text),&error);
		g_markup_parse_context_free(context);

	}

 }

 void v3270_activity_list_load(GtkWidget *widget)
 {
	V3270FTActivityList * list = GTK_V3270_FT_ACTIVITY_LIST(widget);

	gchar * filename = v3270_select_file(
								widget,
								_("Load queue from file"),
								_("Load"), GTK_FILE_CHOOSER_ACTION_OPEN,
								"",
								N_("XML file"), "*.xml",
								NULL );

	if(filename) {
		g_free(list->filename);
		list->filename = filename;
	}

	reload(widget);

	g_signal_emit(widget, v3270_activity_list_signals[V3270_ACTIVITY_LIST_HAS_FILE_SIGNAL], 0, (list->filename == NULL ? FALSE : TRUE));


 }

 void v3270_activity_list_save(GtkWidget *widget)
 {
	V3270FTActivityList * list = GTK_V3270_FT_ACTIVITY_LIST(widget);
	GString * str	= g_string_new("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<filelist>\n");
	GError	* error	= NULL;

	debug("%s",__FUNCTION__);

	// Serialize activities.
	GtkTreeIter		  iter;
	GtkTreeModel	* model	= gtk_tree_view_get_model(GTK_TREE_VIEW(widget));

	if(gtk_tree_model_get_iter_first(model,&iter))
	{
		do
		{
			GObject * activity = NULL;
			gtk_tree_model_get(model, &iter, 0, &activity, -1);
			v3270_ft_activity_xml_encode(activity,str);
		}
		while(gtk_tree_model_iter_next(model,&iter));
	}

	g_string_append(str,"</filelist>\n");


	// Save activity list
	g_autofree gchar * text = g_string_free(str,FALSE);

	debug("Saving %s",list->filename);

	if(!g_file_set_contents(list->filename,text,-1,&error)) {

		GtkWidget *popup = gtk_message_dialog_new_with_markup(
			GTK_WINDOW(gtk_widget_get_toplevel(widget)),
			GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
			_("Can't save %s"),list->filename
		);

		gtk_window_set_title(GTK_WINDOW(popup),_("Operation has failed"));

		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",error->message);
		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG(popup));
		gtk_widget_destroy(popup);

	}


 }

 void v3270_activity_list_save_as(GtkWidget *widget)
 {
	V3270FTActivityList * list = GTK_V3270_FT_ACTIVITY_LIST(widget);

	gchar * filename = v3270_select_file(
								widget,
								_("Save queue to file"),
								_("Save"),
								GTK_FILE_CHOOSER_ACTION_SAVE,
								list->filename,
								N_("XML file"), "*.xml",
								NULL );

	if(filename) {
		g_free(list->filename);
		list->filename = filename;
		v3270_activity_list_save(widget);
	}

	g_signal_emit(widget, v3270_activity_list_signals[V3270_ACTIVITY_LIST_HAS_FILE_SIGNAL], 0, (list->filename == NULL ? FALSE : TRUE));

 }


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
 #include <v3270/filetransfer.h>

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 struct _V3270FTActivityClass
 {
 	GObjectClass parent_class;

 };

 struct _V3270FTActivity
 {
 	GObject parent;

 	struct {
 		gchar * local;
 		gchar * remote;
 	} file;

 };

 enum
 {
  PROP_0,
  PROP_LOCAL,
  PROP_REMOTE
 };

 G_DEFINE_TYPE(V3270FTActivity, V3270FTActivity, G_TYPE_OBJECT);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void dispose(GObject *object)
 {
	debug("%s",__FUNCTION__);

	V3270FTActivity * activity = G_V3270_FT_ACTIVITY(object);

	g_free(activity->file.local);
	g_free(activity->file.remote);

 }

 static void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec G_GNUC_UNUSED(*pspec))
 {
	switch(prop_id)
	{
	case PROP_LOCAL:
		v3270_ft_activity_set_local_filename(object,g_value_get_string(value));
		break;

	case PROP_REMOTE:
		v3270_ft_activity_set_remote_filename(object,g_value_get_string(value));
		break;
	}

 }

 static void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec G_GNUC_UNUSED(*pspec))
 {
	switch(prop_id)
	{
	case PROP_LOCAL:
		g_value_set_string(value,v3270_ft_activity_get_local_filename(object));
		break;

	case PROP_REMOTE:
		g_value_set_string(value,v3270_ft_activity_get_remote_filename(object));
		break;
	}
 }

 static void V3270FTActivity_class_init(V3270FTActivityClass *klass)
 {
	G_OBJECT_CLASS(klass)->dispose = dispose;

	G_OBJECT_CLASS(klass)->set_property = set_property;
	G_OBJECT_CLASS(klass)->get_property = get_property;

	g_object_class_install_property(
		G_OBJECT_CLASS(klass),
		PROP_LOCAL,
		g_param_spec_string ("local", "Local",
			_( "Full path of local file" ),
			FALSE, G_PARAM_READWRITE));

	g_object_class_install_property(
		G_OBJECT_CLASS(klass),
		PROP_LOCAL,
		g_param_spec_string ("remote", "remote",
			_( "Full path of remote file" ),
			FALSE, G_PARAM_READWRITE));

 }

 static void V3270FTActivity_init(V3270FTActivity *widget)
 {
 }

 LIB3270_EXPORT GObject * v3270_ft_activity_new()
 {
	return g_object_new(G_TYPE_V3270_FT_ACTIVITY, NULL);
 }

 LIB3270_EXPORT const gchar * v3270_ft_activity_get_local_filename(GObject *object)
 {
	gchar **ptr = & G_V3270_FT_ACTIVITY(object)->file.local;
	return(*ptr ? *ptr : "");
 }

 LIB3270_EXPORT const gchar * v3270_ft_activity_get_remote_filename(GObject *object)
 {
	gchar **ptr = & G_V3270_FT_ACTIVITY(object)->file.remote;
	return(*ptr ? *ptr : "");
 }

 LIB3270_EXPORT void v3270_ft_activity_set_local_filename(GObject *object, const gchar *filename)
 {
	gchar **ptr = & G_V3270_FT_ACTIVITY(object)->file.local;

	g_free(*ptr);
	*ptr = g_strdup(filename);
 }

 LIB3270_EXPORT void v3270_ft_activity_set_remote_filename(GObject *object, const gchar *filename)
 {
	gchar **ptr = & G_V3270_FT_ACTIVITY(object)->file.remote;

	g_free(*ptr);
	*ptr = g_strdup(filename);
 }

 static void render_local(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell, GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
 {
	V3270FTActivity * activity;
	gtk_tree_model_get(tree_model, iter, 0, &activity, -1);
	g_object_set(G_OBJECT(cell),"text",activity->file.local,NULL);
 }

 static void render_remote(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell, GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
 {
	V3270FTActivity * activity;
	gtk_tree_model_get(tree_model, iter, 0, &activity, -1);
	g_object_set(G_OBJECT(cell),"text",activity->file.remote,NULL);
 }

 GtkWidget * v3270_activity_list_new()
 {
	GtkTreeModel * model = GTK_TREE_MODEL(gtk_list_store_new(1,G_TYPE_OBJECT));
	GtkWidget	 * widget = gtk_tree_view_new_with_model(model);

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

	return widget;
 }

 void v3270_activity_list_append(GtkWidget *widget, GObject *activity)
 {
 	GtkTreeModel * model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	GtkTreeIter iter;
	gtk_list_store_append((GtkListStore *) model,&iter);
	gtk_list_store_set((GtkListStore *) model, &iter, 0, activity, -1);
 }

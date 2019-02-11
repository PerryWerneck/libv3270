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

 	V3270_ACTIVITY_LIST_LAST_SIGNAL
 };

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
	debug("%s",__FUNCTION__);

	V3270FTActivityList * list = GTK_V3270_FT_ACTIVITY_LIST(object);

	g_free(list->filename);

 }

 static void V3270FTActivityList_has_file(GtkWidget G_GNUC_UNUSED(*widget), gboolean G_GNUC_UNUSED(has_file))
 {
 	debug("%s",__FUNCTION__);
 }

 static void V3270FTActivityList_class_init(V3270FTActivityListClass *klass)
 {
	GObjectClass * gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = dispose;

	klass->signal.has_file = V3270FTActivityList_has_file;

	v3270_activity_list_signals[V3270_ACTIVITY_LIST_HAS_FILE_SIGNAL] =
		g_signal_new(	"has-file",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (V3270FTActivityListClass, signal.has_file),
						NULL, NULL,
						v3270ft_VOID__VOID_BOOLEAN,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);


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

 static void V3270FTActivityList_init(V3270FTActivityList *widget)
 {
	GtkTreeModel * model = GTK_TREE_MODEL(gtk_list_store_new(1,G_TYPE_OBJECT));

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

 }

 GtkWidget * v3270_activity_list_new()
 {
	return g_object_new(GTK_TYPE_V3270_FT_ACTIVITY_LIST, NULL);
 }

 void v3270_activity_list_load(GtkWidget *widget)
 {

 }

 void v3270_activity_list_append(GtkWidget *widget, GObject *activity)
 {
 	GtkTreeModel * model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	GtkTreeIter iter;
	gtk_list_store_append((GtkListStore *) model,&iter);
	gtk_list_store_set((GtkListStore *) model, &iter, 0, activity, -1);
 }


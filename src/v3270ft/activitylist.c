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

 static const struct _option_list
 {
 	LIB3270_FT_OPTION	  option;
 	const gchar			* name;
 	const gchar			* value;
 }
 option_list[] =
 {

	{ LIB3270_FT_OPTION_SEND,					"type",				"send",				},
	{ LIB3270_FT_OPTION_RECEIVE,				"type",				"receive",			},
	{ LIB3270_FT_OPTION_ASCII,					"format",			"ascii",			},
	{ LIB3270_FT_OPTION_CRLF,					"format",			"crlf",				},
	{ LIB3270_FT_OPTION_APPEND,					"format",			"append",			},
	{ LIB3270_FT_OPTION_REMAP,					"format",			"remap",			},
	{ LIB3270_FT_OPTION_UNIX,					"file-format",		"unix",				},
	{ LIB3270_FT_RECORD_FORMAT_DEFAULT,			"record-format",	"default",			},
	{ LIB3270_FT_RECORD_FORMAT_FIXED,			"record-format",	"fixed",			},
	{ LIB3270_FT_RECORD_FORMAT_VARIABLE,		"record-format",	"variable",			},
	{ LIB3270_FT_RECORD_FORMAT_UNDEFINED,		"record-format",	"undefined",		},
	{ LIB3270_FT_ALLOCATION_UNITS_DEFAULT,		"units",			"default",			},
	{ LIB3270_FT_ALLOCATION_UNITS_TRACKS,		"units",			"tracks",			},
	{ LIB3270_FT_ALLOCATION_UNITS_CYLINDERS,	"units",			"cylinders",		},
	{ LIB3270_FT_ALLOCATION_UNITS_AVBLOCK,		"units",			"avblock",			},
 };

 static void dispose(GObject *object)
 {
	debug("%s",__FUNCTION__);

	V3270FTActivityList * list = GTK_V3270_FT_ACTIVITY_LIST(object);

	debug("Freeing %s",list->filename);
	g_free(list->filename);
	list->filename = NULL;

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

 void v3270_activity_list_append(GtkWidget *widget, GObject *activity)
 {
 	GtkTreeModel * model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	GtkTreeIter iter;
	gtk_list_store_append((GtkListStore *) model,&iter);
	gtk_list_store_set((GtkListStore *) model, &iter, 0, activity, -1);
 }

 void v3270_activity_list_load(GtkWidget *widget)
 {
	V3270FTActivityList * list = GTK_V3270_FT_ACTIVITY_LIST(widget);

	gchar * filename = v3270ft_select_file(
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

	g_signal_emit(widget, v3270_activity_list_signals[V3270_ACTIVITY_LIST_HAS_FILE_SIGNAL], 0, (list->filename == NULL ? FALSE : TRUE));

 }

 void v3270_activity_list_save(GtkWidget *widget)
 {
	V3270FTActivityList * list = GTK_V3270_FT_ACTIVITY_LIST(widget);
	GString * str	= g_string_new("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<filelist>\n");
	GError	* error	= NULL;
	size_t	  ix;

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

			if(activity)
			{
				g_string_append(str,"\t<entry>\n");

				g_string_append_printf(str,"\t\t<file type=\'local\' path=\'%s\' />\n",v3270_ft_activity_get_local_filename(activity));
				g_string_append_printf(str,"\t\t<file type=\'remote\' path=\'%s\' />\n",v3270_ft_activity_get_remote_filename(activity));

				LIB3270_FT_OPTION options = v3270_ft_activity_get_options(activity);
				for(ix = 0; ix < G_N_ELEMENTS(option_list);ix++)
				{
					if(options & option_list[ix].option)
						g_string_append_printf(str,"\t\t<option name=\'%s\' value=\'%s\' />\n",option_list[ix].name,option_list[ix].value);
				}

				for(ix=0;ix<LIB3270_FT_VALUE_COUNT;ix++) {
					g_string_append_printf(str,"\t\t<parameter name=\"%s\" value=\"%u\"/>\n",ft_value[ix].name,v3270_ft_activity_get_value(activity,(LIB3270_FT_VALUE) ix));
				}

				g_string_append(str,"\t</entry>\n");
			}

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

	gchar * filename = v3270ft_select_file(
								widget,
								_("Save queue to file"),
								_("Save"),
								GTK_FILE_CHOOSER_ACTION_SAVE,
								"",
								N_("XML file"), "*.xml",
								NULL );

	if(filename) {
		g_free(list->filename);
		list->filename = filename;
		v3270_activity_list_save(widget);
	}

	g_signal_emit(widget, v3270_activity_list_signals[V3270_ACTIVITY_LIST_HAS_FILE_SIGNAL], 0, (list->filename == NULL ? FALSE : TRUE));

 }


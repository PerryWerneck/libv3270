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
 #include <stdlib.h>
 #include "private.h"
 #include <v3270/filetransfer.h>

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 struct _V3270FTActivityClass
 {
 	GObjectClass parent_class;

 };

 struct _V3270FTActivity
 {
 	GObject parent;

 	/// @brief Transfer options.
 	LIB3270_FT_OPTION options;

 	/// @brief Values.
	guint values[LIB3270_FT_VALUE_COUNT];

	/// @brief File names
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

	G_OBJECT_CLASS(V3270FTActivity_parent_class)->dispose(object);

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
 	widget->values[LIB3270_FT_VALUE_LRECL] = 0;
 	widget->values[LIB3270_FT_VALUE_BLKSIZE] = 0;
 	widget->values[LIB3270_FT_VALUE_PRIMSPACE] = 0;
 	widget->values[LIB3270_FT_VALUE_SECSPACE] = 0;
 	widget->values[LIB3270_FT_VALUE_DFT] = 4096;
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

 LIB3270_EXPORT void v3270_ft_activity_set_options(GObject * object, LIB3270_FT_OPTION options)
 {
	G_V3270_FT_ACTIVITY(object)->options = options;
 }

 LIB3270_EXPORT LIB3270_FT_OPTION v3270_ft_activity_get_options(GObject *object)
 {
	return G_V3270_FT_ACTIVITY(object)->options;
 }

 guint v3270_ft_activity_get_value(GObject * object,  LIB3270_FT_VALUE id)
 {
	return G_V3270_FT_ACTIVITY(object)->values[id];
 }

 void v3270_ft_activity_set_value(GObject * object, LIB3270_FT_VALUE id, guint value)
 {
	G_V3270_FT_ACTIVITY(object)->values[id] = value;
 }

 static void element_start(GMarkupParseContext G_GNUC_UNUSED(*context), const gchar *element_name, const gchar **names,const gchar **values, V3270FTActivity *activity, GError **error)
 {
 	size_t ix;

 	debug("%s(%s)",__FUNCTION__, element_name);
	if(!g_ascii_strcasecmp(element_name,"file"))
	{
		const gchar *type;
		const gchar *path;

		if(!g_markup_collect_attributes(
				element_name,names,values,error,
				G_MARKUP_COLLECT_STRING, "type", &type,
				G_MARKUP_COLLECT_STRING, "path", &path,
				G_MARKUP_COLLECT_INVALID
				)) {

			return;

		}

		if(g_ascii_strcasecmp(type,"local") == 0)
			v3270_ft_activity_set_local_filename(G_OBJECT(activity),path);
		else if(g_ascii_strcasecmp(type,"remote") == 0)
			v3270_ft_activity_set_remote_filename(G_OBJECT(activity),path);

	}
	else if(!g_ascii_strcasecmp(element_name,"option"))
	{
		const gchar *name;
		const gchar *value;

		if(!g_markup_collect_attributes(
				element_name,names,values,error,
				G_MARKUP_COLLECT_STRING, "name", &name,
				G_MARKUP_COLLECT_STRING, "value", &value,
				G_MARKUP_COLLECT_INVALID
				)) {

			return;

		}

		debug("%s.%s(%s,%s)",__FUNCTION__, element_name, name, value);

		for(ix = 0; v3270_activity_list_options[ix].name; ix++)
		{
			if(! (g_ascii_strcasecmp(name,v3270_activity_list_options[ix].name) || g_ascii_strcasecmp(value,v3270_activity_list_options[ix].value)) )
			{
				activity->options |= v3270_activity_list_options[ix].option;
				debug("Setting option %s.%s(%08lx) =%08lx", v3270_activity_list_options[ix].name, v3270_activity_list_options[ix].value, (unsigned int) v3270_activity_list_options[ix].option,(unsigned int) activity->options)
				break;
			}
		}

	}
	else if(!g_ascii_strcasecmp(element_name,"parameter"))
	{
		const gchar *name;
		const gchar *value;

		if(!g_markup_collect_attributes(
				element_name,names,values,error,
				G_MARKUP_COLLECT_STRING, "name", &name,
				G_MARKUP_COLLECT_STRING, "value", &value,
				G_MARKUP_COLLECT_INVALID
				)) {

			return;

		}

		debug("%s.%s(%s)",__FUNCTION__, element_name, name, value);

		for(ix=0;ix<LIB3270_FT_VALUE_COUNT;ix++)
		{
			if(!g_ascii_strcasecmp(ft_value[ix].name,name))
			{
				activity->values[ix] = atoi(value);
				break;
			}
		}

	}

 }

 void v3270_ft_activity_set_from_context(GObject * activity, GMarkupParseContext * context)
 {
	static const GMarkupParser parser = {
		(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **)) element_start,
		(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **)) NULL,
		(void (*)(GMarkupParseContext *, const gchar *, gsize, gpointer, GError **)) NULL,
		(void (*)(GMarkupParseContext *, const gchar *, gsize,  gpointer, GError **)) NULL,
		(void (*)(GMarkupParseContext *, GError *, gpointer)) NULL
	};

	G_V3270_FT_ACTIVITY(activity)->options = 0;

	g_markup_parse_context_push(context,&parser,activity);

 }

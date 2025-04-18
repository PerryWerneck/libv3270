/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2008 Banco do Brasil S.A.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <internals.h>

 #include <libintl.h>

 #include <v3270.h>
 #include <lib3270/log.h>
 #include <v3270/colorscheme.h>
 #include <v3270_marshalers.h>

 #define V3270_COLOR_BASE V3270_COLOR_GRAY+1

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 struct _V3270ColorSchemeClass
 {
 	GtkComboBoxClass parent_class;

 };

 struct _V3270ColorScheme
 {
 	GtkComboBox	  parent;
	GdkRGBA		* schemes;


 };

 enum
 {
 	CHANGED,
 	LAST_SIGNAL
 };

 static guint signals[LAST_SIGNAL]		= { 0 };

 G_DEFINE_TYPE(V3270ColorScheme, V3270ColorScheme, GTK_TYPE_COMBO_BOX);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void changed(GtkComboBox *widget)
 {
	GdkRGBA		* clr		= NULL;
	GValue		  value	= { 0, };
	GtkTreeIter	  iter;

	if(!gtk_combo_box_get_active_iter(widget,&iter))
		return;

	gtk_tree_model_get_value(gtk_combo_box_get_model(widget),&iter,1,&value);
	clr = g_value_get_pointer(&value);

	debug("%s=%p",__FUNCTION__,clr);
	g_signal_emit(widget, signals[CHANGED], 0, clr);

	g_value_unset(&value);

 }

 static void V3270ColorScheme_class_init(G_GNUC_UNUSED V3270ColorSchemeClass *klass)
 {
	GObjectClass	* gobject_class	= G_OBJECT_CLASS(klass);

	klass->parent_class.changed = changed;

	signals[CHANGED] =
		g_signal_new(
			I_("update-colors"),
			G_OBJECT_CLASS_TYPE (gobject_class),
			G_SIGNAL_RUN_FIRST,
			0,
			NULL, NULL,
			v3270_VOID__VOID_POINTER,
			G_TYPE_NONE, 1, G_TYPE_POINTER, 0
		);

	debug("changed_signal=%d",(int) signals[CHANGED]);

 }

 static void V3270ColorScheme_init(V3270ColorScheme *widget)
 {

	GtkTreeModel	* model		= (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_POINTER);
	GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();

	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), renderer, "text", 0, NULL);

	gtk_combo_box_set_model(GTK_COMBO_BOX(widget), model);

 }

 static void load_color_scheme(GKeyFile *conf, const gchar *group, GdkRGBA *clr)
 {
	gchar	* val;
	size_t 	  f;

	// Load base colors
	val = g_key_file_get_string(conf,group,"base",NULL);
	if(val)
	{
		// Process base colors
		gchar **str = g_strsplit(val,";",V3270_COLOR_BASE);

		switch(g_strv_length(str))
		{
		case 2:	// Only 2 colors, create monocromatic table
			v3270_set_mono_color_table(clr,str[1],str[0]);
			break;

		case V3270_COLOR_BASE:	// All colors, update it
			for(f=0;f<V3270_COLOR_BASE;f++)
				gdk_rgba_parse(clr+f,str[f]);
			break;

		default:

			// Unexpected size, load new colors over the defaults
			g_warning("base color list in %s has %d elements, should have %d",group,g_strv_length(str),V3270_COLOR_GRAY);

			gdk_rgba_parse(clr,str[0]);
			gdk_rgba_parse(clr+1,str[1]);

			for(f=2;f<V3270_COLOR_BASE;f++)
				clr[f] = clr[1];

			clr[V3270_COLOR_BLACK] = *clr;

			for(f=2;f<MIN(g_strv_length(str),V3270_COLOR_BASE-1);f++)
				gdk_rgba_parse(clr+f,str[f]);

		}
		g_strfreev(str);
		g_free(val);
	}
	else
	{
		g_warning("Color scheme [%s] has no \"base\" entry, using green on black",group);

		gdk_rgba_parse(clr,"black");
		gdk_rgba_parse(clr+1,"green");

		for(f=2;f<V3270_COLOR_BASE;f++)
			clr[f] = clr[1];
		clr[V3270_COLOR_BLACK] = *clr;
	}

	// Load field colors
	clr[V3270_COLOR_FIELD]							= clr[V3270_COLOR_GREEN];
	clr[V3270_COLOR_FIELD_INTENSIFIED]				= clr[V3270_COLOR_RED];
	clr[V3270_COLOR_FIELD_PROTECTED]				= clr[V3270_COLOR_BLUE];
	clr[V3270_COLOR_FIELD_PROTECTED_INTENSIFIED]	= clr[V3270_COLOR_WHITE];

	val = g_key_file_get_string(conf,group,"field",NULL);
	if(val)
	{
		gchar **str = g_strsplit(val,";",5);

		for(f=0;f< MIN(g_strv_length(str),4); f++)
			gdk_rgba_parse(clr+V3270_COLOR_FIELD+f,str[f]);

		g_strfreev(str);
		g_free(val);
	}

	// Load selection colors
	clr[V3270_COLOR_SELECTED_BG]	= clr[V3270_COLOR_WHITE];
	clr[V3270_COLOR_SELECTED_FG]	= clr[V3270_COLOR_BLACK];
	val = g_key_file_get_string(conf,group,"selection",NULL);
	if(val)
	{
		gchar **str = g_strsplit(val,";",3);

		for(f=0;f< MIN(g_strv_length(str),2); f++)
			gdk_rgba_parse(clr+V3270_COLOR_SELECTED_BG+f,str[f]);

		g_strfreev(str);
		g_free(val);
	}

	// Load OIA colors
	clr[V3270_COLOR_OIA_BACKGROUND]		= clr[V3270_COLOR_BACKGROUND];
	clr[V3270_COLOR_OIA_FOREGROUND]		= clr[V3270_COLOR_GREEN];
	clr[V3270_COLOR_OIA_SEPARATOR]		= clr[V3270_COLOR_GREEN];
	clr[V3270_COLOR_OIA_STATUS_OK]		= clr[V3270_COLOR_GREEN];
	clr[V3270_COLOR_OIA_STATUS_INVALID]	= clr[V3270_COLOR_RED];
	clr[V3270_COLOR_OIA_STATUS_WARNING]	= clr[V3270_COLOR_YELLOW];

	val = g_key_file_get_string(conf,group,"OIA",NULL);
	if(val)
	{
		gchar **str = g_strsplit(val,";",6);

		// 0 = V3270_COLOR_OIA_BACKGROUND,
		// 1 = V3270_COLOR_OIA_FOREGROUND,
		// 2 = V3270_COLOR_OIA_SEPARATOR,
		// 3 = V3270_COLOR_OIA_STATUS_OK,
		// 4 = V3270_COLOR_OIA_STATUS_WARNING,
		// 5 = V3270_COLOR_OIA_STATUS_INVALID,

		if(g_strv_length(str) == 5)
		{
			for(f=0;f < 5; f++)
				gdk_rgba_parse(clr+V3270_COLOR_OIA_BACKGROUND+f,str[f]);
			clr[V3270_COLOR_OIA_STATUS_INVALID] = clr[V3270_COLOR_OIA_STATUS_WARNING];
		}
		else
		{
			for(f=0;f< MIN(g_strv_length(str),6); f++)
				gdk_rgba_parse(clr+V3270_COLOR_OIA_BACKGROUND+f,str[f]);
		}

		g_strfreev(str);
		g_free(val);
	}

	// Setup extended elements
	clr[V3270_COLOR_CROSS_HAIR] = clr[V3270_COLOR_GREEN];

	val = g_key_file_get_string(conf,group,"cross-hair",NULL);
	if(val)
	{
		gdk_rgba_parse(clr+V3270_COLOR_CROSS_HAIR,val);
		g_free(val);
	}

 }

 LIB3270_EXPORT GtkWidget * v3270_color_scheme_new()
 {
	GtkWidget		* widget	= GTK_WIDGET(g_object_new(GTK_TYPE_V3270_COLOR_SCHEME, NULL));
	GtkTreeModel	* model		= gtk_combo_box_get_model(GTK_COMBO_BOX(widget));

#ifdef DEBUG
	const char *filename = "./conf/colors.conf";
#else
	lib3270_autoptr(char) filename = lib3270_build_data_filename("colors.conf",NULL);
#endif // DEBUG

 	if(!g_file_test(filename,G_FILE_TEST_IS_REGULAR))
	{
#ifdef DEBUG
		g_message("Unable to load color schemes in \"%s\"",filename);
#else
		g_warning("Unable to load color schemes in \"%s\"",filename);
#endif
		gtk_widget_set_sensitive(widget,FALSE);
		return widget;
	}

	// Load color schemes.
	GKeyFile	* conf	= g_key_file_new();
	GError		* error	= NULL;
	int			  index	= 0;
	gsize		  len;
	gsize		  g;

	g_key_file_load_from_file(conf,filename,G_KEY_FILE_NONE,&error);

	if(error)
	{
		g_message("Can't load %s: %s",filename,error->message);
		g_error_free(error);
	}
	else
	{
		gchar **group = g_key_file_get_groups(conf,&len);
		GtkTreeIter	  iter;

		GTK_V3270_COLOR_SCHEME(widget)->schemes = g_new0(GdkRGBA,(len*V3270_COLOR_COUNT));

		for(g=0;g<len;g++)
		{
			// Setup colors for current entry
			GdkRGBA		* clr	= GTK_V3270_COLOR_SCHEME(widget)->schemes+index;
			gchar		* label	= g_key_file_get_locale_string(conf,group[g],"label",NULL,NULL);

			load_color_scheme(conf,group[g],clr);

			// Set it in the combobox
			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter,
												0, label ? label : group[g],
												1, clr,
												-1);

			g_free(label);

			// move to next color list
			index += V3270_COLOR_COUNT;
		}

		g_strfreev(group);

	}

	g_key_file_free(conf);

	return widget;
 }

 static gboolean compare_colors(const GdkRGBA *colora, const GdkRGBA *colorb)
 {
 	int f;

 	for(f=0;f<V3270_COLOR_COUNT;f++)
	{
		if(!gdk_rgba_equal(colora+f,colorb+f))
		{
/*
#ifdef DEBUG
			v3270_autofree gchar * cla = gdk_rgba_to_string(colora+f);
			v3270_autofree gchar * clb = gdk_rgba_to_string(colorb+f);

			debug(
				"diff on %d of %d %s - %s",
					f,
					V3270_COLOR_COUNT,
					cla,
					clb
			);
#endif // DEBUG
*/
			return FALSE;
		}
	}

 	return TRUE;
 }

 gchar * v3270_color_scheme_get_text(GtkWidget *widget)
 {
	GdkRGBA		* clr		= NULL;
	gchar		* rc		= NULL;
	GValue		  value		= { 0, };
	GtkTreeIter	  iter;

	if(!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget),&iter))
		return NULL;

	gtk_tree_model_get_value(gtk_combo_box_get_model(GTK_COMBO_BOX(widget)),&iter,1,&value);
	clr = g_value_get_pointer(&value);

	if(clr)
		rc = v3270_translate_rgba_to_text(clr);

	g_value_unset(&value);

	return rc;

 }

 gchar * v3270_translate_rgba_to_text(GdkRGBA *clr)
 {
 	int f;
	GString *str = g_string_new("");

	for(f=0;f<V3270_COLOR_COUNT;f++)
	{
		if(f)
			g_string_append_c(str,';');

		g_autofree gchar * color = gdk_rgba_to_string(clr+f);
		g_string_append_printf(str,"%s",color);
	}

	return g_string_free(str,FALSE);
 }

 void v3270_color_scheme_set_rgba(GtkWidget *widget, const GdkRGBA *colors)
 {
 	GtkTreeModel	* model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	GtkTreeIter		  iter;

	if(gtk_tree_model_get_iter_first(model,&iter))
	{
		do
		{
			GdkRGBA		* clr		= NULL;
			GValue		  value	= { 0, };

			gtk_tree_model_get_value(model,&iter,1,&value);
			clr = g_value_get_pointer(&value);

			if(clr && compare_colors(clr,colors))
			{
				gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
				return;
			}

		} while(gtk_tree_model_iter_next(model,&iter));
	}

	g_message("Can't find color scheme");

	// TODO: Create an entry for this scheme as "custom" and select it.

 }

 void v3270_color_scheme_set_text(GtkWidget *widget, const gchar *colors)
 {
	GdkRGBA clr[V3270_COLOR_COUNT];
	v3270_translate_text_to_rgba(colors,clr);
	v3270_color_scheme_set_rgba(widget,clr);
 }

 void v3270_translate_text_to_rgba(const gchar *colors, GdkRGBA *clr)
 {
	gchar		**str = g_strsplit(colors,";",V3270_COLOR_BASE);
	size_t 		  f;

	switch(g_strv_length(str))
	{
	case 2:	// Only 2 colors, create monocromatic table
		v3270_set_mono_color_table(clr,str[1],str[0]);
		break;

	case V3270_COLOR_BASE:	// All colors, update it
		for(f=0;f<V3270_COLOR_BASE;f++)
			gdk_rgba_parse(clr+f,str[f]);
		break;

	default:

		// Unexpected size, load new colors over the defaults
		gdk_rgba_parse(clr,str[0]);
		gdk_rgba_parse(clr+1,str[1]);

		for(f=2;f<V3270_COLOR_BASE;f++)
			clr[f] = clr[1];

		clr[V3270_COLOR_BLACK] = *clr;

		for(f=2;f<MIN(g_strv_length(str),V3270_COLOR_BASE-1);f++)
			gdk_rgba_parse(clr+f,str[f]);

	}

	g_strfreev(str);

 }

 int v3270_color_scheme_get_rgba(GtkWidget *widget, GdkRGBA *colors, size_t num_colors)
 {
 	if(num_colors > V3270_COLOR_COUNT)
		num_colors = V3270_COLOR_COUNT;

	GdkRGBA		* clr		= NULL;
	GValue		  value	= { 0, };
	GtkTreeIter	  iter;
	size_t		  f;

	if(!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget),&iter))
		return errno = ENODATA;

	gtk_tree_model_get_value(gtk_combo_box_get_model(GTK_COMBO_BOX(widget)),&iter,1,&value);
	clr = g_value_get_pointer(&value);

	if(!clr)
		return errno = ENODATA;

	for(f=0;f<num_colors;f++)
	{
		colors[f] = clr[f];
	}

 	return 0;
 }


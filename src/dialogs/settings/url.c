/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes paul.mattes@case.edu), de emulação de terminal 3270 para acesso a
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
 #include <v3270dlg_marshalers.h>
 #include <v3270/settings/url.h>
 #include <ctype.h>

 enum {
    PROP_0,
    PROP_URL
 };

 enum {
 	SIGNAL_VALID,
 	LAST_SIGNAL
 };

 static guint signals[LAST_SIGNAL]		= { 0 };

 struct _V3270URLEdit {

	GtkGrid parent;

	/// @brief Entry fields
	struct {
		GtkWidget *host;		///< @brief The hostname
		GtkWidget *service;		///< @brief The service/port name
		GtkWidget *ssl;			///< @brief The Security Drop-Down
	} entry;

	/// @brief Current URL
	gchar *url;

	/// @brief Is the entry valid?
	gboolean isValid;

 };

 struct _V3270URLEditClass {

 	GtkGridClass parent;

	GParamSpec *url;

 };

 G_DEFINE_TYPE(V3270URLEdit, V3270URLEdit, GTK_TYPE_GRID);

 GtkWidget * v3270_url_edit_new() {
	return g_object_new(GTK_TYPE_V3270URLEdit, NULL);
 }

 static void finalize(GObject *object) {

	V3270URLEdit *edit = GTK_V3270URLEdit(object);

	if(edit->url) {
		g_free(edit->url);
		edit->url = NULL;
	}

	G_OBJECT_CLASS(V3270URLEdit_parent_class)->finalize(object);
 }

 static void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

    switch(prop_id) {
    case PROP_URL:
    	v3270_url_edit_set_url(GTK_WIDGET(object),g_value_get_string(value));
        break;
    }
 }

 static void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {
    switch(prop_id) {
    case PROP_URL:
        g_value_set_string(value,v3270_url_edit_get_url(GTK_WIDGET(object)));
        break;
    }
 }

 static void V3270URLEdit_class_init(V3270URLEditClass *klass) {

	GObjectClass *goc = G_OBJECT_CLASS(klass);

	goc->finalize = finalize;
	goc->set_property = set_property;
	goc->get_property = get_property;

	klass->url =
		g_param_spec_string(
			"url", "url",
			_( "TN3270 Host URL" ),
			lib3270_get_default_host(NULL),
			G_PARAM_READWRITE
		);

	g_object_class_install_property(
		goc,
		PROP_URL,
		klass->url
	);

	signals[SIGNAL_VALID] =
		g_signal_new(
			I_("valid"),
			G_OBJECT_CLASS_TYPE(goc),
			G_SIGNAL_RUN_FIRST,
			0,
			NULL, NULL,
			v3270_VOID__BOOLEAN,
			G_TYPE_NONE, 1, G_TYPE_BOOLEAN, 0
		);

 }

 static void entry_changed(GtkEntry *entry, V3270URLEdit *widget) {

	g_object_notify_by_pspec(G_OBJECT(widget),GTK_V3270URLEdit_GET_CLASS(widget)->url);

	gboolean isValid = (gtk_entry_get_text(entry)[0] != 0);
	if(widget->isValid == isValid) {
		return;
	}

	if(isValid) {

		size_t ix;

		// Verify entry-fields
		GtkWidget *widgets[] = {
			widget->entry.host,
			widget->entry.service
		};

		for(ix = 0; ix < G_N_ELEMENTS(widgets); ix++) {

			const gchar *str = gtk_entry_get_text(GTK_ENTRY(widgets[ix]));
			while(*str && isspace(*str)) {
				str++;
			}

			if(!*str) {
				isValid = FALSE;
				break;
			}

		}

	}

	if(isValid == widget->isValid) {
		return;
	}

	debug("The URL box is %s", isValid ? "valid" : "invalid");

	widget->isValid = isValid;
	g_signal_emit(widget, signals[SIGNAL_VALID], 0, isValid);

 }

 static void V3270URLEdit_init(V3270URLEdit *widget) {

	size_t ix;

	widget->isValid = FALSE;

	// Table of constants.
	static const struct _labels {
		int row;
		int col;
		const char *text;
		const char *tooltip;
	} labels[] = {
		{
			.row = 0,
			.col = 0,
			.text = N_("Hostname"),
			.tooltip = N_("Address or name of the host to connect."),
		},
		{
			.row = 1,
			.col = 0,
			.text = N_("Service/Port"),
			.tooltip = N_("Port or service name."),
		},
		{
			.row = 1,
			.col = 2,
			.text = N_("Security"),
			.tooltip = N_("Security engine"),
		}
	};

	// setup grid
	gtk_widget_set_vexpand(GTK_WIDGET(widget),FALSE);
 	gtk_grid_set_row_spacing(GTK_GRID(widget),6);
 	gtk_grid_set_column_spacing(GTK_GRID(widget),12);

	// Create hostname field.
	{
		widget->entry.host = gtk_entry_new();
		gtk_widget_set_hexpand(widget->entry.host,TRUE);
		gtk_entry_set_width_chars(GTK_ENTRY(widget->entry.host),50);
		gtk_widget_set_tooltip_text(widget->entry.host,g_dgettext(GETTEXT_PACKAGE,labels[0].tooltip));
		gtk_grid_attach(GTK_GRID(widget),widget->entry.host,1,0,5,1);
		g_signal_connect(widget->entry.host,"changed",G_CALLBACK(entry_changed),widget);
	}

	// Create the service/port field.
	{
		widget->entry.service = gtk_entry_new();
		gtk_widget_set_hexpand(widget->entry.service,FALSE);
		gtk_entry_set_max_length(GTK_ENTRY(widget->entry.service),6);
		gtk_entry_set_width_chars(GTK_ENTRY(widget->entry.service),7);
		gtk_widget_set_tooltip_text(widget->entry.service,g_dgettext(GETTEXT_PACKAGE,labels[1].tooltip));
		gtk_grid_attach(GTK_GRID(widget),widget->entry.service,1,1,1,1);
		g_signal_connect(widget->entry.service,"changed",G_CALLBACK(entry_changed),widget);
	}

	// Create the security dropbox.
	{
		GtkTreeModel * model = (GtkTreeModel *) gtk_list_store_new(1,G_TYPE_STRING);
		widget->entry.ssl = gtk_combo_box_new_with_model(model);
		gtk_widget_set_hexpand(widget->entry.ssl,TRUE);

		GtkCellRenderer * text_renderer	= gtk_cell_renderer_text_new();
		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget->entry.ssl), text_renderer, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget->entry.ssl), text_renderer, "text", 0, NULL);

		static const gchar * levels[] =
		{
			N_("Unsecure"),
			N_("SSL/TLS")
		};

		size_t level;
		for(level = 0; level < G_N_ELEMENTS(levels); level++)
		{
			GtkTreeIter iter;
			gtk_list_store_append((GtkListStore *) model, &iter);
			gtk_list_store_set((GtkListStore *) model, &iter, 0, g_dgettext(GETTEXT_PACKAGE,levels[level]), -1);
		};

		gtk_widget_set_tooltip_text(widget->entry.ssl,g_dgettext(GETTEXT_PACKAGE,labels[2].tooltip));
		gtk_grid_attach(GTK_GRID(widget),widget->entry.ssl,3,1,3,1);
	}

	// Create labels.
	{

		for(ix = 0; ix < G_N_ELEMENTS(labels);ix++) {
			GtkWidget * label = gtk_label_new(g_dgettext(GETTEXT_PACKAGE,labels[ix].text));
			gtk_widget_set_halign(label,GTK_ALIGN_END);
			gtk_widget_set_hexpand(label,FALSE);
			gtk_grid_attach(GTK_GRID(widget),label,labels[ix].col,labels[ix].row,1,1);
			gtk_widget_set_tooltip_text(label,g_dgettext(GETTEXT_PACKAGE,labels[ix].tooltip));
		}


	}

	v3270_url_edit_set_url(GTK_WIDGET(widget),NULL);

 }

 void v3270_url_edit_set_url(GtkWidget *widget, const gchar *url) {

	g_return_if_fail(GTK_IS_V3270URLEdit(widget));
	V3270URLEdit *edit = GTK_V3270URLEdit(widget);

	if(edit->url) {
		g_free(edit->url);
		edit->url = NULL;
	}

	if(!(url && *url)) {
		gtk_combo_box_set_active(GTK_COMBO_BOX(edit->entry.ssl),0);
		gtk_entry_set_text(GTK_ENTRY(edit->entry.host),"");
		gtk_entry_set_text(GTK_ENTRY(edit->entry.service),"");
		return;
	}

	edit->url = g_strdup(url);

	gtk_combo_box_set_active(GTK_COMBO_BOX(edit->entry.ssl),(g_str_has_prefix(edit->url,"tn3270s") ? 1 : 0));

	gchar *hostname = strstr(url,"://");
	if(!hostname)
	{
		g_message("Invalid URL: \"%s\" (no scheme)",url);
		gtk_entry_set_text(GTK_ENTRY(edit->entry.host),"");
		gtk_entry_set_text(GTK_ENTRY(edit->entry.service),"");
		return;
	}

	hostname += 3;
	g_autofree gchar *str = g_strdup(hostname);

	gchar *srvcname = strchr(str,':');

	if(srvcname) {
		*(srvcname++) = 0;
	} else {
		srvcname = "3270";
	}

	gtk_entry_set_text(GTK_ENTRY(edit->entry.host),str);
	gtk_entry_set_text(GTK_ENTRY(edit->entry.service),srvcname);

 }

 const gchar * v3270_url_edit_get_url(GtkWidget *widget) {

	g_return_val_if_fail(GTK_IS_V3270URLEdit(widget),NULL);
	V3270URLEdit *edit = GTK_V3270URLEdit(widget);

	if(edit->url) {
		g_free(edit->url);
		edit->url = NULL;
	}

	const gchar *host = gtk_entry_get_text(GTK_ENTRY(edit->entry.host));
	if(!*host)
		return "";

	const gchar *service = gtk_entry_get_text(GTK_ENTRY(edit->entry.service));
	if(!*service)
		service = "3270";

	edit->url =
		g_strconcat(
			(gtk_combo_box_get_active(GTK_COMBO_BOX(edit->entry.ssl)) > 0 ? "tn3270s://" : "tn3270://"),
			host,
			":",
			service,
			NULL
		);

	return edit->url;
 }

 gboolean v3270_url_edit_is_valid(GtkWidget *widget) {
	g_return_val_if_fail(GTK_IS_V3270URLEdit(widget),FALSE);

	V3270URLEdit *edit = GTK_V3270URLEdit(widget);

	const gchar *host = gtk_entry_get_text(GTK_ENTRY(edit->entry.host));
	if(!*host)
		return FALSE;

	return TRUE;
 }

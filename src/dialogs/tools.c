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
 #include <v3270/dialogs.h>
 #include <v3270/tools.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 GtkWidget * v3270_dialog_create_grid(GtkAlign align)
 {
 	GtkWidget * grid = gtk_grid_new();

 	gtk_grid_set_row_spacing(GTK_GRID(grid),6);
 	gtk_grid_set_column_spacing(GTK_GRID(grid),12);

	g_object_set(G_OBJECT(grid),"margin-top",6,NULL);
	gtk_widget_set_halign(GTK_WIDGET(grid),align);

	return grid;
 }

 GtkWidget * v3270_dialog_section_get_label_widget(GtkWidget *widget) {
	GtkWidget *rc = NULL;
	g_object_get(G_OBJECT(widget),"label-widget",&rc,NULL);
	return rc;
 }

 GtkWidget * v3270_dialog_section_new(const gchar * title, const gchar *tooltip, GtkWidget *child) {

 	// https://developer.gnome.org/hig/stable/visual-layout.html.en

	GtkFrame 	* frame	= GTK_FRAME(gtk_frame_new(title));
	GtkWidget	* label	= v3270_dialog_section_get_label_widget(GTK_WIDGET(frame));

#ifdef G_OS_UNIX
	{
		// Unix/Linux version, follow gnome guidelines
		gtk_widget_add_class(label,"separator");
		gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_NONE);

	}
#endif // G_OS_UNIX

	if(child) {
		gtk_container_set_border_width(GTK_CONTAINER(child),12);
		gtk_container_add(GTK_CONTAINER(frame),GTK_WIDGET(child));

		if(GTK_IS_GRID(child)) {
			gtk_grid_set_row_spacing(GTK_GRID(child),6);
			gtk_grid_set_column_spacing(GTK_GRID(child),12);
		}

	}

	if(tooltip) {
		gtk_widget_set_tooltip_markup(label,tooltip);
	}

	return GTK_WIDGET(frame);

 }


 GtkWidget * v3270_dialog_create_frame(GtkWidget * child, const gchar *title) {
 	return v3270_dialog_section_new(title,NULL,child);
 }

 GtkWidget * v3270_box_pack_frame(GtkWidget *box, GtkWidget *child, const gchar *title, const gchar *tooltip, GtkAlign align, gboolean expand, gboolean fill, guint padding)
 {
	GtkWidget * frame = v3270_dialog_section_new(title,NULL,child);
	gtk_widget_set_halign(GTK_WIDGET(frame),align);
	gtk_box_pack_start(GTK_BOX(box),frame,expand,fill,padding);

	if(tooltip)
		gtk_widget_set_tooltip_markup(frame,tooltip);

	return child;
 }

 GtkWidget * v3270_box_pack_start(GtkWidget *box, GtkWidget *child, gboolean expand, gboolean fill, guint padding)
 {
	gtk_box_pack_start(GTK_BOX(box),child,expand,fill,padding);
	return child;
 }

 GtkWidget * v3270_box_pack_end(GtkWidget *box, GtkWidget *child, gboolean expand, gboolean fill, guint padding)
 {
	gtk_box_pack_end(GTK_BOX(box),child,expand,fill,padding);
 	return child;
 }

 LIB3270_EXPORT void gtk_entry_set_printf(GtkEntry *entry, const gchar *fmt, ...)
 {
 	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	gchar *text = g_strdup_vprintf(fmt,arg_ptr);
	va_end(arg_ptr);

	gtk_entry_set_text(entry,text);

	g_free(text);

 }

 void v3270_dialog_close(GtkDialog *dialog, gpointer G_GNUC_UNUSED(user_data) )
 {
 	debug("%s",__FUNCTION__);
	gtk_dialog_response(dialog,GTK_RESPONSE_CANCEL);
 }

 #if ! GLIB_CHECK_VERSION(2,44,0)

 // Reference: https://github.com/ImageMagick/glib/blob/master/glib/glib-autocleanups.h
 void v3270_autoptr_cleanup_generic_gfree(void *p)
 {
	void **pp = (void**)p;
	g_free (*pp);
 }

 #endif // ! GLIB(2,44,0)

 #if ! GTK_CHECK_VERSION(3,16,0)

 void gtk_text_view_set_monospace(GtkTextView *text_view, gboolean monospace)
 {

 }

 #endif //! GTK 3.16

 GtkWidget	* v3270_charset_combo_box_new()
 {
	static const struct _charsets
	{
		const gchar *name;
		const gchar *description;
	} charsets[] =
	{
		// http://en.wikipedia.org/wiki/Character_encoding
		{ "UTF-8",		N_( "UTF-8"	)								},
		{ "ISO-8859-1", N_( "Western Europe (ISO 8859-1)" ) 		},
		{ "CP1252",		N_( "Windows Western languages (CP1252)" )	},
	};

	size_t		  ix;
	const gchar	* scharset	= NULL;
	GtkWidget	* widget	= gtk_combo_box_text_new();

	g_get_charset(&scharset);

	g_autofree gchar * text = g_strdup_printf(_("Current (%s)"),scharset);
	gtk_combo_box_text_insert(
		GTK_COMBO_BOX_TEXT(widget),
		0,
		scharset,
		text
	);

	gtk_combo_box_set_active(GTK_COMBO_BOX(widget),0);

	for(ix=0;ix<G_N_ELEMENTS(charsets);ix++)
	{
		if(g_ascii_strcasecmp(charsets[ix].name,scharset))
		{
			gtk_combo_box_text_insert(
				GTK_COMBO_BOX_TEXT(widget),
				ix+1,
				charsets[ix].name,
				g_dgettext(GETTEXT_PACKAGE,charsets[ix].description)
			);
		}
	}

	return widget;

 }

 void v3270_grid_attach(GtkGrid *grid, const struct v3270_entry_field * description, GtkWidget *widget)
 {
	GtkWidget *label = gtk_label_new_with_mnemonic(g_dgettext(GETTEXT_PACKAGE,description->label));
	gtk_label_set_mnemonic_widget(GTK_LABEL(label),widget);

	gtk_widget_set_halign(label,GTK_ALIGN_END);
	gtk_grid_attach(grid,label,description->left,description->top,1,1);
	gtk_grid_attach(grid,widget,description->left+1,description->top,description->width,description->height);

	if(description->tooltip)
		gtk_widget_set_tooltip_markup(widget,g_dgettext(GETTEXT_PACKAGE,description->tooltip));

 }

 gboolean v3270_dialog_get_use_header() {

#ifdef _WIN32
	return FALSE;
#elif GTK_CHECK_VERSION(3,12,0)
	gboolean use_header;
	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &use_header, NULL);
	return use_header;
#else
	return FALSE;
#endif // _WIN32

 }



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

#ifndef V3270_INTERNALS_H_INCLUDED

 #define V3270_INTERNALS_H_INCLUDED 1

 #include <glib.h>
 #include <gtk/gtk.h>
 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <v3270.h>

 #if ! GLIB_CHECK_VERSION(2,44,0)

	// Reference: https://github.com/ImageMagick/glib/blob/master/glib/glib-autocleanups.h
	static inline void v3270_autoptr_cleanup_generic_gfree(void *p)
	{
		void **pp = (void**)p;
		g_free (*pp);
	}

	#define g_autofree __attribute__((cleanup(v3270_autoptr_cleanup_generic_gfree)))

 #endif // ! GLIB(2,44,0)

 G_BEGIN_DECLS

 inline GtkWidget * v3270_box_pack_start(GtkWidget *box, GtkWidget *child, gboolean expand, gboolean fill, guint padding)
 {
	gtk_box_pack_start(GTK_BOX(box),child,expand,fill,padding);
	return child;
 }

 inline GtkWidget * v3270_box_pack_end(GtkWidget *box, GtkWidget *child, gboolean expand, gboolean fill, guint padding)
 {
	gtk_box_pack_end(GTK_BOX(box),child,expand,fill,padding);
 	return child;
 }

 G_GNUC_INTERNAL GtkWidget	* v3270_box_pack_frame(GtkWidget *box, GtkWidget *child, const gchar *title, GtkAlign align, gboolean expand, gboolean fill, guint padding);
 G_GNUC_INTERNAL GtkWidget	* v3270_box_pack_frame(GtkWidget *box, GtkWidget *child, const gchar *title, GtkAlign align, gboolean expand, gboolean fill, guint padding);
 G_GNUC_INTERNAL GtkWidget	* v3270_dialog_create_grid(GtkAlign align);
 G_GNUC_INTERNAL GtkWidget	* v3270_dialog_create_frame(GtkWidget * child, const gchar *title);

 // Activity list widget.
 #define GTK_TYPE_V3270_FT_ACTIVITY_LIST				(V3270FTActivityList_get_type ())
 #define GTK_V3270_FT_ACTIVITY_LIST(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270_FT_ACTIVITY_LIST, V3270FTActivityList))
 #define GTK_V3270_FT_ACTIVITY_LIST_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270_FT_ACTIVITY_LIST, V3270FTActivityListClass))
 #define GTK_IS_V3270_FT_ACTIVITY_LIST(obj)				(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270_FT_ACTIVITY_LIST))
 #define GTK_IS_V3270_FT_ACTIVITY_LIST_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270_FT_ACTIVITY_LIST))
 #define GTK_V3270_FT_ACTIVITY_LIST_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270_FT_ACTIVITY_LIST, V3270FTActivityListClass))

 typedef struct _V3270FTActivityList		V3270FTActivityList;
 typedef struct _V3270FTActivityListClass	V3270FTActivityListClass;

 G_GNUC_INTERNAL GtkWidget	* v3270_activity_list_new();
 G_GNUC_INTERNAL void		  v3270_activity_list_append(GtkWidget *widget, GObject *activity);
 G_GNUC_INTERNAL void		  v3270_activity_list_remove(GtkWidget *widget, GObject *activity);
 G_GNUC_INTERNAL void		  v3270_activity_list_load(GtkWidget *widget);
 G_GNUC_INTERNAL void		  v3270_activity_list_save(GtkWidget *widget);
 G_GNUC_INTERNAL void		  v3270_activity_list_save_as(GtkWidget *widget);

 G_END_DECLS

#endif // V3270_INTERNALS_H_INCLUDED

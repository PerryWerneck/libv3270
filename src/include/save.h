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

#ifndef V3270SAVE_H_INCLUDED

 #define V3270SAVE_H_INCLUDED 1

 #include <config.h>
 #define ENABLE_NLS

 #ifndef GETTEXT_PACKAGE
        #define GETTEXT_PACKAGE PACKAGE_NAME
 #endif

 #include <libintl.h>
 #include <glib/gi18n.h>
 #include <gtk/gtk.h>
 #include <lib3270.h>

 G_BEGIN_DECLS

/*--[ Progress widget ]------------------------------------------------------------------------------*/

 #define GTK_TYPE_V3270SaveDialog			(V3270SaveDialog_get_type ())
 #define V3270_SAVE_DIALOG(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270SaveDialog, V3270SaveDialog))
 #define V3270SaveDialog_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270SaveDialog, V3270SaveDialogClass))
 #define IS_V3270SaveDialog(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270SaveDialog))
 #define IS_V3270SaveDialog_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270SaveDialog))
 #define V3270SaveDialog_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270SaveDialog, V3270SaveDialogClass))

 typedef struct _V3270SaveDialog		V3270SaveDialog;
 typedef struct _V3270SaveDialogClass	V3270SaveDialogClass;

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

 GtkWidget * v3270_save_dialog_new(GtkWidget *widget, LIB3270_CONTENT_OPTION mode, const gchar *filename);
 void v3270_save_dialog_run(GtkWidget *widget);

 G_END_DECLS

#endif // V3270SAVE_H_INCLUDED

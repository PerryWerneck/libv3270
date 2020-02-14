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

 #include <config.h>

 #include <glib.h>
 #include <gtk/gtk.h>

 #include <v3270.h>
 #include <v3270/actions.h>
 #include <internals.h>

 // Lib3270 action

 G_GNUC_INTERNAL GType Lib3270Action_get_type(void) G_GNUC_CONST;

 #define LIB3270_TYPE_ACTION		(Lib3270Action_get_type())
 #define LIB3270_ACTION(inst)		(G_TYPE_CHECK_INSTANCE_CAST ((inst), LIB3270_TYPE_ACTION, Lib3270Action))
 #define LIB3270_IS_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_TYPE ((inst), LIB3270_TYPE_ACTION))

 typedef struct _Lib3270ActionClass {
 	V3270ActionClass parent_class;
 } Lib3270ActionClass;

 typedef struct _Lib3270Action {
 	V3270Action parent;
 	const LIB3270_PROPERTY * definition;
 } Lib3270Action;


 // Internal methods
 G_GNUC_INTERNAL int fire_copy_accelerator(GtkWidget *widget, const V3270_ACTION *action);
 G_GNUC_INTERNAL int fire_paste_accelerator(GtkWidget *widget, const V3270_ACTION *action);
 G_GNUC_INTERNAL int fire_zoom_action(GtkWidget *widget, const V3270_ACTION *action);
 G_GNUC_INTERNAL int fire_save_action(GtkWidget *widget, const V3270_ACTION *action);
 G_GNUC_INTERNAL int fire_print_action(GtkWidget *widget, const V3270_ACTION *action);
 G_GNUC_INTERNAL int fire_keypad_action(GtkWidget *widget, const V3270_ACTION *action);


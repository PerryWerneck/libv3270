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

 /**
  * @brief Implements V3270 Dialog Action.
  *
  */

 #include "private.h"
 #include <v3270.h>
 #include <v3270/settings.h>
 #include <v3270/actions.h>
 #include <v3270/tools.h>

 #define V3270_TYPE_DIALOG_ACTION				(V3270DialogAction_get_type())
 #define V3270_DIALOG_ACTION(inst)				(G_TYPE_CHECK_INSTANCE_CAST ((inst), V3270_TYPE_DIALOG_ACTION, V3270DialogAction))
 #define V3270_DIALOG_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), V3270_TYPE_DIALOG_ACTION, V3270DialogActionClass))
 #define V3270_IS_DIALOG_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), V3270_TYPE_DIALOG_ACTION))
 #define V3270_IS_DIALOG_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), V3270_TYPE_DIALOG_ACTION))
 #define V3270_DIALOG_ACTION_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), V3270_TYPE_DIALOG_ACTION, V3270DialogActionClass))

 typedef struct _V3270DialogAction {

	V3270SimpleAction parent;

	GtkWidget * dialog;
	GtkWidget * (*factory)(V3270SimpleAction *, GtkWidget *);

 } V3270DialogAction;

 typedef struct _V3270DialogActionClass {

	V3270SimpleActionClass parent_class;

 } V3270DialogActionClass;


 static void V3270DialogAction_class_init(V3270DialogActionClass *klass);
 static void V3270DialogAction_init(V3270DialogAction *action);
 static void activate(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal);

 G_DEFINE_TYPE(V3270DialogAction, V3270DialogAction, V3270_TYPE_SIMPLE_ACTION);

 static gboolean get_enabled(GAction *action, GtkWidget *terminal) {

 	if((V3270_DIALOG_ACTION(action)->dialog)) {
		return FALSE;
 	}

	return V3270_ACTION_CLASS(V3270DialogAction_parent_class)->get_enabled(action,terminal);

 }

 static void V3270DialogAction_class_init(V3270DialogActionClass *klass) {
 	klass->parent_class.parent_class.get_enabled	= get_enabled;
 	klass->parent_class.parent_class.activate		= activate;
 }

 static void V3270DialogAction_init(V3270DialogAction *action) {

 	action->dialog = NULL;

 }

 V3270SimpleAction * v3270_dialog_action_new(GtkWidget * (*factory)(V3270SimpleAction *, GtkWidget *)) {

  	V3270DialogAction * action = (V3270DialogAction *) g_object_new(V3270_TYPE_DIALOG_ACTION, NULL);
  	action->factory = factory;
  	return V3270_SIMPLE_ACTION(action);

 }

 static void on_destroy(GtkWidget *dialog, V3270DialogAction *action) {

 	if(action->dialog == dialog) {
		action->dialog = NULL;
		v3270_action_notify_enabled(G_ACTION(action));
 	}
 }

 void activate(GAction *object, GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {

	if(!GTK_IS_V3270(terminal))
		return;

	V3270DialogAction * action = V3270_DIALOG_ACTION(object);

	if(!action->factory) {
		g_warning("Action %s is invalid (no factory method)",g_action_get_name(G_ACTION(action)));
		return;
	}

	if(action->dialog)
		return;

	action->dialog = action->factory((V3270SimpleAction *) object, terminal);
	v3270_action_notify_enabled(G_ACTION(action));

	if(action->dialog) {

		GtkWidget * window = gtk_widget_get_toplevel(terminal);
		if(window) {
			gtk_dialog_set_toplevel(action->dialog,window);
			gtk_window_set_attached_to(GTK_WINDOW(action->dialog), window);
			gtk_dialog_set_toplevel(action->dialog,window);
		}

		g_signal_connect(action->dialog,"destroy",G_CALLBACK(on_destroy),action);
		g_signal_connect(action->dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
		gtk_widget_show(GTK_WIDGET(action->dialog));

	}

 }


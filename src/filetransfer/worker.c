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

 #include <string.h>
 #include <internals.h>
 #include "private.h"
 #include "marshal.h"
 #include <v3270/filetransfer.h>

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 enum _SIGNALS
 {
 	V3270_WORKER_ACTIVITY_SIGNAL,	///< @brief Indicates if the list has an activity set.

 	V3270_WORKER_LAST_SIGNAL
 };

 struct _V3270FTWorkerClass
 {
 	GtkGridClass parent_class;

	struct
	{
		void (*activity)(GtkWidget *, GObject *);
	} signal;

 };

 struct _V3270FTWorker
 {
 	GtkGrid parent;

	GtkProgressBar	* pbar;						///< @brief Progress bar.
	GObject 		* activity;					///< @brief File transfer activity.
	GSource			* pulse;					///< @brief Process pulse.

	GtkEntry * field[PROGRESS_FIELD_COUNT];		///< @brief Transfer information widgets.

 };

 G_DEFINE_TYPE(V3270FTWorker, V3270FTWorker, GTK_TYPE_GRID);

 static guint v3270_worker_signals[V3270_WORKER_LAST_SIGNAL] = { 0 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static gboolean do_pulse(V3270FTWorker *worker) {
 	gtk_progress_bar_pulse(worker->pbar);
	return TRUE;
 }

 static void pulse_start(V3270FTWorker *worker)
 {
 	debug("%s(%p)",__FUNCTION__,worker->pulse);
	if(!worker->pulse)
	{
		worker->pulse = g_timeout_source_new(100);
		g_source_set_callback(worker->pulse,(GSourceFunc) do_pulse,worker,NULL);
		g_source_attach(worker->pulse,NULL);
	}
 }

 static void pulse_stop(V3270FTWorker *worker)
 {
	if(worker->pulse)
	{
		g_source_destroy(worker->pulse);
		worker->pulse = NULL;
	}
 }

 static void finalize(GObject *object)
 {
	debug("%s",__FUNCTION__);

	V3270FTWorker * worker = GTK_V3270_FT_WORKER(object);

	pulse_stop(worker);

	if(worker->activity)
	{
		g_object_unref(worker->activity);
		worker->activity = NULL;
	}

	G_OBJECT_CLASS(V3270FTWorker_parent_class)->finalize(object);

 }

 static void V3270FTWorker_activity(GtkWidget G_GNUC_UNUSED(*widget), GObject G_GNUC_UNUSED(*activity))
 {
 	debug("%s",__FUNCTION__);
 }

 static void V3270FTWorker_class_init(G_GNUC_UNUSED V3270FTWorkerClass *klass)
 {
	GObjectClass * gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->finalize = finalize;

	klass->signal.activity = V3270FTWorker_activity;

	v3270_worker_signals[V3270_WORKER_ACTIVITY_SIGNAL] =
	g_signal_new(	"activity",
					G_OBJECT_CLASS_TYPE (gobject_class),
					G_SIGNAL_RUN_FIRST,
					G_STRUCT_OFFSET (V3270FTWorkerClass, signal.activity),
					NULL, NULL,
					v3270ft_VOID__VOID_OBJECT,
					G_TYPE_NONE, 1, G_TYPE_OBJECT);

 }

 static GtkWidget * create_label(V3270FTWorker *widget, const gchar *text, gint left, gint top)
 {
	GtkWidget * label = gtk_label_new(text);
	gtk_widget_set_halign(GTK_WIDGET(label),GTK_ALIGN_END);
	gtk_grid_attach(GTK_GRID(widget),label,left,top,1,1);
	return label;
 }

 static GtkWidget * create_entry(V3270FTWorker *widget, const gchar *text, GtkWidget *entry, gint left, gint top, gint width)
 {
 	GtkWidget * label = create_label(widget,text,left,top);
	gtk_grid_attach(GTK_GRID(widget),entry,left+1,top,width,1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label),entry);
	gtk_widget_set_hexpand(entry,TRUE);
	return entry;
 }

 static void create_field(V3270FTWorker *widget, PROGRESS_FIELD id, GtkWidget *grid, gint top)
 {
	GtkWidget * label = gtk_label_new(gettext(v3270_ft_worker_fields[id].label));
	gtk_widget_set_halign(GTK_WIDGET(label),GTK_ALIGN_END);
	gtk_grid_attach(GTK_GRID(grid),label,0,top,1,1);

	widget->field[id] = GTK_ENTRY(gtk_entry_new());

	gtk_grid_attach(GTK_GRID(grid),GTK_WIDGET(widget->field[id]),1,top,1,1);

 }

 static void V3270FTWorker_init(V3270FTWorker *widget)
 {
	// https://developer.gnome.org/hig/stable/visual-layout.html.en
 	gtk_grid_set_row_spacing(GTK_GRID(widget),6);
 	gtk_grid_set_column_spacing(GTK_GRID(widget),12);

	widget->field[PROGRESS_FIELD_LOCAL] = GTK_ENTRY(create_entry(widget,gettext(v3270_ft_worker_fields[PROGRESS_FIELD_LOCAL].label),gtk_entry_new(),0,0,9));
	widget->field[PROGRESS_FIELD_REMOTE] = GTK_ENTRY(create_entry(widget,gettext(v3270_ft_worker_fields[PROGRESS_FIELD_REMOTE].label),gtk_entry_new(),0,1,9));

	// Create status elements
	GtkWidget * views = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
	gtk_box_set_homogeneous(GTK_BOX(views),TRUE);
	gtk_grid_attach(GTK_GRID(widget),views,0,2,10,1);
	g_object_set(G_OBJECT(views),"margin-top",8,NULL);
	g_object_set(G_OBJECT(views),"margin-bottom",8,NULL);

	{
		// Left options
		GtkWidget * grid = v3270_box_pack_start(views,v3270_dialog_create_grid(GTK_ALIGN_START),TRUE,TRUE,0);

		create_field(widget, PROGRESS_FIELD_TOTAL, grid,0);
		create_field(widget, PROGRESS_FIELD_SPEED, grid,1);
	}

	{
		// Right options
		GtkWidget * grid = v3270_box_pack_start(views,v3270_dialog_create_grid(GTK_ALIGN_END),TRUE,TRUE,0);

		create_field(widget, PROGRESS_FIELD_CURRENT, grid,0);
		create_field(widget, PROGRESS_FIELD_ETA, grid,1);

	}

	{
		// Setup fields tooltips
		size_t id;

		for(id = 0; id < PROGRESS_FIELD_COUNT; id++)
		{
			gtk_widget_set_tooltip_markup(GTK_WIDGET(widget->field[id]),v3270_ft_worker_fields[id].tooltip);
			gtk_widget_set_can_focus(GTK_WIDGET(widget->field[id]),FALSE);
			gtk_editable_set_editable(GTK_EDITABLE(widget->field[id]),FALSE);
		}

		for(id = PROGRESS_FIELD_TOTAL; id < PROGRESS_FIELD_COUNT; id++)
		{
			gtk_entry_set_width_chars(widget->field[id],15);
		}

	}

	// Create progress bar
	widget->pbar = GTK_PROGRESS_BAR(gtk_progress_bar_new());
	gtk_progress_bar_set_show_text(widget->pbar,TRUE);
	gtk_progress_bar_set_text(widget->pbar,_("No transfer"));

	g_object_set(G_OBJECT(widget->pbar),"margin-top",8,NULL);

	gtk_grid_attach(GTK_GRID(widget),GTK_WIDGET(widget->pbar),0,3,10,1);

 }

 LIB3270_EXPORT GtkWidget * v3270_ft_worker_new()
 {
	return GTK_WIDGET(g_object_new(GTK_TYPE_V3270_FT_WORKER, NULL));
 }

 LIB3270_EXPORT void v3270_ft_worker_set_activity(GtkWidget *widget, GObject *activity)
 {
 	size_t ix;

	V3270FTWorker * worker = GTK_V3270_FT_WORKER(widget);

	if(worker->activity)
	{
		g_object_unref(worker->activity);
		worker->activity = NULL;
	}

	if(activity)
	{
		worker->activity = activity;
		g_object_ref(worker->activity);

		gtk_entry_set_text(worker->field[PROGRESS_FIELD_LOCAL],v3270_ft_activity_get_local_filename(activity));
		gtk_entry_set_text(worker->field[PROGRESS_FIELD_REMOTE],v3270_ft_activity_get_remote_filename(activity));

	}
	else
	{
		gtk_entry_set_text(worker->field[PROGRESS_FIELD_LOCAL],"");
		gtk_entry_set_text(worker->field[PROGRESS_FIELD_REMOTE],"");
	}

	for(ix = PROGRESS_FIELD_TOTAL; ix < PROGRESS_FIELD_COUNT; ix++)
	{
		gtk_entry_set_text(worker->field[ix],"");
	}

	gtk_progress_bar_set_text(worker->pbar,_("Starting transfer"));
	pulse_start(worker);

	g_signal_emit(widget, v3270_worker_signals[V3270_WORKER_ACTIVITY_SIGNAL], 0, worker->activity);

 }


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

 typedef enum _V3270_WORKER_SIGNAL
 {
 	V3270_WORKER_ACTIVITY_SIGNAL,			///< @brief Indicates if the list has an activity set.
 	V3270_WORKER_TRANSFER_FAILED_SIGNAL,	///< @brief Transfer has failed.
 	V3270_WORKER_TRANSFER_SUCCESS_SIGNAL,	///< @brief Transfer complete.
 	V3270_WORKER_TRANSFER_STATE_SIGNAL,		///< @brief Transfer state has changed.

 	V3270_WORKER_LAST_SIGNAL
 } V3270_WORKER_SIGNAL;

 struct _V3270FTWorkerClass
 {
 	GtkGridClass parent_class;


 };

 struct _V3270FTWorker
 {
 	GtkGrid parent;

 	H3270			* hSession;

	GtkProgressBar	* pbar;						///< @brief Progress bar.
	GObject 		* activity;					///< @brief File transfer activity.
	GSource			* pulse;					///< @brief Process pulse.

	struct
	{
		GSource		* timer;					///< @brief Timeout timer.
		time_t		  limit;					///< @brief Timestamp for timeout transfer.
		time_t		  value;					///< @brief Timeout value.

	} timeout;

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

 static void timer_stop(V3270FTWorker *worker)
 {
	if(worker->timeout.timer)
	{
		g_source_destroy(worker->timeout.timer);
		worker->timeout.timer = NULL;
	}
 }

 static void finalize(GObject *object)
 {
	debug("%s",__FUNCTION__);

	V3270FTWorker * worker = GTK_V3270_FT_WORKER(object);

	if(worker->hSession)
	{
		lib3270_reset_ft_callbacks(worker->hSession);
		lib3270_ft_set_user_data(worker->hSession,NULL);
		lib3270_ft_destroy(worker->hSession,NULL);
		worker->hSession = NULL;
	}

	pulse_stop(worker);
	timer_stop(worker);

	g_clear_object(&worker->activity);

	G_OBJECT_CLASS(V3270FTWorker_parent_class)->finalize(object);

 }

 static void V3270FTWorker_class_init(G_GNUC_UNUSED V3270FTWorkerClass *klass)
 {
	GObjectClass * gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->finalize = finalize;

	v3270_worker_signals[V3270_WORKER_ACTIVITY_SIGNAL] =
		g_signal_new(
			"activity",
			G_OBJECT_CLASS_TYPE (gobject_class),
			G_SIGNAL_RUN_FIRST,
			0,
			NULL, NULL,
			v3270ft_VOID__VOID_OBJECT,
			G_TYPE_NONE, 1, G_TYPE_OBJECT
		);

 	v3270_worker_signals[V3270_WORKER_TRANSFER_FAILED_SIGNAL] =
		g_signal_new(
			"failed",
			G_OBJECT_CLASS_TYPE (gobject_class),
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0,
			NULL, NULL,
			v3270ft_VOID__POINTER_POINTER,
			G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_POINTER
	);

 	v3270_worker_signals[V3270_WORKER_TRANSFER_SUCCESS_SIGNAL] =
		g_signal_new(
			"success",
			G_OBJECT_CLASS_TYPE (gobject_class),
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0,
			NULL, NULL,
			v3270ft_VOID__POINTER_POINTER,
			G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_POINTER
	);

 	v3270_worker_signals[V3270_WORKER_TRANSFER_STATE_SIGNAL] =
		g_signal_new(
			"ft-state-changed",
			G_OBJECT_CLASS_TYPE (gobject_class),
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0,
			NULL, NULL,
			v3270ft_VOID__UINT_POINTER,
			G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_POINTER
	);


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
	gtk_entry_set_alignment(widget->field[id],1);

	gtk_grid_attach(GTK_GRID(grid),GTK_WIDGET(widget->field[id]),1,top,1,1);

 }

 static void V3270FTWorker_init(V3270FTWorker *widget)
 {
 	// Set defaults.
 	widget->timeout.value = 10;

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

	if(worker->hSession)
	{
		lib3270_ft_destroy(worker->hSession,NULL);
	}

	g_clear_object(&worker->activity);

	if(activity)
	{
		worker->activity = activity;
		g_object_ref_sink(activity);

		gtk_entry_set_text(worker->field[PROGRESS_FIELD_LOCAL],v3270_ft_activity_get_local_filename(activity));
		gtk_entry_set_text(worker->field[PROGRESS_FIELD_REMOTE],v3270_ft_activity_get_remote_filename(activity));

		gtk_progress_bar_set_text(worker->pbar,_("Starting transfer"));
		pulse_start(worker);

	}
	else
	{
		gtk_entry_set_text(worker->field[PROGRESS_FIELD_LOCAL],"");
		gtk_entry_set_text(worker->field[PROGRESS_FIELD_REMOTE],"");
		pulse_stop(worker);

	}

	for(ix = PROGRESS_FIELD_TOTAL; ix < PROGRESS_FIELD_COUNT; ix++)
	{
		gtk_entry_set_text(worker->field[ix],"");
	}

	g_signal_emit(widget, v3270_worker_signals[V3270_WORKER_ACTIVITY_SIGNAL], 0, worker->activity);

 }

 LIB3270_EXPORT void v3270_ft_worker_set_session(GtkWidget *widget, H3270 *hSession)
 {
	V3270FTWorker * worker = GTK_V3270_FT_WORKER(widget);
	worker->hSession = hSession;

 }

 struct bg_complete
 {
	V3270FTWorker	 	* worker;
 	V3270_WORKER_SIGNAL	  signal;
 	unsigned long		  length;
 	double 				  kbytes_sec;
 	char				  msg[1];
 };

 static gboolean bg_emit_complete(struct bg_complete * cfg)
 {
 	// Try to get more detailed info.
 	const gchar * description	= NULL;

	pulse_stop(cfg->worker);
	timer_stop(cfg->worker);

	if(cfg->length)
	{
		gtk_entry_set_printf(cfg->worker->field[PROGRESS_FIELD_TOTAL],"%lu",cfg->length);
	}
	else
	{
		gtk_entry_set_text(cfg->worker->field[PROGRESS_FIELD_TOTAL],"");
	}

	gtk_entry_set_text(cfg->worker->field[PROGRESS_FIELD_ETA],"");


	const LIB3270_FT_MESSAGE * ftMessage = lib3270_translate_ft_message(cfg->msg);

	if(ftMessage)
	{
		description = ftMessage->description;
		if(ftMessage->failed)
			cfg->signal = V3270_WORKER_TRANSFER_FAILED_SIGNAL;

	}

	gtk_progress_bar_set_text(cfg->worker->pbar,cfg->msg);

	g_signal_emit(
		GTK_WIDGET(cfg->worker),
		v3270_worker_signals[cfg->signal],
		0,
		cfg->msg,
		description
	);

	return FALSE;

 }

 static void ft_complete(H3270 G_GNUC_UNUSED(*hSession), unsigned long length, double kbytes_sec, const char *msg, void *widget)
 {
 	struct bg_complete * state = g_malloc0(sizeof(struct bg_complete)+strlen(msg)+1);

	state->worker		= GTK_V3270_FT_WORKER(widget);
	state->signal		= V3270_WORKER_TRANSFER_SUCCESS_SIGNAL;
	state->length		= length;
	state->kbytes_sec	= kbytes_sec;

	strcpy(state->msg,msg);

	gdk_threads_add_idle_full(G_PRIORITY_LOW,(GSourceFunc) bg_emit_complete, state, g_free);

 }

 static void ft_failed(H3270 G_GNUC_UNUSED(*hSession), unsigned long length, double kbytes_sec, const char *msg, void *widget)
 {
	debug("%s(%p,%s)",__FUNCTION__,widget,msg);

 	struct bg_complete * state = g_malloc0(sizeof(struct bg_complete)+strlen(msg));
	strcpy(state->msg,msg);

	state->worker		= GTK_V3270_FT_WORKER(widget);
	state->signal		= V3270_WORKER_TRANSFER_FAILED_SIGNAL;
	state->length		= length;
	state->kbytes_sec	= kbytes_sec;

	gdk_threads_add_idle_full(G_PRIORITY_LOW,(GSourceFunc) bg_emit_complete, state, g_free);

 }

 static void ft_message(G_GNUC_UNUSED H3270 *hSession, const char *msg, void *widget)
 {
	V3270FTWorker * worker = GTK_V3270_FT_WORKER(widget);
	debug("%s(%p,%s)",__FUNCTION__,worker,msg);

	gtk_progress_bar_set_text(worker->pbar,msg);

 }

 struct bg_update
 {
	V3270FTWorker	 * worker;
	unsigned long 	  current;
	unsigned long	  total;
	double			  kbytes_sec;
 };

 static gboolean bg_emit_update(struct bg_update * update)
 {
	// Update values
	gtk_entry_set_printf(update->worker->field[PROGRESS_FIELD_CURRENT],"%lu",update->current);
	gtk_entry_set_printf(update->worker->field[PROGRESS_FIELD_TOTAL],"%lu",update->total);

	if(update->total)
	{
		// Calculate ETA
		double remaining = ((double) (update->total - update->current))/1024.0;

		if(remaining > 0 && update->kbytes_sec > 0) {

			char buffer[40];
			double	seconds = ((double) remaining) / update->kbytes_sec;
			time_t 	eta		= time(NULL) + ((time_t) seconds);
			strftime(buffer, 39, "%H:%M:%S", localtime(&eta));

			gtk_entry_set_text(update->worker->field[PROGRESS_FIELD_ETA],buffer);

		} else {

			gtk_entry_set_text(update->worker->field[PROGRESS_FIELD_ETA],"");

		}

		if(update->current)
		{
			// Update progress bar
			pulse_stop(update->worker);
			gtk_progress_bar_set_fraction(update->worker->pbar, ((gdouble) update->current) / ((gdouble) update->total));
		}

		gtk_entry_set_printf(update->worker->field[PROGRESS_FIELD_ETA],"%lu",update->total);

	}

 	return FALSE;
 }

 static void ft_update(G_GNUC_UNUSED H3270 *hSession, unsigned long current, unsigned long total, double kbytes_sec, void *widget)
 {
	debug("%s(%p,%p)",__FUNCTION__,widget,widget);

	struct bg_update * update = g_new0(struct bg_update,1);

	update->worker = GTK_V3270_FT_WORKER(widget);
	update->current = current;
	update->total = total;
	update->kbytes_sec = kbytes_sec;

	gdk_threads_add_idle_full(G_PRIORITY_LOW,(GSourceFunc) bg_emit_update, update, g_free);

 }

 static void ft_running(G_GNUC_UNUSED H3270 *hSession, G_GNUC_UNUSED int is_cut, void *widget)
 {
	V3270FTWorker * worker = GTK_V3270_FT_WORKER(widget);
	debug("%s(%p,%p)",__FUNCTION__,worker,widget);

	// Reset timeout.
	worker->timeout.limit = time(NULL) + worker->timeout.value;

 }

 static void ft_aborting(G_GNUC_UNUSED H3270 *hSession, const char *reason, void *widget)
 {
	V3270FTWorker * worker = GTK_V3270_FT_WORKER(widget);
	debug("%s(%p,%p)",__FUNCTION__,worker,widget);

	gtk_progress_bar_set_text(worker->pbar,reason);

 }

 struct bg_state_changed
 {
	V3270FTWorker		* worker;
	LIB3270_FT_STATE	  st;
	char msg[1];
 };

 static gboolean bg_emit_state_changed(struct bg_state_changed * cfg)
 {
	g_signal_emit(cfg->worker, v3270_worker_signals[V3270_WORKER_TRANSFER_STATE_SIGNAL], 0, (guint) cfg->st, cfg->msg);
	return FALSE;
 }

 static void ft_state_changed(G_GNUC_UNUSED H3270 *hSession, LIB3270_FT_STATE st, const char *msg, void *widget)
 {
	debug("%s(%p,%s)",__FUNCTION__,widget,msg);

 	struct bg_state_changed * state = g_malloc0(sizeof(struct bg_state_changed)+strlen(msg)+1);
	state->worker = GTK_V3270_FT_WORKER(widget);
	state->st = st;
	strcpy(state->msg,msg);

	gdk_threads_add_idle_full(G_PRIORITY_LOW,(GSourceFunc) bg_emit_state_changed, state, g_free);

 }

static gboolean do_timer(V3270FTWorker *worker) {

	debug("%d",(int) worker->timeout.limit - time(NULL));

	if(time(NULL) > worker->timeout.limit) {

		// Timeout.
		debug("%s: Transfer timeout",__FUNCTION__);

		pulse_stop(worker);
		timer_stop(worker);

		const gchar * message = _("Transfer operation has timed out");

		if(worker->hSession)
		{
			lib3270_ft_destroy(worker->hSession, message);
		}
		else
		{
			gtk_progress_bar_set_text(worker->pbar,message);

			g_signal_emit(
				GTK_WIDGET(worker),
				v3270_worker_signals[V3270_WORKER_TRANSFER_FAILED_SIGNAL],
				0,
				_( "Transfer failed" ),
				message
			);

		}
	}

	return TRUE;
 }

 LIB3270_EXPORT int	v3270_ft_worker_start(GtkWidget *widget)
 {
	V3270FTWorker * worker = GTK_V3270_FT_WORKER(widget);

	if(!(worker->hSession && worker->activity))
		return errno = EINVAL;

	const char *message = NULL;
	H3270FT * ft = v3270_ft_activity_begin_transfer(worker->activity, worker->hSession, &message);

	if(!ft)
	{
		if(!message)
			message = N_("Can't start file transfer session");

		gtk_progress_bar_set_text(worker->pbar,gettext(message));

		pulse_stop(worker);

		// Emit "transfer failed"
		g_signal_emit(GTK_WIDGET(widget),v3270_worker_signals[V3270_WORKER_TRANSFER_FAILED_SIGNAL], 0, gettext(message), NULL);

		// Emit "no transfer"
		ft_state_changed(worker->hSession, LIB3270_FT_STATE_NONE, _("No transfer in progress"), widget);

		return -1;
	}

	struct lib3270_ft_callbacks * cbk = lib3270_get_ft_callbacks(worker->hSession, sizeof(struct lib3270_ft_callbacks));

	if(!cbk) {

		gchar *message = _( "Can't set callback table" );

		lib3270_ft_destroy(worker->hSession,message);

		gtk_progress_bar_set_text(worker->pbar,message);
		pulse_stop(worker);

		g_signal_emit(
			GTK_WIDGET(widget),
			v3270_worker_signals[V3270_WORKER_TRANSFER_FAILED_SIGNAL],
			0,
			message,
			_("The callback table for file transfer was rejected, possible version mismatch on lib3270")
		);

		return -1;
	}

	lib3270_ft_set_user_data(worker->hSession,widget);

	cbk->complete 		= ft_complete;
	cbk->failed 		= ft_failed;
	cbk->update			= ft_update;
	cbk->running		= ft_running;
	cbk->aborting		= ft_aborting;
	cbk->state_changed	= ft_state_changed;
	cbk->message		= ft_message;

	worker->timeout.limit = time(NULL) + worker->timeout.value;

	if(!worker->timeout.timer)
	{
		worker->timeout.timer = g_timeout_source_new_seconds(1);
		g_source_set_callback(worker->timeout.timer,(GSourceFunc) do_timer,worker,NULL);
		g_source_attach(worker->timeout.timer,NULL);
	}

	lib3270_ft_start(worker->hSession);

 	return 0;
 }


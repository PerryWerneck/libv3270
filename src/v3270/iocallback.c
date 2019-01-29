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
#include <lib3270.h>
#include <lib3270/log.h>
#include "private.h"

static void				* static_AddSource(H3270 *session, int fd, LIB3270_IO_FLAG flag, void(*proc)(H3270 *, int, LIB3270_IO_FLAG, void *), void *userdata);
static void	  			  static_RemoveSource(H3270 *session, void *id);
static void      		  static_SetSourceState(H3270 *session, void *id, int enabled);

static void 			* static_AddTimer(H3270 *session, unsigned long interval_ms, int (*proc)(H3270 *session));
static void 			  static_RemoveTimer(H3270 *session, void * timer);
static int				  static_Sleep(H3270 *hSession, int seconds);
static int 				  static_RunPendingEvents(H3270 *hSession, int wait);

/*---[ Structs ]-------------------------------------------------------------------------------------------*/

 typedef struct _timer
 {
	unsigned char remove;
	void	* userdata;
	int		(*call)(H3270 *session);
	H3270 	* session;
 } TIMER;

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

static void	* static_AddSource(H3270 *session, int fd, LIB3270_IO_FLAG flag, void(*call)(H3270 *, int, LIB3270_IO_FLAG, void *), void *userdata)
{
	return (void *) IO_source_new(session, fd, flag, call, userdata);
}

static void static_RemoveSource(G_GNUC_UNUSED H3270 *session, void *id)
{
	if(id)
		g_source_destroy((GSource *) id);
}

static void static_SetSourceState(G_GNUC_UNUSED H3270 *session, void *id, int enabled)
{
	if(id)
		IO_source_set_state( (GSource *) id, (gboolean) (enabled != 0));
}

static gboolean do_timer(TIMER *t)
{
	if(!t->remove)
		return t->call(t->session) != 0;

	return FALSE;
}

static void * static_AddTimer(H3270 *session, unsigned long interval, int (*call)(H3270 *session))
{
	TIMER *t = g_malloc0(sizeof(TIMER));

	t->call		= call;
	t->session	= session;

	g_timeout_add_full(G_PRIORITY_DEFAULT, (guint) interval, (GSourceFunc) do_timer, t, g_free);

	return t;
}

static void static_RemoveTimer(G_GNUC_UNUSED H3270 *session, void * timer)
{
	((TIMER *) timer)->remove++;
}

static int static_Sleep(G_GNUC_UNUSED H3270 *hSession, int seconds)
{
	time_t end = time(0) + seconds;

	while(time(0) < end)
		gtk_main_iteration();

	return 0;
}

static int static_RunPendingEvents(G_GNUC_UNUSED H3270 *hSession, int wait)
{
	int rc = 0;
	while(gtk_events_pending())
	{
		rc = 1;
		gtk_main_iteration();
	}

	if(wait)
		gtk_main_iteration();

	return rc;
}

static void beep(G_GNUC_UNUSED H3270 *session)
{
	gdk_display_beep(gdk_display_get_default());
}

struct bgParameter
{
	gboolean	  running;
	H3270 		* hSession;
	int			  rc;
	int			  (*callback)(H3270 *session, void *parm);
	void		* parm;

};

gpointer BgCall(struct bgParameter *p)
{
	p->rc = p->callback(p->hSession,p->parm);
	p->running = FALSE;
	return 0;
}

static int static_RunTask(H3270 *hSession, int(*callback)(H3270 *, void *), void *parm)
{
//	return callback(hSession,parm);

	struct bgParameter p = { TRUE, hSession, -1, callback, parm };

	trace("%s starts -------------------------------------", __FUNCTION__);

	p.running = TRUE;

	GThread	*thread = g_thread_new(PACKAGE_NAME, (GThreadFunc) BgCall, &p);

	if(!thread)
	{
		g_error("Can't start background thread");
		return -1;
	}

	while(p.running)
	{
		gtk_main_iteration();
	}

	g_thread_join(thread);

	trace("%s ends ---------------------------------------", __FUNCTION__);

	return p.rc;

}

void v3270_register_io_handlers(G_GNUC_UNUSED v3270Class *cls)
{
	static LIB3270_IO_CONTROLLER hdl =
	{
		sizeof(LIB3270_IO_CONTROLLER),

		static_AddTimer,
		static_RemoveTimer,

		static_AddSource,
		static_RemoveSource,
		static_SetSourceState,

		static_Sleep,
		static_RunPendingEvents,
		beep,
		static_RunTask

	};

	if(lib3270_register_io_controller(&hdl))
	{
		g_error("%s",_( "Can't set lib3270 I/O controller" ) );
	}

}

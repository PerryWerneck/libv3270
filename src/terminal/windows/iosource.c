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

 #include <config.h>

 #ifdef _WIN32
	#include <winsock2.h>
	#include <windows.h>
 #endif // _WIN32

 #include <terminal.h>

/*---[ Structs ]-------------------------------------------------------------------------------------------*/

 typedef struct _IO_Source
 {
	GSource		  gsrc;
	GPollFD		  poll;
 	gboolean	  enabled;
	int			  fd;
	void		  (*call)(H3270 *, int, LIB3270_IO_FLAG, void *);
	H3270		* session;
	void		* userdata;
 } IO_Source;

static gboolean	IO_prepare(GSource *source, gint *timeout);
static gboolean	IO_check(GSource *source);
static gboolean	IO_dispatch(GSource *source, GSourceFunc callback, gpointer user_data);
static void		IO_finalize(GSource *source); /* Can be NULL */
static gboolean	IO_closure(gpointer data);

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

GSource	* IO_source_new(H3270 *session, int fd, LIB3270_IO_FLAG flag, void(*call)(H3270 *, int, LIB3270_IO_FLAG, void *), void *userdata)
{
	static GSourceFuncs IOSources =
	{
		IO_prepare,
		IO_check,
		IO_dispatch,
		IO_finalize,
		IO_closure,
		NULL
	};

	IO_Source *src = (IO_Source *) g_source_new(&IOSources,sizeof(IO_Source));

	src->fd				= fd;
	src->enabled		= TRUE;
	src->call			= call;
	src->userdata		= userdata;
	src->session		= session;

	src->poll.fd		= (int) fd;
	src->poll.events	= G_IO_HUP|G_IO_ERR;

	if(flag & LIB3270_IO_FLAG_READ)
		src->poll.events |= G_IO_IN;

	if(flag & LIB3270_IO_FLAG_WRITE)
		src->poll.events |= G_IO_OUT;

	g_source_attach((GSource *) src,NULL);
	g_source_add_poll((GSource *) src,&src->poll);

	return (GSource *) src;
}

void IO_source_set_state(GSource *source, gboolean enable)
{
	((IO_Source *)source)->enabled = enable;
}

static gboolean IO_prepare(G_GNUC_UNUSED GSource *source, G_GNUC_UNUSED gint *timeout)
{
	/*
 	 * Called before all the file descriptors are polled.
	 * If the source can determine that it is ready here
	 * (without waiting for the results of the poll() call)
	 * it should return TRUE.
	 *
	 * It can also return a timeout_ value which should be the maximum
	 * timeout (in milliseconds) which should be passed to the poll() call.
	 * The actual timeout used will be -1 if all sources returned -1,
	 * or it will be the minimum of all the timeout_ values
	 * returned which were >= 0.
	 *
	 */
	return 0;
}

static gboolean IO_check(GSource *source)
{
	/*
 	 * Called after all the file descriptors are polled.
 	 * The source should return TRUE if it is ready to be dispatched.
	 * Note that some time may have passed since the previous prepare
	 * function was called, so the source should be checked again here.
	 *
	 */
	if(((IO_Source *) source)->enabled)
	{
		fd_set rfds, wfds;
		struct timeval tm;

		memset(&tm,0,sizeof(tm));

		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
	//	FD_ZERO(&xfds);

		if(((IO_Source *) source)->poll.events & G_IO_IN)
		{
			FD_SET(((IO_Source *) source)->poll.fd, &rfds);
		}

		if(((IO_Source *) source)->poll.events & G_IO_OUT)
		{
			FD_SET(((IO_Source *) source)->poll.fd, &wfds);
		}

		if(select(((IO_Source *) source)->poll.fd+1, &rfds, &wfds, NULL, &tm) > 0)
			return TRUE;
	}

	return FALSE;
}

static gboolean IO_dispatch(GSource *source, G_GNUC_UNUSED GSourceFunc callback, G_GNUC_UNUSED gpointer user_data)
{
	/*
	 * Called to dispatch the event source,
	 * after it has returned TRUE in either its prepare or its check function.
	 * The dispatch function is passed in a callback function and data.
	 * The callback function may be NULL if the source was never connected
	 * to a callback using g_source_set_callback(). The dispatch function
	 * should call the callback function with user_data and whatever additional
	 * parameters are needed for this type of event source.
	 */
	IO_Source *obj = (IO_Source *) source;

	obj->call(obj->session,obj->fd,0,obj->userdata);

	return TRUE;
}

static void IO_finalize(G_GNUC_UNUSED GSource *source)
{

}

static gboolean IO_closure(G_GNUC_UNUSED gpointer data)
{
	return 0;
}

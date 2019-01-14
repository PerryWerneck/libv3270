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
 * Este programa está nomeado como trace.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#ifndef PW3270_TRACE_H_INCLUDED

 #include <gtk/gtk.h>
 #include <v3270.h>
 #include <lib3270.h>

 #define PW3270_TRACE_H_INCLUDED 1

 G_BEGIN_DECLS

 #define V3270_TYPE_TRACE				(v3270_trace_get_type ())
 #define V3270_TRACE(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), V3270_TYPE_TRACE, v3270_trace))
 #define V3270_TRACE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), V3270_TYPE_TRACE, pw3270_traceClass))
 #define IS_V3270_TRACE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), V3270_TYPE_TRACE))
 #define IS_V3270_TRACE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), V3270_TYPE_TRACE))
 #define V3270_TRACE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), V3270_TYPE_TRACE, v3270_traceClass))

 typedef struct _v3270_trace		  v3270_trace;
 typedef struct _v3270_traceClass	  v3270_traceClass;

 LIB3270_EXPORT	GtkWidget			* v3270_trace_new();
 LIB3270_EXPORT	GtkWidget			* v3270_trace_new_from_session(H3270 *hSession);
 LIB3270_EXPORT	void				  v3270_trace_set_session(GtkWidget *widget, H3270 *hSession);
 LIB3270_EXPORT	GType 				  v3270_trace_get_type(void);
 LIB3270_EXPORT void				  v3270_trace_vprintf(GtkWidget *widget, const char *fmt, va_list args);
 LIB3270_EXPORT void				  v3270_trace_printf(GtkWidget *widget, const char *fmt, ... );
 LIB3270_EXPORT gchar				* v3270_trace_get_command(GtkWidget *widget);
 LIB3270_EXPORT void				  v3270_trace_set_destroy_on_close(GtkWidget *widget,gboolean on);
 LIB3270_EXPORT void				  v3270_trace_set_font_from_string(GtkWidget *widget, const gchar *name);

 G_END_DECLS

#endif // V3270_H_INCLUDED

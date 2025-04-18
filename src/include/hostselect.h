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

#ifndef V3270HOST_H_INCLUDED

 #define V3270HOST_H_INCLUDED 1

 #include <config.h>
 #define ENABLE_NLS

 #ifndef GETTEXT_PACKAGE
        #define GETTEXT_PACKAGE PACKAGE_NAME
 #endif

 #include <libintl.h>
 #include <glib/gi18n-lib.h>
 #include <gtk/gtk.h>
 #include <lib3270.h>

 G_BEGIN_DECLS

/*--[ Host selection dialog ]------------------------------------------------------------------------*/

 #define GTK_TYPE_V3270HostSelectWidget				(V3270HostSelectWidget_get_type ())
 #define GTK_V3270HostSelectWidget(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270HostSelectWidget, V3270HostSelectWidget))
 #define GTK_V3270HostSelectWidget_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270HostSelectWidget, V3270HostSelectWidgetClass))
 #define GTK_IS_V3270HostSelectWidget(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270HostSelectWidget))
 #define GTK_IS_V3270HostSelectWidget_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270HostSelectWidget))
 #define GTK_V3270HostSelectWidget_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270HostSelectWidget, V3270HostSelectWidgetClass))

 typedef struct _V3270HostSelectWidget		V3270HostSelectWidget;
 typedef struct _V3270HostSelectWidgetClass	V3270HostSelectWidgetClass;

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

 void	  v3270_host_select_set_session(GtkWidget *widget, GtkWidget *session);
 int	  v3270_host_select_apply(GtkWidget *widget);

 G_END_DECLS

#endif // V3270HOST_H_INCLUDED

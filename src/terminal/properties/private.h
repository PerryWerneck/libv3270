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
 * Este programa está nomeado como properties.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifdef WIN32
	#include <winsock2.h>
	#include <windows.h>
	#include <ws2tcpip.h>
#endif // WIN32

 #include <gtk/gtk.h>
 #include <limits.h>
 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/actions.h>
 #include <lib3270/log.h>
 #include <lib3270/properties.h>
 #include <lib3270/toggle.h>
 #include <stdlib.h>
 #include <errno.h>
 #include <v3270.h>
 #include <terminal.h>

 enum _v3270_internal_property
 {
	V3270_PROPERTY_FONT_FAMILY		= 2,	///< @brief Name of the font-family used by widget.
	V3270_PROPERTY_CLIPBOARD		= 3,	///< @brief Name of the selected clipboard.
	V3270_PROPERTY_SESSION_NAME		= 4,	///< @brief Widget's session name.
	V3270_PROPERTY_AUTO_DISCONNECT	= 5,	///< @brief Auto disconnect.
	V3270_PROPERTY_REMAP_FILE		= 6,	///< @brief Path of the remap file.


	V3270_PROPERTY_DYNAMIC			= 7		///< @brief Id of the first LIB3270 internal property.
 };

 G_GNUC_INTERNAL void v3270_get_property(GObject *object,guint prop_id, GValue *value, GParamSpec *pspec);
 G_GNUC_INTERNAL void v3270_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);



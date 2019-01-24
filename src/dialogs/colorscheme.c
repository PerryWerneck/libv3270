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
 #define ENABLE_NLS

 #ifndef GETTEXT_PACKAGE
	#define GETTEXT_PACKAGE PACKAGE_NAME
 #endif

 #include <libintl.h>
 #include <glib/gi18n.h>

 #include <v3270.h>
 #include <v3270/colorscheme.h>

 #define V3270_COLOR_BASE V3270_COLOR_GRAY+1

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 struct _V3270ColorSchemeClass
 {
 	GtkComboBoxClass parent_class;


 };

 struct _V3270ColorScheme
 {
 	GtkComboBox parent;

 };

 G_DEFINE_TYPE(V3270ColorScheme, V3270ColorScheme, GTK_TYPE_COMBO_BOX);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void V3270ColorScheme_class_init(G_GNUC_UNUSED V3270ColorSchemeClass *klass)
 {

 }

 static void V3270ColorScheme_init(V3270ColorScheme *widget)
 {

 }

 LIB3270_EXPORT GtkWidget * v3270_color_scheme_new(GdkRGBA *colors)
 {
	GtkWidget *widget = GTK_WIDGET(g_object_new(GTK_TYPE_V3270_COLOR_SCHEME, NULL));


	return widget;
 }


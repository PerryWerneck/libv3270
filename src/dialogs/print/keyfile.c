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

 #include "private.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void v3270_print_operation_to_key_file(GtkPrintOperation *operation, GKeyFile *key_file) {

	GtkPrintSettings	* settings	= gtk_print_operation_get_print_settings(operation);
	GtkPageSetup		* pgsetup	= gtk_print_operation_get_default_page_setup(operation);
	GtkPaperSize        * papersize = gtk_page_setup_get_paper_size(pgsetup);

	trace("%s(%p)",__FUNCTION__,operation);
	g_message("Saving print settings");

	gtk_print_settings_to_key_file(settings,key_file,"print_settings");
	gtk_page_setup_to_key_file(pgsetup,key_file,"page_setup");
	gtk_paper_size_to_key_file(papersize,key_file,"paper_size");

	// Store font family
	g_autofree gchar * font_family = v3270_print_operation_get_font_family(operation);
	g_key_file_set_string(key_file,"print","font-family",font_family);

	// Store color settings
	g_autofree gchar * colors = v3270_print_operation_get_color_scheme(operation);
	g_key_file_set_string(key_file,"print","colors",colors);

 }

 void v3270_print_operation_load_key_file(GtkPrintOperation *operation, GKeyFile *key_file) {

	GtkPrintSettings 	* settings	= gtk_print_settings_new();
	GtkPageSetup 		* setup 	= gtk_page_setup_new();
	GtkPaperSize        * papersize	= NULL;
	GError				* error		= NULL;

	// Load page and print settings

	if(g_key_file_has_group(key_file,"print_settings") && !gtk_print_settings_load_key_file(settings,key_file,"print_settings",&error)) {
		g_warning("Error getting print settings: %s",error->message);
		g_error_free(error);
		error = NULL;
	}

	if(g_key_file_has_group(key_file,"page_setup") && !gtk_page_setup_load_key_file(setup,key_file,"page_setup",&error)) {
		g_warning("Error getting page setup: %s",error->message);
		g_error_free(error);
		error = NULL;
	}

	if(g_key_file_has_group(key_file,"paper_size")) {
		// Use saved paper size

		papersize = gtk_paper_size_new_from_key_file(key_file,"paper_size",&error);
		if(error)
		{
			g_warning("Error loading paper size: %s",error->message);
			g_error_free(error);
			error = NULL;
		}

	} else {
		// Create default
		papersize = gtk_paper_size_new(NULL);
	}

	gtk_print_operation_set_print_settings(operation,settings);
	gtk_page_setup_set_paper_size_and_default_margins(setup,papersize);
	gtk_print_operation_set_default_page_setup(operation,setup);

	// Load font and colors
	g_autofree gchar * font_family = g_key_file_get_string(key_file,"print","font-family",NULL);
	if(font_family && *font_family)
		v3270_print_operation_set_font_family(operation,font_family);

	g_autofree gchar * color_scheme	= g_key_file_get_string(key_file,"print","colors",NULL);
	if(color_scheme && *color_scheme)
		v3270_print_operation_set_color_scheme(operation,color_scheme);

 }

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

 #pragma once

 #include <v3270.h>
 #include <v3270/print.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>
 #include <internals.h>

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 struct _V3270PrintOperationClass
 {
 	GtkPrintOperationClass parent_class;

 };

 typedef struct _column
 {
	unsigned char	c;
	unsigned short	attr;
 } column;

 struct _V3270PrintOperation
 {
 	GtkPrintOperation		  parent;
    LIB3270_CONTENT_OPTION	  mode;
    v3270					* widget;
    H3270					* session;

	size_t					  lpp;					///< @brief Lines per page (in rows).
	size_t					  pages;				///< @brief Number of pages.

	struct
	{
		GdkRGBA				  colors[V3270_COLOR_COUNT];	///< @brief Color scheme for printing.
		gboolean			  show_selection;				///< @brief Print selection box?

	} settings;

	struct
	{
		unsigned int		  width;				///< @brief Max line width.
		unsigned int		  height;				///< @brief Number of lines to print.
		GList 				* dynamic;
		const GList 		* selection;
	} contents;

    struct
    {
    	gchar				* name;
		v3270FontInfo		  info;
    } font;

 };

 struct _V3270PrintSettingsClass
 {
 	GtkGridClass parent_class;

 };

struct _V3270PrintSettings
 {
 	GtkGrid					  parent;

	GtkWidget				* font;					///< @brief Font selection widget.
	GtkWidget				* color;				///< @brief Color scheme selecting widget.
	GtkWidget 				* selected;				///< @brief Checkbox.

 };


/*--[ Prototypes ]-----------------------------------------------------------------------------------*/

 G_GNUC_INTERNAL void		  V3270PrintOperation_begin_print(GtkPrintOperation *prt, GtkPrintContext *context);
 G_GNUC_INTERNAL void		  V3270PrintOperation_draw_page(GtkPrintOperation *prt, GtkPrintContext *context, gint page);



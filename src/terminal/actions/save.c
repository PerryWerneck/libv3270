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
 * Este programa está nomeado como properties.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"
 #include <v3270.h>
 #include <terminal.h>
 #include <v3270/dialogs.h>
 #include <lib3270/trace.h>
 #include <lib3270/log.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 int fire_save_action(GtkWidget *widget, const V3270_ACTION *action) {

	LIB3270_CONTENT_OPTION mode = (LIB3270_CONTENT_OPTION) action->flags;

	debug("%s(%d)",__FUNCTION__,((int) action->flags));

	if(((int) action->flags) < 0)
		mode = lib3270_get_has_selection(GTK_V3270(widget)->host) ? LIB3270_CONTENT_SELECTED : LIB3270_CONTENT_ALL;

	GtkWidget *dialog = v3270_save_dialog_new(widget,mode,NULL);
	gtk_widget_show_all(dialog);
	v3270_save_dialog_run(dialog);
	gtk_widget_destroy(dialog);

	return 0;
 }



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
 #include <lib3270/trace.h>
 #include <lib3270/log.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 int fire_zoom_action(GtkWidget *widget, const V3270_ACTION *action) {

 	debug("%s",__FUNCTION__);

	switch( ((int) action->flags) )
	{
	case 0:	// Zoom in
		v3270_zoom_in(widget);
		break;

	case 1:	// Zoom out
		v3270_zoom_out(widget);
		break;

	case 2: // Zoom fit best
		v3270_zoom_best(widget);
		break;

	default:
		g_warning("Unexpected zoom flags %u",(unsigned int) action->flags);
	}

	return 0;
 }



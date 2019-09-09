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

 #include "private.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 LIB3270_EXPORT void v3270_set_auto_disconnect(GtkWidget *widget, guint minutes)
 {
	g_return_if_fail(GTK_IS_V3270(widget));
 	GTK_V3270(widget)->activity.disconnect = minutes;
 }

 LIB3270_EXPORT guint v3270_get_auto_disconnect(GtkWidget *widget)
 {
	g_return_val_if_fail(GTK_IS_V3270(widget),0);
 	return GTK_V3270(widget)->activity.disconnect;
 }

LIB3270_EXPORT gboolean v3270_get_toggle(GtkWidget *widget, LIB3270_TOGGLE ix)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);

	if(ix < LIB3270_TOGGLE_COUNT)
		return lib3270_get_toggle(GTK_V3270(widget)->host,ix) ? TRUE : FALSE;

	return FALSE;
}

LIB3270_EXPORT gboolean	v3270_set_toggle(GtkWidget *widget, LIB3270_TOGGLE ix, gboolean state)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);

	if(ix < LIB3270_TOGGLE_COUNT)
		return lib3270_set_toggle(GTK_V3270(widget)->host,ix,state ? 1 : 0) ? TRUE : FALSE;

	return FALSE;

}


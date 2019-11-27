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
 #include <errno.h>
 #include <internals.h>
 #include <terminal.h>
 #include <lib3270/actions.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static gint compare_func(const V3270Accelerator *a, const V3270Accelerator *b)
 {
 	if(a->activate != b->activate)
		return a->activate - b->activate;
	return a->arg - b->arg;
 }

 void v3270_accelerator_map_reset(GtkWidget *widget)
 {
 	v3270 * terminal = GTK_V3270(widget);

 	if(terminal)
	{
		if(terminal->accelerators)
		{
			g_slist_free_full(terminal->accelerators,g_free);
			terminal->accelerators = NULL;
		}
		v3270_init_accelerators(terminal);
	}

 }

 void v3270_accelerator_map_sort(v3270 *widget)
 {
	widget->accelerators = g_slist_sort(widget->accelerators, (GCompareFunc) compare_func);
 }

 gboolean v3270_accelerator_compare(const V3270Accelerator * accell, const guint keyval, const GdkModifierType mods)
 {
 	return accell->key == keyval && accell->mods == mods;
 }

 void v3270_accelerator_activate(V3270Accelerator * accell, GtkWidget *terminal)
 {
 	int rc = ((int (*)(GtkWidget *, const void *)) accell->activate)(terminal,accell->arg);

 	if(rc)
		gdk_display_beep(gdk_display_get_default());

 }



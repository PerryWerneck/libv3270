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

 #include <clipboard.h>
 #include <ctype.h>
 #include <lib3270/selection.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 GList * g_list_append_lib3270_selection(GList *list, H3270 *hSession, gboolean all)
 {
	lib3270_selection * selection = lib3270_selection_new(hSession,0,all);

	if(selection)
	{
		size_t sz = sizeof(lib3270_selection) + (sizeof(lib3270_selection_element) * ((selection->bounds.width * selection->bounds.height)+1));

		debug(
			"width=%u height=%u length=%u (sz=%u, szHeader=%u, szElement=%u)",
				selection->bounds.width,
				selection->bounds.height,
				(selection->bounds.width * selection->bounds.height),
				sz,
				sizeof(lib3270_selection),
				sizeof(lib3270_selection_element)
		);

		gpointer data = g_malloc0(sz);
		memcpy(data,selection,sz);

		lib3270_free(selection);

		return g_list_append(list,data);
	}

	g_warning("Can't get TN3270 active selection");

	return NULL;

 }

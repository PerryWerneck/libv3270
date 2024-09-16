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
 * Este programa está nomeado como tools.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #pragma once

 #include <gtk/gtk.h>
 #include <lib3270.h>

 G_BEGIN_DECLS

 /// @brief Bind pointer to widget; release it when widget is destroyed.
 LIB3270_EXPORT void gtk_widget_bind_ptr(GtkWidget *widget, gpointer ptr);

 /// @brief Add style to widget.
 LIB3270_EXPORT void gtk_widget_add_class(GtkWidget *widget, const char *className);

 /// @brief Bind file chooser with entry field.
 LIB3270_EXPORT void gtk_entry_bind_to_filechooser(GtkWidget *widget, GtkFileChooserAction action, const gchar *title, const gchar *icon_name, const gchar *pattern, const gchar *name);

 /// @brief Bind dialog with toplevel
 LIB3270_EXPORT void gtk_dialog_set_toplevel(GtkWidget *dialog, GtkWidget *widget);

 G_END_DECLS



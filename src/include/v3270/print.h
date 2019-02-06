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
 * Este programa está nomeado como hostdialog.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifndef V3270_PRINT_OPERATION_H_INCLUDED

 #include <gtk/gtk.h>
 #include <lib3270.h>

 #define V3270_PRINT_OPERATION_H_INCLUDED

 G_BEGIN_DECLS

/*--[ Print Operation ]------------------------------------------------------------------------------*/


 #define GTK_TYPE_V3270_PRINT_OPERATION				(V3270PrintOperation_get_type())
 #define GTK_V3270_PRINT_OPERATION(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270_PRINT_OPERATION, V3270PrintOperation))
 #define GTK_V3270_PRINT_OPERATION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270_PRINT_OPERATION, V3270PrintOperationClass))
 #define GTK_IS_V3270_PRINT_OPERATION(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270_PRINT_OPERATION))
 #define GTK_IS_V3270_PRINT_OPERATION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270_PRINT_OPERATION))
 #define GTK_V3270_PRINT_OPERATION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270_PRINT_OPERATION, V3270V3270PrintOperationClass))

 typedef struct _V3270PrintOperation			V3270PrintOperation;
 typedef struct _V3270PrintOperationClass		V3270PrintOperationClass;

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

 LIB3270_EXPORT V3270PrintOperation	* v3270_print_operation_new(GtkWidget *widget, LIB3270_PRINT_MODE mode);
 LIB3270_EXPORT void V3270PrintOperation_set_text_by_mode(V3270PrintOperation * operation, LIB3270_PRINT_MODE mode);

 LIB3270_EXPORT GtkTreeModel	* v3270_font_family_model_new(GtkWidget *widget, const gchar *selected, GtkTreeIter * active);
 LIB3270_EXPORT GtkWidget		* v3270_font_selection_new(const gchar *fontname);


 LIB3270_EXPORT GType				  V3270PrintOperation_get_type(void);

 G_END_DECLS

#endif // V3270_PRINT_OPERATION_H_INCLUDED


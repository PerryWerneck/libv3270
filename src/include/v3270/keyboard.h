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

#ifndef V3270_KEYBOARD_H_INCLUDED

    #define V3270_KEYBOARD_H_INCLUDED 1

    #include <gtk/gtk.h>

    G_BEGIN_DECLS

    #define GTK_TYPE_V3270FT			(V3270Keyboard_get_type ())
    #define GTK_V3270FT(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270Keyboard, V3270Keyboard))
    #define GTK_V3270FT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270Keyboard, V3270KeyboardClass))
    #define GTK_IS_V3270FT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270Keyboard))
    #define GTK_IS_V3270FT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270Keyboard))
    #define GTK_V3270FT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270Keyboard, V3270KeyboardClass))

    typedef struct _V3270Keyboard
    {
        GtkAccelKey parent;
    } V3270Keyboard;

    typedef struct _V3270KeyboardClass
    {
        GtkAccelKeyClass parent_class;
    } V3270KeyboardClass;


 G_END_DECLS

#endif // V3270_DIALOGS_H_INCLUDED

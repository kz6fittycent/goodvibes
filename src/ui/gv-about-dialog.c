/*
 * Goodvibes Radio Player
 *
 * Copyright (C) 2015-2017 Arnaud Rebillout
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>

static const gchar *authors[] = {
	PACKAGE_AUTHOR_NAME " " PACKAGE_AUTHOR_EMAIL,
	NULL
};

static const gchar *artists[] = {
	"Lahminèwski Lab http://lahminewski-lab.net",
	NULL
};

static const gchar *translators =
	"Weblate https://hosted.weblate.org/projects/goodvibes/translations\n" \
	"Michal Čihař <michal@cihar.com> (cs)\n" \
	"Andreas Kleinert <Andy.Kleinert@gmail.com> (de)\n" \
	"Étienne Deparis <etienne@depar.is> (fr)\n" \
	"Allan Nordhøy <epost@anotheragency.no> (nb_NO)\n" \
	"Heimen Stoffels <vistausss@outlook.com> (nl)\n" \
	"Алексей Выскубов <viskubov@gmail.com> (ru)";

void
gv_show_about_dialog(GtkWindow *parent, const gchar *audio_backend_string,
                     const gchar *ui_toolkit_string)
{
	// WISHED "license-type" shouldn't be hardcoded

	gchar *comments;

	comments = g_strdup_printf("Audio Backend: %s\n"
	                           "GUI Toolkit: %s",
	                           audio_backend_string,
	                           ui_toolkit_string);

	gtk_show_about_dialog(parent,
	                      "artists", artists,
	                      "authors", authors,
	                      "comments", comments,
	                      "copyright", PACKAGE_COPYRIGHT " " PACKAGE_AUTHOR_NAME,
	                      "license-type", GTK_LICENSE_GPL_3_0,
	                      "logo-icon-name", PACKAGE_NAME,
			      "translator-credits", translators,
	                      "version", PACKAGE_VERSION,
	                      "website", PACKAGE_HOMEPAGE,
	                      NULL);

	g_free(comments);
}

// vim: set tabstop=4 shiftwidth=4 noexpandtab:
/*
Gwenview: an image viewer
Copyright 2008 Aurélien Gâteau <aurelien.gateau@free.fr>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
#ifndef FAKESEMANTICINFOBACKEND_H
#define FAKESEMANTICINFOBACKEND_H

#include <lib/gwenviewlib_export.h>

// Qt
#include <QHash>

// KDE
#include <kurl.h>

// Local
#include "abstractsemanticinfobackend.h"

namespace Gwenview {


/**
 * A fake metadata backend, useful to test the ui layer.
 * It provides fake rating values based on the image url.
 */
class GWENVIEWLIB_EXPORT FakeSemanticInfoBackEnd : public AbstractSemanticInfoBackEnd {
	Q_OBJECT
public:
	enum InitializeMode { InitializeEmpty, InitializeRandom };
	FakeSemanticInfoBackEnd(QObject* parent, InitializeMode initializeMode);

	virtual TagSet allTags() const;

	virtual void storeSemanticInfo(const KUrl&, const SemanticInfo&);

	virtual void retrieveSemanticInfo(const KUrl&);

	virtual QString labelForTag(const SemanticInfoTag&) const;

	virtual SemanticInfoTag tagForLabel(const QString&) const;

private:
	void mergeTagsWithAllTags(const TagSet&);

	QHash<KUrl, SemanticInfo> mSemanticInfoForUrl;
	InitializeMode mInitializeMode;
	TagSet mAllTags;
};


} // namespace

#endif /* FAKESEMANTICINFOBACKEND_H */

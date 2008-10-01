// vim: set tabstop=4 shiftwidth=4 noexpandtab:
/*
Gwenview: an image viewer
Copyright 2007 Aurélien Gâteau <aurelien.gateau@free.fr>

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
// Self
#include "configdialog.h"

// Qt

// KDE

// Local
#include "ui_generalconfigpage.h"
#include "ui_imageviewconfigpage.h"
#include "ui_performanceconfigpage.h"
#include <lib/gwenviewconfig.h>
#include <lib/invisiblebuttongroup.h>
#include <lib/scrolltool.h>

namespace Gwenview {


struct ConfigDialogPrivate {
	InvisibleButtonGroup* mAlphaBackgroundModeGroup;
	InvisibleButtonGroup* mWheelBehaviorGroup;
	Ui_GeneralConfigPage mGeneralConfigPage;
	Ui_ImageViewConfigPage mImageViewConfigPage;
	Ui_PerformanceConfigPage mPerformanceConfigPage;
};

template <class Ui>
QWidget* setupPage(Ui& ui) {
	QWidget* widget = new QWidget;
	ui.setupUi(widget);
	widget->layout()->setMargin(0);
	return widget;
}

ConfigDialog::ConfigDialog(QWidget* parent)
: KConfigDialog(parent, "Settings", GwenviewConfig::self())
, d(new ConfigDialogPrivate) {
	setFaceType(KPageDialog::List);
	setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply | KDialog::Default);
	showButtonSeparator(true);

	QWidget* widget;
	KPageWidgetItem* pageItem;

	// General
	widget = setupPage(d->mGeneralConfigPage);
	pageItem = addPage(widget, i18n("General"));
	pageItem->setIcon(KIcon("gwenview"));

	// Image View
	widget = setupPage(d->mImageViewConfigPage);

	d->mAlphaBackgroundModeGroup = new InvisibleButtonGroup(widget);
	d->mAlphaBackgroundModeGroup->setObjectName("kcfg_AlphaBackgroundMode");
	d->mAlphaBackgroundModeGroup->addButton(d->mImageViewConfigPage.checkBoardRadioButton, int(ImageView::AlphaBackgroundCheckBoard));
	d->mAlphaBackgroundModeGroup->addButton(d->mImageViewConfigPage.solidColorRadioButton, int(ImageView::AlphaBackgroundSolid));

	d->mWheelBehaviorGroup = new InvisibleButtonGroup(widget);
	d->mWheelBehaviorGroup->setObjectName("kcfg_MouseWheelBehavior");
	d->mWheelBehaviorGroup->addButton(d->mImageViewConfigPage.mouseWheelScrollRadioButton, int(ScrollTool::MouseWheelScroll));
	d->mWheelBehaviorGroup->addButton(d->mImageViewConfigPage.mouseWheelBrowseRadioButton, int(ScrollTool::MouseWheelBrowse));
	pageItem = addPage(widget, i18n("Image View"));
	pageItem->setIcon(KIcon("view-preview"));

	// Performance
	widget = setupPage(d->mPerformanceConfigPage);
	pageItem = addPage(widget, i18n("Performance"));
	pageItem->setIcon(KIcon("preferences-system-performance"));
}


ConfigDialog::~ConfigDialog() {
	delete d;
}


} // namespace

/*
Gwenview - A simple image viewer for KDE
Copyright (C) 2000-2002 Aur�lien G�teau

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// Qt includes
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qmap.h>
#include <qspinbox.h>
#include <qstylesheet.h>

// KDE includes
#include <kcolorbutton.h>
#include <kdirsize.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kurlrequester.h>

// Our includes
#include "fileoperation.h"
#include "gvfilethumbnailview.h"
#include "gvfileviewstack.h"
#include "gvpixmapviewstack.h"
#include "gvslideshow.h"
#include "mainwindow.h"
#include "thumbnailloadjob.h"

#include "configdialog.moc"


ConfigDialog::ConfigDialog(QWidget* parent,MainWindow* mainWindow)
: ConfigDialogBase(parent,0L,true),
mMainWindow(mainWindow)
{
	GVFileViewStack* fileViewStack=mMainWindow->fileViewStack();
	GVPixmapViewStack* pixmapViewStack=mMainWindow->pixmapViewStack();
	GVSlideShow* slideShow=mMainWindow->slideShow();

// Thumbnails tab
	mThumbnailMargin->setValue(fileViewStack->fileThumbnailView()->marginSize());
	mWordWrapFilename->setChecked(fileViewStack->fileThumbnailView()->wordWrapIconText());

	connect(mCalculateCacheSize,SIGNAL(clicked()),
		this,SLOT(calculateCacheSize()));
	connect(mEmptyCache,SIGNAL(clicked()),
		this,SLOT(emptyCache()));

// File operations tab
	mConfirmBeforeDelete->setChecked(FileOperation::confirmDelete());
	mShowCopyDialog->setChecked(FileOperation::confirmCopy());
	mShowMoveDialog->setChecked(FileOperation::confirmMove());

	mDefaultDestDir->setURL(FileOperation::destDir());
	mDefaultDestDir->fileDialog()->setMode(
		static_cast<KFile::Mode>(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly));

// Full screen tab
	mShowPathInFullScreen->setChecked(mMainWindow->pixmapViewStack()->showPathInFullScreen());
	mShowMenuBarInFullScreen->setChecked(mMainWindow->showMenuBarInFullScreen());
	mShowToolBarInFullScreen->setChecked(mMainWindow->showToolBarInFullScreen());
	mShowStatusBarInFullScreen->setChecked(mMainWindow->showStatusBarInFullScreen());

// Mouse wheel behaviour tab
	typedef QMap<GVPixmapViewStack::WheelBehaviour,QString> WheelBehaviours;
	WheelBehaviours behaviours;
	behaviours[GVPixmapViewStack::None]=i18n("None");
	behaviours[GVPixmapViewStack::Scroll]=i18n("Scroll");
	behaviours[GVPixmapViewStack::Browse]=i18n("Browse");
	behaviours[GVPixmapViewStack::Zoom]=i18n("Zoom");

	WheelBehaviours::Iterator it;
	for( it = behaviours.begin(); it!=behaviours.end(); ++it ) {
		int index=int(it.key());
		mWheelOnly->insertItem(*it,index);
		mControlPlusWheel->insertItem(*it,index);
		mShiftPlusWheel->insertItem(*it,index);
		mAltPlusWheel->insertItem(*it,index);
	}
	mWheelOnly->setCurrentItem(int(pixmapViewStack->wheelBehaviours()[NoButton]));
	mControlPlusWheel->setCurrentItem(int(pixmapViewStack->wheelBehaviours()[ControlButton]));
	mShiftPlusWheel->setCurrentItem(int(pixmapViewStack->wheelBehaviours()[ShiftButton]));
	mAltPlusWheel->setCurrentItem(int(pixmapViewStack->wheelBehaviours()[AltButton]));

// Slide show tab
	mLoopInSlideShow->setChecked(slideShow->loop());
	mDelayInSlideShow->setValue(slideShow->delay());
	
// Misc tab
	mExternalEditor->setText(FileOperation::editor());
	mAutoLoadImage->setChecked(fileViewStack->autoLoadImage());
	mShowDirs->setChecked(fileViewStack->showDirs());
	mShownColor->setColor(fileViewStack->shownColor());
}


void ConfigDialog::slotOk() {
	slotApply();
	accept();
}


void ConfigDialog::slotApply() {
	GVFileViewStack* fileViewStack=mMainWindow->fileViewStack();
	GVPixmapViewStack* pixmapViewStack=mMainWindow->pixmapViewStack();
	GVSlideShow* slideShow=mMainWindow->slideShow();

// Thumbnails tab
	fileViewStack->fileThumbnailView()->setMarginSize(mThumbnailMargin->value());
	fileViewStack->fileThumbnailView()->setWordWrapIconText(mWordWrapFilename->isChecked());
	fileViewStack->fileThumbnailView()->arrangeItemsInGrid();

// File operations tab
	FileOperation::setConfirmDelete(mConfirmBeforeDelete->isChecked());
	FileOperation::setConfirmCopy(mShowCopyDialog->isChecked());
	FileOperation::setConfirmMove(mShowMoveDialog->isChecked());
	FileOperation::setDestDir(mDefaultDestDir->url());

// Full screen tab
	mMainWindow->pixmapViewStack()->setShowPathInFullScreen( mShowPathInFullScreen->isChecked() );
	mMainWindow->setShowMenuBarInFullScreen( mShowMenuBarInFullScreen->isChecked() );
	mMainWindow->setShowToolBarInFullScreen( mShowToolBarInFullScreen->isChecked() );
	mMainWindow->setShowStatusBarInFullScreen( mShowStatusBarInFullScreen->isChecked() );

// Mouse wheel behaviour tab		
	pixmapViewStack->wheelBehaviours()[NoButton]=     GVPixmapViewStack::WheelBehaviour(mWheelOnly->currentItem());
	pixmapViewStack->wheelBehaviours()[ControlButton]=GVPixmapViewStack::WheelBehaviour(mControlPlusWheel->currentItem());
	pixmapViewStack->wheelBehaviours()[ShiftButton]=  GVPixmapViewStack::WheelBehaviour(mShiftPlusWheel->currentItem());
	pixmapViewStack->wheelBehaviours()[AltButton]=    GVPixmapViewStack::WheelBehaviour(mAltPlusWheel->currentItem());

// Slide show tab
	slideShow->setDelay(mDelayInSlideShow->value());
	slideShow->setLoop(mLoopInSlideShow->isChecked());

// Misc tab
	FileOperation::setEditor(mExternalEditor->text());
	fileViewStack->setAutoLoadImage(mAutoLoadImage->isChecked());
	fileViewStack->setShowDirs(mShowDirs->isChecked());
	fileViewStack->setShownColor(mShownColor->color());
}


void ConfigDialog::calculateCacheSize() {
	KURL url;
	url.setPath(ThumbnailLoadJob::thumbnailDir());
	unsigned long size=KDirSize::dirSize(url);
	KMessageBox::information( this,i18n("Cache size is %1").arg(KIO::convertSize(size)) );
}


void ConfigDialog::emptyCache() {
	QString dir=ThumbnailLoadJob::thumbnailDir();

	if (!QFile::exists(dir)) {
		KMessageBox::information( this,i18n("Cache is already empty.") );
		return;
	}

	int response=KMessageBox::questionYesNo(this,
		"<qt>" + i18n("Are you sure you want to empty the thumbnail cache?"
		" This will remove the folder <b>%1</b>.").arg(QStyleSheet::escape(dir)) + "</qt>");

	if (response==KMessageBox::No) return;

	KURL url;
	url.setPath(dir);
	if (KIO::NetAccess::del(url)) {
		KMessageBox::information( this,i18n("Cache emptied.") );
	}
}


void ConfigDialog::onCacheEmptied(KIO::Job* job) {
	if ( job->error() ) {
		job->showErrorDialog(this);
		return;
	}
	KMessageBox::information( this,i18n("Cache emptied.") );
}

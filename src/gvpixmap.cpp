// vim: set tabstop=4 shiftwidth=4 noexpandtab
/*
Gwenview - A simple image viewer for KDE
Copyright 2000-2003 Aur�lien G�teau

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

#include <sys/stat.h> // For S_ISDIR

// Qt includes
#include <qfileinfo.h>
#include <qpainter.h>
#include <qwmatrix.h>

// KDE includes
#include <kdebug.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kprinter.h>

// Our includes
#include <gvarchive.h>
#include <gvimagesavedialog.h>
#include <gvjpegtran.h>

#include <gvpixmap.moc>


const char* CONFIG_SAVE_AUTOMATICALLY="save automatically";
const char* CONFIG_NOTIFICATION_MESSAGES_GROUP="Notification Messages";

GVPixmap::GVPixmap(QObject* parent)
: QObject(parent)
, mModified(false)
{}


GVPixmap::~GVPixmap() {
}


void GVPixmap::setURL(const KURL& paramURL) {
	//kdDebug() << "GVPixmap::setURL " << paramURL.prettyURL() << endl;
	KURL URL(paramURL);
	if (URL.cmp(url())) return;
	
	// Ask to save if necessary.
	if (!saveIfModified()) {
		// Not saved, notify others that we stay on the image
		emit loaded(mDirURL,mFilename);
		return;
	}

	if (URL.isEmpty()) {
		reset();
		return;
	}

	// Fix wrong protocol
	if (GVArchive::protocolIsArchive(URL.protocol())) {
		QFileInfo info(URL.path());
		if (info.exists()) {
			URL.setProtocol("file");
		}
	}

	// Check whether this is a dir or a file
	KIO::UDSEntry entry;
	bool isDir=false;
	if (!KIO::NetAccess::stat(URL,entry)) return;
	KIO::UDSEntry::ConstIterator it;
	for(it=entry.begin();it!=entry.end();++it) {
		if ((*it).m_uds==KIO::UDS_FILE_TYPE) {
			isDir=S_ISDIR( (*it).m_long );
			break;
		}
	}

	if (isDir) {
		mDirURL=URL;
		mFilename="";
	} else {
		mDirURL=URL.upURL();
		mFilename=URL.filename();
	}

	if (mFilename.isEmpty()) {
		reset();
		return;
	}

	load();
}


void GVPixmap::setDirURL(const KURL& paramURL) {
	if (!saveIfModified()) {
		emit loaded(mDirURL,mFilename);
		return;
	}
	mDirURL=paramURL;
	mFilename="";
	reset();
}


void GVPixmap::setFilename(const QString& filename) {
	if (mFilename==filename) return;

	if (!saveIfModified()) {
		emit loaded(mDirURL,mFilename);
		return;
	}
	mFilename=filename;
	load();
}


void GVPixmap::reload() {
	load();
	emit reloaded(url());
}


void GVPixmap::print(KPrinter *pPrinter) {
  QPainter printpainter;

  // Is this needed?
  KURL url = this->url();
  QString path=url.path();
  pPrinter->setDocName(mFilename);

  printpainter.begin(pPrinter);
  
  printpainter.drawImage(0,0,mImage);
  
  printpainter.end();

}

KURL GVPixmap::url() const {
	KURL url=mDirURL;
	url.addPath(mFilename);
	return url;
}


void GVPixmap::rotateLeft() {
	// Apply the rotation to the compressed data too if available
	if (mImageFormat=="JPEG" && !mCompressedData.isNull()) {
		mCompressedData=GVJPEGTran::apply(mCompressedData,GVImageUtils::Rot270);
	}
	QWMatrix matrix;
	matrix.rotate(-90);
	mImage=mImage.xForm(matrix);
	mModified=true;
	emit modified();
}


void GVPixmap::rotateRight() {
	if (mImageFormat=="JPEG" && !mCompressedData.isNull()) {
		mCompressedData=GVJPEGTran::apply(mCompressedData,GVImageUtils::Rot90);
	}
	QWMatrix matrix;
	matrix.rotate(90);
	mImage=mImage.xForm(matrix);
	mModified=true;
	emit modified();
}


void GVPixmap::mirror() {
	if (mImageFormat=="JPEG" && !mCompressedData.isNull()) {
		mCompressedData=GVJPEGTran::apply(mCompressedData,GVImageUtils::HFlip);
	}
	QWMatrix matrix;
	matrix.scale(-1,1);
	mImage=mImage.xForm(matrix);
	mModified=true;
	emit modified();
}


void GVPixmap::flip() {
	if (mImageFormat=="JPEG" && !mCompressedData.isNull()) {
		mCompressedData=GVJPEGTran::apply(mCompressedData,GVImageUtils::VFlip);
	}
	QWMatrix matrix;
	matrix.scale(1,-1);
	mImage=mImage.xForm(matrix);
	mModified=true;
	emit modified();
}


bool GVPixmap::save() {
	if (!saveInternal(url(),mImageFormat)) {
		KMessageBox::sorry(0,i18n("Could not save file."));
		return false;
	}
	return true;
}


void GVPixmap::saveAs() {
	QString format=mImageFormat;
	KURL saveURL;
	if (url().isLocalFile()) saveURL=url();

	GVImageSaveDialog dialog(saveURL,format,0);
	if (!dialog.exec()) return;

	if (!saveInternal(saveURL,format)) {
		KMessageBox::sorry(0,i18n("Could not save file."));
	}
}

bool GVPixmap::saveIfModified() {
	if (!mModified) return true;
	QString msg=i18n("<qt>The image <b>%1</b> has been modified, do you want to save the changes?</qt>")
				.arg(url().prettyURL());

	int result=KMessageBox::questionYesNoCancel(0, msg, QString::null,
		i18n("Save"), i18n("Discard"), CONFIG_SAVE_AUTOMATICALLY);

	switch (result) {
	case KMessageBox::Yes:
		return save();
	case KMessageBox::No:
		mModified=false;
		return true;
	default:
		return false;
	}
}


//---------------------------------------------------------------------
//
// Config
//
//---------------------------------------------------------------------
static GVPixmap::ModifiedBehavior stringToModifiedBehavior(const QString& str) {
	if (str=="yes") return GVPixmap::SaveSilently;
	if (str=="no") return GVPixmap::DiscardChanges;
	return GVPixmap::Ask;
}

static QString modifiedBehaviorToString(GVPixmap::ModifiedBehavior behaviour) {
	if (behaviour==GVPixmap::SaveSilently) return "yes";
	if (behaviour==GVPixmap::DiscardChanges) return "no";
	return "";
}


void GVPixmap::setModifiedBehavior(ModifiedBehavior value) {
	KConfig* config=KGlobal::config();
	KConfigGroupSaver saver(config, CONFIG_NOTIFICATION_MESSAGES_GROUP);
	config->setGroup(CONFIG_NOTIFICATION_MESSAGES_GROUP);
	config->writeEntry(CONFIG_SAVE_AUTOMATICALLY,
		modifiedBehaviorToString(value));
}


GVPixmap::ModifiedBehavior GVPixmap::modifiedBehavior() const {
	KConfig* config=KGlobal::config();
	KConfigGroupSaver saver(config, CONFIG_NOTIFICATION_MESSAGES_GROUP);
	QString str=config->readEntry(CONFIG_SAVE_AUTOMATICALLY);
	return stringToModifiedBehavior(str.lower());
}


//---------------------------------------------------------------------
//
// Private stuff
//
//---------------------------------------------------------------------
void GVPixmap::load() {
	emit loading();
	KURL pixURL=url();
	Q_ASSERT(!pixURL.isEmpty());
	//kdDebug() << "GVPixmap::load() " << pixURL.prettyURL() << endl;

	QString path;
	if (!KIO::NetAccess::download(pixURL,path)) {
		mImage.reset();
		emit loaded(mDirURL,"");
		return;
	}

	// Load file. We load it ourself so that we can keep a copy of the
	// compressed data. This is used by JPEG lossless manipulations.
	QFile file(path);
	file.open(IO_ReadOnly);
	QDataStream stream(&file);
	mCompressedData.resize(file.size());
	stream.readRawBytes(mCompressedData.data(),mCompressedData.size());

	// Decode image
	mImageFormat=QString(QImage::imageFormat(path));
	if (mImage.loadFromData(mCompressedData,mImageFormat.ascii())) {
		// Convert depth if necessary
		// (32 bit depth is necessary for alpha-blending)
		if (mImage.depth()<32 && mImage.hasAlphaBuffer()) {
			mImage=mImage.convertDepth(32);
		}

		// If the image is a JPEG, rotate the decode image and the compressed
		// data if necessary, otherwise throw the compressed data away
		if (mImageFormat=="JPEG") {
			GVImageUtils::Orientation orientation=GVImageUtils::getOrientation(mCompressedData);
			
			if (orientation!=GVImageUtils::NotAvailable && orientation!=GVImageUtils::Normal) {
				mImage=GVImageUtils::rotate(mImage, orientation);
				mCompressedData=GVJPEGTran::apply(mCompressedData,orientation);
				mCompressedData=GVImageUtils::setOrientation(mCompressedData,GVImageUtils::Normal);
			}
		} else {
			mCompressedData.resize(0);
		}

	} else {
		mImage.reset();
	}

	KIO::NetAccess::removeTempFile(path);
	emit loaded(mDirURL,mFilename);
}


bool GVPixmap::saveInternal(const KURL& url, const QString& format) {
	bool result;

	KTempFile tmp;
	tmp.setAutoDelete(true);
	QString path;
	if (url.isLocalFile()) {
		path=url.path();
	} else {
		path=tmp.name();
	}
	
	if (format=="JPEG" && !mCompressedData.isNull()) {
		//kdDebug() << "Lossless save\n";
		QFile file(path);
		result=file.open(IO_WriteOnly);
		if (!result) return false;
		QDataStream stream(&file);
		stream.writeRawBytes(mCompressedData.data(),mCompressedData.size());
	} else {
		result=mImage.save(path,format.ascii());
		if (!result) return false;
	}

	if (!url.isLocalFile()) {
		result=KIO::NetAccess::upload(tmp.name(),url);
	}
	
	if (result) {
		emit saved(url);
		mModified=false;
	}

	return result;
}


void GVPixmap::reset() {
	mImage.reset();
	mCompressedData.resize(0);
	emit loaded(mDirURL,mFilename);
}



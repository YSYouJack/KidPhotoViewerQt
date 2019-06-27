#include "KidPhotoViewer.h"

#include <cassert>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QImageReader>
#include <QMessageBox>
#include <QGuiApplication>
#include <QSet>
#include <QFile>
#include <QByteArray>
#include <QDateEdit>
#include <QDebug>

#include "exif.h"

static QFileInfoList removeOverlapFiles(const QFileInfoList &src, const QFileInfoList &dst)
{
	QSet<QString> dstFiles;
	for (int i = 0; i < dst.size(); ++i)
		dstFiles.insert(dst.at(i).fileName());

	QFileInfoList list;
	for (int i = 0; i < src.size(); ++i)
	{
		if (!dstFiles.contains(src.at(i).fileName()))
			list << src.at(i);
	}

	return list;
}

KidPhotoViewer::KidPhotoViewer(QWidget *parent)
	: QMainWindow(parent)
	, m_fileId(0)
{
	m_photoViewer = new QLabel(this);
	m_photoViewer->setBackgroundRole(QPalette::Base);
	m_photoViewer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	m_photoViewer->setScaledContents(true);

	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidget(m_photoViewer);
	m_scrollArea->setVisible(true);
	m_scrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_scrollArea->setMinimumSize(640, 480);

	m_nextButton = new QPushButton(tr("Next Photo"), this);
	m_nextButton->setDisabled(true);
	connect(m_nextButton, SIGNAL(clicked()), this, SLOT(nextPhoto()));

	m_prevButton = new QPushButton(tr("Prev Photo"), this);
	m_prevButton->setDisabled(true);
	connect(m_prevButton, SIGNAL(clicked()), this, SLOT(prevPhoto()));

	m_saveButton = new QPushButton(tr("Save"), this);
	m_saveButton->setDisabled(true);
	connect(m_saveButton, SIGNAL(clicked()), this, SLOT(savePhoto()));

	QPushButton *chooseSrcButton = new QPushButton(tr("Choose Source Directory"), this);
	connect(chooseSrcButton, SIGNAL(clicked()), this, SLOT(chooseSrcDir()));

	QPushButton *chooseDstButton = new QPushButton(tr("Choose Destination Directory"), this);
	connect(chooseDstButton, SIGNAL(clicked()), this, SLOT(chooseDstDir()));

	m_srcDirLabel = new QLabel(tr("-----"), this);
	m_dstDirLabel = new QLabel(tr("-----"), this);
	m_imageCntLabel = new QLabel("0/0", this);

	m_dateEdit = new QDateEdit(QDate::fromString("2017:1:24", "yyyy:M:d"), this);

	QVBoxLayout *buttonLayout = new QVBoxLayout;
	buttonLayout->addWidget(m_nextButton);
	buttonLayout->addWidget(m_prevButton);
	buttonLayout->addWidget(m_saveButton);
	buttonLayout->addWidget(chooseSrcButton);
	buttonLayout->addWidget(chooseDstButton);
	buttonLayout->addWidget(m_srcDirLabel);
	buttonLayout->addWidget(m_dstDirLabel);
	buttonLayout->addWidget(m_imageCntLabel);
	buttonLayout->addWidget(m_dateEdit);
	buttonLayout->addStretch(1);

	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addWidget(m_scrollArea, 1);
	mainLayout->addLayout(buttonLayout);

	QWidget *w = new QWidget(this);
	w->setLayout(mainLayout);

	this->setCentralWidget(w);
}

void KidPhotoViewer::resizeEvent(QResizeEvent * /* event */)
{
	if (m_photoViewer->pixmap())
	{
		QSize areaSize = m_scrollArea->size();
		QSize imgSize = m_photoViewer->pixmap()->size();
		imgSize.scale(areaSize, Qt::KeepAspectRatio);
		m_photoViewer->resize(imgSize);
	}
}

void KidPhotoViewer::prevPhoto()
{
	if (--m_fileId == 0)
		m_prevButton->setEnabled(false);

	m_nextButton->setEnabled(true);

	loadImage(m_fileList.at(m_fileId));
	m_imageCntLabel->setText(QString("%1/%2")
		.arg(m_fileId + 1)
		.arg(m_fileList.size()));
}

void KidPhotoViewer::nextPhoto()
{
	if (++m_fileId == m_fileList.size() - 1)
		m_nextButton->setEnabled(false);
	else
		m_nextButton->setEnabled(true);

	m_prevButton->setEnabled(m_fileId != 0);
	
	qDebug() << m_fileList.at(0).path();

	loadImage(m_fileList.at(m_fileId));
	m_imageCntLabel->setText(QString("%1/%2")
		.arg(m_fileId + 1)
		.arg(m_fileList.size()));
}

void KidPhotoViewer::savePhoto()
{
	QDir dstDir(m_dstDirLabel->text());
	const QFileInfo &info = m_fileList.at(m_fileId);

	QString dstFilePath = dstDir.filePath(info.fileName());
	QFile::copy(info.filePath(), dstFilePath);

	m_dstFileList << QFileInfo(dstFilePath);
}

void KidPhotoViewer::chooseSrcDir()
{
	QDir dir = QFileDialog::getExistingDirectory(this, tr("Choose source directory"));
	m_srcDirLabel->setText(dir.path());
	m_fileList.clear();

	if (!dir.exists())
	{
		QStringList filters;
		filters << "*.jpg" << "*.JPG";
		
		QFileInfoList list = dir.entryInfoList(filters, QDir::Files);
		m_srcFileList.clear();

		for (int i = 0; i < list.size(); ++i)
		{
			QFile file(list.at(i).filePath());
			file.open(QIODevice::ReadOnly);

			QByteArray bytes = file.readAll();

			easyexif::EXIFInfo info;
			info.parseFrom(reinterpret_cast<const unsigned char *>(bytes.constData())
				, static_cast<unsigned int>(bytes.size()));

			QDate takenDate = QDateTime::fromString(info.DateTimeOriginal.c_str()
				, "yyyy:MM:dd hh:mm:ss").date();

			if (takenDate >= m_dateEdit->date())
				m_srcFileList << list.at(i);
		}
	}
	else
	{
		m_srcFileList.clear();
	}

	m_fileId = -1;
	if (!m_srcFileList.isEmpty() && QFileInfo(m_dstDirLabel->text()).isDir())
	{
		m_fileList = removeOverlapFiles(m_srcFileList, m_dstFileList);
		if (!m_fileList.isEmpty())
			nextPhoto();
	}
}

void KidPhotoViewer::chooseDstDir()
{
	QDir dir = QFileDialog::getExistingDirectory(this, tr("Choose destination directory"));
	m_dstDirLabel->setText(dir.path());
	m_fileList.clear();

	if (!dir.exists())
	{
		QStringList filters;
		filters << "*.jpg" << "*.JPG";
		m_dstFileList = dir.entryInfoList(filters, QDir::Files);
	}
	else
	{
		m_dstFileList.clear();
	}

	m_fileId = -1;
	if (!m_srcFileList.isEmpty() && QFileInfo(m_dstDirLabel->text()).isDir())
	{
		m_fileList = removeOverlapFiles(m_srcFileList, m_dstFileList);
		if (!m_fileList.isEmpty())
			nextPhoto();
	}
}

void KidPhotoViewer::loadImage(const QFileInfo &info)
{
	QImageReader reader(info.filePath());
	reader.setAutoTransform(true);

	const QImage newImage = reader.read();
	if (newImage.isNull()) {
		QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
			tr("Cannot load %1: %2")
			.arg(QDir::toNativeSeparators(info.path()), reader.errorString()));
		return;
	}

	m_photoViewer->setPixmap(QPixmap::fromImage(newImage));

	QSize areaSize = m_scrollArea->size();
	QSize imgSize = m_photoViewer->pixmap()->size();
	imgSize.scale(areaSize, Qt::KeepAspectRatio);

	m_photoViewer->resize(imgSize);
	m_saveButton->setEnabled(true);
	setWindowFilePath(info.path());
}

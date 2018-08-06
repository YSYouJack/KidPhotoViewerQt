#pragma once

#include <QtWidgets/QMainWindow>
#include <QFileInfo>
#include <QList>

// Forward declaration
class QLabel;
class QScrollArea;
class QPushButton;
class QDateEdit;

class KidPhotoViewer : public QMainWindow
{
	Q_OBJECT

public:
	KidPhotoViewer(QWidget *parent = Q_NULLPTR);

protected:
	void resizeEvent(QResizeEvent *event);

private slots:
	void prevPhoto();
	void nextPhoto();
	void savePhoto();
	void chooseSrcDir();
	void chooseDstDir();

private:
	QLabel * m_photoViewer;
	QLabel * m_srcDirLabel;
	QLabel * m_dstDirLabel;
	QLabel * m_imageCntLabel;
	QScrollArea *m_scrollArea;
	QPushButton *m_prevButton;
	QPushButton *m_nextButton;
	QPushButton *m_saveButton;
	QDateEdit *m_dateEdit;

	QFileInfoList m_fileList;
	int m_fileId;
	
	QFileInfoList m_srcFileList;
	QFileInfoList m_dstFileList;

	void loadImage(const QFileInfo &info);
};

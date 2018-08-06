#include "KidPhotoViewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	KidPhotoViewer w;
	w.show();
	return a.exec();
}

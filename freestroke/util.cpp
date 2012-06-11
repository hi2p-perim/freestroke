#include "util.h"

Util::Util()
{

}

void Util::ShowStatusMessage( QString mes )
{
	emit StatusMessage(mes);
}

void Util::CreateBrushPathList()
{
	QDir brushDir(QDir::current().absoluteFilePath("brushes"));
	if (brushDir.exists())
	{
		QStringList filters;
		filters << "*.png";
		QFileInfoList infoList = brushDir.entryInfoList(filters, QDir::Files, QDir::Name);
		QListIterator<QFileInfo> it(infoList);
		while (it.hasNext())
		{
			QString path = it.next().absoluteFilePath();
			QImage image(path);
			brushPathList.push_back(path);
		}
	}
	else
	{
		ShowStatusMessage("Path brushes does not exist");
	}
}

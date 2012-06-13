#include "util.h"

Util::Util()
{
	brushSize = 128;
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
		//filters << "*.png";
		QFileInfoList infoList = brushDir.entryInfoList(filters, QDir::Files, QDir::Name);
		QListIterator<QFileInfo> it(infoList);
		while (it.hasNext())
		{
			QString path = it.next().absoluteFilePath();
			QImage image(path);
			if (image.width() != brushSize || image.height() != brushSize)
			{
				ShowStatusMessage("Invalid brush image size: " + path);
				brushPathList.clear();
				return;
			}
			brushPathList.push_back(path);
		}
	}
	else
	{
		ShowStatusMessage("Brush path does not exist");
	}
}

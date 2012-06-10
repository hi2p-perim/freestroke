#include "mainwindow.h"
#include <QtGui/QApplication>
#include <Windows.h>

int main(int argc, char *argv[])
{
	try
	{
		//QApplication::setStyle(new QPlastiqueStyle);
		QApplication a(argc, argv);
		MainWindow w;
		w.show();
		return a.exec();
	}
	catch (const Exception& e)
	{
		std::stringstream ss;

		ss << "[Exception]" << std::endl;
		ss << e.TypeString() << std::endl;
		ss << std::endl;

		ss << "[File]" << std::endl;
		ss << e.FileName() << std::endl;
		ss << std::endl;

		ss << "[Function]" << std::endl;
		ss << e.FuncName() << std::endl;
		ss << std::endl;

		ss << "[Line]" << std::endl;
		ss << e.Line() << std::endl;
		ss << std::endl;

		ss << "[Stack Trace]" << std::endl;
		ss << e.StackTrace();
		ss << std::endl;

		ss << "[Message]" << std::endl;
		ss << e.what() << std::endl;

		MessageBoxA(NULL, ss.str().c_str(), "Error", MB_ICONWARNING | MB_OK | MB_TASKMODAL);
	}
	catch (const std::exception& e)
	{
		MessageBoxA(NULL, e.what(), "Error", MB_ICONWARNING | MB_OK | MB_TASKMODAL);
	}
}

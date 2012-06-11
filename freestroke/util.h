#ifndef __UTIL_H__
#define __UTIL_H__

/*!
	Utility.
	Utility class which can be accessible from everywhere.
*/
class Util : public QObject
{
	Q_OBJECT

public:

	Util();
	DISALLOW_COPY_AND_ASSIGN(Util);

public:

	/*!
		Get instance.
		@return rfRenderer instance.
	*/
	static Util* Get()
	{
		static Util instance;
		return &instance;
	}

public:

	void ShowStatusMessage(QString mes);
	void CreateBrushPathList();
	QString GetBrushPath(int index) { return brushPathList[index]; }
	int GetBrushNum() { return brushPathList.size(); }

signals:

	void StatusMessage(QString mes);

private:

	std::vector<QString> brushPathList;

};

#endif // __UTIL_H__
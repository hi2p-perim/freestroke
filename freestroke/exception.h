#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <exception>

class Exception : public std::exception
{
public:

	enum ErrorType
	{
		FileError,
		InvalidArgument,
		RunTimeError,
		ShaderCompileError,
		ProgramLinkError,
		InvalidOperation,
		OpenGLError
	};

public:

	Exception(ErrorType type, const std::string& message, const char* fileName, const char* funcName,
		const int line, const std::string& stackTrace)
		: type(type)
		, message(message)
		, fileName(fileName)
		, funcName(funcName)
		, line(line)
		, stackTrace(stackTrace)
	{

	}

	ErrorType Type() const { return type; }
	const char* TypeString() const;
	const char* what() const { return message.c_str(); }
	const char* FileName() const { return fileName; }
	const char* FuncName() const { return funcName; }
	const int Line() const { return line; }
	std::string StackTrace() const { return stackTrace; }

private:

	ErrorType type;
	std::string message;
	const char* fileName;
	const char* funcName;
	int line;
	std::string stackTrace;

};

std::string GetStackTrace();

#define THROW_EXCEPTION(type, message) \
	throw Exception(type, message, __FILE__, __FUNCTION__, __LINE__, GetStackTrace())

#endif // __EXCEPTION_H__
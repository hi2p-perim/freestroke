#include "exception.h"
#include <Windows.h>
#include <DbgHelp.h>

std::string GetStackTrace()
{
	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();
	PIMAGEHLP_SYMBOL sym;

	// ------------------------------------------------------------

	const size_t symbol_size = 4096;

	// Allocate buffer for symbols
	sym = (PIMAGEHLP_SYMBOL)GlobalAlloc(GMEM_FIXED, symbol_size);
	sym->SizeOfStruct = symbol_size;
	sym->MaxNameLength = symbol_size - sizeof(IMAGEHLP_SYMBOL);

	// ------------------------------------------------------------

	// Get Context
	CONTEXT context;
	memset(&context, 0, sizeof(CONTEXT));

	__asm call x
	__asm x: pop eax
	__asm mov context.Eip, eax
	__asm mov context.Ebp, ebp
	__asm mov context.Esp, esp

	// ------------------------------------------------------------

	// Get stack frame
	STACKFRAME stackframe;
	memset(&stackframe, 0, sizeof(STACKFRAME));
	stackframe.AddrPC.Offset = context.Eip;
	stackframe.AddrFrame.Offset = context.Ebp;
	stackframe.AddrStack.Offset = context.Esp;
	stackframe.AddrPC.Mode = AddrModeFlat;
	stackframe.AddrFrame.Mode = AddrModeFlat;
	stackframe.AddrStack.Mode = AddrModeFlat;

	// Initialize symbol handler
	SymInitialize(process, NULL, TRUE);
	SymSetOptions(SymGetOptions() | SYMOPT_LOAD_LINES | SYMOPT_FAIL_CRITICAL_ERRORS);

	// ------------------------------------------------------------

	std::stringstream ss;

	// Read stack frame
	BOOL result;
	const int MAX_ENTRY = 20;
	for (int i = 0; ; i++) 
	{
		result = StackWalk(
			IMAGE_FILE_MACHINE_I386,
			process,
			thread,
			&stackframe,
			(void*)&context,
			NULL,
			SymFunctionTableAccess,
			SymGetModuleBase,
			NULL);

		if (!result || stackframe.AddrReturn.Offset == 0) break;

		if (i == MAX_ENTRY)
		{
			ss << "..." << std::endl;
			break;
		}

		// Skip the first entry (the function itself)
		if (i == 0) continue;

		// Get function name
		DWORD lineOffset;
		result = SymGetSymFromAddr(process, stackframe.AddrPC.Offset, &lineOffset, sym);

		if (!result)
		{
			// Failed to get symbol name
			ss << boost::format("[%04d] 0x%08x -----") % i % stackframe.AddrPC.Offset << std::endl;
		}
		else
		{
			ss << boost::format("[%04d] 0x%08x %s + 0x%x") % i % stackframe.AddrPC.Offset % sym->Name % lineOffset << std::endl;
		}
	};

	// ------------------------------------------------------------

	// Cleanup and return
	SymCleanup(process);
	GlobalFree(sym);

	return ss.str();
}

const char* Exception::TypeString() const
{
	switch (type)
	{
	case FileError:
		return "FileError";
		break;
	case InvalidArgument:
		return "InvalidArgument";
		break;
	case RunTimeError:
		return "RunTimeError";
		break;
	case ShaderCompileError:
		return "ShaderCompileError";
		break;
	case ProgramLinkError:
		return "ProgramLinkError";
		break;
	case InvalidOperation:
		return "InvalidOperation";
		break;
	case OpenGLError:
		return "OpenGLError";
		break;
	}
	return "InvalidErrorType";
}

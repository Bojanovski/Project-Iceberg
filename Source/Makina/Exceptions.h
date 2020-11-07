#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <string>

namespace Makina
{
	//Exceptions
	class Exception
	{
	protected:
		std::wstring exText;
		std::wstring exType;

	public:
		Exception(std::wstring exText, std::wstring exType) : exText(exText), exType(exType) {}

		virtual ~Exception() {}

		virtual wchar_t *Message() {return &exText[0];}

		virtual wchar_t *Type() {return &exType[0];}
	};

	class UnexpectedError : public Exception
	{
	public:
		UnexpectedError(std::wstring exText) : Exception(exText, L"UnexpectedError") {}
		~UnexpectedError() {}
	};

	class AllocationError : public Exception
	{
	public:
		AllocationError(std::wstring exText) : Exception(exText, L"AllocationError") {}
		~AllocationError() {}
	};

	class ScriptError : public Exception
	{
	public:
		ScriptError(std::wstring exText) : Exception(exText, L"ScriptError") {}
		~ScriptError() {}
	};

	class IndexOutOfRange : public Exception
	{
	public:
		IndexOutOfRange(std::wstring exText) : Exception(exText, L"IndexOutOfRange") {}
		~IndexOutOfRange() {}
	};

	class FileNotFound : public Exception
	{
	public:
		FileNotFound(std::wstring exText) : Exception(exText, L"FileNotFound") {}
		~FileNotFound() {}
	};

	class InvalidFileType : public Exception
	{
	public:
		InvalidFileType(std::wstring exText) : Exception(exText, L"InvalidFileType") {}
		~InvalidFileType() {}
	};

	class FileCorrupt : public Exception
	{
	public:
		FileCorrupt(std::wstring exText) : Exception(exText, L"FileCorrupt") {}
		~FileCorrupt() {}
	};

	class InvalidOperation : public Exception
	{
	public:
		InvalidOperation(std::wstring exText) : Exception(exText, L"InvalidOperation") {}
		~InvalidOperation() {}
	};

	class NotFound : public Exception
	{
	public:
		NotFound(std::wstring exText) : Exception(exText, L"NotFound") {}
		~NotFound() {}
	};
}

#endif

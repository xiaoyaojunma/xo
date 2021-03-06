#include "pch.h"

static bool AllowGUI = true;

PAPI int AbcPanicMsg(const char* file, int line, const char* msg)
{
	char buf[1024];
	if (msg)	sprintf(buf, "Critical Error: %s(%d)\n%s", file, line, msg);
	else		sprintf(buf, "Critical Error: %s(%d)", file, line);
	buf[arraysize(buf) - 1] = 0;
	bool showGui = AllowGUI;
#if !defined(MessageBox)
	showGui = false;
#endif

	if (showGui)
	{
#if defined(MessageBox)
		MessageBoxA(NULL, buf, "Error", MB_OK | MB_ICONERROR);
#endif
	}
	else
	{
#ifdef ANDROID
		__android_log_write(ANDROID_LOG_ERROR, "AbcPanicMsg", buf);
#endif
		fputs("AbcPanicMsg: ", stdout);
		fputs(buf, stdout);
		fflush(stdout);
	}
	return 0;
}

PAPI NORETURN void AbcDie()
{
#ifdef _MSC_VER
	__debugbreak();
#endif
	exit(1);
}

PAPI void* AbcReallocOrDie(void* p, size_t bytes)
{
	void* buf = realloc(p, bytes);
	AbcCheckAlloc(buf);
	return buf;
}

PAPI void* AbcMallocOrDie(size_t bytes)
{
	void* buf = malloc(bytes);
	AbcCheckAlloc(buf);
	return buf;
}

PAPI bool AbcAllowGUI()
{
	return AllowGUI;
}

PAPI void AbcSetAllowGUI(bool allowGUI)
{
	AllowGUI = allowGUI;
}


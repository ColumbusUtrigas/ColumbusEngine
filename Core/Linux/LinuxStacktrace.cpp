#include <Core/Stacktrace.h>
#include <System/Log.h>

#include <stdio.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>

#define UNW_LOCAL_ONLY

#include <elfutils/libdwfl.h>
#include <libunwind.h>
#include <cassert>
#include <unistd.h>
#include <string.h>

static void GetStackFileName(char* out, size_t max, const void* ip)
{
	char* debuginfo_path = NULL;

	Dwfl_Callbacks callbacks = {
		.find_elf = dwfl_linux_proc_find_elf,
		.find_debuginfo = dwfl_standard_find_debuginfo,
		.debuginfo_path = &debuginfo_path,
	};

	Dwfl* dwfl = dwfl_begin(&callbacks);
	assert(dwfl != NULL);

	assert(dwfl_linux_proc_report(dwfl, getpid()) == 0);
	assert(dwfl_report_end(dwfl, NULL, NULL) == 0);

	Dwarf_Addr addr = (uintptr_t)ip;

	Dwfl_Module* module = dwfl_addrmodule (dwfl, addr);
	Dwfl_Line* line = dwfl_getsrc(dwfl, addr);

	// const char* function_name = dwfl_module_addrname(module, addr);
	// fprintf(out,"%s(",function_name);

	if (line != NULL)
	{
		int nline;
		Dwarf_Addr addr;
		const char* filename = dwfl_lineinfo(line, &addr,&nline,NULL,NULL,NULL);
		snprintf(out, max, "%s:%d", strrchr(filename,'/') + 1, nline);
	}
	else
	{
		const char* module_name = dwfl_module_info(module, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		snprintf(out, max, "%s", module_name);
		// snprintf(out, max, "%p", ip);
	}
}

void WriteStacktraceToLog()
{
	unw_cursor_t cursor;
	unw_context_t context;

	unw_getcontext(&context);
	unw_init_local(&cursor, &context);

	int n = 0;
	while (unw_step(&cursor))
	{
		unw_word_t ip, sp, off;

		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);

		char symbol[256] = {"<unknown>"};
		char* name = symbol;
		char buf[1024];

		if (!unw_get_proc_name(&cursor, symbol, sizeof(symbol), &off))
		{
			int status;
			if ((name = abi::__cxa_demangle(symbol, NULL, NULL, &status)) == 0)
				name = symbol;
		}

		GetStackFileName(buf, 1024, (void*)(ip-4));

		Columbus::Log::_InternalStack("#%i %s at %s", ++n, name, buf);
		// Columbus::Log::Message("#%i 0x%016x sp=0x%016x %s + 0x%x", ++n, ip, sp, name, off);

		if (name != symbol)
			free(name);
	}
}

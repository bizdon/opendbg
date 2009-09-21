/*
    *
    * Copyright (c) 2008 - 2009, OpenDbg Team
	* ntldr <ntldr@freed0m.org> PGP key ID - 0xC48251EB4F8E4E6E
	* d1mk4 <d1mk4nah@gmail.com>
	* Vladimir <progopis@jabber.ru> PGP key ID - 0x59CF2D8A75CB8417
    *

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include <cstdio>
#include "dbgapi.h"
#include "test.h"
#include "breakpoints.h"

// modified version for xp sp3
static uintptr_t
CALLBACK get_symbols_callback(
		int sym_type, char * sym_name, char * sym_subname, pdb::pdb_parser& pdb
		)
{

	if (sym_type == SYM_TIMESTAMP)
	{
		return 0x45E53F9C;
	}

	if (sym_type == SYM_NTAPI_NUM)
	{
		if (strcmp(sym_name, "ZwTerminateProcess") == 0) {
			return 0x101;
		}

		if (strcmp(sym_name, "ZwCreateThread") == 0) {
			return 0x035;
		}

		if (strcmp(sym_name, "ZwTerminateThread") == 0) {
			return 0x102;
		}
	}

	wchar_t sym_name_w[255];
	MultiByteToWideChar(
		CP_ACP, 0, sym_name, strlen(sym_name)+1,
		sym_name_w, sizeof(sym_name_w)/sizeof(sym_name_w[0])
	);

	wchar_t sym_subname_w[255];
	if (sym_subname && strlen(sym_subname)) {
		MultiByteToWideChar(
			CP_ACP, 0, sym_subname, strlen(sym_subname)+1,
			sym_subname_w, sizeof(sym_subname_w)/sizeof(sym_subname_w[0])
		);
	}

	if (sym_type == SYM_OFFSET) {
		return pdb.get_symbol(sym_name_w).get_rva();
	}

	if (sym_type == SYM_STRUCT_OFFSET)
		return pdb.get_type(sym_name_w).get_member(sym_subname_w).get_offset();

	return 0;
}

int
#ifdef _MSC_VER
__cdecl
#endif
main(int argc, char* argv[])
{
	event_filt filter;
	HANDLE     pid;
	dbg_msg    *msg = new dbg_msg;
	u8         flag;
	std::vector<trc::breakpoint> breaks;

	printf("dbgapi test tool started\n");

	try {
		do
		{

			if (dbg_initialize_api(0x1234, L"c:\\ntoskrnl.pdb", (dbg_sym_get)get_symbols_callback) != 1) {
				printf("dbgapi initialization error\n");
				break;
			}

			printf("dbgapi initialized\n");
			printf("dbgapi version as %d\n", dbg_drv_version());

			if ( (pid = dbg_create_process("C:\\Windows\\system32\\notepad.exe", CREATE_NEW_CONSOLE | DEBUG_ONLY_THIS_PROCESS)) == NULL) {
				printf("process not started\n");
				break;
			}

			printf("process started with pid %x\n", pid);

			if (dbg_attach_debugger(pid) == 0) {
				printf("debugger not attached\n");
				break;
			}

			printf("debugger attached\n");

			filter.event_mask  = DBG_EXCEPTION | DBG_TERMINATED | DBG_START_THREAD | DBG_EXIT_THREAD | DBG_LOAD_DLL;
			filter.filtr_count = 0;

			if (dbg_set_filter(pid, &filter) == 0) {
				printf("dbg_set_filter error\n");
				break;
			}

			printf("debug events filter set up\n");

			do
			{
				u32 continue_status = DBG_CONTINUE;
				if (dbg_get_msg_event(pid, msg) == 0) {
					printf("get debug message error\n");
					break;
				}

				if (msg->event_code == DBG_TERMINATED)
				{
					printf("DBG_TERMINATED %x by %x\n",
						msg->terminated.proc_id,
						msg->process_id
						);

					continue_status = DBG_CONTINUE;
					//dbg_continue_event(NULL, pid, RES_NOT_HANDLED, NULL);
				}

				if (msg->event_code == DBG_START_THREAD)
				{
					printf("DBG_START_THREAD %x by %x, teb: %x\n",
						msg->thread_start.thread_id,
						msg->process_id,
						msg->thread_start.teb_addr
						);

					continue_status = DBG_CONTINUE;
					//dbg_continue_event(NULL, pid, RES_NOT_HANDLED, NULL);
				}

				if (msg->event_code == DBG_EXIT_THREAD)
				{
					printf("DBG_EXIT_THREAD %x in %x by %x\n",
						msg->thread_exit.thread_id,
						msg->thread_exit.proc_id,
						msg->process_id
						);

					continue_status = DBG_CONTINUE;
					//dbg_continue_event(NULL, pid, RES_NOT_HANDLED, NULL);
				}

				if (msg->event_code == DBG_EXCEPTION)
				{
					printf("DBG_EXCEPTION %0.8x in %x:%x\n",
						msg->exception.except_record.ExceptionCode,
						msg->thread_id,
						msg->process_id
						);

					switch (msg->exception.except_record.ExceptionCode)
					{
						case EXCEPTION_BREAKPOINT :
						{
							flag = 0; // �� ���� �������� �����
							// ��������� ������� �����
							//if (breaks(NULL, msg->process_id, msg->thread_id, msg->exception.except_record.ExceptionAddress) != NO_BREAK)
							//{
								//
								//�������� ��� ��������� �����
								//
								flag = 1; //��� ��������� ����
							//}
							if ( msg->exception.first_chance | flag )
								continue_status = DBG_CONTINUE ;
							else
								continue_status = DBG_EXCEPTION_NOT_HANDLED ;
						}
						break ;

					default:
						continue_status  = DBG_CONTINUE ;
						break ;
					}
					//dbg_continue_event(NULL, pid, RES_NOT_HANDLED, NULL);
				}

				if (msg->event_code == DBG_LOAD_DLL)
				{
					printf("DBG_LOAD_DLL %ws adr 0x%p sz 0x%x in %x:%x\n",
							msg->dll_load.dll_name,
							msg->dll_load.dll_image_base,
							msg->dll_load.dll_image_size,
							msg->thread_id,
							msg->process_id
							);

					continue_status = DBG_CONTINUE;
					//dbg_continue_event(NULL, pid, RES_NOT_HANDLED, NULL);
				}
				
				if (!ContinueDebugEvent((u32)msg->process_id, (u32)msg->thread_id, continue_status))
					break;
			} while (1);

		} while (0);
	} catch (pdb::pdb_error& e) {
		std::cout << e.what() << "\n";
	}

	//Sleep(INFINITE);

	return 0;
}

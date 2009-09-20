/*
*
* Copyright (c) 2009
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

#include "trc_breakpoints.h"
#include "dbgapi.h"

int breakpoint::set_breakpoint (PVOID remote_id, u32 proc_id, u32 thread_id, u3264 address, BRK_TYPE brk_type)
{
	BYTE buf;
	u32 readed;
	// � ������ ��������� soft-���������� (int 3) ���������
	// ����� ��� �� ���� ��� ����������, � ���� ����
	// �������� ���� ��� ��������� ���������� ����� ������������
	switch (brk_type)
	{
	case BRK_TYPE_INT3:
		if (!dbg_read_memory(remote_id, proc_id, address, buf, sizeof(buf), &readed))
			return 1; // ������ �� ����� ���� ���������
		
		break;
	case BRK_TYPE_HWR:
	case BRK_TYPE_HWE:
	case BRK_TYPE_HWE:
		break;
	}
	return 0;
}

int breakpoint::delete_breakpoint (PVOID remote_id, u32 proc_id, u32 thread_id, u3264 address)
{
	return 0;
}

int breakpoint::is_breakpoint_exists (PVOID remote_id, u32 proc_id, u32 thread_id, u3264 address)
{
	// � ������ ��������� soft-���������� (int 3) ���������
	// ����� ��� �� ���� ��� ����������, � ���� ����
	// �������� ���� ��� ��������� ���������� ����� ������������
	switch (brk_type)
	{
	case BRK_TYPE_INT3:
		//std::find(breaks.begin(), breaks.end(), address);
		//dbg_read_memory(,,address,,);
		break;
	case BRK_TYPE_HWR:
	case BRK_TYPE_HWE:
	case BRK_TYPE_HWE:
		break;
	}
	return 0;
}
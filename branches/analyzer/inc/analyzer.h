#ifndef _ANALYSER_H
#define _ANALYSER_H

#include <cstdlib>
#include <vector>
#include <map>
#include <Windows.h>
#include "../disasm/mediana.h"
#include "../../inc/types.h"

#define OUT_BUFF_SIZE 0x200
#define IN_BUFF_SIZE  0x1000
#define SEEK_TO       0x0
#define INT3		  0xCC
#define ARMADILLO

namespace analyser
{

class block
{
public:
	typedef std::vector<u32> REFS;
	block (u32 addr, u32 referer): is_valid(true), address(addr), size(0), count(0), is_analyzed(false)
	{
		if (referer)
		{
			RefFrom.push_back(referer);
		}
	}
//private:
	bool is_analyzed; // ��������������� �� ����?
	bool is_valid; // �������� �� ���������� �����?
	u32 address; // VA ������ �����
	u32 address_end; // VA ����� ����� (����� ���������� ����� ��������� ���������� �����)
	u32 count; // ���-�� ���������� � �����
	u32 size; // ������ ����� � ������
	u32 delta; // ������ �����
	u32 crc; // CRC �������� �����
	u8 type; // ��� ����� (������ ������ ����� �� ����� �����:
	// 0.  ������ ������ ����� (������ �����, ����� ����������� ���� ������)
	// 1.  ������� ���� Jcc/JCXZ
	// 2.  ������� JMP ��� ������������
	// 3.  ������� JMP � ����������
	// 4.  ������� JMP � ��������� � ������� ������� (switch)
	// 5.  ������� JMP/CALL ds:[X] - ������
	// 6.  ������� INT 3
	// 7.  ����� ������� ���������� �������� ������/��������
	// 8.  ������� CALL � ���� �������� ������ ���������
	// 9.  ������� CALL ��� ���� ��������� ��������
	// 10. ������� ������ LOOP
	// 11. ������� RET
	// 12. ������� INT N (�� 3)
	// 13. ������� IRET
	// 14. SYSEXIT/SYSRET
	// ��� ������ ���� Jcc/JCXZ (����������� �������� ��� ������� ���������� SWITCH)
	u32 regs; // �� ������ ����� ��������� �������� �������� ������� ���� �� ������ ������
	u32 val; // �������� � ������� �������������� ���������
	u16 id; // ID ������� ������� ������������ �������
	REFS RefTo; // ������ ���� ������ �� �����
	REFS RefFrom; // ������ ������� ������, ������ ����������� �� ����
	char label[16]; // ����� � ������ �����
	u32 flags; // ��������� �����
};

typedef class block* block_ptr;
typedef struct {
	u32 from;
	u32 to;
} ref;
typedef std::map<u32,block_ptr> BLOCKS;
typedef std::pair <u32,block_ptr> block_pair;
class blocks_analyser
{
private:
	BLOCKS main_tree;
	// Mediana disassembler vars
	u8 sf_prefixes[MAX_INSTRUCTION_LEN];
	char buff[OUT_BUFF_SIZE];
	int reallen;
	u32 res;
	struct INSTRUCTION instr;
	struct DISASM_INOUT_PARAMS params;
	// END Mediana disassembler vars

public:
	u8 *base;
	u16 NumberOfSections;
	u32 ImageBase, EntryPoint, EP;
	u32 code_size, code_section_va, code_section_rva;

	blocks_analyser (u8 *header, u32 *file_offset, u32 *size);
	~blocks_analyser ();
	void code_load (u8 *pointer_to_code, u32 size);
#ifdef ARMADILLO
	void blocks_analyser::ananlyze_from_va (u32 v_start_addr, nano_db *nanos);
	int block_process (block *node, nano_db *nanos);
#else
	void blocks_analyser::ananlyze_from_va (u32 v_start_addr);
	int block_process (block *node);
#endif
};

}

#endif
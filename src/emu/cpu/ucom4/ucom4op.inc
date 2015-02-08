// uCOM-4 opcode handlers

// internal helpers

inline UINT8 ucom4_cpu_device::ram_r()
{
	UINT16 address = m_dph << 4 | m_dpl;
	return m_data->read_byte(address & m_datamask) & 0xf;
}

inline void ucom4_cpu_device::ram_w(UINT8 data)
{
	UINT16 address = m_dph << 4 | m_dpl;
	m_data->write_byte(address & m_datamask, data & 0xf);
}

void ucom4_cpu_device::pop_stack()
{
	m_pc = m_stack[0] & m_prgmask;
	for (int i = 0; i < m_stack_levels; i++)
	{
		m_stack[i] = m_stack[i+1];
		m_stack[i+1] = 0;
	}
}

void ucom4_cpu_device::push_stack()
{
	for (int i = m_stack_levels-1; i >= 1; i--)
		m_stack[i] = m_stack[i-1];
	m_stack[0] = m_pc;
}

UINT8 ucom4_cpu_device::input_r(int index)
{
	index &= 0xf;
	UINT8 inp = 0xf;
	
	switch (index)
	{
		case NEC_UCOM4_PORTA: inp = m_read_a(index, 0xff); break;
		case NEC_UCOM4_PORTB: inp = m_read_b(index, 0xff); break;
		case NEC_UCOM4_PORTC: inp = m_read_c(index, 0xff); break;
		case NEC_UCOM4_PORTD: inp = m_read_d(index, 0xff); break;

		default:
			logerror("%s read from unknown port %c at $%03X\n", tag(), 'A' + index, m_pc);
			break;
	}

	return inp & 0xf;
}

void ucom4_cpu_device::output_w(int index, UINT8 data)
{
	index &= 0xf;
	data &= 0xf;

	switch (index)
	{
		case NEC_UCOM4_PORTC: m_write_c(index, data, 0xff); break;
		case NEC_UCOM4_PORTD: m_write_d(index, data, 0xff); break;
		case NEC_UCOM4_PORTE: m_write_e(index, data, 0xff); break;
		case NEC_UCOM4_PORTF: m_write_f(index, data, 0xff); break;
		case NEC_UCOM4_PORTG: m_write_g(index, data, 0xff); break;
		case NEC_UCOM4_PORTH: m_write_h(index, data, 0xff); break;
		case NEC_UCOM4_PORTI: m_write_i(index, data & 7, 0xff); break;
		
		default:
			logerror("%s write to unknown port %c = $%X at $%03X\n", tag(), 'A' + index, data & 0xf, m_pc);
			break;
	}

	m_port_out[index] = data;
}

void ucom4_cpu_device::op_illegal()
{
	logerror("%s unknown opcode $%02X at $%03X\n", tag(), m_op, m_pc);
}



// basic instruction set

// Load

void ucom4_cpu_device::op_li()
{
	// LI X: Load ACC with X
	// note: only execute the first one in a sequence of LI
	if ((m_prev_op & 0xf0) != (m_op & 0xf0))
		m_acc = m_op & 0x0f;
}

void ucom4_cpu_device::op_lm()
{
	// LM X: Load ACC with RAM, xor DPh with X
	m_acc = ram_r();
	m_dph ^= (m_op & 0x03);
}

void ucom4_cpu_device::op_ldi()
{
	// LDI X: Load DP with X
	m_dph = m_arg >> 4 & 0xf;
	m_dpl = m_arg & 0x0f;
}

void ucom4_cpu_device::op_ldz()
{
	// LDZ X: Load DPh with 0, Load DPl with X
	m_dph = 0;
	m_dpl = m_op & 0x0f;
}


// Store

void ucom4_cpu_device::op_s()
{
	// S: Store ACC into RAM
	ram_w(m_acc);
}


// Transfer

void ucom4_cpu_device::op_tal()
{
	// TAL: Transfer ACC to DPl
	m_dpl = m_acc;
}

void ucom4_cpu_device::op_tla()
{
	// TLA: Transfer DPl to ACC
	m_acc = m_dpl;
}


// Exchange

void ucom4_cpu_device::op_xm()
{
	// XM X: Exchange ACC with RAM, xor DPh with X
	UINT8 old_acc = m_acc;
	m_acc = ram_r();
	ram_w(old_acc);
	m_dph ^= (m_op & 0x03);
}

void ucom4_cpu_device::op_xmi()
{
	// XMI X: Exchange ACC with RAM, xor DPh with X, Increment DPl, skip next on carry
	op_xm();
	m_dpl = (m_dpl + 1) & 0xf;
	m_skip = (m_dpl == 0);
}

void ucom4_cpu_device::op_xmd()
{
	// XMD X: Exchange ACC with RAM, xor DPh with X, Decrement DPl, skip next on carry
	op_xm();
	m_dpl = (m_dpl - 1) & 0xf;
	m_skip = (m_dpl == 0xf);
}


// Arithmetic

void ucom4_cpu_device::op_ad()
{
	// AD: Add RAM to ACC, skip next on carry
	m_acc += ram_r();
	m_skip = ((m_acc & 0x10) != 0);
	m_acc &= 0xf;
}

void ucom4_cpu_device::op_adc()
{
	// ADC: Add RAM and carry to ACC, store Carry F/F
	op_illegal();
}

void ucom4_cpu_device::op_ads()
{
	// ADS: Add RAM and carry to ACC, store Carry F/F, skip next on carry
	op_illegal();
}

void ucom4_cpu_device::op_daa()
{
	// DAA: Add 6 to ACC to adjust decimal for BCD Addition
	m_acc = (m_acc + 6) & 0xf;
}

void ucom4_cpu_device::op_das()
{
	// DAS: Add 10 to ACC to adjust decimal for BCD Subtraction
	m_acc = (m_acc + 10) & 0xf;
}


// Logical

void ucom4_cpu_device::op_exl()
{
	// EXL: Xor ACC with RAM
	m_acc ^= ram_r();
}


// Accumulator

void ucom4_cpu_device::op_cma()
{
	// CMA: Complement ACC
	m_acc ^= 0xf;
}

void ucom4_cpu_device::op_cia()
{
	// CIA: Complement ACC, Increment ACC
	m_acc = ((m_acc ^ 0xf) + 1) & 0xf;
}


// Carry Flag

void ucom4_cpu_device::op_clc()
{
	// CLC: Reset Carry F/F
	m_carry_f = 0;
}

void ucom4_cpu_device::op_stc()
{
	// STC: Set Carry F/F
	m_carry_f = 1;
}

void ucom4_cpu_device::op_tc()
{
	// TC: skip next on Carry F/F
	m_skip = (m_carry_f != 0);
}


// Increment and Decrement

void ucom4_cpu_device::op_inc()
{
	// INC: Increment ACC, skip next on carry
	m_acc = (m_acc + 1) & 0xf;
	m_skip = (m_acc == 0);
}

void ucom4_cpu_device::op_dec()
{
	// DEC: Decrement ACC, skip next on carry
	m_acc = (m_acc - 1) & 0xf;
	m_skip = (m_acc == 0xf);
}

void ucom4_cpu_device::op_ind()
{
	// IND: Increment DPl, skip next on carry
	m_dpl = (m_dpl + 1) & 0xf;
	m_skip = (m_dpl == 0);
}

void ucom4_cpu_device::op_ded()
{
	// DED: Decrement DPl, skip next on carry
	m_dpl = (m_dpl - 1) & 0xf;
	m_skip = (m_dpl == 0xf);
}


// Bit Manipulation

void ucom4_cpu_device::op_rmb()
{
	// RMB B: Reset a single bit of RAM
	ram_w(ram_r() & ~m_bitmask);
}

void ucom4_cpu_device::op_smb()
{
	// SMB B: Set a single bit of RAM
	ram_w(ram_r() | m_bitmask);
}

void ucom4_cpu_device::op_reb()
{
	// REB B: Reset a single bit of output port E
	m_icount--;
	output_w(NEC_UCOM4_PORTE, m_port_out[NEC_UCOM4_PORTE] & ~m_bitmask);
}

void ucom4_cpu_device::op_seb()
{
	// SEB B: Set a single bit of output port E
	m_icount--;
	output_w(NEC_UCOM4_PORTE, m_port_out[NEC_UCOM4_PORTE] | m_bitmask);
}

void ucom4_cpu_device::op_rpb()
{
	// RPB B: Reset a single bit of output port (DPl)
	output_w(m_dpl, m_port_out[m_dpl] & ~m_bitmask);
}

void ucom4_cpu_device::op_spb()
{
	// SPB B: Set a single bit of output port (DPl)
	output_w(m_dpl, m_port_out[m_dpl] | m_bitmask);
}


// Jump, Call and Return

void ucom4_cpu_device::op_jmpcal()
{
	// JMP A: Jump to Address / CAL A: Call Address
	if (m_op & 0x08)
		push_stack();
	m_pc = (m_op & 0x07) << 8 | m_arg;
}

void ucom4_cpu_device::op_jcp()
{
	// JCP A: Jump to Address in current page
	m_pc = (m_pc & ~0x3f) | (m_op & 0x3f);
}

void ucom4_cpu_device::op_jpa()
{
	// JPA: Jump to (ACC) in current page
	m_pc = (m_pc & ~0x3f) | (m_acc << 2);
}

void ucom4_cpu_device::op_czp()
{
	// CZP A: Call Address (short)
	push_stack();
	m_pc = (m_op & 0x0f) << 2;
}

void ucom4_cpu_device::op_rt()
{
	// RT: Return from subroutine
	m_icount--;
	pop_stack();
}

void ucom4_cpu_device::op_rts()
{
	// RTS: Return from subroutine, skip next
	pop_stack();
	m_skip = true;
}


// Skip

void ucom4_cpu_device::op_ci()
{
	// CI X: skip next on ACC equals X
	m_skip = (m_acc == (m_arg & 0x0f));
	
	if ((m_arg & 0xf0) != 0xc0)
		logerror("%s CI opcode unexpected upper arg $%02X at $%03X\n", tag(), m_arg & 0xf0, m_pc);
}

void ucom4_cpu_device::op_cm()
{
	// CM: skip next on ACC equals RAM
	m_skip = (m_acc == ram_r());
}

void ucom4_cpu_device::op_cmb()
{
	// CMB B: skip next on bit(ACC) equals bit(RAM)
	m_skip = ((m_acc & m_bitmask) == (ram_r() & m_bitmask));
}

void ucom4_cpu_device::op_tab()
{
	// TAB B: skip next on bit(ACC)
	m_skip = ((m_acc & m_bitmask) != 0);
}

void ucom4_cpu_device::op_cli()
{
	// CLI X: skip next on DPl equals X
	m_skip = (m_dpl == (m_arg & 0x0f));
	
	if ((m_arg & 0xf0) != 0xe0)
		logerror("%s CLI opcode unexpected upper arg $%02X at $%03X\n", tag(), m_arg & 0xf0, m_pc);
}

void ucom4_cpu_device::op_tmb()
{
	// TMB B: skip next on bit(RAM)
	m_skip = ((ram_r() & m_bitmask) != 0);
}

void ucom4_cpu_device::op_tpa()
{
	// TPA B: skip next on bit(input port A)
	m_skip = ((input_r(NEC_UCOM4_PORTA) & m_bitmask) != 0);
}

void ucom4_cpu_device::op_tpb()
{
	// TPB B: skip next on bit(input port (DPl))
	m_skip = ((input_r(m_dpl) & m_bitmask) != 0);
}


// Interrupt

void ucom4_cpu_device::op_tit()
{
	// TIT: skip next on Interrupt F/F, reset Interrupt F/F
	op_illegal();
}


// Parallel I/O

void ucom4_cpu_device::op_ia()
{
	// IA: Input port A to ACC
	m_acc = input_r(NEC_UCOM4_PORTA);
}

void ucom4_cpu_device::op_ip()
{
	// IP: Input port (DPl) to ACC
	m_acc = input_r(m_dpl);
}

void ucom4_cpu_device::op_oe()
{
	// OE: Output ACC to port E
	output_w(NEC_UCOM4_PORTE, m_acc);
}

void ucom4_cpu_device::op_op()
{
	// OP: Output ACC to port (DPl)
	output_w(m_dpl, m_acc);
}

void ucom4_cpu_device::op_ocd()
{
	// OCD X: Output X to ports C and D
	output_w(NEC_UCOM4_PORTD, m_arg >> 4);
	output_w(NEC_UCOM4_PORTC, m_arg & 0xf);
}


// CPU Control

void ucom4_cpu_device::op_nop()
{
	// NOP: No Operation
}



// uCOM-43 extended instructions

inline bool ucom4_cpu_device::check_op_43()
{
	// these opcodes are officially only supported on uCOM-43
	if (m_family != NEC_UCOM43)
		logerror("%s using uCOM-43 opcode $%02X at $%03X\n", tag(), m_op, m_pc);

	return (m_family == NEC_UCOM43);
}

// Transfer

void ucom4_cpu_device::op_taw()
{
	if (!check_op_43()) return;

	// TAW: Transfer ACC to W
	op_illegal();
}

void ucom4_cpu_device::op_taz()
{
	if (!check_op_43()) return;
	
	// TAZ: Transfer ACC to Z
	op_illegal();
}

void ucom4_cpu_device::op_thx()
{
	if (!check_op_43()) return;
	
	// THX: Transfer DPh to X
	op_illegal();
}

void ucom4_cpu_device::op_tly()
{
	if (!check_op_43()) return;
	
	// TLY: Transfer DPl to Y
	op_illegal();
}


// Exchange

void ucom4_cpu_device::op_xaw()
{
	if (!check_op_43()) return;
	
	// XAW: Exchange ACC with W
	op_illegal();
}

void ucom4_cpu_device::op_xaz()
{
	if (!check_op_43()) return;
	
	// XAZ: Exchange ACC with Z
	op_illegal();
}

void ucom4_cpu_device::op_xhr()
{
	if (!check_op_43()) return;
	
	// XHR: Exchange DPh with R
	op_illegal();
}

void ucom4_cpu_device::op_xhx()
{
	if (!check_op_43()) return;
	
	// XHX: Exchange DPh with X
	op_illegal();
}

void ucom4_cpu_device::op_xls()
{
	if (!check_op_43()) return;
	
	// XLS: Exchange DPl with S
	op_illegal();
}

void ucom4_cpu_device::op_xly()
{
	if (!check_op_43()) return;
	
	// XLY: Exchange DPl with Y
	op_illegal();
}

void ucom4_cpu_device::op_xc()
{
	if (!check_op_43()) return;
	
	// XC: Exchange Carry F/F with Carry Save F/F
	op_illegal();
}


// Flag

void ucom4_cpu_device::op_sfb()
{
	if (!check_op_43()) return;
	
	// SFB B: Set a single bit of FLAG
	op_illegal();
}

void ucom4_cpu_device::op_rfb()
{
	if (!check_op_43()) return;
	
	// RFB B: Reset a single bit of FLAG
	op_illegal();
}

void ucom4_cpu_device::op_fbt()
{
	if (!check_op_43()) return;
	
	// FBT B: skip next on bit(FLAG)
	op_illegal();
}

void ucom4_cpu_device::op_fbf()
{
	if (!check_op_43()) return;
	
	// FBF B: skip next on not bit(FLAG)
	op_illegal();
}


// Accumulator

void ucom4_cpu_device::op_rar()
{
	if (!check_op_43()) return;
	
	// RAR: Rotate ACC Right through Carry F/F
	op_illegal();
}


// Increment and Decrement

void ucom4_cpu_device::op_inm()
{
	if (!check_op_43()) return;
	
	// INM: Increment RAM, skip next on carry
	op_illegal();
}

void ucom4_cpu_device::op_dem()
{
	if (!check_op_43()) return;
	
	// DEM: Decrement RAM, skip next on carry
	op_illegal();
}


// Timer

void ucom4_cpu_device::op_stm()
{
	if (!check_op_43()) return;
	
	// STM X: Reset Timer F/F, Start Timer with X
	op_illegal();
}

void ucom4_cpu_device::op_ttm()
{
	if (!check_op_43()) return;
	
	// TTM: skip next on Timer F/F
	op_illegal();
}


// Interrupt

void ucom4_cpu_device::op_ei()
{
	if (!check_op_43()) return;
	
	// EI: Set Interrupt Enable F/F
	op_illegal();
}

void ucom4_cpu_device::op_di()
{
	if (!check_op_43()) return;
	
	// DI: Reset Interrupt Enable F/F
	op_illegal();
}
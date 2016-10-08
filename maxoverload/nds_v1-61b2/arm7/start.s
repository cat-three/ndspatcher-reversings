@ file header of .nds reports;
@ arm7 offset:			0x26600
@ arm7 entry address:	0x37F8000
@ arm7 ram address:		0x37F8000
@ arm7 code size:		0x68C


@ disassembly is copied, the syntax here likely is not perfect

_start:
		mov             r0, #0x4000000
		str             r0, [r0,#0x208]
		ldr             r0, =0x380FFC0			@0x380FFC0-0x37F8000=0x17FC0		
		mov             r1, #0x13 ; r1 = 0x13
		msr             CPSR_c, r1
		mov             sp, r0
		sub             r0, r0, #0x40
		mov             r1, #0x12
		msr             CPSR_c, r1
		mov             sp, r0
		sub             r0, r0, #0x400
		mov             r1, #0x1F
		msr             CPSR_c, r1
		mov             sp, r0
		bl              clearmem
		mov             r0, #0
		mov             r1, #0
		ldr             r3, =0x23DF220
		bx              r3

@ is this cache related?
clearmem:
		ldr             r3, =0x23DF690
		ldr             r0, =0x23DF690
		cmp             r3, r0
		bcs             skip
		mov             r2, #0

		loopA:
				strb            r2, [r3],#1
				cmp             r3, r0
				bcc             loopA

		skip:
				ldr             r3, =0x23DF000
				cmp             r3, r0
				bxcs            lr
				mov             r1, #0x37C0000
				mov             r2, r3
				add             r1, r1, #0x38000

		loopB:
				ldrb            r3, [r1],#1
				strb            r3, [r2],#1
				cmp             r2, r0
				bcc             loopB
				bx              lr
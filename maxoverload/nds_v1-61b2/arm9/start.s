@ disassembly is copied, the syntax here likely is not perfect

_start:
		mov             r0, #0x4000000
		str             r0, [r0,#0x208]
		ldr             r1, =0x2078
		mcr             p15, 0, r1,c1,c0, 0
		ldr             r0, =0x80000A
		mcr             p15, 0, r0,c9,c1, 0
		mrc             p15, 0, r0,c1,c0, 0
		Orr             r1, r1, #0x10000
		mcr             p15, 0, r1,c1,c0, 0
		mov             r1, #0
		strb            r1, [r0,#0x3F]
		ldr             r1, =0x2078
		mcr             p15, 0, r1,c1,c0, 0
		mov             r0, #0
		mcr             p15, 0, r0,c7,c5, 0
		mcr             p15, 0, r0,c7,c6, 0
		mcr             p15, 0, r0,c7,c10, 4
		ldr             r0, =0x80000A

microprocessorSetup:
		mcr             p15, 0, r0,c9,c1, 0
		ldr             r0, =0x4000033
		mcr             p15, 0, r0,c6,c0, 0
		ldr             r0, =0x200002B
		mcr             p15, 0, r0,c6,c1, 0
		ldr             r0, =0x37F800F
		mcr             p15, 0, r0,c6,c2, 0
		ldr             r0, =0x8000035
		mcr             p15, 0, r0,c6,c3, 0
		ldr             r0, =0x80001B
		mcr             p15, 0, r0,c6,c4, 0
		ldr             r0, =0x100001D
		mcr             p15, 0, r0,c6,c5, 0
		ldr             r0, =0xFFFF001D
		mcr             p15, 0, r0,c6,c6, 0
		ldr             r0, =0x27FF017
		mcr             p15, 0, r0,c6,c7, 0
		ldr             r0, =0x36633333
		mcr             p15, 0, r0,c5,c0, 2
		ldr             r0, =0x36303333
		mcr             p15, 0, r0,c5,c0, 3
		mov             r0, #6
		mcr             p15, 0, r0,c3,c0, 0
		mov             r0, #0x42 ; 'B'
		mcr             p15, 0, r0,c2,c0, 0
		mcr             p15, 0, r0,c2,c0, 1
		mrc             p15, 0, r0,c1,c0, 0
		ldr             r1, =0x55005
		orr             r0, r0, r1
		mcr             p15, 0, r0,c1,c0, 0
		mov             r0, #0x2380000
		msr             CPSr_c, #0x12
		mov             sp, r0
		sub             r0, r0, #0x100
		msr             CPSr_c, #0x13
		mov             sp, r0
		sub             r0, r0, #0x100
		msr             CPSr_c, #0x1F
		mov             sp, r0
		bl              clearmem
		mov             r0, #0
		mov             r1, #0
		ldr             r3, =main		@ =0x2002110
		tst             r3, #1
		ldreq           lr, =infloop		@ =0x2000904
		ldrne           lr, =fault  		@ =0x2000909
		bx              r3

infloop:
		B               loc_904


@ is this thumb?
@ ROM:908 - ROM:90C
fault:
		b.n #0			@ FE E7 00 00

@ is this cache related?
clearmem:
		ldr             r2, =0x20225D0
		ldr             r3, =0x20227E4
		cmp             r2, r3
		bxcs            lr
		mov             r3, r2
		mov             r1, #0
		ldr             r2, =0x20227E4

		loop:                                 
				strb            r1, [r3],#1
				cmp             r3, r2
				bcc             loop
				bx              lr
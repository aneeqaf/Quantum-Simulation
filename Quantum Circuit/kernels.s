	.file	"kernels.cpp"
	.section	.text._ZN4GateD2Ev,"axG",@progbits,_ZN4GateD5Ev,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN4GateD2Ev
	.type	_ZN4GateD2Ev, @function
_ZN4GateD2Ev:
.LFB10338:
	.cfi_startproc
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	movq	%rdi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	movq	72(%rdi), %rdi
	testq	%rdi, %rdi
	je	.L2
	call	_ZdlPv
.L2:
	movq	48(%rbp), %rdi
	testq	%rdi, %rdi
	je	.L3
	call	_ZdlPv
.L3:
	movq	24(%rbp), %rdi
	testq	%rdi, %rdi
	je	.L4
	call	_ZdlPv
.L4:
	movq	8(%rbp), %rax
	movq	0(%rbp), %rdi
	cmpq	%rdi, %rax
	je	.L5
	leaq	24(%rdi), %rbx
	subq	%rbx, %rax
	shrq	$3, %rax
	leaq	24(%rdi,%rax,8), %r12
	jmp	.L6
	.p2align 4,,10
	.p2align 3
.L23:
	call	_ZdlPv
	cmpq	%rbx, %r12
	movq	%rbx, %rdi
	je	.L8
.L9:
	addq	$24, %rbx
.L6:
	movq	(%rdi), %rdi
	testq	%rdi, %rdi
	jne	.L23
	cmpq	%r12, %rbx
	movq	%rbx, %rdi
	jne	.L9
.L8:
	movq	0(%rbp), %rdi
.L5:
	testq	%rdi, %rdi
	je	.L20
	popq	%rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	jmp	_ZdlPv
	.p2align 4,,10
	.p2align 3
.L20:
	.cfi_restore_state
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE10338:
	.size	_ZN4GateD2Ev, .-_ZN4GateD2Ev
	.weak	_ZN4GateD1Ev
	.set	_ZN4GateD1Ev,_ZN4GateD2Ev
	.text
	.p2align 4,,15
	.globl	_Z12GroupCZGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE
	.type	_Z12GroupCZGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE, @function
_Z12GroupCZGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE:
.LFB10369:
	.cfi_startproc
	movq	(%rdx), %rax
	movq	8(%rdx), %r10
	subl	$1, %esi
	cmpq	%r10, %rax
	je	.L32
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movq	%rax, %rdx
	movq	8(%rdi), %r8
	xorl	%r9d, %r9d
	movq	%rax, %rdi
	movl	$1, %r11d
	.p2align 4,,10
	.p2align 3
.L26:
	movslq	(%rdx), %rax
	movl	%esi, %ecx
	movq	%r11, %rbx
	addq	$4, %rdx
	subl	%eax, %ecx
	salq	%cl, %rbx
	orq	%rbx, (%r8,%rax,8)
	orq	%rbx, %r9
	cmpq	%r10, %rdx
	jne	.L26
	.p2align 4,,10
	.p2align 3
.L27:
	movslq	(%rdi), %rax
	addq	$4, %rdi
	xorq	%r9, (%r8,%rax,8)
	cmpq	%rdi, %rdx
	jne	.L27
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L32:
	.cfi_restore 3
	rep ret
	.cfi_endproc
.LFE10369:
	.size	_Z12GroupCZGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE, .-_Z12GroupCZGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE
	.p2align 4,,15
	.globl	_Z11GroupTGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE
	.type	_Z11GroupTGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE, @function
_Z11GroupTGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE:
.LFB10370:
	.cfi_startproc
	movq	(%rdx), %rax
	leal	-1(%rsi), %ecx
	movq	8(%rdi), %rsi
	subl	(%rax), %ecx
	movq	(%rsi), %rdx
	movl	$1, %eax
	salq	%cl, %rax
	movq	%rdx, %rcx
	andq	%rax, %rcx
	cmpq	%rax, %rcx
	je	.L36
	orq	%rdx, %rax
	movq	%rax, (%rsi)
	ret
	.p2align 4,,10
	.p2align 3
.L36:
	movq	(%rdi), %rdx
	xorl	%r9d, %r9d
	leaq	(%rsi,%rdx,8), %r8
	movq	%rsi, %rdx
	cmpq	%r8, %rsi
	je	.L41
.L47:
	addq	$8, %rdx
	cmpq	%r8, %rdx
	je	.L50
.L40:
	movq	(%rdx), %rcx
	movq	%rcx, %rdi
	andq	%rax, %rdi
	cmpq	%rax, %rdi
	je	.L47
	orq	%rax, %rcx
	addq	$8, %rdx
	movl	$1, %r9d
	movq	%rcx, -8(%rdx)
	cmpq	%r8, %rdx
	jne	.L40
.L50:
	testb	%r9b, %r9b
	je	.L41
	rep ret
	.p2align 4,,10
	.p2align 3
.L41:
	movq	%rax, 8(%rsi)
	ret
	.cfi_endproc
.LFE10370:
	.size	_Z11GroupTGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE, .-_Z11GroupTGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE
	.p2align 4,,15
	.globl	_Z20ExtractIndicesForAmpPmRKSt6vectorIiSaIiEEim
	.type	_Z20ExtractIndicesForAmpPmRKSt6vectorIiSaIiEEim, @function
_Z20ExtractIndicesForAmpPmRKSt6vectorIiSaIiEEim:
.LFB10371:
	.cfi_startproc
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	movl	$1, %r8d
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	movq	%rcx, %rbx
	movq	(%rsi), %r13
	movq	8(%rsi), %rbp
	movq	%rbx, (%rdi)
	subq	%r13, %rbp
	sarq	$2, %rbp
	leal	-1(%rbp), %ecx
	salq	%cl, %r8
	cmpq	%rbp, %rbx
	jnb	.L58
	addq	$1, %rbx
	addq	$1, %rbp
	movq	%r8, %r9
	movl	$1, %esi
	movl	$1, %r12d
	subl	$1, %edx
	.p2align 4,,10
	.p2align 3
.L55:
	movq	%r12, %r10
	movl	%ebx, %ecx
	salq	%cl, %r10
	cmpq	%rsi, %r10
	jbe	.L53
	movl	%edx, %ecx
	subl	-4(%r13,%rbx,4), %ecx
	movq	%r12, %r11
	salq	$3, %r8
	movq	%rdi, %rax
	salq	%cl, %r11
	.p2align 4,,10
	.p2align 3
.L54:
	movq	(%rax), %rcx
	addq	$1, %rsi
	addq	%r11, %rcx
	movq	%rcx, (%rax,%r9,8)
	addq	%r8, %rax
	cmpq	%r10, %rsi
	jne	.L54
.L53:
	movq	%r9, %rax
	addq	$1, %rbx
	movq	%r9, %r8
	shrq	%rax
	cmpq	%rbx, %rbp
	je	.L58
	movq	%rax, %r9
	jmp	.L55
	.p2align 4,,10
	.p2align 3
.L58:
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE10371:
	.size	_Z20ExtractIndicesForAmpPmRKSt6vectorIiSaIiEEim, .-_Z20ExtractIndicesForAmpPmRKSt6vectorIiSaIiEEim
	.p2align 4,,15
	.globl	_Z19FormBlockOfCZTGatesRKSt6vectorI4GateSaIS0_EEiRmRSt8valarrayImES8_
	.type	_Z19FormBlockOfCZTGatesRKSt6vectorI4GateSaIS0_EEiRmRSt8valarrayImES8_, @function
_Z19FormBlockOfCZTGatesRKSt6vectorI4GateSaIS0_EEiRmRSt8valarrayImES8_:
.LFB10372:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movq	%rcx, %rbx
	movq	(%rdi), %r11
	movq	8(%rdi), %r9
	movabsq	$5675921253449092805, %rcx
	movq	(%rdx), %rax
	subq	%r11, %r9
	sarq	$3, %r9
	imulq	%rcx, %r9
	cmpq	%r9, %rax
	jnb	.L82
	subl	$1, %esi
	movl	$1, %r10d
	jmp	.L71
	.p2align 4,,10
	.p2align 3
.L86:
	orq	%rax, %rcx
	movq	%rcx, 0(%rbp)
	movq	(%rdx), %rax
.L63:
	addq	$1, %rax
	cmpq	%r9, %rax
	movq	%rax, (%rdx)
	jnb	.L82
.L71:
	leaq	(%rax,%rax,2), %rcx
	leaq	(%rax,%rcx,4), %rcx
	leaq	(%r11,%rcx,8), %rcx
	movq	56(%rcx), %rdi
	movl	-4(%rdi), %edi
	cmpl	$3, %edi
	je	.L85
	cmpl	$11, %edi
	jne	.L82
	movq	24(%rcx), %rax
	movl	%esi, %ecx
	movq	8(%r8), %rbp
	subl	(%rax), %ecx
	movq	%r10, %rax
	salq	%cl, %rax
	movq	%rax, %rcx
	movq	0(%rbp), %rax
	movq	%rcx, %rdi
	andq	%rax, %rdi
	cmpq	%rdi, %rcx
	jne	.L86
	movq	(%r8), %rax
	xorl	%r14d, %r14d
	leaq	0(%rbp,%rax,8), %r13
	movq	%rbp, %rax
	cmpq	%r13, %rbp
	je	.L70
.L80:
	addq	$8, %rax
	cmpq	%rax, %r13
	je	.L87
.L69:
	movq	(%rax), %rdi
	movq	%rcx, %r12
	andq	%rdi, %r12
	cmpq	%r12, %rcx
	je	.L80
	orq	%rcx, %rdi
	addq	$8, %rax
	movl	$1, %r14d
	movq	%rdi, -8(%rax)
	cmpq	%rax, %r13
	jne	.L69
.L87:
	testb	%r14b, %r14b
	je	.L70
.L84:
	movq	(%rdx), %rax
	addq	$1, %rax
	cmpq	%r9, %rax
	movq	%rax, (%rdx)
	jb	.L71
.L82:
	popq	%rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L85:
	.cfi_restore_state
	movq	24(%rcx), %rbp
	movq	32(%rcx), %r14
	cmpq	%r14, %rbp
	je	.L63
	movq	8(%rbx), %r12
	movq	%rbp, %rdi
	xorl	%r13d, %r13d
	.p2align 4,,10
	.p2align 3
.L64:
	movslq	(%rdi), %rax
	movl	%esi, %ecx
	movq	%r10, %r15
	addq	$4, %rdi
	subl	%eax, %ecx
	salq	%cl, %r15
	orq	%r15, (%r12,%rax,8)
	orq	%r15, %r13
	cmpq	%rdi, %r14
	jne	.L64
	.p2align 4,,10
	.p2align 3
.L65:
	movslq	0(%rbp), %rax
	addq	$4, %rbp
	xorq	%r13, (%r12,%rax,8)
	cmpq	%rbp, %rdi
	jne	.L65
	jmp	.L84
	.p2align 4,,10
	.p2align 3
.L70:
	movq	%rcx, 8(%rbp)
	movq	(%rdx), %rax
	jmp	.L63
	.cfi_endproc
.LFE10372:
	.size	_Z19FormBlockOfCZTGatesRKSt6vectorI4GateSaIS0_EEiRmRSt8valarrayImES8_, .-_Z19FormBlockOfCZTGatesRKSt6vectorI4GateSaIS0_EEiRmRSt8valarrayImES8_
	.p2align 4,,15
	.globl	_Z19FormBlockOfXYHGatesRKSt6vectorI4GateSaIS0_EERmNS0_4TypeE
	.type	_Z19FormBlockOfXYHGatesRKSt6vectorI4GateSaIS0_EERmNS0_4TypeE, @function
_Z19FormBlockOfXYHGatesRKSt6vectorI4GateSaIS0_EERmNS0_4TypeE:
.LFB10373:
	.cfi_startproc
	.cfi_personality 0x3,__gxx_personality_v0
	.cfi_lsda 0x3,.LLSDA10373
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	movq	%rsi, %rax
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	movabsq	$5675921253449092805, %r8
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movq	%rdi, %rbp
	movq	%rdx, %r12
	subq	$56, %rsp
	.cfi_def_cfa_offset 112
	movq	$0, (%rdi)
	movq	$0, 8(%rdi)
	movq	$0, 16(%rdi)
	movq	8(%rax), %rdi
	movq	%rsi, 32(%rsp)
	movq	(%rsi), %rsi
	movq	(%rdx), %rdx
	movl	%ecx, 20(%rsp)
	movq	%rdi, %rax
	subq	%rsi, %rax
	sarq	$3, %rax
	imulq	%r8, %rax
	cmpq	%rax, %rdx
	jnb	.L116
	leaq	(%rdx,%rdx,2), %rax
	leaq	(%rdx,%rax,4), %rax
	leaq	(%rsi,%rax,8), %rdx
	movq	56(%rdx), %rax
	cmpl	-4(%rax), %ecx
	jne	.L116
	xorl	%eax, %eax
	xorl	%ecx, %ecx
	xorl	%r13d, %r13d
	xorl	%r15d, %r15d
	xorl	%ebx, %ebx
	movq	%r8, 8(%rsp)
	jmp	.L90
	.p2align 4,,10
	.p2align 3
.L118:
	movl	-4(%r14), %edx
	addq	$4, %rbx
	movl	%edx, -4(%rbx)
	movq	%rbx, 8(%rbp)
.L93:
	movq	%rdi, %rcx
	movq	(%r12), %rdx
	subq	%rsi, %rcx
	sarq	$3, %rcx
	imulq	8(%rsp), %rcx
	addq	$1, %rdx
	movq	%rdx, (%r12)
	cmpq	%rcx, %rdx
	jnb	.L116
	movq	%rbx, %r13
	subq	0(%rbp), %r13
	movq	%rbx, %r15
	movq	%r13, %rcx
	sarq	$2, %rcx
	cmpq	$11, %rcx
	ja	.L116
	leaq	(%rdx,%rdx,2), %r8
	movl	20(%rsp), %r10d
	leaq	(%rdx,%r8,4), %rdx
	leaq	(%rsi,%rdx,8), %rdx
	movq	56(%rdx), %r8
	cmpl	%r10d, -4(%r8)
	jne	.L116
.L90:
	cmpq	%rax, %rbx
	movq	32(%rdx), %r14
	jne	.L118
	testq	%rcx, %rcx
	movq	$4, 24(%rsp)
	je	.L94
	leaq	(%rcx,%rcx), %rdx
	leaq	0(,%rcx,8), %rax
	movq	$-4, %rsi
	cmpq	%rdx, %rcx
	cmova	%rsi, %rax
	movq	%rax, 24(%rsp)
.L94:
	movq	24(%rsp), %rdi
.LEHB0:
	call	_Znwm
.LEHE0:
	movq	%rax, %rcx
	movl	-4(%r14), %eax
	movq	%r15, %r14
	movl	%eax, (%rcx,%r13)
	movq	0(%rbp), %r13
	subq	%r13, %r14
	cmpq	%r13, %r15
	je	.L95
	movq	%rcx, %rdi
	movq	%r14, %rdx
	movq	%r13, %rsi
	call	memmove
	movq	%rax, %rcx
.L95:
	movq	8(%rbp), %rax
	leaq	4(%rcx,%r14), %r8
	movq	%rax, %r14
	subq	%r15, %r14
	cmpq	%rax, %r15
	je	.L96
	movq	%r8, %rdi
	movq	%r14, %rdx
	movq	%rbx, %rsi
	movq	%rcx, 40(%rsp)
	call	memmove
	movq	40(%rsp), %rcx
	movq	%rax, %r8
.L96:
	testq	%r13, %r13
	leaq	(%r8,%r14), %rbx
	je	.L97
	movq	%r13, %rdi
	movq	%rcx, 40(%rsp)
	call	_ZdlPv
	movq	40(%rsp), %rcx
.L97:
	movq	24(%rsp), %rax
	movq	32(%rsp), %rdi
	movq	%rcx, 0(%rbp)
	movq	%rbx, 8(%rbp)
	addq	%rcx, %rax
	movq	(%rdi), %rsi
	movq	8(%rdi), %rdi
	movq	%rax, 16(%rbp)
	jmp	.L93
	.p2align 4,,10
	.p2align 3
.L116:
	addq	$56, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movq	%rbp, %rax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
.L105:
	.cfi_restore_state
	movq	0(%rbp), %rdi
	movq	%rax, %rbx
	testq	%rdi, %rdi
	je	.L115
	vzeroupper
	call	_ZdlPv
.L100:
	movq	%rbx, %rdi
.LEHB1:
	call	_Unwind_Resume
.LEHE1:
.L115:
	vzeroupper
	jmp	.L100
	.cfi_endproc
.LFE10373:
	.globl	__gxx_personality_v0
	.section	.gcc_except_table,"a",@progbits
.LLSDA10373:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE10373-.LLSDACSB10373
.LLSDACSB10373:
	.uleb128 .LEHB0-.LFB10373
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L105-.LFB10373
	.uleb128 0
	.uleb128 .LEHB1-.LFB10373
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
.LLSDACSE10373:
	.text
	.size	_Z19FormBlockOfXYHGatesRKSt6vectorI4GateSaIS0_EERmNS0_4TypeE, .-_Z19FormBlockOfXYHGatesRKSt6vectorI4GateSaIS0_EERmNS0_4TypeE
	.p2align 4,,15
	.globl	_Z20ApplyBlockOfCZTGatesiRKSt8valarrayImES2_PSt7complexIfEmS4_
	.type	_Z20ApplyBlockOfCZTGatesiRKSt8valarrayImES2_PSt7complexIfEmS4_, @function
_Z20ApplyBlockOfCZTGatesiRKSt8valarrayImES2_PSt7complexIfEmS4_:
.LFB10374:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	subl	$1, %edi
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	movslq	%edi, %rax
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movl	$1, %r13d
	vmovq	%xmm0, -8(%rsp)
	vxorps	%xmm0, %xmm0, %xmm0
	movq	%rax, -24(%rsp)
	vmovss	-8(%rsp), %xmm4
	vcomiss	%xmm0, %xmm4
	vmovss	-4(%rsp), %xmm5
	jne	.L120
	vcomiss	%xmm0, %xmm5
	setne	%r13b
.L120:
	movq	8(%rdx), %r12
	testq	%r8, %r8
	movq	(%r12), %r14
	je	.L143
	leaq	8(%r12), %rax
	movq	8(%rsi), %r15
	xorl	%r9d, %r9d
	movl	$64, %esi
	xorl	%ebx, %ebx
	vmovss	.LC1(%rip), %xmm6
	movq	%rax, -16(%rsp)
	jmp	.L128
	.p2align 4,,10
	.p2align 3
.L125:
	addq	$1, %r9
	vmovss	%xmm0, (%r11)
	cmpq	%r9, %r8
	vmovss	%xmm2, 4(%r11)
	je	.L143
	movq	%r9, %rsi
	shrq	%rsi
	xorq	%rdi, %rsi
	xorq	%r9, %rsi
	rep bsfq	%rsi, %rsi
.L128:
	movq	%r9, %rdi
	movq	-24(%rsp), %rax
	movslq	%esi, %rsi
	shrq	%rdi
	xorq	%r9, %rdi
	leaq	(%rcx,%rdi,8), %r11
	subq	%rsi, %rax
	testb	%r13b, %r13b
	vmovss	(%r11), %xmm0
	vmovss	4(%r11), %xmm2
	je	.L122
	vmulss	%xmm5, %xmm0, %xmm3
	vmulss	%xmm5, %xmm2, %xmm1
	vmulss	%xmm4, %xmm0, %xmm0
	vmulss	%xmm4, %xmm2, %xmm2
	vsubss	%xmm1, %xmm0, %xmm0
	vaddss	%xmm3, %xmm2, %xmm2
.L122:
	movq	(%r15,%rax,8), %rsi
	xorl	%eax, %eax
	andq	%rdi, %rsi
	popcntq	%rsi, %rax
	movl	%ebx, %esi
	xorl	$1, %esi
	testb	$1, %al
	cmovne	%esi, %ebx
	testb	%bl, %bl
	je	.L124
	vxorps	%xmm6, %xmm2, %xmm2
	vxorps	%xmm6, %xmm0, %xmm0
.L124:
	testq	%r14, %r14
	je	.L125
	movq	(%rdx), %rax
	leaq	0(,%rax,8), %rsi
	leaq	(%r12,%rsi), %rax
	cmpq	%rax, %r12
	je	.L125
	movq	-16(%rsp), %r10
	movq	%r12, %rax
	leaq	-8(%r10,%rsi), %rbp
	xorl	%r10d, %r10d
	.p2align 4,,10
	.p2align 3
.L127:
	movq	(%rax), %rsi
	addq	$8, %rax
	andq	%rdi, %rsi
	popcntq	%rsi, %rsi
	addq	%rsi, %r10
	cmpq	%rbp, %rax
	jne	.L127
	andl	$7, %r10d
	vmovss	_ZL6kTGate(,%r10,8), %xmm3
	vmovss	_ZL6kTGate+4(,%r10,8), %xmm1
	vmulss	%xmm1, %xmm0, %xmm7
	vmulss	%xmm1, %xmm2, %xmm1
	vmulss	%xmm3, %xmm0, %xmm0
	vmulss	%xmm3, %xmm2, %xmm2
	vsubss	%xmm1, %xmm0, %xmm0
	vaddss	%xmm7, %xmm2, %xmm2
	jmp	.L125
	.p2align 4,,10
	.p2align 3
.L143:
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE10374:
	.size	_Z20ApplyBlockOfCZTGatesiRKSt8valarrayImES2_PSt7complexIfEmS4_, .-_Z20ApplyBlockOfCZTGatesiRKSt8valarrayImES2_PSt7complexIfEmS4_
	.p2align 4,,15
	.globl	_Z16ApplyControlGateiRKSt6vectorIiSaIiEEPSt7complexIfEiRK4GateNS7_4TypeE
	.type	_Z16ApplyControlGateiRKSt6vectorIiSaIiEEPSt7complexIfEiRK4GateNS7_4TypeE, @function
_Z16ApplyControlGateiRKSt6vectorIiSaIiEEPSt7complexIfEiRK4GateNS7_4TypeE:
.LFB10376:
	.cfi_startproc
	.cfi_personality 0x3,__gxx_personality_v0
	.cfi_lsda 0x3,.LLSDA10376
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	.cfi_offset 15, -24
	.cfi_offset 14, -32
	.cfi_offset 13, -40
	.cfi_offset 12, -48
	movslq	%edi, %r15
	pushq	%r10
	pushq	%rbx
	leal	-1(%rcx), %r12d
	.cfi_offset 10, -56
	.cfi_offset 3, -64
	movq	%rdx, %rbx
	subq	$272, %rsp
	movq	%r8, -288(%rbp)
	movl	%r9d, -176(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -56(%rbp)
	xorl	%eax, %eax
	movl	%ecx, %eax
	movq	(%rsi), %r13
	subl	%r15d, %eax
	testl	%r15d, %r15d
	movl	%eax, -172(%rbp)
	jle	.L293
	leal	-1(%r15), %edx
	movq	%r13, %rax
	xorl	%r9d, %r9d
	movl	$1, %edi
	leaq	4(%r13,%rdx,4), %r8
	.p2align 4,,10
	.p2align 3
.L151:
	movl	%r12d, %ecx
	subl	(%rax), %ecx
	movl	%edi, %edx
	addq	$4, %rax
	sall	%cl, %edx
	movslq	%edx, %rdx
	orq	%rdx, %r9
	cmpq	%r8, %rax
	jne	.L151
	movq	%r9, -152(%rbp)
.L150:
	movq	8(%rsi), %rsi
	subq	%r13, %rsi
	movq	%rsi, %r8
	sarq	$2, %r8
	testq	%r8, %r8
	je	.L294
	movq	%r13, %rax
	addq	%r13, %rsi
	xorl	%edx, %edx
	movl	$1, %edi
	.p2align 4,,10
	.p2align 3
.L152:
	movl	%r12d, %ecx
	subl	(%rax), %ecx
	movq	%rdi, %r11
	addq	$4, %rax
	salq	%cl, %r11
	orq	%r11, %rdx
	cmpq	%rsi, %rax
	jne	.L152
	movq	%rdx, -144(%rbp)
.L149:
	movl	-172(%rbp), %esi
	movl	$1, %r11d
	movq	%r8, -192(%rbp)
	movq	%r11, %rax
	movq	%r11, -168(%rbp)
	movl	%esi, %ecx
	xorl	%esi, %esi
	salq	%cl, %rax
	leaq	22(,%rax,8), %r14
	leaq	-1(%rax), %rdi
	movq	%rax, -120(%rbp)
	movq	%r14, %rax
	movq	%rdi, -160(%rbp)
	andq	$-16, %rax
	subq	%rax, %rsp
	movl	$8, %eax
	leaq	7(%rsp), %r9
	salq	%cl, %rax
	movq	%rax, %rdx
	movq	%rax, -184(%rbp)
	shrq	$3, %r9
	leaq	0(,%r9,8), %rdi
	movq	%r9, -136(%rbp)
	movq	%rdi, -128(%rbp)
	call	memset
	movq	-192(%rbp), %r8
	movq	-168(%rbp), %r11
	movq	-136(%rbp), %r9
	leal	-1(%r8), %ecx
	movq	%r11, %rdi
	movq	%r15, 0(,%r9,8)
	salq	%cl, %rdi
	cmpq	%r8, %r15
	jnb	.L153
	leaq	1(%r15), %r10
	movl	$1, %edx
	leaq	1(%r8), %r15
	movl	$1, %r11d
	movq	%rdi, %r8
	movq	%rdi, %rsi
	.p2align 4,,10
	.p2align 3
.L156:
	movq	%r11, %r9
	movl	%r10d, %ecx
	salq	%cl, %r9
	cmpq	%r9, %rdx
	jnb	.L154
	movl	%r12d, %ecx
	subl	-4(%r13,%r10,4), %ecx
	movq	-128(%rbp), %rax
	movq	%r11, %rdi
	salq	$3, %rsi
	salq	%cl, %rdi
	.p2align 4,,10
	.p2align 3
.L155:
	movq	(%rax), %rcx
	addq	$1, %rdx
	addq	%rdi, %rcx
	movq	%rcx, (%rax,%r8,8)
	addq	%rsi, %rax
	cmpq	%r9, %rdx
	jne	.L155
.L154:
	movq	%r8, %rax
	addq	$1, %r10
	movq	%r8, %rsi
	shrq	%rax
	cmpq	%r10, %r15
	je	.L153
	movq	%rax, %r8
	jmp	.L156
	.p2align 4,,10
	.p2align 3
.L153:
	andq	$-16, %r14
	xorl	%esi, %esi
	subq	%r14, %rsp
	movq	-184(%rbp), %r14
	movq	%rsp, %r15
	movq	%r15, %rdi
	movq	%r14, %rdx
	call	memset
	movq	-120(%rbp), %rsi
	testq	%rsi, %rsi
	je	.L146
	movq	-128(%rbp), %r11
	movq	%r14, %rax
	movq	%rsi, %rdi
	sarq	$3, %rax
	xorl	%r12d, %r12d
	movq	$0, -136(%rbp)
	movq	%rax, -304(%rbp)
	movq	%r11, %rax
	shrq	$3, %rax
	movq	%rax, %rdx
	andl	$1, %edx
	subq	%rdx, %rdi
	movq	%rdx, -240(%rbp)
	movq	%rdi, %rax
	movq	%rdi, -232(%rbp)
	andq	$-2, %rdi
	shrq	%rax
	movq	%rdi, -216(%rbp)
	movq	%rax, -224(%rbp)
	leaq	0(,%rdx,8), %rax
	addq	%rdi, %rdx
	movq	%rdx, -272(%rbp)
	addq	%rax, %r11
	addq	%r15, %rax
	movq	%rax, -200(%rbp)
	movq	%rsi, %rax
	andq	$-4, %rsi
	shrq	$2, %rax
	movq	%r11, -208(%rbp)
	movq	%rsi, -256(%rbp)
	movq	%rax, -264(%rbp)
	movq	-160(%rbp), %rax
	subq	%rsi, %rax
	movq	%rax, -280(%rbp)
	leaq	0(,%rsi,8), %rax
	movq	%rax, -248(%rbp)
	leaq	18(%r14), %rax
	shrq	$4, %rax
	salq	$4, %rax
	movq	%rax, -192(%rbp)
	jmp	.L158
	.p2align 4,,10
	.p2align 3
.L296:
	movq	-152(%rbp), %rsi
	movq	%rsi, %rax
	andq	%r12, %rax
	cmpq	%rsi, %rax
	je	.L295
	addq	-152(%rbp), %r12
.L205:
	movq	-136(%rbp), %rdi
	cmpq	%rdi, -120(%rbp)
	jbe	.L276
.L158:
	movq	-144(%rbp), %r14
	andq	%r12, %r14
	je	.L296
	addq	%r14, %r12
	movq	-136(%rbp), %rdi
	cmpq	%rdi, -120(%rbp)
	ja	.L158
.L276:
	vzeroupper
.L146:
	movq	-56(%rbp), %rax
	xorq	%fs:40, %rax
	jne	.L297
	leaq	-48(%rbp), %rsp
	popq	%rbx
	popq	%r10
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	ret
	.p2align 4,,10
	.p2align 3
.L295:
	.cfi_restore_state
	addq	$2, -136(%rbp)
	cmpq	$2, -160(%rbp)
	jbe	.L213
	cmpq	$0, -240(%rbp)
	je	.L162
	movq	-128(%rbp), %rdi
	movq	(%rdi), %rax
	addq	%r12, %rax
	movq	%rax, (%r15)
.L162:
	movq	%r12, -168(%rbp)
	movq	-200(%rbp), %rcx
	xorl	%eax, %eax
	vmovddup	-168(%rbp), %xmm1
	movq	-208(%rbp), %rsi
	movq	-224(%rbp), %rdi
	xorl	%edx, %edx
	.p2align 4,,10
	.p2align 3
.L163:
	addq	$1, %rdx
	vpaddq	(%rsi,%rax), %xmm1, %xmm0
	vmovups	%xmm0, (%rcx,%rax)
	addq	$16, %rax
	cmpq	%rdx, %rdi
	ja	.L163
	movq	-232(%rbp), %rdi
	cmpq	%rdi, -216(%rbp)
	je	.L164
	movq	-272(%rbp), %rax
.L161:
	movq	-128(%rbp), %rdi
	movq	-120(%rbp), %rsi
	movq	(%rdi,%rax,8), %rdx
	addq	%r12, %rdx
	movq	%rdx, (%r15,%rax,8)
	leaq	1(%rax), %rdx
	cmpq	%rdx, %rsi
	jbe	.L165
	movq	(%rdi,%rdx,8), %rcx
	addq	$2, %rax
	addq	%r12, %rcx
	cmpq	%rax, %rsi
	movq	%rcx, (%r15,%rdx,8)
	jbe	.L165
	movq	(%rdi,%rax,8), %rdx
	addq	%r12, %rdx
	movq	%rdx, (%r15,%rax,8)
.L165:
	movq	%rsp, -296(%rbp)
	movq	-160(%rbp), %rax
	subq	-192(%rbp), %rsp
	testq	%rax, %rax
	movq	%rsp, -168(%rbp)
	js	.L166
	cmpq	$2, %rax
	jbe	.L214
.L210:
	movq	-168(%rbp), %rax
	vxorps	%xmm0, %xmm0, %xmm0
	movq	-264(%rbp), %rcx
	xorl	%edx, %edx
	.p2align 4,,10
	.p2align 3
.L168:
	addq	$1, %rdx
	vextractf128	$0x1, %ymm0, 16(%rax)
	vmovups	%xmm0, (%rax)
	addq	$32, %rax
	cmpq	%rdx, %rcx
	ja	.L168
	movq	-168(%rbp), %rax
	movq	-256(%rbp), %rsi
	addq	-248(%rbp), %rax
	cmpq	%rsi, -120(%rbp)
	je	.L166
	movq	-280(%rbp), %rdx
.L167:
	testq	%rdx, %rdx
	movl	$0x00000000, (%rax)
	movl	$0x00000000, 4(%rax)
	je	.L166
	cmpq	$1, %rdx
	movl	$0x00000000, 8(%rax)
	movl	$0x00000000, 12(%rax)
	je	.L166
	movl	$0x00000000, 16(%rax)
	movl	$0x00000000, 20(%rax)
.L166:
	movq	-120(%rbp), %rcx
	movq	-168(%rbp), %rdx
	xorl	%r13d, %r13d
	.p2align 4,,10
	.p2align 3
.L170:
	movq	(%r15,%r13,8), %rax
	vmovss	(%rbx,%rax,8), %xmm0
	vmovss	%xmm0, (%rdx,%r13,8)
	vmovss	4(%rbx,%rax,8), %xmm0
	vmovss	%xmm0, 4(%rdx,%r13,8)
	addq	$1, %r13
	cmpq	%r13, %rcx
	jne	.L170
	movl	-172(%rbp), %eax
	testl	%eax, %eax
	je	.L171
	movq	(%r15), %rax
	vxorps	%xmm0, %xmm0, %xmm0
	leaq	(%rbx,%rax,8), %rax
	vmovss	(%rax), %xmm1
	vcomiss	%xmm0, %xmm1
	jne	.L172
	vcomiss	4(%rax), %xmm0
	jne	.L172
	movq	8(%r15), %rdx
	leaq	(%rbx,%rdx,8), %rdx
	vcomiss	(%rdx), %xmm0
	jne	.L172
	vcomiss	4(%rdx), %xmm0
	jne	.L172
.L171:
	movq	-168(%rbp), %rax
	movq	-304(%rbp), %rsi
	addq	-184(%rbp), %rax
	vpxor	%xmm0, %xmm0, %xmm0
	movq	$0, -96(%rbp)
	testq	%rsi, %rsi
	vmovaps	%xmm0, -112(%rbp)
	movq	%rax, -312(%rbp)
	je	.L215
	movabsq	$2305843009213693951, %rax
	cmpq	%rax, %rsi
	ja	.L298
	movq	-184(%rbp), %rdi
	vzeroupper
.LEHB2:
	call	_Znwm
.LEHE2:
	movq	%rax, %rcx
.L184:
	movq	-184(%rbp), %rdi
	movq	-168(%rbp), %rsi
	movq	%rcx, -112(%rbp)
	leaq	(%rcx,%rdi), %rax
	movq	%rax, -96(%rbp)
	movq	-312(%rbp), %rax
	cmpq	%rax, %rsi
	je	.L186
	leaq	-8(%rdi), %rax
	movq	%rcx, %rdi
	shrq	$3, %rax
	leaq	8(,%rax,8), %r8
	movq	%r8, %rdx
	movq	%r8, -312(%rbp)
	call	memcpy
	movq	-312(%rbp), %r8
	movq	%rax, %rcx
	addq	%r8, %rcx
.L186:
	movq	-288(%rbp), %rsi
	leaq	-112(%rbp), %rdx
	leaq	-80(%rbp), %rdi
	movq	%rcx, -104(%rbp)
.LEHB3:
	call	_Z13matrix_v_multRKSt6vectorIS_ISt7complexIfESaIS1_EESaIS3_EERKS3_
.LEHE3:
	movq	-72(%rbp), %rax
	movq	-112(%rbp), %rdi
	movq	-80(%rbp), %rdx
	vpxor	%xmm0, %xmm0, %xmm0
	movq	%rax, -104(%rbp)
	movq	-64(%rbp), %rax
	testq	%rdi, %rdi
	movq	%rdx, -112(%rbp)
	movq	$0, -64(%rbp)
	vmovaps	%xmm0, -80(%rbp)
	movq	%rax, -96(%rbp)
	je	.L203
	call	_ZdlPv
	movq	-80(%rbp), %rdi
	testq	%rdi, %rdi
	je	.L284
	call	_ZdlPv
.L284:
	movq	-112(%rbp), %rdx
.L203:
	movq	-168(%rbp), %rcx
	.p2align 4,,10
	.p2align 3
.L272:
	movq	(%r15,%r14,8), %rax
	vmovss	(%rcx,%r14,8), %xmm0
	vmovss	%xmm0, (%rbx,%rax,8)
	vmovss	4(%rcx,%r14,8), %xmm0
	addq	$1, %r14
	cmpq	%r14, %r13
	vmovss	%xmm0, 4(%rbx,%rax,8)
	jne	.L272
.L287:
	testq	%rdx, %rdx
	je	.L190
	movq	%rdx, %rdi
	call	_ZdlPv
	jmp	.L190
	.p2align 4,,10
	.p2align 3
.L172:
	movl	-176(%rbp), %esi
	cmpl	$14, %esi
	ja	.L176
	movl	%esi, %edx
	jmp	*.L178(,%rdx,8)
	.section	.rodata
	.align 8
	.align 4
.L178:
	.quad	.L176
	.quad	.L177
	.quad	.L179
	.quad	.L180
	.quad	.L176
	.quad	.L176
	.quad	.L176
	.quad	.L176
	.quad	.L176
	.quad	.L176
	.quad	.L176
	.quad	.L181
	.quad	.L176
	.quad	.L182
	.quad	.L183
	.text
.L164:
	movq	%rsp, -296(%rbp)
	subq	-192(%rbp), %rsp
	cmpq	$0, -160(%rbp)
	movq	%rsp, -168(%rbp)
	jns	.L210
	jmp	.L166
.L183:
	movq	-168(%rbp), %rdi
	vmovss	(%rdi), %xmm5
	vmovss	4(%rdi), %xmm1
	vsubss	%xmm1, %xmm5, %xmm0
	vmovss	8(%rdi), %xmm3
	vaddss	%xmm5, %xmm1, %xmm1
	vmovss	12(%rdi), %xmm4
	vaddss	%xmm4, %xmm3, %xmm2
	vsubss	%xmm3, %xmm0, %xmm5
	vsubss	%xmm4, %xmm0, %xmm0
	vaddss	%xmm4, %xmm5, %xmm5
	vaddss	%xmm3, %xmm0, %xmm0
	vmovss	%xmm5, (%rax)
	vsubss	%xmm2, %xmm1, %xmm5
	vaddss	%xmm1, %xmm2, %xmm1
	vmovss	%xmm5, 4(%rax)
	movq	8(%r15), %rax
	leaq	(%rbx,%rax,8), %rax
	vmovss	%xmm0, (%rax)
	vmovss	%xmm1, 4(%rax)
	.p2align 4,,10
	.p2align 3
.L190:
	addq	$1, %r12
	movq	-296(%rbp), %rsp
	jmp	.L205
.L182:
	movq	-168(%rbp), %rsi
	vmovss	8(%rsi), %xmm4
	vmovss	12(%rsi), %xmm5
	vaddss	%xmm4, %xmm5, %xmm0
	vmovss	4(%rsi), %xmm2
	vmovss	(%rsi), %xmm3
	vaddss	%xmm2, %xmm3, %xmm1
	vsubss	%xmm2, %xmm0, %xmm6
	vsubss	%xmm3, %xmm0, %xmm0
	vaddss	%xmm3, %xmm6, %xmm6
	vaddss	%xmm2, %xmm0, %xmm0
	vmovss	%xmm6, (%rax)
	vsubss	%xmm4, %xmm1, %xmm6
	vsubss	%xmm5, %xmm1, %xmm1
	vaddss	%xmm5, %xmm6, %xmm6
	vaddss	%xmm4, %xmm1, %xmm1
	vmovss	%xmm6, 4(%rax)
	movq	8(%r15), %rax
	leaq	(%rbx,%rax,8), %rax
	vmovss	%xmm1, (%rax)
	vmovss	%xmm0, 4(%rax)
	jmp	.L190
.L181:
	movq	8(%r15), %rax
	vmovss	.LC2(%rip), %xmm0
	leaq	(%rbx,%rax,8), %rax
	vmulss	(%rax), %xmm0, %xmm1
	vmulss	4(%rax), %xmm0, %xmm0
	vsubss	%xmm0, %xmm1, %xmm2
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm2, (%rax)
	vmovss	%xmm0, 4(%rax)
	jmp	.L190
.L180:
	movq	8(%r15), %rax
	vmovss	.LC1(%rip), %xmm2
	leaq	(%rbx,%rax,8), %rax
	vmovss	4(%rax), %xmm0
	vmovss	(%rax), %xmm1
	vxorps	%xmm2, %xmm0, %xmm0
	vxorps	%xmm2, %xmm1, %xmm1
	vmovss	%xmm0, 4(%rax)
	vmovss	%xmm1, (%rax)
	jmp	.L190
.L179:
	movq	-168(%rbp), %rsi
	vmovss	8(%rsi), %xmm0
	vxorps	.LC1(%rip), %xmm0, %xmm0
	vmovss	12(%rsi), %xmm1
	vmovss	%xmm1, (%rax)
	vmovss	(%rsi), %xmm1
	vmovss	%xmm0, 4(%rax)
	movq	8(%r15), %rax
	vmovss	4(%rsi), %xmm0
	vxorps	.LC1(%rip), %xmm0, %xmm0
	leaq	(%rbx,%rax,8), %rax
	vmovss	%xmm1, 4(%rax)
	vmovss	%xmm0, (%rax)
	jmp	.L190
.L177:
	movq	8(%r15), %rdx
	vmovss	4(%rax), %xmm0
	leaq	(%rbx,%rdx,8), %rdx
	vmovss	(%rdx), %xmm2
	vmovss	%xmm2, (%rax)
	vmovss	4(%rdx), %xmm2
	vmovss	%xmm2, 4(%rax)
	vmovss	%xmm1, (%rdx)
	vmovss	%xmm0, 4(%rdx)
	jmp	.L190
.L176:
	movq	-168(%rbp), %rax
	movq	-304(%rbp), %rdi
	addq	-184(%rbp), %rax
	vpxor	%xmm0, %xmm0, %xmm0
	movq	$0, -96(%rbp)
	testq	%rdi, %rdi
	vmovaps	%xmm0, -112(%rbp)
	movq	%rax, -312(%rbp)
	je	.L216
	movabsq	$2305843009213693951, %rax
	cmpq	%rax, %rdi
	ja	.L299
	movq	-184(%rbp), %rdi
	vzeroupper
.LEHB4:
	call	_Znwm
.LEHE4:
	movq	%rax, %rcx
.L191:
	movq	-184(%rbp), %rdi
	movq	-168(%rbp), %rsi
	movq	%rcx, -112(%rbp)
	leaq	(%rcx,%rdi), %rax
	movq	%rax, -96(%rbp)
	movq	-312(%rbp), %rax
	cmpq	%rax, %rsi
	je	.L193
	leaq	-8(%rdi), %rax
	movq	%rcx, %rdi
	shrq	$3, %rax
	leaq	8(,%rax,8), %r8
	movq	%r8, %rdx
	movq	%r8, -312(%rbp)
	call	memcpy
	movq	-312(%rbp), %r8
	movq	%rax, %rcx
	addq	%r8, %rcx
.L193:
	movq	-288(%rbp), %rsi
	leaq	-112(%rbp), %rdx
	leaq	-80(%rbp), %rdi
	movq	%rcx, -104(%rbp)
.LEHB5:
	call	_Z13matrix_v_multRKSt6vectorIS_ISt7complexIfESaIS1_EESaIS3_EERKS3_
.LEHE5:
	movq	-72(%rbp), %rax
	movq	-112(%rbp), %rdi
	movq	-80(%rbp), %rdx
	vpxor	%xmm0, %xmm0, %xmm0
	movq	%rax, -104(%rbp)
	movq	-64(%rbp), %rax
	testq	%rdi, %rdi
	movq	%rdx, -112(%rbp)
	movq	$0, -64(%rbp)
	vmovaps	%xmm0, -80(%rbp)
	movq	%rax, -96(%rbp)
	je	.L199
	call	_ZdlPv
	movq	-80(%rbp), %rdi
	testq	%rdi, %rdi
	je	.L283
	call	_ZdlPv
.L283:
	movq	-112(%rbp), %rdx
.L199:
	movq	-168(%rbp), %rcx
	.p2align 4,,10
	.p2align 3
.L273:
	movq	(%r15,%r14,8), %rax
	vmovss	(%rcx,%r14,8), %xmm0
	vmovss	%xmm0, (%rbx,%rax,8)
	vmovss	4(%rcx,%r14,8), %xmm0
	addq	$1, %r14
	cmpq	%r14, %r13
	vmovss	%xmm0, 4(%rbx,%rax,8)
	jne	.L273
	jmp	.L287
.L293:
	movq	$0, -152(%rbp)
	jmp	.L150
.L294:
	movq	$0, -144(%rbp)
	jmp	.L149
.L213:
	xorl	%eax, %eax
	jmp	.L161
.L214:
	movq	%rsp, %rax
	movq	-160(%rbp), %rdx
	jmp	.L167
.L215:
	xorl	%ecx, %ecx
	vzeroupper
	jmp	.L184
.L216:
	xorl	%ecx, %ecx
	vzeroupper
	jmp	.L191
.L217:
.L290:
	movq	-112(%rbp), %rdi
	movq	%rax, %rbx
	testq	%rdi, %rdi
	je	.L280
	vzeroupper
	call	_ZdlPv
.L209:
	movq	%rbx, %rdi
.LEHB6:
	call	_Unwind_Resume
.LEHE6:
.L220:
	jmp	.L290
.L299:
	vzeroupper
.LEHB7:
	call	_ZSt17__throw_bad_allocv
.LEHE7:
.L280:
	vzeroupper
	jmp	.L209
.L218:
	jmp	.L290
.L297:
	call	__stack_chk_fail
.L219:
	jmp	.L290
.L298:
	vzeroupper
.LEHB8:
	call	_ZSt17__throw_bad_allocv
.LEHE8:
	.cfi_endproc
.LFE10376:
	.section	.gcc_except_table
.LLSDA10376:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE10376-.LLSDACSB10376
.LLSDACSB10376:
	.uleb128 .LEHB2-.LFB10376
	.uleb128 .LEHE2-.LEHB2
	.uleb128 .L219-.LFB10376
	.uleb128 0
	.uleb128 .LEHB3-.LFB10376
	.uleb128 .LEHE3-.LEHB3
	.uleb128 .L217-.LFB10376
	.uleb128 0
	.uleb128 .LEHB4-.LFB10376
	.uleb128 .LEHE4-.LEHB4
	.uleb128 .L220-.LFB10376
	.uleb128 0
	.uleb128 .LEHB5-.LFB10376
	.uleb128 .LEHE5-.LEHB5
	.uleb128 .L218-.LFB10376
	.uleb128 0
	.uleb128 .LEHB6-.LFB10376
	.uleb128 .LEHE6-.LEHB6
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB7-.LFB10376
	.uleb128 .LEHE7-.LEHB7
	.uleb128 .L220-.LFB10376
	.uleb128 0
	.uleb128 .LEHB8-.LFB10376
	.uleb128 .LEHE8-.LEHB8
	.uleb128 .L219-.LFB10376
	.uleb128 0
.LLSDACSE10376:
	.text
	.size	_Z16ApplyControlGateiRKSt6vectorIiSaIiEEPSt7complexIfEiRK4GateNS7_4TypeE, .-_Z16ApplyControlGateiRKSt6vectorIiSaIiEEPSt7complexIfEiRK4GateNS7_4TypeE
	.p2align 4,,15
	.globl	_Z22ApplyNonControl1QGatesiPSt7complexIfEmiRK4GateNS2_4TypeE
	.type	_Z22ApplyNonControl1QGatesiPSt7complexIfEmiRK4GateNS2_4TypeE, @function
_Z22ApplyNonControl1QGatesiPSt7complexIfEmiRK4GateNS2_4TypeE:
.LFB10377:
	.cfi_startproc
	.cfi_personality 0x3,__gxx_personality_v0
	.cfi_lsda 0x3,.LLSDA10377
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	vpxor	%xmm0, %xmm0, %xmm0
	subl	$1, %ecx
	shrq	%rdx
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	subl	%edi, %ecx
	pushq	%rbx
	.cfi_offset 15, -24
	.cfi_offset 14, -32
	.cfi_offset 13, -40
	.cfi_offset 12, -48
	.cfi_offset 3, -56
	movq	%rdx, %r12
	xorl	%ebx, %ebx
	xorl	%r13d, %r13d
	subq	$168, %rsp
	movq	%rsi, -160(%rbp)
	movq	%r8, -200(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -56(%rbp)
	xorl	%eax, %eax
	vmovaps	%xmm0, -80(%rbp)
	movl	$1, %eax
	movl	%r9d, -172(%rbp)
	salq	%cl, %rax
	movq	%rax, -152(%rbp)
.L369:
	cmpq	%r12, %r13
	jnb	.L300
.L387:
	movq	-152(%rbp), %rax
	andq	%rbx, %rax
	je	.L386
	addq	%rax, %rbx
	cmpq	%r12, %r13
	jb	.L387
.L300:
	movq	-56(%rbp), %rax
	xorq	%fs:40, %rax
	jne	.L388
	leaq	-40(%rbp), %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	ret
	.p2align 4,,10
	.p2align 3
.L386:
	.cfi_restore_state
	movq	-160(%rbp), %rsi
	movq	-152(%rbp), %rax
	addq	$1, %r13
	vxorps	%xmm0, %xmm0, %xmm0
	movq	%rbx, -80(%rbp)
	movq	%rsp, -168(%rbp)
	leaq	(%rsi,%rbx,8), %r15
	addq	%rbx, %rax
	leaq	(%rsi,%rax,8), %r14
	movq	%rax, -72(%rbp)
	vmovss	(%r15), %xmm4
	vcomiss	%xmm0, %xmm4
	vmovss	4(%r15), %xmm5
	vmovss	(%r14), %xmm6
	vmovss	4(%r14), %xmm7
	vmovss	%xmm4, -176(%rbp)
	vmovss	%xmm5, -180(%rbp)
	vmovss	%xmm6, -184(%rbp)
	vmovss	%xmm7, -188(%rbp)
	jne	.L304
	vcomiss	%xmm5, %xmm0
	jne	.L304
	vcomiss	%xmm6, %xmm0
	jne	.L304
	vcomiss	%xmm7, %xmm0
	jne	.L304
	vpxor	%xmm0, %xmm0, %xmm0
	movl	$16, %edi
	movq	$0, -128(%rbp)
	vmovaps	%xmm0, -144(%rbp)
.LEHB9:
	call	_Znwm
.LEHE9:
	leaq	16(%rax), %rdx
	leaq	-112(%rbp), %rdi
	vmovss	-176(%rbp), %xmm1
	movq	-200(%rbp), %rsi
	vmovss	%xmm1, (%rax)
	movq	%rdx, -128(%rbp)
	movq	%rdx, -136(%rbp)
	vmovss	-180(%rbp), %xmm2
	leaq	-144(%rbp), %rdx
	vmovss	-184(%rbp), %xmm3
	vmovss	-188(%rbp), %xmm1
	movq	%rax, -144(%rbp)
	vmovss	%xmm2, 4(%rax)
	vmovss	%xmm3, 8(%rax)
	vmovss	%xmm1, 12(%rax)
.LEHB10:
	call	_Z13matrix_v_multRKSt6vectorIS_ISt7complexIfESaIS1_EESaIS3_EERKS3_
.LEHE10:
	movq	-104(%rbp), %rdx
	movq	-144(%rbp), %rdi
	movq	-112(%rbp), %rax
	vpxor	%xmm0, %xmm0, %xmm0
	movq	%rdx, -136(%rbp)
	movq	-96(%rbp), %rdx
	testq	%rdi, %rdi
	movq	%rax, -144(%rbp)
	movq	$0, -96(%rbp)
	vmovaps	%xmm0, -112(%rbp)
	movq	%rdx, -128(%rbp)
	je	.L327
	call	_ZdlPv
	movq	-112(%rbp), %rdi
	testq	%rdi, %rdi
	je	.L379
	call	_ZdlPv
.L379:
	movq	-144(%rbp), %rax
.L327:
	vmovss	-176(%rbp), %xmm2
	testq	%rax, %rax
	vmovss	%xmm2, (%r15)
	vmovss	-180(%rbp), %xmm3
	vmovss	-184(%rbp), %xmm1
	vmovss	-188(%rbp), %xmm2
	vmovss	%xmm3, 4(%r15)
	vmovss	%xmm1, (%r14)
	vmovss	%xmm2, 4(%r14)
	jne	.L380
	jmp	.L320
	.p2align 4,,10
	.p2align 3
.L304:
	movl	-172(%rbp), %eax
	cmpl	$14, %eax
	ja	.L309
	jmp	*.L311(,%rax,8)
	.section	.rodata
	.align 8
	.align 4
.L311:
	.quad	.L309
	.quad	.L310
	.quad	.L312
	.quad	.L313
	.quad	.L309
	.quad	.L309
	.quad	.L309
	.quad	.L309
	.quad	.L309
	.quad	.L309
	.quad	.L309
	.quad	.L314
	.quad	.L309
	.quad	.L315
	.quad	.L316
	.text
	.p2align 4,,10
	.p2align 3
.L316:
	vmovss	-176(%rbp), %xmm3
	vmovss	-180(%rbp), %xmm7
	vsubss	%xmm7, %xmm3, %xmm0
	vmovss	-188(%rbp), %xmm5
	vmovss	-184(%rbp), %xmm6
	vaddss	%xmm3, %xmm7, %xmm3
	vaddss	%xmm6, %xmm5, %xmm1
	vaddss	%xmm5, %xmm0, %xmm2
	vaddss	%xmm6, %xmm0, %xmm0
	vsubss	%xmm6, %xmm2, %xmm2
	vsubss	%xmm5, %xmm0, %xmm0
	vmovss	%xmm2, (%r15)
	vsubss	%xmm1, %xmm3, %xmm2
	vaddss	%xmm3, %xmm1, %xmm1
	vmovss	%xmm2, 4(%r15)
	vmovss	%xmm0, (%r14)
	vmovss	%xmm1, 4(%r14)
	.p2align 4,,10
	.p2align 3
.L320:
	addq	$1, %rbx
	movq	-168(%rbp), %rsp
	jmp	.L369
	.p2align 4,,10
	.p2align 3
.L310:
	vmovss	(%r14), %xmm1
	vmovss	%xmm1, (%r15)
	vmovss	4(%r15), %xmm0
	vmovss	4(%r14), %xmm1
	vmovss	-176(%rbp), %xmm2
	vmovss	%xmm1, 4(%r15)
	vmovss	%xmm2, (%r14)
	vmovss	%xmm0, 4(%r14)
	jmp	.L320
	.p2align 4,,10
	.p2align 3
.L312:
	vmovss	-184(%rbp), %xmm0
	vxorps	.LC1(%rip), %xmm0, %xmm0
	vmovss	-188(%rbp), %xmm2
	vmovss	%xmm2, (%r15)
	vmovss	-176(%rbp), %xmm2
	vmovss	%xmm0, 4(%r15)
	vmovss	-180(%rbp), %xmm0
	vmovss	%xmm2, 4(%r14)
	vxorps	.LC1(%rip), %xmm0, %xmm0
	vmovss	%xmm0, (%r14)
	jmp	.L320
	.p2align 4,,10
	.p2align 3
.L313:
	vmovss	4(%r14), %xmm0
	vmovss	(%r14), %xmm1
	vxorps	.LC1(%rip), %xmm0, %xmm0
	vxorps	.LC1(%rip), %xmm1, %xmm1
	vmovss	%xmm0, 4(%r14)
	vmovss	%xmm1, (%r14)
	jmp	.L320
	.p2align 4,,10
	.p2align 3
.L314:
	vmovss	.LC2(%rip), %xmm0
	vmulss	(%r14), %xmm0, %xmm1
	vmulss	4(%r14), %xmm0, %xmm0
	vsubss	%xmm0, %xmm1, %xmm2
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm2, (%r14)
	vmovss	%xmm0, 4(%r14)
	jmp	.L320
	.p2align 4,,10
	.p2align 3
.L315:
	vmovss	-184(%rbp), %xmm3
	vmovss	-188(%rbp), %xmm5
	vmovss	-176(%rbp), %xmm6
	vaddss	%xmm5, %xmm3, %xmm2
	vmovss	-180(%rbp), %xmm7
	vsubss	%xmm7, %xmm6, %xmm1
	vaddss	%xmm7, %xmm6, %xmm0
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm1, (%r15)
	vaddss	%xmm5, %xmm0, %xmm1
	vaddss	%xmm3, %xmm0, %xmm0
	vsubss	%xmm3, %xmm1, %xmm1
	vsubss	%xmm5, %xmm0, %xmm0
	vmovss	%xmm1, 4(%r15)
	vmovss	%xmm0, (%r14)
	vsubss	%xmm6, %xmm7, %xmm0
	vaddss	%xmm2, %xmm0, %xmm0
	vmovss	%xmm0, 4(%r14)
	jmp	.L320
	.p2align 4,,10
	.p2align 3
.L309:
	vpxor	%xmm0, %xmm0, %xmm0
	movl	$16, %edi
	movq	$0, -128(%rbp)
	vmovaps	%xmm0, -144(%rbp)
.LEHB11:
	call	_Znwm
.LEHE11:
	leaq	16(%rax), %rdx
	leaq	-112(%rbp), %rdi
	vmovss	-176(%rbp), %xmm2
	movq	-200(%rbp), %rsi
	vmovss	%xmm2, (%rax)
	movq	%rdx, -128(%rbp)
	movq	%rdx, -136(%rbp)
	vmovss	-180(%rbp), %xmm3
	leaq	-144(%rbp), %rdx
	vmovss	-184(%rbp), %xmm1
	vmovss	-188(%rbp), %xmm2
	movq	%rax, -144(%rbp)
	vmovss	%xmm3, 4(%rax)
	vmovss	%xmm1, 8(%rax)
	vmovss	%xmm2, 12(%rax)
.LEHB12:
	call	_Z13matrix_v_multRKSt6vectorIS_ISt7complexIfESaIS1_EESaIS3_EERKS3_
.LEHE12:
	movq	-104(%rbp), %rdx
	movq	-144(%rbp), %rdi
	movq	-112(%rbp), %rax
	vpxor	%xmm0, %xmm0, %xmm0
	movq	%rdx, -136(%rbp)
	movq	-96(%rbp), %rdx
	testq	%rdi, %rdi
	movq	%rax, -144(%rbp)
	movq	$0, -96(%rbp)
	vmovaps	%xmm0, -112(%rbp)
	movq	%rdx, -128(%rbp)
	je	.L324
	call	_ZdlPv
	movq	-112(%rbp), %rdi
	testq	%rdi, %rdi
	je	.L378
	call	_ZdlPv
.L378:
	movq	-144(%rbp), %rax
.L324:
	vmovss	-176(%rbp), %xmm3
	testq	%rax, %rax
	vmovss	%xmm3, (%r15)
	vmovss	-180(%rbp), %xmm1
	vmovss	-184(%rbp), %xmm2
	vmovss	-188(%rbp), %xmm3
	vmovss	%xmm1, 4(%r15)
	vmovss	%xmm2, (%r14)
	vmovss	%xmm3, 4(%r14)
	je	.L320
.L380:
	movq	%rax, %rdi
	call	_ZdlPv
	jmp	.L320
.L388:
	call	__stack_chk_fail
.L336:
.L383:
	movq	-144(%rbp), %rdi
	movq	%rax, %rbx
	testq	%rdi, %rdi
	je	.L375
	vzeroupper
	call	_ZdlPv
.L334:
	movq	%rbx, %rdi
.LEHB13:
	call	_Unwind_Resume
.LEHE13:
.L338:
	jmp	.L383
.L375:
	vzeroupper
	jmp	.L334
.L337:
	jmp	.L383
.L339:
	jmp	.L383
	.cfi_endproc
.LFE10377:
	.section	.gcc_except_table
.LLSDA10377:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE10377-.LLSDACSB10377
.LLSDACSB10377:
	.uleb128 .LEHB9-.LFB10377
	.uleb128 .LEHE9-.LEHB9
	.uleb128 .L338-.LFB10377
	.uleb128 0
	.uleb128 .LEHB10-.LFB10377
	.uleb128 .LEHE10-.LEHB10
	.uleb128 .L336-.LFB10377
	.uleb128 0
	.uleb128 .LEHB11-.LFB10377
	.uleb128 .LEHE11-.LEHB11
	.uleb128 .L339-.LFB10377
	.uleb128 0
	.uleb128 .LEHB12-.LFB10377
	.uleb128 .LEHE12-.LEHB12
	.uleb128 .L337-.LFB10377
	.uleb128 0
	.uleb128 .LEHB13-.LFB10377
	.uleb128 .LEHE13-.LEHB13
	.uleb128 0
	.uleb128 0
.LLSDACSE10377:
	.text
	.size	_Z22ApplyNonControl1QGatesiPSt7complexIfEmiRK4GateNS2_4TypeE, .-_Z22ApplyNonControl1QGatesiPSt7complexIfEmiRK4GateNS2_4TypeE
	.p2align 4,,15
	.globl	_Z17ApplyManyXOnSlicemPSt7complexIfE
	.type	_Z17ApplyManyXOnSlicemPSt7complexIfE, @function
_Z17ApplyManyXOnSlicemPSt7complexIfE:
.LFB10380:
	.cfi_startproc
	leaq	8(%rsp), %r10
	.cfi_def_cfa 10, 0
	andq	$-32, %rsp
	testq	%rdi, %rdi
	pushq	-8(%r10)
	pushq	%rbp
	.cfi_escape 0x10,0x6,0x2,0x76,0
	movq	%rsp, %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%r10
	.cfi_escape 0xf,0x3,0x76,0x58,0x6
	.cfi_escape 0x10,0xf,0x2,0x76,0x78
	.cfi_escape 0x10,0xe,0x2,0x76,0x70
	.cfi_escape 0x10,0xd,0x2,0x76,0x68
	.cfi_escape 0x10,0xc,0x2,0x76,0x60
	pushq	%rbx
	.cfi_escape 0x10,0x3,0x2,0x76,0x50
	movq	%rdi, -96(%rbp)
	je	.L420
	movl	$1, %eax
	movl	%edi, %ecx
	sall	%cl, %eax
	cltq
	testq	%rax, %rax
	movq	%rax, -56(%rbp)
	je	.L420
	movl	$32, %eax
	movq	%rsi, %r15
	movq	$0, -80(%rbp)
	subq	%rsi, %rax
	movq	%rax, -64(%rbp)
	.p2align 4,,10
	.p2align 3
.L401:
	movq	-80(%rbp), %rdi
	movl	$1, %ebx
	movl	$1, %eax
	leal	1(%rdi), %ecx
	sall	%cl, %ebx
	movl	%edi, %ecx
	sall	%cl, %eax
	movslq	%ebx, %rbx
	movslq	%eax, %rcx
	testq	%rcx, %rcx
	je	.L392
	leaq	0(,%rcx,8), %rsi
	leaq	-1(%rcx), %rax
	movq	%rcx, %r8
	movq	%rcx, %r12
	leaq	0(,%rbx,8), %r11
	shrq	$2, %r8
	movq	%rsi, %r13
	leaq	(%r15,%rsi), %rdx
	addq	$4, %rsi
	subq	%r15, %r13
	movq	%rax, -72(%rbp)
	andq	$-4, %r12
	leaq	32(%r13), %r14
	movq	%r15, %rax
	xorl	%edi, %edi
	movq	%rsi, -88(%rbp)
	.p2align 4,,10
	.p2align 3
.L397:
	movq	-64(%rbp), %rsi
	leaq	(%rax,%r13), %r9
	leaq	0(,%rdi,8), %r10
	addq	%rax, %rsi
	cmpq	%rsi, %r9
	leaq	(%r14,%rax), %r9
	setge	%sil
	cmpq	%r9, %r10
	setge	%r9b
	orb	%r9b, %sil
	je	.L393
	cmpq	$20, %rcx
	jbe	.L393
	xorl	%esi, %esi
	cmpq	$2, -72(%rbp)
	jbe	.L394
	xorl	%r9d, %r9d
	.p2align 4,,10
	.p2align 3
.L396:
	vmovups	(%rdx,%rsi), %xmm0
	addq	$1, %r9
	vinsertf128	$0x1, 16(%rdx,%rsi), %ymm0, %ymm0
	vmovups	(%rax,%rsi), %xmm4
	vinsertf128	$0x1, 16(%rax,%rsi), %ymm4, %ymm4
	vpermilps	$160, %ymm0, %ymm6
	vpermilps	$245, %ymm0, %ymm7
	vpermilps	$177, %ymm0, %ymm1
	vpermilps	$177, %ymm4, %ymm5
	vaddps	%ymm6, %ymm7, %ymm8
	vpermilps	$160, %ymm4, %ymm2
	vpermilps	$245, %ymm4, %ymm3
	vsubps	%ymm6, %ymm7, %ymm6
	vaddsubps	%ymm5, %ymm4, %ymm4
	vaddsubps	%ymm1, %ymm0, %ymm0
	vblendps	$170, %ymm6, %ymm8, %ymm6
	vaddps	%ymm4, %ymm6, %ymm4
	vmovups	%xmm4, (%rax,%rsi)
	vextractf128	$0x1, %ymm4, 16(%rax,%rsi)
	vaddps	%ymm2, %ymm3, %ymm4
	vsubps	%ymm2, %ymm3, %ymm2
	vblendps	$170, %ymm2, %ymm4, %ymm2
	vaddps	%ymm0, %ymm2, %ymm0
	vmovups	%xmm0, (%rdx,%rsi)
	vextractf128	$0x1, %ymm0, 16(%rdx,%rsi)
	addq	$32, %rsi
	cmpq	%r9, %r8
	ja	.L396
	cmpq	%r12, %rcx
	je	.L399
	movq	%r12, %rsi
.L394:
	leaq	(%rdi,%rsi), %r9
	leaq	(%r15,%r9,8), %r10
	addq	%rcx, %r9
	leaq	(%r15,%r9,8), %r9
	vmovss	(%r10), %xmm0
	vmovss	(%r9), %xmm4
	vmovss	4(%r9), %xmm3
	vmovss	4(%r10), %xmm1
	vaddss	%xmm4, %xmm3, %xmm2
	vsubss	%xmm1, %xmm0, %xmm6
	vaddss	%xmm0, %xmm1, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm2, %xmm6, %xmm6
	vaddss	%xmm2, %xmm0, %xmm2
	vmovss	%xmm6, (%r10)
	vaddss	%xmm5, %xmm3, %xmm6
	vsubss	%xmm4, %xmm6, %xmm6
	vaddss	%xmm5, %xmm4, %xmm4
	vmovss	%xmm6, 4(%r10)
	vsubss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm2, 4(%r9)
	vmovss	%xmm3, (%r9)
	leaq	1(%rsi), %r9
	cmpq	%r9, %rcx
	jbe	.L399
	addq	%rdi, %r9
	addq	$2, %rsi
	leaq	(%r15,%r9,8), %r10
	addq	%rcx, %r9
	cmpq	%rsi, %rcx
	leaq	(%r15,%r9,8), %r9
	vmovss	(%r10), %xmm0
	vmovss	4(%r10), %xmm1
	vmovss	(%r9), %xmm4
	vsubss	%xmm1, %xmm0, %xmm6
	vmovss	4(%r9), %xmm3
	vaddss	%xmm1, %xmm0, %xmm5
	vaddss	%xmm3, %xmm4, %xmm2
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm2, %xmm6, %xmm6
	vaddss	%xmm2, %xmm0, %xmm2
	vmovss	%xmm6, (%r10)
	vaddss	%xmm5, %xmm3, %xmm6
	vsubss	%xmm4, %xmm6, %xmm6
	vaddss	%xmm5, %xmm4, %xmm4
	vmovss	%xmm6, 4(%r10)
	vsubss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm2, 4(%r9)
	vmovss	%xmm3, (%r9)
	jbe	.L399
	addq	%rdi, %rsi
	leaq	(%r15,%rsi,8), %r9
	addq	%rcx, %rsi
	leaq	(%r15,%rsi,8), %rsi
	vmovss	(%r9), %xmm0
	vmovss	4(%r9), %xmm1
	vmovss	(%rsi), %xmm4
	vsubss	%xmm1, %xmm0, %xmm6
	vmovss	4(%rsi), %xmm3
	vaddss	%xmm0, %xmm1, %xmm5
	vaddss	%xmm4, %xmm3, %xmm2
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm2, %xmm6, %xmm6
	vaddss	%xmm2, %xmm0, %xmm2
	vmovss	%xmm6, (%r9)
	vaddss	%xmm3, %xmm5, %xmm6
	vsubss	%xmm4, %xmm6, %xmm6
	vaddss	%xmm4, %xmm5, %xmm4
	vmovss	%xmm6, 4(%r9)
	vsubss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm2, 4(%rsi)
	vmovss	%xmm3, (%rsi)
.L399:
	addq	%rbx, %rdi
	addq	%r11, %rax
	addq	%r11, %rdx
	cmpq	-56(%rbp), %rdi
	jb	.L397
.L392:
	addq	$1, -80(%rbp)
	movq	-80(%rbp), %rax
	cmpq	%rax, -96(%rbp)
	jne	.L401
	vzeroupper
.L420:
	popq	%rbx
	popq	%r10
	.cfi_remember_state
	.cfi_def_cfa 10, 0
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	leaq	-8(%r10), %rsp
	.cfi_def_cfa 7, 8
	ret
	.p2align 4,,10
	.p2align 3
.L393:
	.cfi_restore_state
	movq	-88(%rbp), %r10
	leaq	4(%rax), %rsi
	leaq	(%r10,%rax), %r9
	.p2align 4,,10
	.p2align 3
.L400:
	vmovss	-4(%rsi,%rcx,8), %xmm3
	vmovss	(%rsi,%rcx,8), %xmm4
	vaddss	%xmm3, %xmm4, %xmm2
	vmovss	(%rsi), %xmm0
	vmovss	-4(%rsi), %xmm1
	vaddss	%xmm1, %xmm0, %xmm5
	vsubss	%xmm0, %xmm2, %xmm6
	vaddss	%xmm1, %xmm6, %xmm6
	vsubss	%xmm1, %xmm2, %xmm1
	vmovss	%xmm6, -4(%rsi)
	vsubss	%xmm3, %xmm5, %xmm6
	vaddss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm4, %xmm6, %xmm6
	vsubss	%xmm4, %xmm5, %xmm4
	vmovss	%xmm6, (%rsi)
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm0, (%rsi,%rcx,8)
	vmovss	%xmm3, -4(%rsi,%rcx,8)
	addq	$8, %rsi
	cmpq	%r9, %rsi
	jne	.L400
	jmp	.L399
	.cfi_endproc
.LFE10380:
	.size	_Z17ApplyManyXOnSlicemPSt7complexIfE, .-_Z17ApplyManyXOnSlicemPSt7complexIfE
	.p2align 4,,15
	.globl	_Z17ApplyManyYOnSlicemPSt7complexIfE
	.type	_Z17ApplyManyYOnSlicemPSt7complexIfE, @function
_Z17ApplyManyYOnSlicemPSt7complexIfE:
.LFB10381:
	.cfi_startproc
	leaq	8(%rsp), %r10
	.cfi_def_cfa 10, 0
	andq	$-32, %rsp
	testq	%rdi, %rdi
	pushq	-8(%r10)
	pushq	%rbp
	.cfi_escape 0x10,0x6,0x2,0x76,0
	movq	%rsp, %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%r10
	.cfi_escape 0xf,0x3,0x76,0x58,0x6
	.cfi_escape 0x10,0xf,0x2,0x76,0x78
	.cfi_escape 0x10,0xe,0x2,0x76,0x70
	.cfi_escape 0x10,0xd,0x2,0x76,0x68
	.cfi_escape 0x10,0xc,0x2,0x76,0x60
	pushq	%rbx
	.cfi_escape 0x10,0x3,0x2,0x76,0x50
	movq	%rdi, -96(%rbp)
	je	.L452
	movl	$1, %eax
	movl	%edi, %ecx
	sall	%cl, %eax
	cltq
	testq	%rax, %rax
	movq	%rax, -56(%rbp)
	je	.L452
	movl	$64, %eax
	movq	%rsi, %r15
	movq	$0, -80(%rbp)
	subq	%rsi, %rax
	movq	%rax, -64(%rbp)
	.p2align 4,,10
	.p2align 3
.L432:
	movq	-80(%rbp), %rsi
	movl	$1, %ebx
	movl	$1, %eax
	leal	1(%rsi), %ecx
	sall	%cl, %ebx
	movl	%esi, %ecx
	sall	%cl, %eax
	movslq	%ebx, %rbx
	movslq	%eax, %rcx
	testq	%rcx, %rcx
	je	.L424
	leaq	0(,%rcx,8), %rdi
	leaq	-1(%rcx), %rax
	movq	%rcx, %r9
	movq	%rcx, %r12
	leaq	0(,%rbx,8), %r11
	shrq	$3, %r9
	movq	%rdi, %r13
	leaq	(%r15,%rdi), %rdx
	addq	$4, %rdi
	subq	%r15, %r13
	movq	%rax, -72(%rbp)
	andq	$-8, %r12
	leaq	64(%r13), %r14
	movq	%r15, %rax
	xorl	%esi, %esi
	movq	%rdi, -88(%rbp)
	.p2align 4,,10
	.p2align 3
.L431:
	movq	-64(%rbp), %rdi
	leaq	(%rax,%r13), %r8
	leaq	0(,%rsi,8), %r10
	addq	%rax, %rdi
	cmpq	%rdi, %r8
	leaq	(%r14,%rax), %r8
	setge	%dil
	cmpq	%r8, %r10
	setge	%r8b
	orb	%r8b, %dil
	je	.L425
	cmpq	$7, %rcx
	jbe	.L425
	xorl	%edi, %edi
	cmpq	$6, -72(%rbp)
	jbe	.L426
	xorl	%r8d, %r8d
	.p2align 4,,10
	.p2align 3
.L427:
	vmovups	(%rax,%rdi), %xmm10
	addq	$1, %r8
	vmovups	32(%rax,%rdi), %xmm6
	vinsertf128	$0x1, 16(%rax,%rdi), %ymm10, %ymm10
	vmovups	(%rdx,%rdi), %xmm9
	vinsertf128	$0x1, 48(%rax,%rdi), %ymm6, %ymm6
	vmovups	32(%rdx,%rdi), %xmm5
	vinsertf128	$0x1, 16(%rdx,%rdi), %ymm9, %ymm9
	vinsertf128	$0x1, 48(%rdx,%rdi), %ymm5, %ymm5
	vshufps	$136, %ymm6, %ymm10, %ymm1
	vpermilps	$177, %ymm10, %ymm11
	vpermilps	$177, %ymm6, %ymm8
	vperm2f128	$3, %ymm1, %ymm1, %ymm0
	vshufps	$136, %ymm5, %ymm9, %ymm2
	vpermilps	$177, %ymm9, %ymm12
	vshufps	$68, %ymm0, %ymm1, %ymm13
	vshufps	$238, %ymm0, %ymm1, %ymm0
	vshufps	$221, %ymm6, %ymm10, %ymm1
	vaddsubps	%ymm11, %ymm10, %ymm10
	vpermilps	$177, %ymm5, %ymm7
	vinsertf128	$1, %xmm0, %ymm13, %ymm13
	vaddsubps	%ymm8, %ymm6, %ymm6
	vperm2f128	$3, %ymm1, %ymm1, %ymm0
	vshufps	$68, %ymm0, %ymm1, %ymm4
	vshufps	$238, %ymm0, %ymm1, %ymm0
	vinsertf128	$1, %xmm0, %ymm4, %ymm4
	vperm2f128	$3, %ymm2, %ymm2, %ymm0
	vshufps	$68, %ymm0, %ymm2, %ymm1
	vshufps	$238, %ymm0, %ymm2, %ymm0
	vshufps	$221, %ymm5, %ymm9, %ymm2
	vaddsubps	%ymm12, %ymm9, %ymm9
	vinsertf128	$1, %xmm0, %ymm1, %ymm1
	vperm2f128	$3, %ymm2, %ymm2, %ymm0
	vaddsubps	%ymm7, %ymm5, %ymm5
	vaddps	%ymm9, %ymm10, %ymm10
	vshufps	$68, %ymm0, %ymm2, %ymm3
	vshufps	$238, %ymm0, %ymm2, %ymm0
	vaddps	%ymm5, %ymm6, %ymm5
	vinsertf128	$1, %xmm0, %ymm3, %ymm3
	vaddps	%ymm3, %ymm13, %ymm0
	vsubps	%ymm4, %ymm0, %ymm0
	vsubps	%ymm1, %ymm0, %ymm2
	vaddps	%ymm4, %ymm13, %ymm0
	vsubps	%ymm3, %ymm0, %ymm0
	vsubps	%ymm1, %ymm0, %ymm0
	vunpcklps	%ymm0, %ymm2, %ymm1
	vunpckhps	%ymm0, %ymm2, %ymm0
	vinsertf128	$1, %xmm0, %ymm1, %ymm2
	vperm2f128	$49, %ymm0, %ymm1, %ymm0
	vmovups	%xmm2, (%rax,%rdi)
	vextractf128	$0x1, %ymm2, 16(%rax,%rdi)
	vmovups	%xmm0, 32(%rax,%rdi)
	vextractf128	$0x1, %ymm0, 48(%rax,%rdi)
	vmovups	%xmm10, (%rdx,%rdi)
	vextractf128	$0x1, %ymm10, 16(%rdx,%rdi)
	vmovups	%xmm5, 32(%rdx,%rdi)
	vextractf128	$0x1, %ymm5, 48(%rdx,%rdi)
	addq	$64, %rdi
	cmpq	%r8, %r9
	ja	.L427
	cmpq	%r12, %rcx
	je	.L429
	movq	%r12, %rdi
.L426:
	leaq	(%rsi,%rdi), %r8
	leaq	(%r15,%r8,8), %r10
	addq	%rcx, %r8
	leaq	(%r15,%r8,8), %r8
	vmovss	(%r10), %xmm3
	vmovss	4(%r10), %xmm4
	vsubss	%xmm4, %xmm3, %xmm2
	vmovss	4(%r8), %xmm0
	vmovss	(%r8), %xmm1
	vaddss	%xmm3, %xmm4, %xmm4
	vaddss	%xmm1, %xmm0, %xmm3
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r10)
	vsubss	%xmm3, %xmm4, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r10)
	vmovss	%xmm0, (%r8)
	vmovss	%xmm3, 4(%r8)
	leaq	1(%rdi), %r8
	cmpq	%r8, %rcx
	jbe	.L429
	addq	%rsi, %r8
	leaq	(%r15,%r8,8), %r10
	addq	%rcx, %r8
	leaq	(%r15,%r8,8), %r8
	vmovss	4(%r10), %xmm3
	vmovss	(%r10), %xmm4
	vsubss	%xmm3, %xmm4, %xmm2
	vmovss	4(%r8), %xmm0
	vmovss	(%r8), %xmm1
	vaddss	%xmm3, %xmm4, %xmm4
	vaddss	%xmm0, %xmm1, %xmm3
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r10)
	vsubss	%xmm3, %xmm4, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r10)
	vmovss	%xmm0, (%r8)
	vmovss	%xmm3, 4(%r8)
	leaq	2(%rdi), %r8
	cmpq	%r8, %rcx
	jbe	.L429
	addq	%rsi, %r8
	leaq	(%r15,%r8,8), %r10
	addq	%rcx, %r8
	leaq	(%r15,%r8,8), %r8
	vmovss	4(%r10), %xmm3
	vmovss	(%r10), %xmm4
	vsubss	%xmm3, %xmm4, %xmm2
	vmovss	4(%r8), %xmm0
	vmovss	(%r8), %xmm1
	vaddss	%xmm3, %xmm4, %xmm4
	vaddss	%xmm0, %xmm1, %xmm3
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r10)
	vsubss	%xmm3, %xmm4, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r10)
	vmovss	%xmm0, (%r8)
	vmovss	%xmm3, 4(%r8)
	leaq	3(%rdi), %r8
	cmpq	%r8, %rcx
	jbe	.L429
	addq	%rsi, %r8
	leaq	(%r15,%r8,8), %r10
	addq	%rcx, %r8
	leaq	(%r15,%r8,8), %r8
	vmovss	(%r10), %xmm4
	vmovss	4(%r10), %xmm3
	vsubss	%xmm3, %xmm4, %xmm2
	vmovss	4(%r8), %xmm0
	vmovss	(%r8), %xmm1
	vaddss	%xmm3, %xmm4, %xmm3
	vaddss	%xmm0, %xmm1, %xmm4
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r10)
	vsubss	%xmm4, %xmm3, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r10)
	vmovss	%xmm0, (%r8)
	vmovss	%xmm3, 4(%r8)
	leaq	4(%rdi), %r8
	cmpq	%r8, %rcx
	jbe	.L429
	addq	%rsi, %r8
	leaq	(%r15,%r8,8), %r10
	addq	%rcx, %r8
	leaq	(%r15,%r8,8), %r8
	vmovss	(%r10), %xmm4
	vmovss	4(%r10), %xmm3
	vsubss	%xmm3, %xmm4, %xmm2
	vmovss	4(%r8), %xmm0
	vmovss	(%r8), %xmm1
	vaddss	%xmm3, %xmm4, %xmm3
	vaddss	%xmm0, %xmm1, %xmm4
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r10)
	vsubss	%xmm4, %xmm3, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r10)
	vmovss	%xmm0, (%r8)
	vmovss	%xmm3, 4(%r8)
	leaq	5(%rdi), %r8
	cmpq	%r8, %rcx
	jbe	.L429
	addq	%rsi, %r8
	addq	$6, %rdi
	leaq	(%r15,%r8,8), %r10
	addq	%rcx, %r8
	cmpq	%rdi, %rcx
	leaq	(%r15,%r8,8), %r8
	vmovss	(%r10), %xmm4
	vmovss	4(%r10), %xmm3
	vsubss	%xmm3, %xmm4, %xmm2
	vmovss	4(%r8), %xmm0
	vmovss	(%r8), %xmm1
	vaddss	%xmm3, %xmm4, %xmm3
	vaddss	%xmm0, %xmm1, %xmm4
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r10)
	vsubss	%xmm4, %xmm3, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r10)
	vmovss	%xmm0, (%r8)
	vmovss	%xmm3, 4(%r8)
	jbe	.L429
	addq	%rsi, %rdi
	leaq	(%r15,%rdi,8), %r8
	addq	%rcx, %rdi
	leaq	(%r15,%rdi,8), %rdi
	vmovss	4(%r8), %xmm4
	vmovss	(%r8), %xmm3
	vsubss	%xmm4, %xmm3, %xmm2
	vmovss	4(%rdi), %xmm0
	vmovss	(%rdi), %xmm1
	vaddss	%xmm3, %xmm4, %xmm3
	vaddss	%xmm1, %xmm0, %xmm4
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r8)
	vsubss	%xmm4, %xmm3, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r8)
	vmovss	%xmm0, (%rdi)
	vmovss	%xmm3, 4(%rdi)
.L429:
	addq	%rbx, %rsi
	addq	%r11, %rax
	addq	%r11, %rdx
	cmpq	-56(%rbp), %rsi
	jb	.L431
.L424:
	addq	$1, -80(%rbp)
	movq	-80(%rbp), %rax
	cmpq	%rax, -96(%rbp)
	jne	.L432
	vzeroupper
.L452:
	popq	%rbx
	popq	%r10
	.cfi_remember_state
	.cfi_def_cfa 10, 0
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	leaq	-8(%r10), %rsp
	.cfi_def_cfa 7, 8
	ret
	.p2align 4,,10
	.p2align 3
.L425:
	.cfi_restore_state
	movq	-88(%rbp), %r10
	leaq	4(%rax), %rdi
	leaq	(%r10,%rax), %r8
	.p2align 4,,10
	.p2align 3
.L430:
	vmovss	(%rdi), %xmm3
	vmovss	-4(%rdi), %xmm2
	vsubss	%xmm3, %xmm2, %xmm4
	vmovss	-4(%rdi,%rcx,8), %xmm0
	vaddss	%xmm2, %xmm3, %xmm2
	vmovss	(%rdi,%rcx,8), %xmm1
	vsubss	%xmm0, %xmm4, %xmm3
	vsubss	%xmm1, %xmm4, %xmm4
	vaddss	%xmm1, %xmm3, %xmm3
	vaddss	%xmm0, %xmm4, %xmm4
	vmovss	%xmm3, -4(%rdi)
	vsubss	%xmm1, %xmm2, %xmm3
	vsubss	%xmm0, %xmm3, %xmm3
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	%xmm3, (%rdi)
	vaddss	%xmm2, %xmm0, %xmm2
	vmovss	%xmm4, -4(%rdi,%rcx,8)
	vmovss	%xmm2, (%rdi,%rcx,8)
	addq	$8, %rdi
	cmpq	%r8, %rdi
	jne	.L430
	jmp	.L429
	.cfi_endproc
.LFE10381:
	.size	_Z17ApplyManyYOnSlicemPSt7complexIfE, .-_Z17ApplyManyYOnSlicemPSt7complexIfE
	.p2align 4,,15
	.globl	_Z9ApplyFWHTPSt7complexIfEmRKSt6vectorIiSaIiEEiN4Gate4TypeE
	.type	_Z9ApplyFWHTPSt7complexIfEmRKSt6vectorIiSaIiEEiN4Gate4TypeE, @function
_Z9ApplyFWHTPSt7complexIfEmRKSt6vectorIiSaIiEEiN4Gate4TypeE:
.LFB10382:
	.cfi_startproc
	leaq	8(%rsp), %r10
	.cfi_def_cfa 10, 0
	andq	$-32, %rsp
	pushq	-8(%r10)
	pushq	%rbp
	.cfi_escape 0x10,0x6,0x2,0x76,0
	movq	%rsp, %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	.cfi_escape 0x10,0xf,0x2,0x76,0x78
	.cfi_escape 0x10,0xe,0x2,0x76,0x70
	.cfi_escape 0x10,0xd,0x2,0x76,0x68
	.cfi_escape 0x10,0xc,0x2,0x76,0x60
	movq	%rdi, %r15
	pushq	%r10
	.cfi_escape 0xf,0x3,0x76,0x58,0x6
	pushq	%rbx
	.cfi_escape 0x10,0x3,0x2,0x76,0x50
	movl	%ecx, %ebx
	subq	$160, %rsp
	movq	%rsi, -96(%rbp)
	movq	(%rdx), %r12
	movq	8(%rdx), %rsi
	movl	%r8d, -172(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -56(%rbp)
	xorl	%eax, %eax
	movl	$1, %eax
	subq	%r12, %rsi
	movq	%rsi, %rdx
	sarq	$2, %rdx
	movl	%edx, %ecx
	movq	%rdx, -120(%rbp)
	salq	%cl, %rax
	movq	%rax, -152(%rbp)
	subq	$1, %rax
	movq	%rax, -80(%rbp)
	movq	%rax, -88(%rbp)
	movl	$8, %eax
	salq	%cl, %rax
	testq	%rdx, %rdx
	movq	%rax, -72(%rbp)
	je	.L455
	subl	$1, %ebx
	movq	%r12, %rax
	addq	%r12, %rsi
	xorl	%edx, %edx
	movl	$1, %r14d
	.p2align 4,,10
	.p2align 3
.L457:
	movl	%ebx, %ecx
	subl	(%rax), %ecx
	movq	%r14, %rdi
	addq	$4, %rax
	salq	%cl, %rdi
	orq	%rdi, %rdx
	cmpq	%rsi, %rax
	jne	.L457
	movq	%rdx, -160(%rbp)
	movq	-72(%rbp), %rdx
	xorl	%esi, %esi
	leaq	22(%rdx), %rax
	andq	$-16, %rax
	subq	%rax, %rsp
	leaq	7(%rsp), %r13
	shrq	$3, %r13
	leaq	0(,%r13,8), %r8
	movq	%r8, %rdi
	movq	%r8, -168(%rbp)
	call	memset
	movq	%rax, %r8
	movq	-120(%rbp), %rax
	movq	%r14, %rsi
	movq	$0, 0(,%r13,8)
	movl	$1, %r10d
	movl	$1, %edx
	movl	$1, %r11d
	movl	%eax, %ecx
	leaq	1(%rax), %r13
	subl	$1, %ecx
	salq	%cl, %rsi
	movq	%rsi, %rdi
	.p2align 4,,10
	.p2align 3
.L460:
	movq	%r11, %r9
	movl	%r10d, %ecx
	salq	%cl, %r9
	cmpq	%rdx, %r9
	jbe	.L458
	movl	%ebx, %ecx
	subl	-4(%r12,%r10,4), %ecx
	movq	%r11, %r14
	salq	$3, %rsi
	movq	%r8, %rax
	salq	%cl, %r14
	.p2align 4,,10
	.p2align 3
.L459:
	movq	(%rax), %rcx
	addq	$1, %rdx
	addq	%r14, %rcx
	movq	%rcx, (%rax,%rdi,8)
	addq	%rsi, %rax
	cmpq	%rdx, %r9
	jne	.L459
.L458:
	movq	%rdi, %rax
	addq	$1, %r10
	movq	%rdi, %rsi
	shrq	%rax
	cmpq	%r13, %r10
	je	.L494
	movq	%rax, %rdi
	jmp	.L460
.L455:
	movq	%rax, %rdx
	leaq	22(%rax), %rax
	xorl	%esi, %esi
	andq	$-16, %rax
	subq	%rax, %rsp
	leaq	7(%rsp), %rbx
	shrq	$3, %rbx
	leaq	0(,%rbx,8), %rax
	movq	%rax, %rdi
	movq	%rax, -168(%rbp)
	call	memset
	movq	$0, 0(,%rbx,8)
	movq	$0, -160(%rbp)
.L494:
	movq	-72(%rbp), %rax
	addq	$18, %rax
	andq	$-16, %rax
	subq	%rax, %rsp
	movq	-80(%rbp), %rax
	movq	%rsp, %r13
	testq	%rax, %rax
	js	.L461
	cmpq	$2, %rax
	jbe	.L497
	movq	-152(%rbp), %rcx
	vxorps	%xmm0, %xmm0, %xmm0
	movq	%r13, %rax
	xorl	%edx, %edx
	shrq	$2, %rcx
	.p2align 4,,10
	.p2align 3
.L463:
	addq	$1, %rdx
	vextractf128	$0x1, %ymm0, 16(%rax)
	vmovups	%xmm0, (%rax)
	addq	$32, %rax
	cmpq	%rdx, %rcx
	ja	.L463
	movq	-152(%rbp), %rbx
	movq	-80(%rbp), %rax
	movq	%rbx, %rdx
	andq	$-4, %rdx
	subq	%rdx, %rax
	cmpq	%rdx, %rbx
	movq	%rax, -88(%rbp)
	leaq	0(%r13,%rdx,8), %rax
	je	.L554
	vzeroupper
.L462:
	movq	-88(%rbp), %rbx
	movl	$0x00000000, (%rax)
	movl	$0x00000000, 4(%rax)
	testq	%rbx, %rbx
	je	.L461
	cmpq	$1, %rbx
	movl	$0x00000000, 8(%rax)
	movl	$0x00000000, 12(%rax)
	je	.L461
	movl	$0x00000000, 16(%rax)
	movl	$0x00000000, 20(%rax)
.L461:
	movq	-72(%rbp), %rdx
	xorl	%esi, %esi
	movq	%r13, %rdi
	movq	%r13, %r12
	call	memset
	movq	-96(%rbp), %rax
	xorl	%edx, %edx
	movzbl	-120(%rbp), %ecx
	movq	$0, -136(%rbp)
	movq	$0, -112(%rbp)
	divq	-152(%rbp)
	movq	%rax, -144(%rbp)
	movl	$1, %eax
	movq	-144(%rbp), %rbx
	sall	%cl, %eax
	cltq
	movq	%rax, -72(%rbp)
	movl	$64, %eax
	subq	%r13, %rax
	movq	%rax, -80(%rbp)
	movl	$32, %eax
	subq	%r13, %rax
	movq	%rax, -104(%rbp)
.L549:
	cmpq	%rbx, -136(%rbp)
	jnb	.L454
.L556:
	movq	-112(%rbp), %r14
	andq	-160(%rbp), %r14
	je	.L555
	addq	%r14, -112(%rbp)
	cmpq	%rbx, -136(%rbp)
	jb	.L556
.L454:
	movq	-56(%rbp), %rax
	xorq	%fs:40, %rax
	jne	.L557
	vzeroupper
	leaq	-48(%rbp), %rsp
	popq	%rbx
	popq	%r10
	.cfi_remember_state
	.cfi_def_cfa 10, 0
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	leaq	-8(%r10), %rsp
	.cfi_def_cfa 7, 8
	ret
	.p2align 4,,10
	.p2align 3
.L555:
	.cfi_restore_state
	movq	-152(%rbp), %rdi
	addq	$1, -136(%rbp)
	testq	%rdi, %rdi
	je	.L468
	movq	-112(%rbp), %rcx
	movq	-168(%rbp), %rsi
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
.L469:
	movq	(%rsi,%rax,8), %rdx
	addq	%rcx, %rdx
	vmovss	(%r15,%rdx,8), %xmm0
	vmovss	%xmm0, (%r12,%rax,8)
	vmovss	4(%r15,%rdx,8), %xmm0
	vmovss	%xmm0, 4(%r12,%rax,8)
	addq	$1, %rax
	cmpq	%rax, %rdi
	jne	.L469
.L468:
	cmpl	$13, -172(%rbp)
	je	.L558
	cmpq	$0, -120(%rbp)
	je	.L474
	cmpq	$0, -72(%rbp)
	je	.L474
	movq	$0, -96(%rbp)
	movq	%r14, -184(%rbp)
	movq	%r15, -192(%rbp)
	.p2align 4,,10
	.p2align 3
.L492:
	movq	-96(%rbp), %rsi
	movl	$1, %ebx
	movl	$1, %eax
	leal	1(%rsi), %ecx
	sall	%cl, %ebx
	movl	%esi, %ecx
	sall	%cl, %eax
	movslq	%ebx, %rbx
	movslq	%eax, %rcx
	testq	%rcx, %rcx
	je	.L484
	leaq	0(,%rcx,8), %rdi
	leaq	-1(%rcx), %rax
	movq	%rcx, %r9
	movq	%rcx, %r13
	leaq	0(,%rbx,8), %r11
	shrq	$3, %r9
	movq	%rdi, %r14
	leaq	(%r12,%rdi), %rdx
	addq	$4, %rdi
	subq	%r12, %r14
	movq	%rax, -88(%rbp)
	andq	$-8, %r13
	leaq	64(%r14), %r15
	movq	%r12, %rax
	xorl	%esi, %esi
	movq	%rdi, -128(%rbp)
	.p2align 4,,10
	.p2align 3
.L491:
	leaq	0(,%rsi,8), %r8
	leaq	(%r15,%rax), %rdi
	movq	-80(%rbp), %r10
	cmpq	%rdi, %r8
	leaq	(%rax,%r14), %r8
	setge	%dil
	addq	%rax, %r10
	cmpq	%r8, %r10
	setle	%r8b
	orb	%r8b, %dil
	je	.L485
	cmpq	$7, %rcx
	jbe	.L485
	xorl	%edi, %edi
	cmpq	$6, -88(%rbp)
	jbe	.L486
	xorl	%r8d, %r8d
	.p2align 4,,10
	.p2align 3
.L487:
	vmovups	(%rax,%rdi), %xmm10
	addq	$1, %r8
	vmovups	32(%rax,%rdi), %xmm6
	vinsertf128	$0x1, 16(%rax,%rdi), %ymm10, %ymm10
	vmovups	(%rdx,%rdi), %xmm9
	vinsertf128	$0x1, 48(%rax,%rdi), %ymm6, %ymm6
	vmovups	32(%rdx,%rdi), %xmm5
	vinsertf128	$0x1, 16(%rdx,%rdi), %ymm9, %ymm9
	vinsertf128	$0x1, 48(%rdx,%rdi), %ymm5, %ymm5
	vshufps	$136, %ymm6, %ymm10, %ymm1
	vpermilps	$177, %ymm10, %ymm11
	vpermilps	$177, %ymm6, %ymm8
	vperm2f128	$3, %ymm1, %ymm1, %ymm0
	vshufps	$136, %ymm5, %ymm9, %ymm2
	vpermilps	$177, %ymm9, %ymm12
	vshufps	$68, %ymm0, %ymm1, %ymm13
	vshufps	$238, %ymm0, %ymm1, %ymm0
	vshufps	$221, %ymm6, %ymm10, %ymm1
	vaddsubps	%ymm11, %ymm10, %ymm10
	vpermilps	$177, %ymm5, %ymm7
	vinsertf128	$1, %xmm0, %ymm13, %ymm13
	vaddsubps	%ymm8, %ymm6, %ymm6
	vperm2f128	$3, %ymm1, %ymm1, %ymm0
	vshufps	$68, %ymm0, %ymm1, %ymm4
	vshufps	$238, %ymm0, %ymm1, %ymm0
	vinsertf128	$1, %xmm0, %ymm4, %ymm4
	vperm2f128	$3, %ymm2, %ymm2, %ymm0
	vshufps	$68, %ymm0, %ymm2, %ymm1
	vshufps	$238, %ymm0, %ymm2, %ymm0
	vshufps	$221, %ymm5, %ymm9, %ymm2
	vaddsubps	%ymm12, %ymm9, %ymm9
	vinsertf128	$1, %xmm0, %ymm1, %ymm1
	vperm2f128	$3, %ymm2, %ymm2, %ymm0
	vaddsubps	%ymm7, %ymm5, %ymm5
	vaddps	%ymm9, %ymm10, %ymm10
	vshufps	$68, %ymm0, %ymm2, %ymm3
	vshufps	$238, %ymm0, %ymm2, %ymm0
	vaddps	%ymm5, %ymm6, %ymm5
	vinsertf128	$1, %xmm0, %ymm3, %ymm3
	vaddps	%ymm3, %ymm13, %ymm0
	vsubps	%ymm4, %ymm0, %ymm0
	vsubps	%ymm1, %ymm0, %ymm2
	vaddps	%ymm4, %ymm13, %ymm0
	vsubps	%ymm3, %ymm0, %ymm0
	vsubps	%ymm1, %ymm0, %ymm0
	vunpcklps	%ymm0, %ymm2, %ymm1
	vunpckhps	%ymm0, %ymm2, %ymm0
	vinsertf128	$1, %xmm0, %ymm1, %ymm2
	vperm2f128	$49, %ymm0, %ymm1, %ymm0
	vmovups	%xmm2, (%rax,%rdi)
	vextractf128	$0x1, %ymm2, 16(%rax,%rdi)
	vmovups	%xmm0, 32(%rax,%rdi)
	vextractf128	$0x1, %ymm0, 48(%rax,%rdi)
	vmovups	%xmm10, (%rdx,%rdi)
	vextractf128	$0x1, %ymm10, 16(%rdx,%rdi)
	vmovups	%xmm5, 32(%rdx,%rdi)
	vextractf128	$0x1, %ymm5, 48(%rdx,%rdi)
	addq	$64, %rdi
	cmpq	%r8, %r9
	ja	.L487
	cmpq	%r13, %rcx
	je	.L489
	movq	%r13, %rdi
.L486:
	leaq	(%rsi,%rdi), %r8
	leaq	(%r12,%r8,8), %r10
	addq	%rcx, %r8
	leaq	(%r12,%r8,8), %r8
	vmovss	(%r10), %xmm4
	vmovss	4(%r10), %xmm3
	vsubss	%xmm3, %xmm4, %xmm2
	vmovss	4(%r8), %xmm0
	vmovss	(%r8), %xmm1
	vaddss	%xmm3, %xmm4, %xmm3
	vaddss	%xmm1, %xmm0, %xmm4
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r10)
	vsubss	%xmm4, %xmm3, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r10)
	vmovss	%xmm0, (%r8)
	vmovss	%xmm3, 4(%r8)
	leaq	1(%rdi), %r8
	cmpq	%r8, %rcx
	jbe	.L489
	addq	%rsi, %r8
	leaq	(%r12,%r8,8), %r10
	addq	%rcx, %r8
	leaq	(%r12,%r8,8), %r8
	vmovss	(%r10), %xmm4
	vmovss	4(%r10), %xmm3
	vsubss	%xmm3, %xmm4, %xmm2
	vmovss	4(%r8), %xmm0
	vmovss	(%r8), %xmm1
	vaddss	%xmm3, %xmm4, %xmm3
	vaddss	%xmm0, %xmm1, %xmm4
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r10)
	vsubss	%xmm4, %xmm3, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r10)
	vmovss	%xmm0, (%r8)
	vmovss	%xmm3, 4(%r8)
	leaq	2(%rdi), %r8
	cmpq	%r8, %rcx
	jbe	.L489
	addq	%rsi, %r8
	leaq	(%r12,%r8,8), %r10
	addq	%rcx, %r8
	leaq	(%r12,%r8,8), %r8
	vmovss	(%r10), %xmm4
	vmovss	4(%r10), %xmm3
	vsubss	%xmm3, %xmm4, %xmm2
	vmovss	4(%r8), %xmm0
	vmovss	(%r8), %xmm1
	vaddss	%xmm3, %xmm4, %xmm3
	vaddss	%xmm0, %xmm1, %xmm4
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r10)
	vsubss	%xmm4, %xmm3, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r10)
	vmovss	%xmm0, (%r8)
	vmovss	%xmm3, 4(%r8)
	leaq	3(%rdi), %r8
	cmpq	%r8, %rcx
	jbe	.L489
	addq	%rsi, %r8
	leaq	(%r12,%r8,8), %r10
	addq	%rcx, %r8
	leaq	(%r12,%r8,8), %r8
	vmovss	(%r10), %xmm4
	vmovss	4(%r10), %xmm3
	vsubss	%xmm3, %xmm4, %xmm2
	vmovss	4(%r8), %xmm0
	vmovss	(%r8), %xmm1
	vaddss	%xmm3, %xmm4, %xmm3
	vaddss	%xmm0, %xmm1, %xmm4
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r10)
	vsubss	%xmm4, %xmm3, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r10)
	vmovss	%xmm0, (%r8)
	vmovss	%xmm3, 4(%r8)
	leaq	4(%rdi), %r8
	cmpq	%r8, %rcx
	jbe	.L489
	addq	%rsi, %r8
	leaq	(%r12,%r8,8), %r10
	addq	%rcx, %r8
	leaq	(%r12,%r8,8), %r8
	vmovss	(%r10), %xmm4
	vmovss	4(%r10), %xmm3
	vsubss	%xmm3, %xmm4, %xmm2
	vmovss	4(%r8), %xmm0
	vmovss	(%r8), %xmm1
	vaddss	%xmm3, %xmm4, %xmm3
	vaddss	%xmm0, %xmm1, %xmm4
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r10)
	vsubss	%xmm4, %xmm3, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r10)
	vmovss	%xmm0, (%r8)
	vmovss	%xmm3, 4(%r8)
	leaq	5(%rdi), %r8
	cmpq	%r8, %rcx
	jbe	.L489
	addq	%rsi, %r8
	addq	$6, %rdi
	leaq	(%r12,%r8,8), %r10
	addq	%rcx, %r8
	cmpq	%rdi, %rcx
	leaq	(%r12,%r8,8), %r8
	vmovss	(%r10), %xmm4
	vmovss	4(%r10), %xmm3
	vsubss	%xmm3, %xmm4, %xmm2
	vmovss	4(%r8), %xmm0
	vmovss	(%r8), %xmm1
	vaddss	%xmm3, %xmm4, %xmm3
	vaddss	%xmm0, %xmm1, %xmm4
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r10)
	vsubss	%xmm4, %xmm3, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r10)
	vmovss	%xmm0, (%r8)
	vmovss	%xmm3, 4(%r8)
	jbe	.L489
	addq	%rsi, %rdi
	leaq	(%r12,%rdi,8), %r8
	addq	%rcx, %rdi
	leaq	(%r12,%rdi,8), %rdi
	vmovss	4(%r8), %xmm4
	vmovss	(%r8), %xmm3
	vsubss	%xmm4, %xmm3, %xmm2
	vmovss	4(%rdi), %xmm0
	vmovss	(%rdi), %xmm1
	vaddss	%xmm3, %xmm4, %xmm3
	vaddss	%xmm0, %xmm1, %xmm4
	vaddss	%xmm2, %xmm0, %xmm5
	vsubss	%xmm1, %xmm5, %xmm5
	vaddss	%xmm2, %xmm1, %xmm1
	vmovss	%xmm5, (%r8)
	vsubss	%xmm4, %xmm3, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm5, 4(%r8)
	vmovss	%xmm0, (%rdi)
	vmovss	%xmm3, 4(%rdi)
.L489:
	addq	%rbx, %rsi
	addq	%r11, %rax
	addq	%r11, %rdx
	cmpq	%rsi, -72(%rbp)
	ja	.L491
.L484:
	addq	$1, -96(%rbp)
	movq	-96(%rbp), %rax
	cmpq	%rax, -120(%rbp)
	jne	.L492
.L553:
	movq	-184(%rbp), %r14
	movq	-192(%rbp), %r15
.L474:
	movq	-152(%rbp), %rsi
	testq	%rsi, %rsi
	je	.L472
	movq	-112(%rbp), %rdx
	movq	-168(%rbp), %rcx
	.p2align 4,,10
	.p2align 3
.L473:
	movq	(%rcx,%r14,8), %rax
	vmovss	(%r12,%r14,8), %xmm0
	addq	%rdx, %rax
	vmovss	%xmm0, (%r15,%rax,8)
	vmovss	4(%r12,%r14,8), %xmm0
	addq	$1, %r14
	cmpq	%r14, %rsi
	vmovss	%xmm0, 4(%r15,%rax,8)
	jne	.L473
.L472:
	addq	$1, -112(%rbp)
	movq	-144(%rbp), %rbx
	jmp	.L549
	.p2align 4,,10
	.p2align 3
.L485:
	movq	-128(%rbp), %r10
	leaq	4(%rax), %rdi
	leaq	(%r10,%rax), %r8
	.p2align 4,,10
	.p2align 3
.L490:
	vmovss	(%rdi), %xmm3
	vmovss	-4(%rdi), %xmm2
	vsubss	%xmm3, %xmm2, %xmm4
	vmovss	-4(%rdi,%rcx,8), %xmm0
	vaddss	%xmm2, %xmm3, %xmm2
	vmovss	(%rdi,%rcx,8), %xmm1
	vsubss	%xmm0, %xmm4, %xmm3
	vsubss	%xmm1, %xmm4, %xmm4
	vaddss	%xmm1, %xmm3, %xmm3
	vaddss	%xmm0, %xmm4, %xmm4
	vmovss	%xmm3, -4(%rdi)
	vsubss	%xmm1, %xmm2, %xmm3
	vsubss	%xmm0, %xmm3, %xmm3
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	%xmm3, (%rdi)
	vaddss	%xmm2, %xmm0, %xmm2
	vmovss	%xmm4, -4(%rdi,%rcx,8)
	vmovss	%xmm2, (%rdi,%rcx,8)
	addq	$8, %rdi
	cmpq	%r8, %rdi
	jne	.L490
	jmp	.L489
.L558:
	cmpq	$0, -120(%rbp)
	je	.L474
	cmpq	$0, -72(%rbp)
	je	.L474
	movq	$0, -96(%rbp)
	movq	%r14, -184(%rbp)
	movq	%r15, -192(%rbp)
	.p2align 4,,10
	.p2align 3
.L483:
	movq	-96(%rbp), %rsi
	movl	$1, %ebx
	movl	$1, %eax
	leal	1(%rsi), %ecx
	sall	%cl, %ebx
	movl	%esi, %ecx
	sall	%cl, %eax
	movslq	%ebx, %rbx
	movslq	%eax, %rcx
	testq	%rcx, %rcx
	je	.L475
	leaq	0(,%rcx,8), %rsi
	leaq	-1(%rcx), %rax
	movq	%rcx, %r8
	movq	%rcx, %r13
	leaq	0(,%rbx,8), %r11
	shrq	$2, %r8
	movq	%rsi, %r14
	leaq	(%r12,%rsi), %rdx
	addq	$4, %rsi
	subq	%r12, %r14
	movq	%rax, -88(%rbp)
	andq	$-4, %r13
	leaq	32(%r14), %r15
	movq	%r12, %rax
	xorl	%edi, %edi
	movq	%rsi, -128(%rbp)
	.p2align 4,,10
	.p2align 3
.L482:
	movq	-104(%rbp), %rsi
	leaq	(%rax,%r14), %r9
	leaq	0(,%rdi,8), %r10
	addq	%rax, %rsi
	cmpq	%rsi, %r9
	leaq	(%r15,%rax), %r9
	setge	%sil
	cmpq	%r9, %r10
	setge	%r9b
	orb	%r9b, %sil
	je	.L476
	cmpq	$20, %rcx
	jbe	.L476
	xorl	%esi, %esi
	cmpq	$2, -88(%rbp)
	jbe	.L477
	xorl	%r9d, %r9d
	.p2align 4,,10
	.p2align 3
.L478:
	vmovups	(%rax,%rsi), %xmm0
	addq	$1, %r9
	vmovups	(%rdx,%rsi), %xmm4
	vinsertf128	$0x1, 16(%rax,%rsi), %ymm0, %ymm0
	vinsertf128	$0x1, 16(%rdx,%rsi), %ymm4, %ymm4
	vpermilps	$160, %ymm0, %ymm7
	vpermilps	$245, %ymm0, %ymm6
	vpermilps	$177, %ymm4, %ymm5
	vpermilps	$160, %ymm4, %ymm2
	vpermilps	$245, %ymm4, %ymm1
	vaddsubps	%ymm6, %ymm7, %ymm6
	vpermilps	$177, %ymm0, %ymm3
	vaddps	%ymm4, %ymm5, %ymm7
	vsubps	%ymm5, %ymm4, %ymm4
	vaddsubps	%ymm1, %ymm2, %ymm1
	vblendps	$170, %ymm4, %ymm7, %ymm5
	vaddps	%ymm5, %ymm6, %ymm4
	vmovups	%xmm4, (%rax,%rsi)
	vextractf128	$0x1, %ymm4, 16(%rax,%rsi)
	vaddps	%ymm3, %ymm0, %ymm4
	vsubps	%ymm3, %ymm0, %ymm0
	vblendps	$170, %ymm0, %ymm4, %ymm0
	vaddps	%ymm1, %ymm0, %ymm0
	vmovups	%xmm0, (%rdx,%rsi)
	vextractf128	$0x1, %ymm0, 16(%rdx,%rsi)
	addq	$32, %rsi
	cmpq	%r9, %r8
	ja	.L478
	cmpq	%r13, %rcx
	je	.L480
	movq	%r13, %rsi
.L477:
	leaq	(%rdi,%rsi), %r9
	leaq	(%r12,%r9,8), %r10
	addq	%rcx, %r9
	leaq	(%r12,%r9,8), %r9
	vmovss	(%r10), %xmm0
	vmovss	(%r9), %xmm4
	vmovss	4(%r9), %xmm3
	vmovss	4(%r10), %xmm1
	vaddss	%xmm3, %xmm4, %xmm2
	vsubss	%xmm1, %xmm0, %xmm6
	vaddss	%xmm0, %xmm1, %xmm5
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm2, %xmm6, %xmm6
	vaddss	%xmm2, %xmm0, %xmm2
	vmovss	%xmm6, (%r10)
	vaddss	%xmm5, %xmm3, %xmm6
	vsubss	%xmm4, %xmm6, %xmm6
	vaddss	%xmm5, %xmm4, %xmm4
	vmovss	%xmm6, 4(%r10)
	vsubss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm2, 4(%r9)
	vmovss	%xmm3, (%r9)
	leaq	1(%rsi), %r9
	cmpq	%r9, %rcx
	jbe	.L480
	addq	%rdi, %r9
	addq	$2, %rsi
	leaq	(%r12,%r9,8), %r10
	addq	%rcx, %r9
	cmpq	%rsi, %rcx
	leaq	(%r12,%r9,8), %r9
	vmovss	(%r10), %xmm0
	vmovss	4(%r10), %xmm1
	vmovss	(%r9), %xmm4
	vsubss	%xmm1, %xmm0, %xmm6
	vmovss	4(%r9), %xmm3
	vaddss	%xmm0, %xmm1, %xmm5
	vaddss	%xmm3, %xmm4, %xmm2
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm2, %xmm6, %xmm6
	vaddss	%xmm2, %xmm0, %xmm2
	vmovss	%xmm6, (%r10)
	vaddss	%xmm3, %xmm5, %xmm6
	vsubss	%xmm4, %xmm6, %xmm6
	vaddss	%xmm4, %xmm5, %xmm4
	vmovss	%xmm6, 4(%r10)
	vsubss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm2, 4(%r9)
	vmovss	%xmm3, (%r9)
	jbe	.L480
	addq	%rdi, %rsi
	leaq	(%r12,%rsi,8), %r9
	addq	%rcx, %rsi
	leaq	(%r12,%rsi,8), %rsi
	vmovss	(%r9), %xmm0
	vmovss	4(%r9), %xmm1
	vmovss	(%rsi), %xmm4
	vsubss	%xmm1, %xmm0, %xmm6
	vmovss	4(%rsi), %xmm3
	vaddss	%xmm0, %xmm1, %xmm5
	vaddss	%xmm4, %xmm3, %xmm2
	vsubss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm2, %xmm6, %xmm6
	vaddss	%xmm2, %xmm0, %xmm2
	vmovss	%xmm6, (%r9)
	vaddss	%xmm3, %xmm5, %xmm6
	vsubss	%xmm4, %xmm6, %xmm6
	vaddss	%xmm4, %xmm5, %xmm4
	vmovss	%xmm6, 4(%r9)
	vsubss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm2, 4(%rsi)
	vmovss	%xmm3, (%rsi)
.L480:
	addq	%rbx, %rdi
	addq	%r11, %rax
	addq	%r11, %rdx
	cmpq	%rdi, -72(%rbp)
	ja	.L482
.L475:
	addq	$1, -96(%rbp)
	movq	-96(%rbp), %rax
	cmpq	%rax, -120(%rbp)
	jne	.L483
	jmp	.L553
	.p2align 4,,10
	.p2align 3
.L476:
	movq	-128(%rbp), %r10
	leaq	4(%rax), %rsi
	leaq	(%r10,%rax), %r9
	.p2align 4,,10
	.p2align 3
.L481:
	vmovss	-4(%rsi,%rcx,8), %xmm3
	vmovss	(%rsi,%rcx,8), %xmm4
	vaddss	%xmm3, %xmm4, %xmm2
	vmovss	(%rsi), %xmm0
	vmovss	-4(%rsi), %xmm1
	vaddss	%xmm1, %xmm0, %xmm5
	vsubss	%xmm0, %xmm2, %xmm6
	vaddss	%xmm1, %xmm6, %xmm6
	vsubss	%xmm1, %xmm2, %xmm1
	vmovss	%xmm6, -4(%rsi)
	vsubss	%xmm3, %xmm5, %xmm6
	vaddss	%xmm0, %xmm1, %xmm0
	vaddss	%xmm4, %xmm6, %xmm6
	vsubss	%xmm4, %xmm5, %xmm4
	vmovss	%xmm6, (%rsi)
	vaddss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm0, (%rsi,%rcx,8)
	vmovss	%xmm3, -4(%rsi,%rcx,8)
	addq	$8, %rsi
	cmpq	%r9, %rsi
	jne	.L481
	jmp	.L480
.L554:
	vzeroupper
	jmp	.L461
.L497:
	movq	%r13, %rax
	jmp	.L462
.L557:
	vzeroupper
	call	__stack_chk_fail
	.cfi_endproc
.LFE10382:
	.size	_Z9ApplyFWHTPSt7complexIfEmRKSt6vectorIiSaIiEEiN4Gate4TypeE, .-_Z9ApplyFWHTPSt7complexIfEmRKSt6vectorIiSaIiEEiN4Gate4TypeE
	.section	.text._ZNSt6vectorIiSaIiEE17_M_realloc_insertIJRKiEEEvN9__gnu_cxx17__normal_iteratorIPiS1_EEDpOT_,"axG",@progbits,_ZNSt6vectorIiSaIiEE17_M_realloc_insertIJRKiEEEvN9__gnu_cxx17__normal_iteratorIPiS1_EEDpOT_,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZNSt6vectorIiSaIiEE17_M_realloc_insertIJRKiEEEvN9__gnu_cxx17__normal_iteratorIPiS1_EEDpOT_
	.type	_ZNSt6vectorIiSaIiEE17_M_realloc_insertIJRKiEEEvN9__gnu_cxx17__normal_iteratorIPiS1_EEDpOT_, @function
_ZNSt6vectorIiSaIiEE17_M_realloc_insertIJRKiEEEvN9__gnu_cxx17__normal_iteratorIPiS1_EEDpOT_:
.LFB11031:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	movq	%rsi, %r14
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	movq	%rsi, %r12
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movq	%rdi, %rbp
	subq	$24, %rsp
	.cfi_def_cfa_offset 80
	movq	8(%rdi), %rcx
	movq	(%rdi), %rbx
	movq	%rcx, %rax
	subq	%rbx, %r14
	subq	%rbx, %rax
	sarq	$2, %rax
	testq	%rax, %rax
	je	.L566
	leaq	(%rax,%rax), %rsi
	movq	$-4, %r15
	cmpq	%rsi, %rax
	jbe	.L575
.L561:
	movq	%r15, %rdi
	movq	%rdx, (%rsp)
	call	_Znwm
	movq	0(%rbp), %rbx
	movq	8(%rbp), %rcx
	movq	%r12, %r8
	movq	(%rsp), %rdx
	movq	%rax, %r13
	addq	%rax, %r15
	subq	%rbx, %r8
.L562:
	movl	(%rdx), %eax
	cmpq	%r12, %rbx
	movl	%eax, 0(%r13,%r14)
	je	.L563
	movq	%r8, %rdx
	movq	%rbx, %rsi
	movq	%r13, %rdi
	movq	%rcx, 8(%rsp)
	movq	%r8, (%rsp)
	call	memmove
	movq	8(%rsp), %rcx
	movq	(%rsp), %r8
.L563:
	movq	%rcx, %r14
	leaq	4(%r13,%r8), %r8
	subq	%r12, %r14
	cmpq	%r12, %rcx
	je	.L564
	movq	%r8, %rdi
	movq	%r14, %rdx
	movq	%r12, %rsi
	call	memcpy
	movq	%rax, %r8
.L564:
	addq	%r8, %r14
	testq	%rbx, %rbx
	je	.L565
	movq	%rbx, %rdi
	call	_ZdlPv
.L565:
	movq	%r13, 0(%rbp)
	movq	%r14, 8(%rbp)
	movq	%r15, 16(%rbp)
	addq	$24, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L566:
	.cfi_restore_state
	movl	$1, %esi
.L560:
	leaq	0(,%rsi,4), %r15
	jmp	.L561
	.p2align 4,,10
	.p2align 3
.L575:
	movabsq	$4611686018427387903, %rax
	cmpq	%rax, %rsi
	ja	.L561
	testq	%rsi, %rsi
	jne	.L560
	movq	%r14, %r8
	xorl	%r15d, %r15d
	xorl	%r13d, %r13d
	jmp	.L562
	.cfi_endproc
.LFE11031:
	.size	_ZNSt6vectorIiSaIiEE17_M_realloc_insertIJRKiEEEvN9__gnu_cxx17__normal_iteratorIPiS1_EEDpOT_, .-_ZNSt6vectorIiSaIiEE17_M_realloc_insertIJRKiEEEvN9__gnu_cxx17__normal_iteratorIPiS1_EEDpOT_
	.text
	.p2align 4,,15
	.globl	_Z16Merge2QXY12GatesR4GateS0_iPSt7complexIfEm
	.type	_Z16Merge2QXY12GatesR4GateS0_iPSt7complexIfEm, @function
_Z16Merge2QXY12GatesR4GateS0_iPSt7complexIfEm:
.LFB10379:
	.cfi_startproc
	.cfi_personality 0x3,__gxx_personality_v0
	.cfi_lsda 0x3,.LLSDA10379
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	.cfi_offset 15, -24
	.cfi_offset 14, -32
	.cfi_offset 13, -40
	.cfi_offset 12, -48
	movq	%rsi, %r13
	pushq	%rbx
	movq	%rdi, %r12
	movq	%rdi, %rsi
	leaq	-272(%rbp), %rdi
	.cfi_offset 3, -56
	movl	%edx, %ebx
	subq	$296, %rsp
	movq	%rcx, -288(%rbp)
	movq	%r8, -296(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -56(%rbp)
	xorl	%eax, %eax
.LEHB14:
	call	_ZN4GateC1ERKS_
.LEHE14:
	movq	32(%r13), %rdx
	movq	32(%r12), %rcx
	movl	-4(%rdx), %eax
	cmpl	%eax, -4(%rcx)
	jge	.L577
	movq	-240(%rbp), %rsi
	cmpq	-232(%rbp), %rsi
	je	.L578
	movl	%eax, (%rsi)
	addq	$4, %rsi
	movq	%rsi, -240(%rbp)
.L579:
	movq	56(%r12), %rax
	movl	-4(%rax), %ecx
	movq	56(%r13), %rax
	movl	-4(%rax), %edx
	cmpl	$13, %ecx
	sete	%al
	cmpl	$13, %edx
	sete	%sil
	testb	%al, %al
	je	.L593
	testb	%sil, %sil
	je	.L593
	movq	-248(%rbp), %r13
	movq	-240(%rbp), %rsi
	movl	$1, %eax
	movq	%rsp, -328(%rbp)
	subq	%r13, %rsi
	movq	%rsi, %r14
	sarq	$2, %r14
	movl	%r14d, %ecx
	salq	%cl, %rax
	movq	%rax, -280(%rbp)
	leaq	0(,%rax,8), %rdi
	movl	$8, %eax
	salq	%cl, %rax
	testq	%r14, %r14
	movq	%rax, -312(%rbp)
	je	.L594
	subl	$1, %ebx
	movq	%r13, %rax
	addq	%r13, %rsi
	xorl	%r12d, %r12d
	movl	$1, %r9d
	.p2align 4,,10
	.p2align 3
.L596:
	movl	%ebx, %ecx
	subl	(%rax), %ecx
	movq	%r9, %rdx
	addq	$4, %rax
	salq	%cl, %rdx
	orq	%rdx, %r12
	cmpq	%rsi, %rax
	jne	.L596
	leaq	22(%rdi), %rax
	movq	-312(%rbp), %rdx
	xorl	%esi, %esi
	movq	%r9, -336(%rbp)
	movq	%rax, -320(%rbp)
	andq	$-16, %rax
	subq	%rax, %rsp
	leaq	7(%rsp), %r15
	shrq	$3, %r15
	leaq	0(,%r15,8), %r8
	movq	%r8, %rdi
	movq	%r8, -304(%rbp)
	call	memset
	movq	-336(%rbp), %r9
	leal	-1(%r14), %ecx
	leaq	1(%r14), %r11
	movq	%rax, %r8
	movq	$0, 0(,%r15,8)
	movl	$1, %r10d
	movl	$1, %edx
	movl	$1, %r14d
	movq	%r9, %rsi
	salq	%cl, %rsi
	movq	%rsi, %rdi
	.p2align 4,,10
	.p2align 3
.L599:
	movq	%r14, %r9
	movl	%r10d, %ecx
	salq	%cl, %r9
	cmpq	%r9, %rdx
	jnb	.L597
	movl	%ebx, %ecx
	subl	-4(%r13,%r10,4), %ecx
	movq	%r14, %r15
	salq	$3, %rsi
	movq	%r8, %rax
	salq	%cl, %r15
	.p2align 4,,10
	.p2align 3
.L598:
	movq	(%rax), %rcx
	addq	$1, %rdx
	addq	%r15, %rcx
	movq	%rcx, (%rax,%rdi,8)
	addq	%rsi, %rax
	cmpq	%r9, %rdx
	jne	.L598
.L597:
	movq	%rdi, %rax
	addq	$1, %r10
	movq	%rdi, %rsi
	shrq	%rax
	cmpq	%r11, %r10
	je	.L666
	movq	%rax, %rdi
	jmp	.L599
	.p2align 4,,10
	.p2align 3
.L577:
	leaq	-272(%rbp), %rdi
	movq	%r13, %rsi
.LEHB15:
	call	_ZN4GateaSERKS_
	movq	-240(%rbp), %rsi
	cmpq	-232(%rbp), %rsi
	movq	32(%r12), %rax
	je	.L580
	movl	-4(%rax), %eax
	addq	$4, %rsi
	movl	%eax, -4(%rsi)
	movq	%rsi, -240(%rbp)
.L581:
	leaq	-160(%rbp), %rdi
	movq	%r12, %rsi
	call	_ZN4GateC1ERKS_
.LEHE15:
	movq	%r13, %rsi
	movq	%r12, %rdi
.LEHB16:
	call	_ZN4GateaSERKS_
	leaq	-160(%rbp), %rsi
	movq	%r13, %rdi
	call	_ZN4GateaSERKS_
.LEHE16:
	movq	-88(%rbp), %rdi
	testq	%rdi, %rdi
	je	.L582
	call	_ZdlPv
.L582:
	movq	-112(%rbp), %rdi
	testq	%rdi, %rdi
	je	.L583
	call	_ZdlPv
.L583:
	movq	-136(%rbp), %rdi
	testq	%rdi, %rdi
	je	.L584
	call	_ZdlPv
.L584:
	movq	-152(%rbp), %rax
	movq	-160(%rbp), %rdi
	cmpq	%rdi, %rax
	je	.L585
	leaq	24(%rdi), %r15
	subq	%r15, %rax
	shrq	$3, %rax
	leaq	24(%rdi,%rax,8), %r14
	jmp	.L586
	.p2align 4,,10
	.p2align 3
.L767:
	call	_ZdlPv
	cmpq	%r15, %r14
	movq	%r15, %rdi
	je	.L588
.L589:
	addq	$24, %r15
.L586:
	movq	(%rdi), %rdi
	testq	%rdi, %rdi
	jne	.L767
	cmpq	%r14, %r15
	movq	%r15, %rdi
	jne	.L589
.L588:
	movq	-160(%rbp), %rdi
.L585:
	testq	%rdi, %rdi
	je	.L579
	call	_ZdlPv
	jmp	.L579
	.p2align 4,,10
	.p2align 3
.L593:
	cmpl	$14, %edx
	sete	%dl
	testb	%al, %al
	je	.L610
	testb	%dl, %dl
	je	.L610
	movq	-248(%rbp), %r13
	movq	-240(%rbp), %rsi
	movl	$1, %eax
	movq	%rsp, -328(%rbp)
	subq	%r13, %rsi
	movq	%rsi, %r14
	sarq	$2, %r14
	movl	%r14d, %ecx
	salq	%cl, %rax
	movq	%rax, -280(%rbp)
	leaq	0(,%rax,8), %rdi
	movl	$8, %eax
	salq	%cl, %rax
	testq	%r14, %r14
	movq	%rax, -312(%rbp)
	je	.L611
	leal	-1(%rbx), %r12d
	movq	%r13, %rax
	addq	%r13, %rsi
	xorl	%ebx, %ebx
	movl	$1, %r9d
	.p2align 4,,10
	.p2align 3
.L613:
	movl	%r12d, %ecx
	subl	(%rax), %ecx
	movq	%r9, %rdx
	addq	$4, %rax
	salq	%cl, %rdx
	orq	%rdx, %rbx
	cmpq	%rax, %rsi
	jne	.L613
	leaq	22(%rdi), %rax
	movq	-312(%rbp), %rdx
	xorl	%esi, %esi
	movq	%r9, -336(%rbp)
	movq	%rax, -320(%rbp)
	andq	$-16, %rax
	subq	%rax, %rsp
	leaq	7(%rsp), %r15
	shrq	$3, %r15
	leaq	0(,%r15,8), %r8
	movq	%r8, %rdi
	movq	%r8, -304(%rbp)
	call	memset
	movq	-336(%rbp), %r9
	leal	-1(%r14), %ecx
	leaq	1(%r14), %r11
	movq	%rax, %r8
	movq	$0, 0(,%r15,8)
	movl	$1, %r10d
	movl	$1, %edx
	movl	$1, %r14d
	movq	%r9, %rsi
	salq	%cl, %rsi
	movq	%rsi, %rdi
	.p2align 4,,10
	.p2align 3
.L616:
	movq	%r14, %r9
	movl	%r10d, %ecx
	salq	%cl, %r9
	cmpq	%r9, %rdx
	jnb	.L614
	movl	%r12d, %ecx
	subl	-4(%r13,%r10,4), %ecx
	movq	%r14, %r15
	salq	$3, %rsi
	movq	%r8, %rax
	salq	%cl, %r15
	.p2align 4,,10
	.p2align 3
.L615:
	movq	(%rax), %rcx
	addq	$1, %rdx
	addq	%r15, %rcx
	movq	%rcx, (%rax,%rdi,8)
	addq	%rsi, %rax
	cmpq	%r9, %rdx
	jne	.L615
.L614:
	movq	%rdi, %rax
	addq	$1, %r10
	movq	%rdi, %rsi
	shrq	%rax
	cmpq	%r10, %r11
	je	.L667
	movq	%rax, %rdi
	jmp	.L616
	.p2align 4,,10
	.p2align 3
.L610:
	cmpl	$14, %ecx
	sete	%al
	testb	%sil, %sil
	je	.L626
	testb	%al, %al
	jne	.L768
.L626:
	testb	%dl, %dl
	je	.L609
	testb	%al, %al
	je	.L609
	movq	-248(%rbp), %r12
	movq	-240(%rbp), %rsi
	movl	$1, %eax
	movq	%rsp, -328(%rbp)
	subq	%r12, %rsi
	movq	%rsi, %r13
	sarq	$2, %r13
	movl	%r13d, %ecx
	salq	%cl, %rax
	movq	%rax, -280(%rbp)
	leaq	0(,%rax,8), %rdi
	movl	$8, %eax
	salq	%cl, %rax
	testq	%r13, %r13
	movq	%rax, -312(%rbp)
	je	.L642
	subl	$1, %ebx
	movq	%r12, %rax
	addq	%r12, %rsi
	xorl	%r15d, %r15d
	movl	$1, %r9d
	.p2align 4,,10
	.p2align 3
.L644:
	movl	%ebx, %ecx
	subl	(%rax), %ecx
	movq	%r9, %rdx
	addq	$4, %rax
	salq	%cl, %rdx
	orq	%rdx, %r15
	cmpq	%rax, %rsi
	jne	.L644
	leaq	22(%rdi), %rax
	movq	-312(%rbp), %rdx
	xorl	%esi, %esi
	movq	%r9, -336(%rbp)
	movq	%rax, -320(%rbp)
	andq	$-16, %rax
	subq	%rax, %rsp
	leaq	7(%rsp), %r14
	shrq	$3, %r14
	leaq	0(,%r14,8), %r8
	movq	%r8, %rdi
	movq	%r8, -304(%rbp)
	call	memset
	movq	-336(%rbp), %r9
	leal	-1(%r13), %ecx
	leaq	1(%r13), %r11
	movq	%rax, %r8
	movq	$0, 0(,%r14,8)
	movl	$1, %r10d
	movl	$1, %edx
	movl	$1, %r13d
	movq	%r9, %rsi
	salq	%cl, %rsi
	movq	%rsi, %rdi
	.p2align 4,,10
	.p2align 3
.L647:
	movq	%r13, %r9
	movl	%r10d, %ecx
	salq	%cl, %r9
	cmpq	%r9, %rdx
	jnb	.L645
	movl	%ebx, %ecx
	subl	-4(%r12,%r10,4), %ecx
	movq	%r13, %r14
	salq	$3, %rsi
	movq	%r8, %rax
	salq	%cl, %r14
	.p2align 4,,10
	.p2align 3
.L646:
	movq	(%rax), %rcx
	addq	$1, %rdx
	addq	%r14, %rcx
	movq	%rcx, (%rax,%rdi,8)
	addq	%rsi, %rax
	cmpq	%r9, %rdx
	jne	.L646
.L645:
	movq	%rdi, %rax
	addq	$1, %r10
	movq	%rdi, %rsi
	shrq	%rax
	cmpq	%r10, %r11
	je	.L669
	movq	%rax, %rdi
	jmp	.L647
.L594:
	movq	%rax, %rdx
	leaq	22(%rdi), %rax
	xorl	%esi, %esi
	xorl	%r12d, %r12d
	movq	%rax, -320(%rbp)
	andq	$-16, %rax
	subq	%rax, %rsp
	leaq	7(%rsp), %rbx
	shrq	$3, %rbx
	leaq	0(,%rbx,8), %rax
	movq	%rax, %rdi
	movq	%rax, -304(%rbp)
	call	memset
	movq	$0, 0(,%rbx,8)
	.p2align 4,,10
	.p2align 3
.L666:
	movq	-320(%rbp), %rax
	movq	-312(%rbp), %rdx
	xorl	%esi, %esi
	andq	$-16, %rax
	subq	%rax, %rsp
	movq	%rsp, %rcx
	movq	%rcx, %rdi
	call	memset
	movq	-280(%rbp), %rbx
	movq	%rax, %rcx
	movq	-296(%rbp), %rax
	xorl	%edx, %edx
	xorl	%edi, %edi
	divq	%rbx
	movq	-304(%rbp), %rdx
	leaq	-1(%rbx), %rsi
	movq	%rsi, -296(%rbp)
	movq	%rdx, %r14
	shrq	$3, %r14
	andl	$1, %r14d
	subq	%r14, %rbx
	leaq	0(,%r14,8), %rsi
	movq	%rbx, %r15
	movq	%rbx, %r9
	andq	$-2, %rbx
	movq	%r15, -320(%rbp)
	movq	-288(%rbp), %r15
	leaq	(%rdx,%rsi), %r8
	movq	%rbx, -312(%rbp)
	addq	%r14, %rbx
	shrq	%r9
	addq	%rcx, %rsi
	movq	%rbx, -336(%rbp)
	xorl	%edx, %edx
.L758:
	cmpq	%rdi, %rax
	jbe	.L649
.L770:
	movq	%rdx, %r10
	andq	%r12, %r10
	je	.L769
	addq	%r10, %rdx
	cmpq	%rdi, %rax
	ja	.L770
.L649:
	movq	-328(%rbp), %rsp
.L609:
	movq	-200(%rbp), %rdi
	testq	%rdi, %rdi
	je	.L657
	call	_ZdlPv
.L657:
	movq	-224(%rbp), %rdi
	testq	%rdi, %rdi
	je	.L658
	call	_ZdlPv
.L658:
	movq	-248(%rbp), %rdi
	testq	%rdi, %rdi
	je	.L659
	call	_ZdlPv
.L659:
	movq	-264(%rbp), %rax
	movq	-272(%rbp), %rdi
	cmpq	%rdi, %rax
	je	.L660
	leaq	24(%rdi), %rbx
	subq	%rbx, %rax
	shrq	$3, %rax
	leaq	24(%rdi,%rax,8), %r12
	jmp	.L661
	.p2align 4,,10
	.p2align 3
.L771:
	call	_ZdlPv
	cmpq	%r12, %rbx
	movq	%rbx, %rdi
	je	.L663
.L664:
	addq	$24, %rbx
.L661:
	movq	(%rdi), %rdi
	testq	%rdi, %rdi
	jne	.L771
	cmpq	%r12, %rbx
	movq	%rbx, %rdi
	jne	.L664
.L663:
	movq	-272(%rbp), %rdi
.L660:
	testq	%rdi, %rdi
	je	.L576
	call	_ZdlPv
.L576:
	movq	-56(%rbp), %rax
	xorq	%fs:40, %rax
	jne	.L772
	leaq	-40(%rbp), %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	ret
	.p2align 4,,10
	.p2align 3
.L769:
	.cfi_restore_state
	addq	$1, %rdi
	cmpq	$0, -280(%rbp)
	je	.L603
	cmpq	$2, -296(%rbp)
	jbe	.L604
	testq	%r14, %r14
	je	.L605
	movq	-304(%rbp), %rbx
	movq	(%rbx), %r10
	addq	%rdx, %r10
	movq	%r10, (%rcx)
.L605:
	movq	%rdx, -288(%rbp)
	xorl	%r10d, %r10d
	xorl	%r11d, %r11d
	vmovddup	-288(%rbp), %xmm1
	.p2align 4,,10
	.p2align 3
.L606:
	addq	$1, %r11
	vpaddq	(%r8,%r10), %xmm1, %xmm0
	vmovups	%xmm0, (%rsi,%r10)
	addq	$16, %r10
	cmpq	%r9, %r11
	jb	.L606
	movq	-312(%rbp), %r11
	cmpq	%r11, -320(%rbp)
	je	.L603
	movq	-336(%rbp), %r10
.L604:
	movq	-304(%rbp), %r13
	movq	0(%r13,%r10,8), %r11
	addq	%rdx, %r11
	movq	%r11, (%rcx,%r10,8)
	leaq	1(%r10), %r11
	cmpq	-280(%rbp), %r11
	jnb	.L603
	movq	0(%r13,%r11,8), %rbx
	addq	$2, %r10
	addq	%rdx, %rbx
	cmpq	%r10, -280(%rbp)
	movq	%rbx, (%rcx,%r11,8)
	jbe	.L603
	movq	0(%r13,%r10,8), %r11
	addq	%rdx, %r11
	movq	%r11, (%rcx,%r10,8)
.L603:
	movq	(%rcx), %r10
	addq	$1, %rdx
	leaq	(%r15,%r10,8), %r13
	movq	8(%rcx), %r10
	vmovss	4(%r13), %xmm8
	leaq	(%r15,%r10,8), %rbx
	movq	16(%rcx), %r10
	vmovss	0(%r13), %xmm11
	vmovss	(%rbx), %xmm4
	leaq	(%r15,%r10,8), %r11
	movq	24(%rcx), %r10
	vmovss	4(%rbx), %xmm7
	vmovss	(%r11), %xmm5
	leaq	(%r15,%r10,8), %r10
	vaddss	%xmm5, %xmm4, %xmm3
	vmovss	4(%r11), %xmm6
	vsubss	%xmm5, %xmm4, %xmm4
	vmovss	4(%r10), %xmm2
	vaddss	%xmm6, %xmm7, %xmm1
	vaddss	%xmm2, %xmm8, %xmm9
	vmovss	(%r10), %xmm0
	vsubss	%xmm2, %xmm8, %xmm2
	vaddss	%xmm0, %xmm11, %xmm10
	vsubss	%xmm6, %xmm7, %xmm5
	vsubss	%xmm0, %xmm11, %xmm0
	vsubss	%xmm2, %xmm3, %xmm6
	vaddss	%xmm2, %xmm3, %xmm2
	vmovss	%xmm6, 0(%r13)
	vaddss	%xmm0, %xmm1, %xmm6
	vsubss	%xmm0, %xmm1, %xmm0
	vmovss	%xmm6, 4(%r13)
	vsubss	%xmm5, %xmm10, %xmm6
	vaddss	%xmm5, %xmm10, %xmm5
	vmovss	%xmm6, (%rbx)
	vaddss	%xmm4, %xmm9, %xmm6
	vsubss	%xmm4, %xmm9, %xmm4
	vmovss	%xmm6, 4(%rbx)
	vmovss	%xmm5, (%r11)
	vmovss	%xmm4, 4(%r11)
	vmovss	%xmm2, (%r10)
	vmovss	%xmm0, 4(%r10)
	jmp	.L758
	.p2align 4,,10
	.p2align 3
.L768:
	movq	-248(%rbp), %r12
	movq	-240(%rbp), %rsi
	movl	$1, %eax
	movq	%rsp, -328(%rbp)
	subq	%r12, %rsi
	movq	%rsi, %r13
	sarq	$2, %r13
	movl	%r13d, %ecx
	salq	%cl, %rax
	movq	%rax, -280(%rbp)
	leaq	0(,%rax,8), %rdi
	movl	$8, %eax
	salq	%cl, %rax
	testq	%r13, %r13
	movq	%rax, -312(%rbp)
	je	.L627
	subl	$1, %ebx
	movq	%r12, %rax
	addq	%r12, %rsi
	xorl	%r15d, %r15d
	movl	$1, %r9d
	.p2align 4,,10
	.p2align 3
.L629:
	movl	%ebx, %ecx
	subl	(%rax), %ecx
	movq	%r9, %rdx
	addq	$4, %rax
	salq	%cl, %rdx
	orq	%rdx, %r15
	cmpq	%rax, %rsi
	jne	.L629
	leaq	22(%rdi), %rax
	movq	-312(%rbp), %rdx
	xorl	%esi, %esi
	movq	%r9, -336(%rbp)
	movq	%rax, -320(%rbp)
	andq	$-16, %rax
	subq	%rax, %rsp
	leaq	7(%rsp), %r14
	shrq	$3, %r14
	leaq	0(,%r14,8), %r8
	movq	%r8, %rdi
	movq	%r8, -304(%rbp)
	call	memset
	movq	-336(%rbp), %r9
	leal	-1(%r13), %ecx
	leaq	1(%r13), %r11
	movq	%rax, %r8
	movq	$0, 0(,%r14,8)
	movl	$1, %r10d
	movl	$1, %edx
	movl	$1, %r13d
	movq	%r9, %rsi
	salq	%cl, %rsi
	movq	%rsi, %rdi
	.p2align 4,,10
	.p2align 3
.L632:
	movq	%r13, %r9
	movl	%r10d, %ecx
	salq	%cl, %r9
	cmpq	%rdx, %r9
	jbe	.L630
	movl	%ebx, %ecx
	subl	-4(%r12,%r10,4), %ecx
	movq	%r13, %r14
	salq	$3, %rsi
	movq	%r8, %rax
	salq	%cl, %r14
	.p2align 4,,10
	.p2align 3
.L631:
	movq	(%rax), %rcx
	addq	$1, %rdx
	addq	%r14, %rcx
	movq	%rcx, (%rax,%rdi,8)
	addq	%rsi, %rax
	cmpq	%r9, %rdx
	jne	.L631
.L630:
	movq	%rdi, %rax
	addq	$1, %r10
	movq	%rdi, %rsi
	shrq	%rax
	cmpq	%r10, %r11
	je	.L668
	movq	%rax, %rdi
	jmp	.L632
.L580:
	leaq	-4(%rax), %rdx
	leaq	-272(%rbp), %rax
	leaq	24(%rax), %rdi
.LEHB17:
	call	_ZNSt6vectorIiSaIiEE17_M_realloc_insertIJRKiEEEvN9__gnu_cxx17__normal_iteratorIPiS1_EEDpOT_
	jmp	.L581
.L578:
	leaq	-272(%rbp), %rax
	subq	$4, %rdx
	leaq	24(%rax), %rdi
	call	_ZNSt6vectorIiSaIiEE17_M_realloc_insertIJRKiEEEvN9__gnu_cxx17__normal_iteratorIPiS1_EEDpOT_
.LEHE17:
	jmp	.L579
.L611:
	movq	%rax, %rdx
	leaq	22(%rdi), %rax
	xorl	%esi, %esi
	movq	%rax, -320(%rbp)
	andq	$-16, %rax
	subq	%rax, %rsp
	leaq	7(%rsp), %rbx
	shrq	$3, %rbx
	leaq	0(,%rbx,8), %rax
	movq	%rax, %rdi
	movq	%rax, -304(%rbp)
	call	memset
	movq	$0, 0(,%rbx,8)
	xorl	%ebx, %ebx
	.p2align 4,,10
	.p2align 3
.L667:
	movq	-320(%rbp), %rax
	movq	-312(%rbp), %rdx
	xorl	%esi, %esi
	andq	$-16, %rax
	subq	%rax, %rsp
	movq	%rsp, %rcx
	movq	%rcx, %rdi
	call	memset
	movq	-280(%rbp), %rsi
	movq	%rax, %rcx
	movq	-296(%rbp), %rax
	xorl	%edx, %edx
	divq	%rsi
	movq	-304(%rbp), %rdx
	leaq	-1(%rsi), %rdi
	movq	%rdi, -296(%rbp)
	xorl	%edi, %edi
	movq	%rdx, %r14
	shrq	$3, %r14
	andl	$1, %r14d
	subq	%r14, %rsi
	movq	%rsi, %r15
	movq	%rsi, %r9
	leaq	0(,%r14,8), %rsi
	movq	%r15, -320(%rbp)
	shrq	%r9
	leaq	(%rdx,%rsi), %r8
	movq	%r15, %rdx
	movq	-288(%rbp), %r15
	andq	$-2, %rdx
	addq	%rcx, %rsi
	movq	%rdx, -312(%rbp)
	addq	%r14, %rdx
	movq	%rdx, -336(%rbp)
	xorl	%edx, %edx
.L760:
	cmpq	%rdi, %rax
	jbe	.L649
	movq	%rdx, %r10
	andq	%rbx, %r10
	je	.L773
.L619:
	addq	%r10, %rdx
	cmpq	%rdi, %rax
	jbe	.L649
	movq	%rdx, %r10
	andq	%rbx, %r10
	jne	.L619
.L773:
	addq	$1, %rdi
	cmpq	$0, -280(%rbp)
	je	.L620
	cmpq	$2, -296(%rbp)
	jbe	.L621
	testq	%r14, %r14
	je	.L622
	movq	-304(%rbp), %r11
	movq	(%r11), %r10
	addq	%rdx, %r10
	movq	%r10, (%rcx)
.L622:
	movq	%rdx, -288(%rbp)
	xorl	%r10d, %r10d
	xorl	%r11d, %r11d
	vmovddup	-288(%rbp), %xmm1
	.p2align 4,,10
	.p2align 3
.L623:
	addq	$1, %r11
	vpaddq	(%r8,%r10), %xmm1, %xmm0
	vmovups	%xmm0, (%rsi,%r10)
	addq	$16, %r10
	cmpq	%r9, %r11
	jb	.L623
	movq	-320(%rbp), %r10
	cmpq	%r10, -312(%rbp)
	je	.L620
	movq	-336(%rbp), %r10
.L621:
	movq	-304(%rbp), %r13
	movq	0(%r13,%r10,8), %r11
	addq	%rdx, %r11
	movq	%r11, (%rcx,%r10,8)
	leaq	1(%r10), %r11
	cmpq	-280(%rbp), %r11
	jnb	.L620
	movq	0(%r13,%r11,8), %r12
	addq	$2, %r10
	addq	%rdx, %r12
	cmpq	-280(%rbp), %r10
	movq	%r12, (%rcx,%r11,8)
	jnb	.L620
	movq	0(%r13,%r10,8), %r11
	addq	%rdx, %r11
	movq	%r11, (%rcx,%r10,8)
.L620:
	movq	(%rcx), %r10
	addq	$1, %rdx
	leaq	(%r15,%r10,8), %r13
	movq	8(%rcx), %r10
	vmovss	4(%r13), %xmm5
	leaq	(%r15,%r10,8), %r12
	movq	16(%rcx), %r10
	vmovss	0(%r13), %xmm11
	vmovss	4(%r12), %xmm2
	leaq	(%r15,%r10,8), %r11
	movq	24(%rcx), %r10
	vsubss	%xmm2, %xmm5, %xmm10
	vmovss	(%r12), %xmm0
	vaddss	%xmm2, %xmm5, %xmm2
	vmovss	(%r11), %xmm1
	vsubss	%xmm0, %xmm11, %xmm8
	leaq	(%r15,%r10,8), %r10
	vmovss	4(%r11), %xmm7
	vmovss	(%r10), %xmm3
	vsubss	%xmm3, %xmm1, %xmm9
	vmovss	4(%r10), %xmm6
	vaddss	%xmm3, %xmm1, %xmm1
	vaddss	%xmm6, %xmm7, %xmm3
	vsubss	%xmm10, %xmm9, %xmm4
	vmovss	%xmm4, 0(%r13)
	vsubss	%xmm6, %xmm8, %xmm4
	vaddss	%xmm7, %xmm4, %xmm4
	vmovss	%xmm4, 4(%r13)
	vaddss	%xmm0, %xmm11, %xmm4
	vsubss	%xmm2, %xmm1, %xmm0
	vaddss	%xmm1, %xmm2, %xmm1
	vmovss	%xmm0, (%r12)
	vaddss	%xmm3, %xmm4, %xmm0
	vsubss	%xmm3, %xmm4, %xmm3
	vmovss	%xmm0, 4(%r12)
	vsubss	%xmm6, %xmm7, %xmm0
	vsubss	%xmm0, %xmm8, %xmm0
	vmovss	%xmm0, (%r11)
	vaddss	%xmm9, %xmm10, %xmm0
	vmovss	%xmm0, 4(%r11)
	vmovss	%xmm3, (%r10)
	vmovss	%xmm1, 4(%r10)
	jmp	.L760
.L627:
	movq	%rax, %rdx
	leaq	22(%rdi), %rax
	xorl	%esi, %esi
	xorl	%r15d, %r15d
	movq	%rax, -320(%rbp)
	andq	$-16, %rax
	subq	%rax, %rsp
	leaq	7(%rsp), %rbx
	shrq	$3, %rbx
	leaq	0(,%rbx,8), %rax
	movq	%rax, %rdi
	movq	%rax, -304(%rbp)
	call	memset
	movq	$0, 0(,%rbx,8)
	.p2align 4,,10
	.p2align 3
.L668:
	movq	-320(%rbp), %rax
	movq	-312(%rbp), %rdx
	xorl	%esi, %esi
	andq	$-16, %rax
	subq	%rax, %rsp
	movq	%rsp, %rcx
	movq	%rcx, %rdi
	call	memset
	movq	-280(%rbp), %rbx
	movq	%rax, %rcx
	movq	-296(%rbp), %rax
	xorl	%edx, %edx
	movq	-288(%rbp), %r14
	divq	%rbx
	leaq	-1(%rbx), %rsi
	movq	%rsi, -296(%rbp)
	xorl	%esi, %esi
	movq	%rax, %r12
	movq	-304(%rbp), %rax
	movq	%rax, %r13
	shrq	$3, %r13
	andl	$1, %r13d
	leaq	0(,%r13,8), %rdx
	subq	%r13, %rbx
	movq	%rbx, %r8
	movq	%rbx, -320(%rbp)
	leaq	(%rax,%rdx), %rdi
	movq	%rbx, %rax
	shrq	%r8
	andq	$-2, %rax
	addq	%rcx, %rdx
	movq	%rax, -312(%rbp)
	addq	%r13, %rax
	movq	%rax, -336(%rbp)
	xorl	%eax, %eax
.L762:
	cmpq	%rsi, %r12
	jbe	.L649
	movq	%rax, %r9
	andq	%r15, %r9
	je	.L774
.L635:
	addq	%r9, %rax
	cmpq	%rsi, %r12
	jbe	.L649
	movq	%rax, %r9
	andq	%r15, %r9
	jne	.L635
.L774:
	addq	$1, %rsi
	cmpq	$0, -280(%rbp)
	je	.L636
	cmpq	$2, -296(%rbp)
	jbe	.L637
	testq	%r13, %r13
	je	.L638
	movq	-304(%rbp), %rbx
	movq	(%rbx), %r9
	addq	%rax, %r9
	movq	%r9, (%rcx)
.L638:
	movq	%rax, -288(%rbp)
	xorl	%r9d, %r9d
	xorl	%r10d, %r10d
	vmovddup	-288(%rbp), %xmm1
	.p2align 4,,10
	.p2align 3
.L639:
	addq	$1, %r10
	vpaddq	(%rdi,%r9), %xmm1, %xmm0
	vmovups	%xmm0, (%rdx,%r9)
	addq	$16, %r9
	cmpq	%r8, %r10
	jb	.L639
	movq	-320(%rbp), %r11
	cmpq	%r11, -312(%rbp)
	je	.L636
	movq	-336(%rbp), %r9
.L637:
	movq	-304(%rbp), %rbx
	movq	(%rbx,%r9,8), %r10
	addq	%rax, %r10
	movq	%r10, (%rcx,%r9,8)
	leaq	1(%r9), %r10
	cmpq	-280(%rbp), %r10
	jnb	.L636
	movq	(%rbx,%r10,8), %r11
	addq	$2, %r9
	addq	%rax, %r11
	cmpq	-280(%rbp), %r9
	movq	%r11, (%rcx,%r10,8)
	jnb	.L636
	movq	(%rbx,%r9,8), %r10
	addq	%rax, %r10
	movq	%r10, (%rcx,%r9,8)
.L636:
	movq	(%rcx), %r9
	addq	$1, %rax
	leaq	(%r14,%r9,8), %rbx
	movq	8(%rcx), %r9
	vmovss	4(%rbx), %xmm6
	leaq	(%r14,%r9,8), %r11
	movq	16(%rcx), %r9
	vmovss	(%rbx), %xmm8
	vmovss	(%r11), %xmm1
	leaq	(%r14,%r9,8), %r10
	movq	24(%rcx), %r9
	vsubss	%xmm6, %xmm1, %xmm10
	vmovss	4(%r11), %xmm7
	vaddss	%xmm1, %xmm6, %xmm1
	vmovss	4(%r10), %xmm0
	vaddss	%xmm7, %xmm8, %xmm9
	leaq	(%r14,%r9,8), %r9
	vmovss	(%r10), %xmm3
	vsubss	%xmm7, %xmm8, %xmm7
	vmovss	(%r9), %xmm2
	vsubss	%xmm0, %xmm2, %xmm5
	vmovss	4(%r9), %xmm4
	vaddss	%xmm4, %xmm3, %xmm6
	vaddss	%xmm2, %xmm0, %xmm0
	vsubss	%xmm4, %xmm3, %xmm3
	vsubss	%xmm5, %xmm10, %xmm2
	vaddss	%xmm5, %xmm10, %xmm5
	vmovss	%xmm2, (%rbx)
	vsubss	%xmm6, %xmm9, %xmm2
	vmovss	%xmm2, 4(%rbx)
	vsubss	%xmm3, %xmm7, %xmm2
	vaddss	%xmm3, %xmm7, %xmm3
	vmovss	%xmm2, (%r11)
	vsubss	%xmm0, %xmm1, %xmm2
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	%xmm2, 4(%r11)
	vaddss	%xmm6, %xmm9, %xmm2
	vmovss	%xmm5, (%r10)
	vmovss	%xmm2, 4(%r10)
	vmovss	%xmm3, (%r9)
	vmovss	%xmm0, 4(%r9)
	jmp	.L762
.L642:
	movq	%rax, %rdx
	leaq	22(%rdi), %rax
	xorl	%esi, %esi
	xorl	%r15d, %r15d
	movq	%rax, -320(%rbp)
	andq	$-16, %rax
	subq	%rax, %rsp
	leaq	7(%rsp), %rbx
	shrq	$3, %rbx
	leaq	0(,%rbx,8), %rax
	movq	%rax, %rdi
	movq	%rax, -304(%rbp)
	call	memset
	movq	$0, 0(,%rbx,8)
	.p2align 4,,10
	.p2align 3
.L669:
	movq	-320(%rbp), %rax
	movq	-312(%rbp), %rdx
	xorl	%esi, %esi
	andq	$-16, %rax
	subq	%rax, %rsp
	movq	%rsp, %rcx
	movq	%rcx, %rdi
	call	memset
	movq	-280(%rbp), %rbx
	movq	%rax, %rcx
	movq	-296(%rbp), %rax
	xorl	%edx, %edx
	vmovss	.LC1(%rip), %xmm5
	xorl	%edi, %edi
	divq	%rbx
	movq	-304(%rbp), %rdx
	leaq	-1(%rbx), %rsi
	movq	%rsi, -296(%rbp)
	movq	%rdx, %r13
	shrq	$3, %r13
	andl	$1, %r13d
	subq	%r13, %rbx
	leaq	0(,%r13,8), %rsi
	movq	%rbx, %r14
	movq	%rbx, %r9
	andq	$-2, %rbx
	movq	%r14, -320(%rbp)
	movq	-288(%rbp), %r14
	leaq	(%rdx,%rsi), %r8
	movq	%rbx, -312(%rbp)
	addq	%r13, %rbx
	shrq	%r9
	addq	%rcx, %rsi
	movq	%rbx, -336(%rbp)
	xorl	%edx, %edx
.L764:
	cmpq	%rdi, %rax
	jbe	.L649
	movq	%r15, %r10
	andq	%rdx, %r10
	je	.L775
.L650:
	addq	%r10, %rdx
	cmpq	%rdi, %rax
	jbe	.L649
	movq	%r15, %r10
	andq	%rdx, %r10
	jne	.L650
.L775:
	addq	$1, %rdi
	cmpq	$0, -280(%rbp)
	je	.L651
	cmpq	$2, -296(%rbp)
	jbe	.L652
	testq	%r13, %r13
	je	.L653
	movq	-304(%rbp), %rbx
	movq	(%rbx), %r10
	addq	%rdx, %r10
	movq	%r10, (%rcx)
.L653:
	movq	%rdx, -288(%rbp)
	xorl	%r10d, %r10d
	xorl	%r11d, %r11d
	vmovddup	-288(%rbp), %xmm1
	.p2align 4,,10
	.p2align 3
.L654:
	addq	$1, %r11
	vpaddq	(%r8,%r10), %xmm1, %xmm0
	vmovups	%xmm0, (%rsi,%r10)
	addq	$16, %r10
	cmpq	%r9, %r11
	jb	.L654
	movq	-320(%rbp), %r11
	cmpq	%r11, -312(%rbp)
	je	.L651
	movq	-336(%rbp), %r10
.L652:
	movq	-304(%rbp), %r12
	movq	(%r12,%r10,8), %r11
	addq	%rdx, %r11
	movq	%r11, (%rcx,%r10,8)
	leaq	1(%r10), %r11
	cmpq	%r11, -280(%rbp)
	jbe	.L651
	movq	(%r12,%r11,8), %rbx
	addq	$2, %r10
	addq	%rdx, %rbx
	cmpq	-280(%rbp), %r10
	movq	%rbx, (%rcx,%r11,8)
	jnb	.L651
	movq	(%r12,%r10,8), %r11
	addq	%rdx, %r11
	movq	%r11, (%rcx,%r10,8)
.L651:
	movq	(%rcx), %r10
	addq	$1, %rdx
	leaq	(%r14,%r10,8), %r12
	movq	8(%rcx), %r10
	vmovss	4(%r12), %xmm7
	leaq	(%r14,%r10,8), %rbx
	movq	16(%rcx), %r10
	vmovss	(%r12), %xmm4
	vmovss	4(%rbx), %xmm3
	leaq	(%r14,%r10,8), %r11
	movq	24(%rcx), %r10
	vmovss	(%rbx), %xmm11
	vmovss	4(%r11), %xmm2
	leaq	(%r14,%r10,8), %r10
	vaddss	%xmm2, %xmm3, %xmm9
	vmovss	(%r11), %xmm6
	vsubss	%xmm2, %xmm3, %xmm2
	vmovss	4(%r10), %xmm0
	vaddss	%xmm0, %xmm7, %xmm10
	vmovss	(%r10), %xmm1
	vaddss	%xmm1, %xmm4, %xmm8
	vsubss	%xmm0, %xmm7, %xmm0
	vaddss	%xmm6, %xmm11, %xmm7
	vsubss	%xmm10, %xmm9, %xmm3
	vsubss	%xmm1, %xmm4, %xmm4
	vsubss	%xmm6, %xmm11, %xmm1
	vmovss	%xmm3, (%r12)
	vsubss	%xmm7, %xmm8, %xmm3
	vmovss	%xmm3, 4(%r12)
	vaddss	%xmm2, %xmm0, %xmm3
	vsubss	%xmm0, %xmm2, %xmm0
	vxorps	%xmm5, %xmm3, %xmm3
	vmovss	%xmm3, (%rbx)
	vaddss	%xmm1, %xmm4, %xmm3
	vsubss	%xmm1, %xmm4, %xmm1
	vmovss	%xmm3, 4(%rbx)
	vmovss	%xmm0, (%r11)
	vaddss	%xmm9, %xmm10, %xmm0
	vmovss	%xmm1, 4(%r11)
	vxorps	%xmm5, %xmm0, %xmm0
	vmovss	%xmm0, (%r10)
	vaddss	%xmm7, %xmm8, %xmm0
	vmovss	%xmm0, 4(%r10)
	jmp	.L764
.L772:
	call	__stack_chk_fail
.L675:
	movq	%rax, %rbx
	vzeroupper
.L592:
	leaq	-272(%rbp), %rdi
	call	_ZN4GateD1Ev
	movq	%rbx, %rdi
.LEHB18:
	call	_Unwind_Resume
.LEHE18:
.L676:
	leaq	-160(%rbp), %rdi
	movq	%rax, %rbx
	vzeroupper
	call	_ZN4GateD1Ev
	jmp	.L592
	.cfi_endproc
.LFE10379:
	.section	.gcc_except_table
.LLSDA10379:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE10379-.LLSDACSB10379
.LLSDACSB10379:
	.uleb128 .LEHB14-.LFB10379
	.uleb128 .LEHE14-.LEHB14
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB15-.LFB10379
	.uleb128 .LEHE15-.LEHB15
	.uleb128 .L675-.LFB10379
	.uleb128 0
	.uleb128 .LEHB16-.LFB10379
	.uleb128 .LEHE16-.LEHB16
	.uleb128 .L676-.LFB10379
	.uleb128 0
	.uleb128 .LEHB17-.LFB10379
	.uleb128 .LEHE17-.LEHB17
	.uleb128 .L675-.LFB10379
	.uleb128 0
	.uleb128 .LEHB18-.LFB10379
	.uleb128 .LEHE18-.LEHB18
	.uleb128 0
	.uleb128 0
.LLSDACSE10379:
	.text
	.size	_Z16Merge2QXY12GatesR4GateS0_iPSt7complexIfEm, .-_Z16Merge2QXY12GatesR4GateS0_iPSt7complexIfEm
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.type	_GLOBAL__sub_I__Z12GroupCZGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE, @function
_GLOBAL__sub_I__Z12GroupCZGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE:
.LFB11646:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	$_ZStL8__ioinit, %edi
	call	_ZNSt8ios_base4InitC1Ev
	movl	$__dso_handle, %edx
	movl	$_ZStL8__ioinit, %esi
	movl	$_ZNSt8ios_base4InitD1Ev, %edi
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	jmp	__cxa_atexit
	.cfi_endproc
.LFE11646:
	.size	_GLOBAL__sub_I__Z12GroupCZGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE, .-_GLOBAL__sub_I__Z12GroupCZGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE
	.section	.init_array,"aw"
	.align 8
	.quad	_GLOBAL__sub_I__Z12GroupCZGatesRSt8valarrayImEiRKSt6vectorIiSaIiEE
	.section	.rodata
	.align 32
	.type	_ZL6kTGate, @object
	.size	_ZL6kTGate, 64
_ZL6kTGate:
	.long	1065353216
	.long	0
	.long	1060439283
	.long	1060439283
	.long	0
	.long	1065353216
	.long	3207922931
	.long	1060439283
	.long	3212836864
	.long	0
	.long	3207922931
	.long	3207922931
	.long	0
	.long	3212836864
	.long	1060439283
	.long	3207922931
	.local	_ZStL8__ioinit
	.comm	_ZStL8__ioinit,1,1
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC1:
	.long	2147483648
	.long	0
	.long	0
	.long	0
	.section	.rodata.cst4,"aM",@progbits,4
	.align 4
.LC2:
	.long	1060439283
	.hidden	__dso_handle
	.ident	"GCC: (Ubuntu 7.1.0-10ubuntu1~16.04.york0) 7.1.0"
	.section	.note.GNU-stack,"",@progbits

	.file	1 "xhci-mtk-scheduler.c"
	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 3
	.abicalls
	.local	ss_out_eps
	.comm	ss_out_eps,256,4
	.local	ss_in_eps
	.comm	ss_in_eps,256,4
	.local	hs_eps
	.comm	hs_eps,256,4
	.local	tt_intr_eps
	.comm	tt_intr_eps,256,4
	.text
	.align	2
	.globl	mtk_xhci_scheduler_init
	.set	nomips16
	.ent	mtk_xhci_scheduler_init
	.type	mtk_xhci_scheduler_init, @function
mtk_xhci_scheduler_init:
	.frame	$fp,24,$31		# vars= 8, regs= 1/0, args= 0, gp= 8
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$fp,20($sp)
	move	$fp,$sp
	sw	$0,8($fp)
	.option	pic0
	j	$L2
	nop

	.option	pic2
$L3:
	lui	$2,%hi(ss_out_eps)
	lw	$3,8($fp)
	nop
	sll	$3,$3,2
	addiu	$2,$2,%lo(ss_out_eps)
	addu	$2,$3,$2
	sw	$0,0($2)
	lw	$2,8($fp)
	nop
	addiu	$2,$2,1
	sw	$2,8($fp)
$L2:
	lw	$2,8($fp)
	nop
	slt	$2,$2,64
	bne	$2,$0,$L3
	nop

	sw	$0,8($fp)
	.option	pic0
	j	$L4
	nop

	.option	pic2
$L5:
	lui	$2,%hi(ss_in_eps)
	lw	$3,8($fp)
	nop
	sll	$3,$3,2
	addiu	$2,$2,%lo(ss_in_eps)
	addu	$2,$3,$2
	sw	$0,0($2)
	lw	$2,8($fp)
	nop
	addiu	$2,$2,1
	sw	$2,8($fp)
$L4:
	lw	$2,8($fp)
	nop
	slt	$2,$2,64
	bne	$2,$0,$L5
	nop

	sw	$0,8($fp)
	.option	pic0
	j	$L6
	nop

	.option	pic2
$L7:
	lui	$2,%hi(hs_eps)
	lw	$3,8($fp)
	nop
	sll	$3,$3,2
	addiu	$2,$2,%lo(hs_eps)
	addu	$2,$3,$2
	sw	$0,0($2)
	lw	$2,8($fp)
	nop
	addiu	$2,$2,1
	sw	$2,8($fp)
$L6:
	lw	$2,8($fp)
	nop
	slt	$2,$2,64
	bne	$2,$0,$L7
	nop

	sw	$0,8($fp)
	.option	pic0
	j	$L8
	nop

	.option	pic2
$L9:
	lui	$2,%hi(tt_intr_eps)
	lw	$3,8($fp)
	nop
	sll	$3,$3,2
	addiu	$2,$2,%lo(tt_intr_eps)
	addu	$2,$3,$2
	sw	$0,0($2)
	lw	$2,8($fp)
	nop
	addiu	$2,$2,1
	sw	$2,8($fp)
$L8:
	lw	$2,8($fp)
	nop
	slt	$2,$2,64
	bne	$2,$0,$L9
	nop

	move	$2,$0
	move	$sp,$fp
	lw	$fp,20($sp)
	addiu	$sp,$sp,24
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	mtk_xhci_scheduler_init
	.size	mtk_xhci_scheduler_init, .-mtk_xhci_scheduler_init
	.align	2
	.globl	add_sch_ep
	.set	nomips16
	.ent	add_sch_ep
	.type	add_sch_ep, @function
add_sch_ep:
	.frame	$fp,24,$31		# vars= 8, regs= 1/0, args= 0, gp= 8
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$fp,20($sp)
	move	$fp,$sp
	sw	$4,24($fp)
	sw	$5,28($fp)
	sw	$6,32($fp)
	sw	$7,36($fp)
	lw	$2,28($fp)
	nop
	beq	$2,$0,$L11
	nop

	lw	$3,24($fp)
	li	$2,5			# 0x5
	bne	$3,$2,$L11
	nop

	lui	$2,%hi(ss_in_eps)
	addiu	$2,$2,%lo(ss_in_eps)
	sw	$2,8($fp)
	.option	pic0
	j	$L12
	nop

	.option	pic2
$L11:
	lw	$3,24($fp)
	li	$2,5			# 0x5
	bne	$3,$2,$L13
	nop

	lui	$2,%hi(ss_out_eps)
	addiu	$2,$2,%lo(ss_out_eps)
	sw	$2,8($fp)
	.option	pic0
	j	$L12
	nop

	.option	pic2
$L13:
	lw	$3,24($fp)
	li	$2,3			# 0x3
	beq	$3,$2,$L14
	nop

	lw	$2,32($fp)
	nop
	beq	$2,$0,$L15
	nop

	lw	$3,36($fp)
	li	$2,1			# 0x1
	bne	$3,$2,$L15
	nop

$L14:
	lui	$2,%hi(hs_eps)
	addiu	$2,$2,%lo(hs_eps)
	sw	$2,8($fp)
	.option	pic0
	j	$L12
	nop

	.option	pic2
$L15:
	lui	$2,%hi(tt_intr_eps)
	addiu	$2,$2,%lo(tt_intr_eps)
	sw	$2,8($fp)
$L12:
	sw	$0,12($fp)
	.option	pic0
	j	$L16
	nop

	.option	pic2
$L19:
	lw	$2,12($fp)
	nop
	sll	$2,$2,2
	lw	$3,8($fp)
	nop
	addu	$2,$3,$2
	lw	$2,0($2)
	nop
	bne	$2,$0,$L17
	nop

	lw	$2,84($fp)
	lw	$3,24($fp)
	nop
	sw	$3,0($2)
	lw	$2,84($fp)
	lw	$3,32($fp)
	nop
	sw	$3,4($2)
	lw	$2,84($fp)
	lw	$3,28($fp)
	nop
	sw	$3,8($2)
	lw	$2,84($fp)
	lw	$3,36($fp)
	nop
	sw	$3,12($2)
	lw	$2,84($fp)
	lw	$3,40($fp)
	nop
	sw	$3,16($2)
	lw	$2,84($fp)
	lw	$3,44($fp)
	nop
	sw	$3,20($2)
	lw	$2,84($fp)
	lw	$3,48($fp)
	nop
	sw	$3,24($2)
	lw	$2,84($fp)
	lw	$3,52($fp)
	nop
	sw	$3,28($2)
	lw	$2,84($fp)
	lw	$3,56($fp)
	nop
	sw	$3,32($2)
	lw	$2,84($fp)
	lw	$3,60($fp)
	nop
	sw	$3,36($2)
	lw	$2,84($fp)
	lw	$3,64($fp)
	nop
	sw	$3,40($2)
	lw	$2,84($fp)
	lw	$3,68($fp)
	nop
	sw	$3,44($2)
	lw	$2,84($fp)
	lw	$3,72($fp)
	nop
	sw	$3,48($2)
	lw	$2,84($fp)
	lw	$3,76($fp)
	nop
	sw	$3,52($2)
	lw	$2,84($fp)
	lw	$3,80($fp)
	nop
	sw	$3,56($2)
	lw	$2,12($fp)
	nop
	sll	$2,$2,2
	lw	$3,8($fp)
	nop
	addu	$2,$3,$2
	lw	$3,84($fp)
	nop
	sw	$3,0($2)
	li	$2,1			# 0x1
	.option	pic0
	j	$L18
	nop

	.option	pic2
$L17:
	lw	$2,12($fp)
	nop
	addiu	$2,$2,1
	sw	$2,12($fp)
$L16:
	lw	$2,12($fp)
	nop
	slt	$2,$2,64
	bne	$2,$0,$L19
	nop

	move	$2,$0
$L18:
	move	$sp,$fp
	lw	$fp,20($sp)
	addiu	$sp,$sp,24
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	add_sch_ep
	.size	add_sch_ep, .-add_sch_ep
	.align	2
	.globl	count_ss_bw
	.set	nomips16
	.ent	count_ss_bw
	.type	count_ss_bw, @function
count_ss_bw:
	.frame	$fp,96,$31		# vars= 80, regs= 1/0, args= 0, gp= 8
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-96
	sw	$fp,92($sp)
	move	$fp,$sp
	sw	$4,96($fp)
	sw	$5,100($fp)
	sw	$6,104($fp)
	sw	$7,108($fp)
	lw	$2,96($fp)
	nop
	beq	$2,$0,$L21
	nop

	lui	$2,%hi(ss_in_eps)
	addiu	$2,$2,%lo(ss_in_eps)
	sw	$2,28($fp)
	.option	pic0
	j	$L22
	nop

	.option	pic2
$L21:
	lui	$2,%hi(ss_out_eps)
	addiu	$2,$2,%lo(ss_out_eps)
	sw	$2,28($fp)
$L22:
	sw	$0,72($fp)
	sw	$0,76($fp)
	sw	$0,80($fp)
	lw	$2,124($fp)
	nop
	bne	$2,$0,$L23
	nop

	sw	$0,20($fp)
	sw	$0,8($fp)
	.option	pic0
	j	$L24
	nop

	.option	pic2
$L35:
	lw	$2,8($fp)
	nop
	sll	$2,$2,2
	lw	$3,28($fp)
	nop
	addu	$2,$3,$2
	lw	$2,0($2)
	nop
	sw	$2,40($fp)
	lw	$2,40($fp)
	nop
	beq	$2,$0,$L54
	nop

$L25:
	lw	$2,40($fp)
	nop
	lw	$2,20($2)
	nop
	sw	$2,44($fp)
	lw	$2,40($fp)
	nop
	lw	$2,32($2)
	nop
	sw	$2,48($fp)
	lw	$2,40($fp)
	nop
	lw	$2,36($2)
	nop
	bne	$2,$0,$L27
	nop

	lw	$3,44($fp)
	lw	$2,108($fp)
	nop
	slt	$2,$3,$2
	bne	$2,$0,$L28
	nop

	lw	$3,48($fp)
	lw	$2,44($fp)
	nop
	addu	$3,$3,$2
	lw	$2,120($fp)
	nop
	subu	$2,$3,$2
	sw	$2,32($fp)
	lw	$2,108($fp)
	nop
	sw	$2,36($fp)
	.option	pic0
	j	$L29
	nop

	.option	pic2
$L28:
	lw	$3,120($fp)
	lw	$2,108($fp)
	nop
	addu	$3,$3,$2
	lw	$2,48($fp)
	nop
	subu	$2,$3,$2
	sw	$2,32($fp)
	lw	$2,44($fp)
	nop
	sw	$2,36($fp)
$L29:
	lw	$3,32($fp)
	lw	$2,36($fp)
	nop
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$2
	bne	$2,$0,$L26
	nop

	lw	$2,40($fp)
	nop
	lw	$2,52($2)
	lw	$3,20($fp)
	nop
	addu	$2,$3,$2
	sw	$2,20($fp)
	.option	pic0
	j	$L26
	nop

	.option	pic2
$L27:
	lw	$2,40($fp)
	nop
	lw	$2,36($2)
	nop
	sw	$2,52($fp)
	lw	$2,40($fp)
	nop
	lw	$2,28($2)
	nop
	sw	$2,56($fp)
	sw	$0,16($fp)
	.option	pic0
	j	$L30
	nop

	.option	pic2
$L34:
	lw	$3,16($fp)
	lw	$2,56($fp)
	nop
	mult	$3,$2
	mflo	$3
	lw	$2,48($fp)
	nop
	addu	$2,$3,$2
	sw	$2,60($fp)
	lw	$3,44($fp)
	lw	$2,108($fp)
	nop
	slt	$2,$3,$2
	bne	$2,$0,$L31
	nop

	lw	$3,60($fp)
	lw	$2,44($fp)
	nop
	addu	$3,$3,$2
	lw	$2,120($fp)
	nop
	subu	$2,$3,$2
	sw	$2,32($fp)
	lw	$2,108($fp)
	nop
	sw	$2,36($fp)
	.option	pic0
	j	$L32
	nop

	.option	pic2
$L31:
	lw	$3,120($fp)
	lw	$2,108($fp)
	nop
	addu	$3,$3,$2
	lw	$2,60($fp)
	nop
	subu	$2,$3,$2
	sw	$2,32($fp)
	lw	$2,44($fp)
	nop
	sw	$2,36($fp)
$L32:
	lw	$3,32($fp)
	lw	$2,36($fp)
	nop
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$2
	bne	$2,$0,$L33
	nop

	lw	$2,40($fp)
	nop
	lw	$2,52($2)
	lw	$3,20($fp)
	nop
	addu	$2,$3,$2
	sw	$2,20($fp)
	.option	pic0
	j	$L26
	nop

	.option	pic2
$L33:
	lw	$2,16($fp)
	nop
	addiu	$2,$2,1
	sw	$2,16($fp)
$L30:
	lw	$3,16($fp)
	lw	$2,56($fp)
	nop
	slt	$2,$2,$3
	beq	$2,$0,$L34
	nop

	.option	pic0
	j	$L26
	nop

	.option	pic2
$L54:
	nop
$L26:
	lw	$2,8($fp)
	nop
	addiu	$2,$2,1
	sw	$2,8($fp)
$L24:
	lw	$2,8($fp)
	nop
	slt	$2,$2,64
	bne	$2,$0,$L35
	nop

	lw	$3,20($fp)
	lw	$2,128($fp)
	nop
	addu	$2,$3,$2
	sw	$2,20($fp)
	.option	pic0
	j	$L36
	nop

	.option	pic2
$L23:
	lw	$2,112($fp)
	nop
	addiu	$3,$2,1
	lw	$2,104($fp)
	nop
	mult	$3,$2
	mflo	$2
	sw	$2,64($fp)
	sw	$0,12($fp)
	.option	pic0
	j	$L37
	nop

	.option	pic2
$L50:
	sw	$0,24($fp)
	lw	$3,12($fp)
	lw	$2,124($fp)
	nop
	mult	$3,$2
	mflo	$3
	lw	$2,120($fp)
	nop
	addu	$2,$3,$2
	sw	$2,68($fp)
	sw	$0,8($fp)
	.option	pic0
	j	$L38
	nop

	.option	pic2
$L49:
	lw	$2,8($fp)
	nop
	sll	$2,$2,2
	lw	$3,28($fp)
	nop
	addu	$2,$3,$2
	lw	$2,0($2)
	nop
	sw	$2,40($fp)
	lw	$2,40($fp)
	nop
	beq	$2,$0,$L55
	nop

$L39:
	lw	$2,40($fp)
	nop
	lw	$2,20($2)
	nop
	sw	$2,44($fp)
	lw	$2,40($fp)
	nop
	lw	$2,32($2)
	nop
	sw	$2,48($fp)
	lw	$2,40($fp)
	nop
	lw	$2,36($2)
	nop
	bne	$2,$0,$L41
	nop

	lw	$3,44($fp)
	lw	$2,108($fp)
	nop
	slt	$2,$3,$2
	bne	$2,$0,$L42
	nop

	lw	$3,48($fp)
	lw	$2,44($fp)
	nop
	addu	$3,$3,$2
	lw	$2,68($fp)
	nop
	subu	$2,$3,$2
	sw	$2,32($fp)
	lw	$2,108($fp)
	nop
	sw	$2,36($fp)
	.option	pic0
	j	$L43
	nop

	.option	pic2
$L42:
	lw	$3,68($fp)
	lw	$2,108($fp)
	nop
	addu	$3,$3,$2
	lw	$2,48($fp)
	nop
	subu	$2,$3,$2
	sw	$2,32($fp)
	lw	$2,44($fp)
	nop
	sw	$2,36($fp)
$L43:
	lw	$3,32($fp)
	lw	$2,36($fp)
	nop
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$2
	bne	$2,$0,$L40
	nop

	lw	$2,40($fp)
	nop
	lw	$2,52($2)
	lw	$3,24($fp)
	nop
	addu	$2,$3,$2
	sw	$2,24($fp)
	.option	pic0
	j	$L40
	nop

	.option	pic2
$L41:
	lw	$2,40($fp)
	nop
	lw	$2,36($2)
	nop
	sw	$2,52($fp)
	lw	$2,40($fp)
	nop
	lw	$2,28($2)
	nop
	sw	$2,56($fp)
	sw	$0,16($fp)
	.option	pic0
	j	$L44
	nop

	.option	pic2
$L48:
	lw	$3,16($fp)
	lw	$2,52($fp)
	nop
	mult	$3,$2
	mflo	$3
	lw	$2,48($fp)
	nop
	addu	$2,$3,$2
	sw	$2,60($fp)
	lw	$3,44($fp)
	lw	$2,108($fp)
	nop
	slt	$2,$3,$2
	bne	$2,$0,$L45
	nop

	lw	$3,60($fp)
	lw	$2,44($fp)
	nop
	addu	$3,$3,$2
	lw	$2,68($fp)
	nop
	subu	$2,$3,$2
	sw	$2,32($fp)
	lw	$2,108($fp)
	nop
	sw	$2,36($fp)
	.option	pic0
	j	$L46
	nop

	.option	pic2
$L45:
	lw	$3,68($fp)
	lw	$2,108($fp)
	nop
	addu	$3,$3,$2
	lw	$2,60($fp)
	nop
	subu	$2,$3,$2
	sw	$2,32($fp)
	lw	$2,44($fp)
	nop
	sw	$2,36($fp)
$L46:
	lw	$3,32($fp)
	lw	$2,36($fp)
	nop
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$2
	bne	$2,$0,$L47
	nop

	lw	$2,40($fp)
	nop
	lw	$2,52($2)
	lw	$3,24($fp)
	nop
	addu	$2,$3,$2
	sw	$2,24($fp)
	.option	pic0
	j	$L40
	nop

	.option	pic2
$L47:
	lw	$2,16($fp)
	nop
	addiu	$2,$2,1
	sw	$2,16($fp)
$L44:
	lw	$3,16($fp)
	lw	$2,56($fp)
	nop
	slt	$2,$2,$3
	beq	$2,$0,$L48
	nop

	.option	pic0
	j	$L40
	nop

	.option	pic2
$L55:
	nop
$L40:
	lw	$2,8($fp)
	nop
	addiu	$2,$2,1
	sw	$2,8($fp)
$L38:
	lw	$2,8($fp)
	nop
	slt	$2,$2,64
	bne	$2,$0,$L49
	nop

	lw	$2,12($fp)
	nop
	sll	$2,$2,2
	addiu	$3,$fp,8
	addu	$2,$3,$2
	lw	$3,24($fp)
	nop
	sw	$3,64($2)
	lw	$2,12($fp)
	nop
	addiu	$2,$2,1
	sw	$2,12($fp)
$L37:
	lw	$3,12($fp)
	lw	$2,116($fp)
	nop
	slt	$2,$2,$3
	beq	$2,$0,$L50
	nop

	li	$2,51000			# 0xc738
	sw	$2,20($fp)
	sw	$0,12($fp)
	.option	pic0
	j	$L51
	nop

	.option	pic2
$L53:
	lw	$2,12($fp)
	nop
	sll	$2,$2,2
	addiu	$3,$fp,8
	addu	$2,$3,$2
	lw	$3,64($2)
	lw	$2,20($fp)
	nop
	slt	$2,$3,$2
	beq	$2,$0,$L52
	nop

	lw	$2,12($fp)
	nop
	sll	$2,$2,2
	addiu	$3,$fp,8
	addu	$2,$3,$2
	lw	$2,64($2)
	nop
	sw	$2,20($fp)
$L52:
	lw	$2,12($fp)
	nop
	addiu	$2,$2,1
	sw	$2,12($fp)
$L51:
	lw	$3,12($fp)
	lw	$2,116($fp)
	nop
	slt	$2,$2,$3
	beq	$2,$0,$L53
	nop

	lw	$3,20($fp)
	lw	$2,64($fp)
	nop
	addu	$2,$3,$2
	sw	$2,20($fp)
$L36:
	lw	$2,20($fp)
	move	$sp,$fp
	lw	$fp,92($sp)
	addiu	$sp,$sp,96
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	count_ss_bw
	.size	count_ss_bw, .-count_ss_bw
	.align	2
	.globl	count_hs_bw
	.set	nomips16
	.ent	count_hs_bw
	.type	count_hs_bw, @function
count_hs_bw:
	.frame	$fp,48,$31		# vars= 32, regs= 1/0, args= 0, gp= 8
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-48
	sw	$fp,44($sp)
	move	$fp,$sp
	sw	$4,48($fp)
	sw	$5,52($fp)
	sw	$6,56($fp)
	sw	$7,60($fp)
	sw	$0,12($fp)
	sw	$0,8($fp)
	.option	pic0
	j	$L57
	nop

	.option	pic2
$L67:
	lui	$2,%hi(hs_eps)
	lw	$3,8($fp)
	nop
	sll	$3,$3,2
	addiu	$2,$2,%lo(hs_eps)
	addu	$2,$3,$2
	lw	$2,0($2)
	nop
	sw	$2,24($fp)
	lw	$2,24($fp)
	nop
	beq	$2,$0,$L68
	nop

$L58:
	lw	$2,24($fp)
	nop
	lw	$2,32($2)
	nop
	sw	$2,28($fp)
	lw	$2,24($fp)
	nop
	lw	$2,20($2)
	nop
	sw	$2,32($fp)
	lw	$2,24($fp)
	nop
	lw	$2,4($2)
	nop
	beq	$2,$0,$L60
	nop

	lw	$2,24($fp)
	nop
	lw	$3,12($2)
	li	$2,1			# 0x1
	bne	$3,$2,$L60
	nop

	lw	$2,32($fp)
	nop
	sll	$2,$2,3
	sw	$2,36($fp)
	lw	$3,32($fp)
	lw	$2,56($fp)
	nop
	slt	$2,$3,$2
	bne	$2,$0,$L61
	nop

	lw	$3,28($fp)
	lw	$2,36($fp)
	nop
	addu	$3,$3,$2
	lw	$2,60($fp)
	nop
	subu	$2,$3,$2
	sw	$2,16($fp)
	lw	$2,56($fp)
	nop
	sw	$2,20($fp)
	.option	pic0
	j	$L62
	nop

	.option	pic2
$L61:
	lw	$3,60($fp)
	lw	$2,56($fp)
	nop
	addu	$3,$3,$2
	lw	$2,28($fp)
	nop
	subu	$2,$3,$2
	sw	$2,16($fp)
	lw	$2,36($fp)
	nop
	sw	$2,20($fp)
$L62:
	lw	$2,24($fp)
	nop
	lw	$2,8($2)
	nop
	beq	$2,$0,$L63
	nop

	lw	$3,16($fp)
	lw	$2,20($fp)
	nop
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$2
	slt	$2,$2,2
	bne	$2,$0,$L69
	nop

	lw	$3,16($fp)
	lw	$2,20($fp)
	nop
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$2
	move	$3,$2
	lw	$2,24($fp)
	nop
	lw	$2,44($2)
	nop
	slt	$2,$2,$3
	bne	$2,$0,$L69
	nop

	lw	$2,12($fp)
	nop
	addiu	$2,$2,188
	sw	$2,12($fp)
	.option	pic0
	j	$L69
	nop

	.option	pic2
$L63:
	lw	$3,16($fp)
	lw	$2,20($fp)
	nop
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$2
	move	$3,$2
	lw	$2,24($fp)
	nop
	lw	$2,44($2)
	nop
	slt	$2,$2,$3
	bne	$2,$0,$L69
	nop

	lw	$2,12($fp)
	nop
	addiu	$2,$2,188
	sw	$2,12($fp)
	.option	pic0
	j	$L69
	nop

	.option	pic2
$L60:
	lw	$3,32($fp)
	lw	$2,56($fp)
	nop
	slt	$2,$3,$2
	bne	$2,$0,$L65
	nop

	lw	$3,28($fp)
	lw	$2,32($fp)
	nop
	addu	$3,$3,$2
	lw	$2,60($fp)
	nop
	subu	$2,$3,$2
	sw	$2,16($fp)
	lw	$2,56($fp)
	nop
	sw	$2,20($fp)
	.option	pic0
	j	$L66
	nop

	.option	pic2
$L65:
	lw	$3,60($fp)
	lw	$2,56($fp)
	nop
	addu	$3,$3,$2
	lw	$2,28($fp)
	nop
	subu	$2,$3,$2
	sw	$2,16($fp)
	lw	$2,32($fp)
	nop
	sw	$2,20($fp)
$L66:
	lw	$3,16($fp)
	lw	$2,20($fp)
	nop
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$2
	bne	$2,$0,$L59
	nop

	lw	$2,24($fp)
	nop
	lw	$2,52($2)
	lw	$3,12($fp)
	nop
	addu	$2,$3,$2
	sw	$2,12($fp)
	.option	pic0
	j	$L59
	nop

	.option	pic2
$L68:
	nop
	.option	pic0
	j	$L59
	nop

	.option	pic2
$L69:
	nop
$L59:
	lw	$2,8($fp)
	nop
	addiu	$2,$2,1
	sw	$2,8($fp)
$L57:
	lw	$2,8($fp)
	nop
	slt	$2,$2,64
	bne	$2,$0,$L67
	nop

	lw	$3,12($fp)
	lw	$2,64($fp)
	nop
	addu	$2,$3,$2
	sw	$2,12($fp)
	lw	$2,12($fp)
	move	$sp,$fp
	lw	$fp,44($sp)
	addiu	$sp,$sp,48
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	count_hs_bw
	.size	count_hs_bw, .-count_hs_bw
	.align	2
	.globl	count_tt_isoc_bw
	.set	nomips16
	.ent	count_tt_isoc_bw
	.type	count_tt_isoc_bw, @function
count_tt_isoc_bw:
	.frame	$fp,88,$31		# vars= 72, regs= 1/0, args= 0, gp= 8
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-88
	sw	$fp,84($sp)
	move	$fp,$sp
	sw	$4,88($fp)
	sw	$5,92($fp)
	sw	$6,96($fp)
	sw	$7,100($fp)
	sb	$0,8($fp)
	lw	$2,96($fp)
	nop
	sll	$2,$2,3
	sw	$2,44($fp)
	lw	$2,88($fp)
	nop
	beq	$2,$0,$L71
	nop

	li	$2,1			# 0x1
	sb	$2,8($fp)
$L71:
	lw	$2,100($fp)
	nop
	sra	$3,$2,31
	srl	$3,$3,29
	addu	$2,$3,$2
	sra	$2,$2,3
	sw	$2,48($fp)
	lw	$3,100($fp)
	nop
	sra	$2,$3,31
	srl	$2,$2,29
	addu	$3,$3,$2
	andi	$3,$3,0x7
	subu	$2,$3,$2
	sw	$2,52($fp)
	lw	$2,92($fp)
	nop
	addiu	$2,$2,187
	li	$3,-1370750976			# 0xffffffffae4c0000
	ori	$3,$3,0x415d
	mult	$2,$3
	mfhi	$3
	addu	$3,$3,$2
	sra	$3,$3,7
	sra	$2,$2,31
	subu	$2,$3,$2
	sw	$2,24($fp)
	lb	$2,8($fp)
	nop
	beq	$2,$0,$L72
	nop

	lw	$3,100($fp)
	nop
	sra	$2,$3,31
	srl	$2,$2,29
	addu	$3,$3,$2
	andi	$3,$3,0x7
	subu	$2,$3,$2
	addiu	$3,$2,2
	lw	$2,24($fp)
	nop
	addu	$2,$3,$2
	sw	$2,56($fp)
	lw	$2,56($fp)
	nop
	slt	$2,$2,7
	beq	$2,$0,$L73
	nop

	lw	$2,24($fp)
	nop
	addiu	$2,$2,2
	sw	$2,24($fp)
	.option	pic0
	j	$L72
	nop

	.option	pic2
$L73:
	lw	$3,56($fp)
	li	$2,7			# 0x7
	bne	$3,$2,$L74
	nop

	lw	$2,24($fp)
	nop
	addiu	$2,$2,1
	sw	$2,24($fp)
	.option	pic0
	j	$L72
	nop

	.option	pic2
$L74:
	lw	$2,56($fp)
	nop
	slt	$2,$2,9
	bne	$2,$0,$L72
	nop

	li	$2,-1			# 0xffffffffffffffff
	.option	pic0
	j	$L75
	nop

	.option	pic2
$L72:
	sw	$0,20($fp)
	lw	$2,88($fp)
	nop
	beq	$2,$0,$L76
	nop

	li	$2,2			# 0x2
	sw	$2,28($fp)
$L76:
	lw	$3,100($fp)
	lw	$2,28($fp)
	nop
	addu	$2,$3,$2
	sw	$2,12($fp)
	.option	pic0
	j	$L77
	nop

	.option	pic2
$L91:
	sw	$0,16($fp)
	sw	$0,32($fp)
	.option	pic0
	j	$L78
	nop

	.option	pic2
$L89:
	lui	$2,%hi(hs_eps)
	lw	$3,32($fp)
	nop
	sll	$3,$3,2
	addiu	$2,$2,%lo(hs_eps)
	addu	$2,$3,$2
	lw	$2,0($2)
	nop
	sw	$2,60($fp)
	lw	$2,60($fp)
	nop
	beq	$2,$0,$L92
	nop

$L79:
	lw	$2,60($fp)
	nop
	lw	$2,32($2)
	nop
	sw	$2,64($fp)
	lw	$2,60($fp)
	nop
	lw	$2,20($2)
	nop
	sw	$2,68($fp)
	lw	$2,60($fp)
	nop
	lw	$2,4($2)
	nop
	beq	$2,$0,$L81
	nop

	lw	$2,60($fp)
	nop
	lw	$3,12($2)
	li	$2,1			# 0x1
	bne	$3,$2,$L81
	nop

	lw	$2,68($fp)
	nop
	sll	$2,$2,3
	sw	$2,72($fp)
	lw	$3,72($fp)
	lw	$2,44($fp)
	nop
	slt	$2,$3,$2
	bne	$2,$0,$L82
	nop

	lw	$3,64($fp)
	lw	$2,72($fp)
	nop
	addu	$3,$3,$2
	lw	$2,12($fp)
	nop
	subu	$2,$3,$2
	sw	$2,36($fp)
	lw	$2,44($fp)
	nop
	sw	$2,40($fp)
	.option	pic0
	j	$L83
	nop

	.option	pic2
$L82:
	lw	$3,12($fp)
	lw	$2,44($fp)
	nop
	addu	$3,$3,$2
	lw	$2,64($fp)
	nop
	subu	$2,$3,$2
	sw	$2,36($fp)
	lw	$2,72($fp)
	nop
	sw	$2,40($fp)
$L83:
	lw	$2,60($fp)
	nop
	lw	$2,8($2)
	nop
	beq	$2,$0,$L84
	nop

	lw	$3,36($fp)
	lw	$2,40($fp)
	nop
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$2
	slt	$2,$2,2
	bne	$2,$0,$L93
	nop

	lw	$3,36($fp)
	lw	$2,40($fp)
	nop
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$2
	move	$3,$2
	lw	$2,60($fp)
	nop
	lw	$2,44($2)
	nop
	slt	$2,$2,$3
	bne	$2,$0,$L93
	nop

	lw	$2,16($fp)
	nop
	addiu	$2,$2,188
	sw	$2,16($fp)
	.option	pic0
	j	$L93
	nop

	.option	pic2
$L84:
	lw	$3,36($fp)
	lw	$2,40($fp)
	nop
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$2
	move	$3,$2
	lw	$2,60($fp)
	nop
	lw	$2,44($2)
	nop
	slt	$2,$2,$3
	bne	$2,$0,$L93
	nop

	lw	$2,16($fp)
	nop
	addiu	$2,$2,188
	sw	$2,16($fp)
	.option	pic0
	j	$L93
	nop

	.option	pic2
$L81:
	lw	$2,60($fp)
	nop
	lw	$3,12($2)
	li	$2,3			# 0x3
	beq	$3,$2,$L86
	nop

	lw	$2,60($fp)
	nop
	lw	$3,12($2)
	li	$2,1			# 0x1
	bne	$3,$2,$L80
	nop

$L86:
	lw	$3,68($fp)
	lw	$2,44($fp)
	nop
	slt	$2,$3,$2
	bne	$2,$0,$L87
	nop

	lw	$3,64($fp)
	lw	$2,68($fp)
	nop
	addu	$3,$3,$2
	lw	$2,12($fp)
	nop
	subu	$2,$3,$2
	sw	$2,36($fp)
	lw	$2,44($fp)
	nop
	sw	$2,40($fp)
	.option	pic0
	j	$L88
	nop

	.option	pic2
$L87:
	lw	$3,12($fp)
	lw	$2,44($fp)
	nop
	addu	$3,$3,$2
	lw	$2,64($fp)
	nop
	subu	$2,$3,$2
	sw	$2,36($fp)
	lw	$2,68($fp)
	nop
	sw	$2,40($fp)
$L88:
	lw	$3,36($fp)
	lw	$2,40($fp)
	nop
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$2
	bne	$2,$0,$L80
	nop

	lw	$2,60($fp)
	nop
	lw	$2,52($2)
	lw	$3,16($fp)
	nop
	addu	$2,$3,$2
	sw	$2,16($fp)
	.option	pic0
	j	$L80
	nop

	.option	pic2
$L92:
	nop
	.option	pic0
	j	$L80
	nop

	.option	pic2
$L93:
	nop
$L80:
	lw	$2,32($fp)
	nop
	addiu	$2,$2,1
	sw	$2,32($fp)
$L78:
	lw	$2,32($fp)
	nop
	slt	$2,$2,64
	bne	$2,$0,$L89
	nop

	lw	$2,16($fp)
	nop
	addiu	$2,$2,188
	sw	$2,16($fp)
	lw	$3,16($fp)
	lw	$2,20($fp)
	nop
	slt	$2,$2,$3
	beq	$2,$0,$L90
	nop

	lw	$2,16($fp)
	nop
	sw	$2,20($fp)
$L90:
	lw	$2,12($fp)
	nop
	addiu	$2,$2,1
	sw	$2,12($fp)
	lw	$2,28($fp)
	nop
	addiu	$2,$2,1
	sw	$2,28($fp)
$L77:
	lw	$3,28($fp)
	lw	$2,24($fp)
	nop
	slt	$2,$3,$2
	bne	$2,$0,$L91
	nop

	lw	$2,20($fp)
$L75:
	move	$sp,$fp
	lw	$fp,84($sp)
	addiu	$sp,$sp,88
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	count_tt_isoc_bw
	.size	count_tt_isoc_bw, .-count_tt_isoc_bw
	.align	2
	.globl	count_tt_intr_bw
	.set	nomips16
	.ent	count_tt_intr_bw
	.type	count_tt_intr_bw, @function
count_tt_intr_bw:
	.frame	$fp,48,$31		# vars= 32, regs= 1/0, args= 0, gp= 8
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-48
	sw	$fp,44($sp)
	move	$fp,$sp
	sw	$4,48($fp)
	sw	$5,52($fp)
	li	$2,1			# 0x1
	sw	$2,20($fp)
	sw	$0,8($fp)
	.option	pic0
	j	$L95
	nop

	.option	pic2
$L101:
	lui	$2,%hi(tt_intr_eps)
	lw	$3,8($fp)
	nop
	sll	$3,$3,2
	addiu	$2,$2,%lo(tt_intr_eps)
	addu	$2,$3,$2
	lw	$2,0($2)
	nop
	sw	$2,24($fp)
	lw	$2,24($fp)
	nop
	beq	$2,$0,$L102
	nop

$L96:
	lw	$2,24($fp)
	nop
	lw	$2,32($2)
	nop
	sw	$2,28($fp)
	lw	$2,24($fp)
	nop
	lw	$2,20($2)
	nop
	sw	$2,32($fp)
	lw	$3,32($fp)
	lw	$2,48($fp)
	nop
	slt	$2,$3,$2
	bne	$2,$0,$L98
	nop

	lw	$3,28($fp)
	lw	$2,32($fp)
	nop
	addu	$3,$3,$2
	lw	$2,52($fp)
	nop
	subu	$2,$3,$2
	sw	$2,12($fp)
	lw	$2,48($fp)
	nop
	sw	$2,16($fp)
	.option	pic0
	j	$L99
	nop

	.option	pic2
$L98:
	lw	$3,52($fp)
	lw	$2,48($fp)
	nop
	addu	$3,$3,$2
	lw	$2,28($fp)
	nop
	subu	$2,$3,$2
	sw	$2,12($fp)
	lw	$2,32($fp)
	nop
	sw	$2,16($fp)
$L99:
	lw	$3,12($fp)
	lw	$2,16($fp)
	nop
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$2
	bne	$2,$0,$L97
	nop

	move	$2,$0
	.option	pic0
	j	$L100
	nop

	.option	pic2
$L102:
	nop
$L97:
	lw	$2,8($fp)
	nop
	addiu	$2,$2,1
	sw	$2,8($fp)
$L95:
	lw	$2,8($fp)
	nop
	slt	$2,$2,64
	bne	$2,$0,$L101
	nop

	li	$2,1			# 0x1
$L100:
	move	$sp,$fp
	lw	$fp,44($sp)
	addiu	$sp,$sp,48
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	count_tt_intr_bw
	.size	count_tt_intr_bw, .-count_tt_intr_bw
	.align	2
	.globl	mtk_xhci_scheduler_remove_ep
	.set	nomips16
	.ent	mtk_xhci_scheduler_remove_ep
	.type	mtk_xhci_scheduler_remove_ep, @function
mtk_xhci_scheduler_remove_ep:
	.frame	$fp,32,$31		# vars= 16, regs= 1/0, args= 0, gp= 8
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$fp,28($sp)
	move	$fp,$sp
	sw	$4,32($fp)
	sw	$5,36($fp)
	sw	$6,40($fp)
	sw	$7,44($fp)
	lw	$2,36($fp)
	nop
	beq	$2,$0,$L104
	nop

	lw	$3,32($fp)
	li	$2,5			# 0x5
	bne	$3,$2,$L104
	nop

	lui	$2,%hi(ss_in_eps)
	addiu	$2,$2,%lo(ss_in_eps)
	sw	$2,12($fp)
	.option	pic0
	j	$L105
	nop

	.option	pic2
$L104:
	lw	$3,32($fp)
	li	$2,5			# 0x5
	bne	$3,$2,$L106
	nop

	lui	$2,%hi(ss_out_eps)
	addiu	$2,$2,%lo(ss_out_eps)
	sw	$2,12($fp)
	.option	pic0
	j	$L105
	nop

	.option	pic2
$L106:
	lw	$3,32($fp)
	li	$2,3			# 0x3
	beq	$3,$2,$L107
	nop

	lw	$2,40($fp)
	nop
	beq	$2,$0,$L108
	nop

	lw	$3,44($fp)
	li	$2,1			# 0x1
	bne	$3,$2,$L108
	nop

$L107:
	lui	$2,%hi(hs_eps)
	addiu	$2,$2,%lo(hs_eps)
	sw	$2,12($fp)
	.option	pic0
	j	$L105
	nop

	.option	pic2
$L108:
	lui	$2,%hi(tt_intr_eps)
	addiu	$2,$2,%lo(tt_intr_eps)
	sw	$2,12($fp)
$L105:
	sw	$0,8($fp)
	.option	pic0
	j	$L109
	nop

	.option	pic2
$L112:
	lw	$2,8($fp)
	nop
	sll	$2,$2,2
	lw	$3,12($fp)
	nop
	addu	$2,$3,$2
	lw	$2,0($2)
	nop
	sw	$2,16($fp)
	lw	$2,16($fp)
	nop
	beq	$2,$0,$L110
	nop

	lw	$2,16($fp)
	nop
	lw	$3,56($2)
	lw	$2,48($fp)
	nop
	bne	$3,$2,$L110
	nop

	lw	$2,8($fp)
	nop
	sll	$2,$2,2
	lw	$3,12($fp)
	nop
	addu	$2,$3,$2
	sw	$0,0($2)
	lw	$2,16($fp)
	.option	pic0
	j	$L111
	nop

	.option	pic2
$L110:
	lw	$2,8($fp)
	nop
	addiu	$2,$2,1
	sw	$2,8($fp)
$L109:
	lw	$2,8($fp)
	nop
	slt	$2,$2,64
	bne	$2,$0,$L112
	nop

	move	$2,$0
$L111:
	move	$sp,$fp
	lw	$fp,28($sp)
	addiu	$sp,$sp,32
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	mtk_xhci_scheduler_remove_ep
	.size	mtk_xhci_scheduler_remove_ep, .-mtk_xhci_scheduler_remove_ep
	.align	2
	.globl	mtk_xhci_scheduler_add_ep
	.set	nomips16
	.ent	mtk_xhci_scheduler_add_ep
	.type	mtk_xhci_scheduler_add_ep, @function
mtk_xhci_scheduler_add_ep:
	.frame	$fp,168,$31		# vars= 88, regs= 2/0, args= 64, gp= 8
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-168
	sw	$31,164($sp)
	sw	$fp,160($sp)
	move	$fp,$sp
	sw	$4,168($fp)
	sw	$5,172($fp)
	sw	$6,176($fp)
	sw	$7,180($fp)
	sw	$0,72($fp)
	sw	$0,76($fp)
	li	$2,1			# 0x1
	sw	$2,132($fp)
	sw	$0,80($fp)
	sw	$0,84($fp)
	lw	$2,176($fp)
	nop
	beq	$2,$0,$L114
	nop

	lw	$3,180($fp)
	li	$2,3			# 0x3
	bne	$3,$2,$L114
	nop

	lw	$3,168($fp)
	li	$2,1			# 0x1
	beq	$3,$2,$L115
	nop

	lw	$3,168($fp)
	li	$2,2			# 0x2
	bne	$3,$2,$L114
	nop

$L115:
	lw	$2,188($fp)
	nop
	sra	$2,$2,3
	sw	$2,136($fp)
	sw	$0,96($fp)
	.option	pic0
	j	$L116
	nop

	.option	pic2
$L121:
	lw	$4,136($fp)
	lw	$5,96($fp)
	.option	pic0
	jal	count_tt_intr_bw
	nop

	.option	pic2
	move	$3,$2
	li	$2,1			# 0x1
	bne	$3,$2,$L117
	nop

	lw	$2,96($fp)
	nop
	sll	$2,$2,3
	sw	$2,80($fp)
	li	$2,1			# 0x1
	sw	$2,72($fp)
	li	$2,3			# 0x3
	sw	$2,76($fp)
	lw	$2,184($fp)
	nop
	sw	$2,100($fp)
	sw	$0,84($fp)
	lw	$6,80($fp)
	lw	$5,84($fp)
	lw	$4,72($fp)
	lw	$3,76($fp)
	lw	$2,132($fp)
	lw	$7,184($fp)
	nop
	sw	$7,16($sp)
	lw	$7,136($fp)
	nop
	sw	$7,20($sp)
	lw	$7,192($fp)
	nop
	sw	$7,24($sp)
	lw	$7,196($fp)
	nop
	sw	$7,28($sp)
	sw	$6,32($sp)
	sw	$5,36($sp)
	sw	$4,40($sp)
	sw	$3,44($sp)
	sw	$2,48($sp)
	lw	$2,184($fp)
	nop
	sw	$2,52($sp)
	lw	$2,200($fp)
	nop
	sw	$2,56($sp)
	lw	$2,208($fp)
	nop
	sw	$2,60($sp)
	lw	$4,168($fp)
	lw	$5,172($fp)
	lw	$6,176($fp)
	lw	$7,180($fp)
	.option	pic0
	jal	add_sch_ep
	nop

	.option	pic2
	bne	$2,$0,$L118
	nop

	move	$2,$0
	.option	pic0
	j	$L119
	nop

	.option	pic2
$L118:
	li	$2,1			# 0x1
	sw	$2,88($fp)
	nop
	.option	pic0
	j	$L122
	nop

	.option	pic2
$L117:
	lw	$2,96($fp)
	nop
	addiu	$2,$2,1
	sw	$2,96($fp)
$L116:
	lw	$3,96($fp)
	lw	$2,136($fp)
	nop
	slt	$2,$3,$2
	bne	$2,$0,$L121
	nop

	.option	pic0
	j	$L122
	nop

	.option	pic2
$L114:
	lw	$2,176($fp)
	nop
	beq	$2,$0,$L123
	nop

	lw	$3,180($fp)
	li	$2,1			# 0x1
	bne	$3,$2,$L123
	nop

	li	$2,6144			# 0x1800
	sw	$2,104($fp)
	li	$2,-1			# 0xffffffffffffffff
	sw	$2,108($fp)
	sw	$0,140($fp)
	lw	$2,184($fp)
	nop
	sw	$2,144($fp)
	sw	$0,128($fp)
	lw	$2,188($fp)
	nop
	sra	$2,$2,3
	sw	$2,136($fp)
	sw	$0,96($fp)
	.option	pic0
	j	$L124
	nop

	.option	pic2
$L131:
	sw	$0,92($fp)
	.option	pic0
	j	$L125
	nop

	.option	pic2
$L129:
	lw	$2,96($fp)
	nop
	sll	$3,$2,3
	lw	$2,92($fp)
	nop
	addu	$2,$3,$2
	sw	$2,124($fp)
	lw	$2,144($fp)
	nop
	sw	$2,16($sp)
	lw	$4,172($fp)
	lw	$5,184($fp)
	lw	$6,136($fp)
	lw	$7,124($fp)
	.option	pic0
	jal	count_tt_isoc_bw
	nop

	.option	pic2
	sw	$2,140($fp)
	lw	$2,140($fp)
	nop
	blez	$2,$L126
	nop

	lw	$3,140($fp)
	lw	$2,104($fp)
	nop
	slt	$2,$3,$2
	beq	$2,$0,$L126
	nop

	lw	$2,124($fp)
	nop
	sw	$2,108($fp)
	lw	$2,140($fp)
	nop
	sw	$2,104($fp)
	lw	$3,140($fp)
	lw	$2,144($fp)
	nop
	beq	$3,$2,$L127
	nop

	lw	$2,140($fp)
	nop
	slt	$2,$2,3072
	beq	$2,$0,$L126
	nop

$L127:
	li	$2,1			# 0x1
	sw	$2,128($fp)
	.option	pic0
	j	$L128
	nop

	.option	pic2
$L126:
	lw	$2,92($fp)
	nop
	addiu	$2,$2,1
	sw	$2,92($fp)
$L125:
	lw	$2,92($fp)
	nop
	slt	$2,$2,8
	bne	$2,$0,$L129
	nop

$L128:
	lw	$2,96($fp)
	nop
	addiu	$2,$2,1
	sw	$2,96($fp)
$L124:
	lw	$3,96($fp)
	lw	$2,136($fp)
	nop
	slt	$2,$3,$2
	beq	$2,$0,$L130
	nop

	lw	$2,128($fp)
	nop
	beq	$2,$0,$L131
	nop

$L130:
	lw	$3,108($fp)
	li	$2,-1			# 0xffffffffffffffff
	bne	$3,$2,$L132
	nop

	move	$2,$0
	.option	pic0
	j	$L119
	nop

	.option	pic2
$L132:
	lw	$2,108($fp)
	nop
	sw	$2,80($fp)
	li	$2,1			# 0x1
	sw	$2,72($fp)
	lw	$2,184($fp)
	nop
	addiu	$2,$2,187
	li	$3,-1370750976			# 0xffffffffae4c0000
	ori	$3,$3,0x415d
	mult	$2,$3
	mfhi	$3
	addu	$3,$3,$2
	sra	$3,$3,7
	sra	$2,$2,31
	subu	$2,$3,$2
	sw	$2,76($fp)
	lw	$2,172($fp)
	nop
	beq	$2,$0,$L133
	nop

	lw	$2,80($fp)
	nop
	andi	$3,$2,0x7
	lw	$2,76($fp)
	nop
	addu	$2,$3,$2
	addiu	$2,$2,2
	sw	$2,148($fp)
	lw	$2,148($fp)
	nop
	slt	$2,$2,7
	beq	$2,$0,$L134
	nop

	lw	$2,76($fp)
	nop
	addiu	$2,$2,2
	sw	$2,76($fp)
	.option	pic0
	j	$L133
	nop

	.option	pic2
$L134:
	lw	$3,148($fp)
	li	$2,7			# 0x7
	bne	$3,$2,$L133
	nop

	lw	$2,76($fp)
	nop
	addiu	$2,$2,1
	sw	$2,76($fp)
$L133:
	li	$2,188			# 0xbc
	sw	$2,100($fp)
	sw	$0,84($fp)
	lw	$6,80($fp)
	lw	$5,84($fp)
	lw	$4,72($fp)
	lw	$3,76($fp)
	lw	$2,132($fp)
	lw	$7,184($fp)
	nop
	sw	$7,16($sp)
	lw	$7,188($fp)
	nop
	sw	$7,20($sp)
	lw	$7,192($fp)
	nop
	sw	$7,24($sp)
	lw	$7,196($fp)
	nop
	sw	$7,28($sp)
	sw	$6,32($sp)
	sw	$5,36($sp)
	sw	$4,40($sp)
	sw	$3,44($sp)
	sw	$2,48($sp)
	lw	$2,100($fp)
	nop
	sw	$2,52($sp)
	lw	$2,200($fp)
	nop
	sw	$2,56($sp)
	lw	$2,208($fp)
	nop
	sw	$2,60($sp)
	lw	$4,168($fp)
	lw	$5,172($fp)
	lw	$6,176($fp)
	lw	$7,180($fp)
	.option	pic0
	jal	add_sch_ep
	nop

	.option	pic2
	bne	$2,$0,$L135
	nop

	move	$2,$0
	.option	pic0
	j	$L119
	nop

	.option	pic2
$L135:
	li	$2,1			# 0x1
	sw	$2,88($fp)
	.option	pic0
	j	$L122
	nop

	.option	pic2
$L123:
	lw	$3,168($fp)
	li	$2,2			# 0x2
	beq	$3,$2,$L136
	nop

	lw	$3,168($fp)
	li	$2,1			# 0x1
	bne	$3,$2,$L137
	nop

$L136:
	lw	$3,180($fp)
	li	$2,3			# 0x3
	bne	$3,$2,$L137
	nop

	li	$2,1			# 0x1
	sw	$2,72($fp)
	li	$2,1			# 0x1
	sw	$2,88($fp)
	.option	pic0
	j	$L122
	nop

	.option	pic2
$L137:
	lw	$3,168($fp)
	li	$2,2			# 0x2
	bne	$3,$2,$L138
	nop

	lw	$3,180($fp)
	li	$2,1			# 0x1
	bne	$3,$2,$L138
	nop

	li	$2,1			# 0x1
	sw	$2,72($fp)
	li	$2,1			# 0x1
	sw	$2,88($fp)
	.option	pic0
	j	$L122
	nop

	.option	pic2
$L138:
	lw	$3,168($fp)
	li	$2,3			# 0x3
	bne	$3,$2,$L139
	nop

	lw	$3,180($fp)
	li	$2,3			# 0x3
	beq	$3,$2,$L140
	nop

	lw	$3,180($fp)
	li	$2,1			# 0x1
	bne	$3,$2,$L139
	nop

$L140:
	li	$2,6144			# 0x1800
	sw	$2,104($fp)
	li	$2,-1			# 0xffffffffffffffff
	sw	$2,108($fp)
	sw	$0,140($fp)
	lw	$2,192($fp)
	nop
	addiu	$3,$2,1
	lw	$2,184($fp)
	nop
	mult	$3,$2
	mflo	$2
	sw	$2,144($fp)
	sw	$0,124($fp)
	.option	pic0
	j	$L141
	nop

	.option	pic2
$L144:
	lw	$2,144($fp)
	nop
	sw	$2,16($sp)
	lw	$4,180($fp)
	lw	$5,184($fp)
	lw	$6,188($fp)
	lw	$7,124($fp)
	.option	pic0
	jal	count_hs_bw
	nop

	.option	pic2
	sw	$2,140($fp)
	lw	$2,140($fp)
	nop
	blez	$2,$L142
	nop

	lw	$3,140($fp)
	lw	$2,104($fp)
	nop
	slt	$2,$3,$2
	beq	$2,$0,$L142
	nop

	lw	$2,124($fp)
	nop
	sw	$2,108($fp)
	lw	$2,140($fp)
	nop
	sw	$2,104($fp)
	lw	$3,140($fp)
	lw	$2,144($fp)
	nop
	beq	$3,$2,$L143
	nop

	lw	$2,140($fp)
	nop
	slt	$2,$2,3072
	bne	$2,$0,$L143
	nop

$L142:
	lw	$2,124($fp)
	nop
	addiu	$2,$2,1
	sw	$2,124($fp)
$L141:
	lw	$3,124($fp)
	lw	$2,188($fp)
	nop
	slt	$2,$3,$2
	bne	$2,$0,$L144
	nop

$L143:
	lw	$3,108($fp)
	li	$2,-1			# 0xffffffffffffffff
	bne	$3,$2,$L145
	nop

	move	$2,$0
	.option	pic0
	j	$L119
	nop

	.option	pic2
$L145:
	lw	$2,108($fp)
	nop
	sw	$2,80($fp)
	lw	$2,192($fp)
	nop
	addiu	$2,$2,1
	sw	$2,72($fp)
	sw	$0,76($fp)
	lw	$2,144($fp)
	nop
	sw	$2,100($fp)
	sw	$0,84($fp)
	lw	$6,80($fp)
	lw	$5,84($fp)
	lw	$4,72($fp)
	lw	$3,76($fp)
	lw	$2,132($fp)
	lw	$7,184($fp)
	nop
	sw	$7,16($sp)
	lw	$7,188($fp)
	nop
	sw	$7,20($sp)
	lw	$7,192($fp)
	nop
	sw	$7,24($sp)
	lw	$7,196($fp)
	nop
	sw	$7,28($sp)
	sw	$6,32($sp)
	sw	$5,36($sp)
	sw	$4,40($sp)
	sw	$3,44($sp)
	sw	$2,48($sp)
	lw	$2,100($fp)
	nop
	sw	$2,52($sp)
	lw	$2,200($fp)
	nop
	sw	$2,56($sp)
	lw	$2,208($fp)
	nop
	sw	$2,60($sp)
	lw	$4,168($fp)
	lw	$5,172($fp)
	lw	$6,176($fp)
	lw	$7,180($fp)
	.option	pic0
	jal	add_sch_ep
	nop

	.option	pic2
	bne	$2,$0,$L146
	nop

	move	$2,$0
	.option	pic0
	j	$L119
	nop

	.option	pic2
$L146:
	li	$2,1			# 0x1
	sw	$2,88($fp)
	.option	pic0
	j	$L122
	nop

	.option	pic2
$L139:
	lw	$3,168($fp)
	li	$2,5			# 0x5
	bne	$3,$2,$L147
	nop

	lw	$3,180($fp)
	li	$2,3			# 0x3
	beq	$3,$2,$L148
	nop

	lw	$3,180($fp)
	li	$2,1			# 0x1
	bne	$3,$2,$L147
	nop

$L148:
	li	$2,51000			# 0xc738
	sw	$2,104($fp)
	li	$2,-1			# 0xffffffffffffffff
	sw	$2,108($fp)
	sw	$0,140($fp)
	lw	$2,196($fp)
	nop
	addiu	$3,$2,1
	lw	$2,184($fp)
	nop
	mult	$3,$2
	mflo	$3
	lw	$2,192($fp)
	nop
	addiu	$2,$2,1
	mult	$3,$2
	mflo	$2
	sw	$2,144($fp)
	lw	$2,196($fp)
	nop
	bne	$2,$0,$L149
	nop

	sw	$0,116($fp)
	.option	pic0
	j	$L150
	nop

	.option	pic2
$L149:
	lw	$2,188($fp)
	nop
	addiu	$3,$2,-1
	lw	$2,196($fp)
	nop
	addiu	$2,$2,1
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mfhi	$3
	mflo	$2
	sw	$2,116($fp)
$L150:
	sw	$0,128($fp)
	sw	$0,96($fp)
	.option	pic0
	j	$L151
	nop

	.option	pic2
$L158:
	lw	$2,116($fp)
	nop
	sw	$2,112($fp)
	.option	pic0
	j	$L152
	nop

	.option	pic2
$L156:
	lw	$2,192($fp)
	nop
	sw	$2,16($sp)
	lw	$2,196($fp)
	nop
	sw	$2,20($sp)
	lw	$2,96($fp)
	nop
	sw	$2,24($sp)
	lw	$2,112($fp)
	nop
	sw	$2,28($sp)
	lw	$2,144($fp)
	nop
	sw	$2,32($sp)
	lw	$4,172($fp)
	lw	$5,180($fp)
	lw	$6,184($fp)
	lw	$7,188($fp)
	.option	pic0
	jal	count_ss_bw
	nop

	.option	pic2
	sw	$2,140($fp)
	lw	$2,140($fp)
	nop
	blez	$2,$L153
	nop

	lw	$3,140($fp)
	lw	$2,104($fp)
	nop
	slt	$2,$3,$2
	beq	$2,$0,$L153
	nop

	lw	$2,96($fp)
	nop
	sw	$2,108($fp)
	lw	$2,112($fp)
	nop
	sw	$2,120($fp)
	lw	$2,140($fp)
	nop
	sw	$2,104($fp)
	lw	$3,140($fp)
	lw	$2,144($fp)
	nop
	slt	$2,$2,$3
	beq	$2,$0,$L154
	nop

	lw	$2,140($fp)
	nop
	slt	$2,$2,3072
	beq	$2,$0,$L153
	nop

$L154:
	li	$2,1			# 0x1
	sw	$2,128($fp)
	.option	pic0
	j	$L155
	nop

	.option	pic2
$L153:
	lw	$2,112($fp)
	nop
	addiu	$2,$2,-1
	sw	$2,112($fp)
$L152:
	lw	$2,112($fp)
	nop
	bgez	$2,$L156
	nop

$L155:
	lw	$2,96($fp)
	nop
	addiu	$2,$2,1
	sw	$2,96($fp)
$L151:
	lw	$3,96($fp)
	lw	$2,188($fp)
	nop
	slt	$2,$3,$2
	beq	$2,$0,$L157
	nop

	lw	$2,128($fp)
	nop
	beq	$2,$0,$L158
	nop

$L157:
	lw	$3,108($fp)
	li	$2,-1			# 0xffffffffffffffff
	bne	$3,$2,$L159
	nop

	move	$2,$0
	.option	pic0
	j	$L119
	nop

	.option	pic2
$L159:
	lw	$2,108($fp)
	nop
	sw	$2,80($fp)
	sw	$0,76($fp)
	lw	$2,120($fp)
	nop
	sw	$2,84($fp)
	lw	$2,84($fp)
	nop
	bne	$2,$0,$L160
	nop

	lw	$2,192($fp)
	nop
	addiu	$3,$2,1
	lw	$2,196($fp)
	nop
	addiu	$2,$2,1
	mult	$3,$2
	mflo	$3
	lw	$2,184($fp)
	nop
	mult	$3,$2
	mflo	$2
	sw	$2,100($fp)
	lw	$2,192($fp)
	nop
	addiu	$3,$2,1
	lw	$2,196($fp)
	nop
	addiu	$2,$2,1
	mult	$3,$2
	mflo	$2
	sw	$2,72($fp)
	.option	pic0
	j	$L161
	nop

	.option	pic2
$L160:
	lw	$2,192($fp)
	nop
	addiu	$3,$2,1
	lw	$2,184($fp)
	nop
	mult	$3,$2
	mflo	$2
	sw	$2,100($fp)
	lw	$2,192($fp)
	nop
	addiu	$2,$2,1
	sw	$2,72($fp)
$L161:
	lw	$6,80($fp)
	lw	$5,84($fp)
	lw	$4,72($fp)
	lw	$3,76($fp)
	lw	$2,132($fp)
	lw	$7,184($fp)
	nop
	sw	$7,16($sp)
	lw	$7,188($fp)
	nop
	sw	$7,20($sp)
	lw	$7,192($fp)
	nop
	sw	$7,24($sp)
	lw	$7,196($fp)
	nop
	sw	$7,28($sp)
	sw	$6,32($sp)
	sw	$5,36($sp)
	sw	$4,40($sp)
	sw	$3,44($sp)
	sw	$2,48($sp)
	lw	$2,100($fp)
	nop
	sw	$2,52($sp)
	lw	$2,200($fp)
	nop
	sw	$2,56($sp)
	lw	$2,208($fp)
	nop
	sw	$2,60($sp)
	lw	$4,168($fp)
	lw	$5,172($fp)
	lw	$6,176($fp)
	lw	$7,180($fp)
	.option	pic0
	jal	add_sch_ep
	nop

	.option	pic2
	bne	$2,$0,$L162
	nop

	move	$2,$0
	.option	pic0
	j	$L119
	nop

	.option	pic2
$L162:
	li	$2,1			# 0x1
	sw	$2,88($fp)
	.option	pic0
	j	$L122
	nop

	.option	pic2
$L147:
	li	$2,1			# 0x1
	sw	$2,72($fp)
	li	$2,1			# 0x1
	sw	$2,88($fp)
$L122:
	lw	$3,88($fp)
	li	$2,1			# 0x1
	bne	$3,$2,$L163
	nop

	lw	$2,204($fp)
	nop
	sw	$2,152($fp)
	lw	$2,152($fp)
	nop
	lw	$3,20($2)
	lw	$2,72($fp)
	nop
	andi	$4,$2,0x3f
	lw	$2,76($fp)
	nop
	andi	$2,$2,0x7
	sll	$2,$2,8
	or	$4,$4,$2
	lw	$2,132($fp)
	nop
	sll	$2,$2,11
	or	$2,$4,$2
	or	$3,$3,$2
	lw	$2,152($fp)
	nop
	sw	$3,20($2)
	lw	$2,152($fp)
	nop
	lw	$3,24($2)
	lw	$2,80($fp)
	nop
	andi	$4,$2,0x3fff
	lw	$2,84($fp)
	nop
	andi	$2,$2,0x7fff
	sll	$2,$2,16
	or	$2,$4,$2
	or	$3,$3,$2
	lw	$2,152($fp)
	nop
	sw	$3,24($2)
	li	$2,1			# 0x1
	.option	pic0
	j	$L119
	nop

	.option	pic2
$L163:
	move	$2,$0
$L119:
	move	$sp,$fp
	lw	$31,164($sp)
	lw	$fp,160($sp)
	addiu	$sp,$sp,168
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	mtk_xhci_scheduler_add_ep
	.size	mtk_xhci_scheduler_add_ep, .-mtk_xhci_scheduler_add_ep
	.ident	"GCC: (Buildroot 2012.11.1) 4.6.3"

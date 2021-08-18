;edited by Avraham Tsurel
			

thing:	.dh		5,14,6,2,35,1   
thing2:	.asciz	"He will Return!" 


jmp	thing2
jmp thing    		

;	checker for	I Commands
;				reg		immed	reg
HelloI:	addi	$14  ,	123  ,  $2
		subi	$26 , 	0	,	$2  
		andi	$28,	-30,	$29
		ori		$10,-32768,$31 
		nori	$7,32767,$20
;				reg		reg		label
		bne		$23 , 	$23,	LABEL1   
		beq		$16,	$6,		LABEL1
		blt		$15,	$0,		LABEL2  
		bgt		$20,	$15,	LABEL3
;				reg		immed	reg 
		lb		$24  ,	123  ,  $25    
		sb		$26 , 	0	,	$27
		lw		$28,	-30,	$29
		sw		$30,-32768,$31 
		lh		$17,32767,$20
		sh		$26 , 	0	,	$27  


;	checker for	J Commands
HelloJ:	jmp		$7
		jmp		LABEL3
		la		LABEL3
		la		LABEL4
		call	LABEL4
		call	LABEL5
BYE:	stop

;	checker for	K Commands
;				reg reg reg
DownTo:	add		$0 ,$1 ,$1  
		sub		$0 ,$1 ,$1
		and		$2 ,$4 ,$3
		or		$7 ,$5,$3
		nor		$24 ,$10 ,$1 
		move	$15,$16
		mvhi	$18,$19
		mvlo	$21,$22


SameLine:	call	SameLine
SameLine2:	bne		$23,$23,SameLine2

;	Data Commands 
LABEL1:	.db		0,1,-3,+80, 1,1 ,-128,127  
LABEL2:	.dh		0,1,-3,+80, 1,1 ,-32768,32767 
LABEL3:	.dw		0,1,-3,+80, 1,1 ,-2147483648,2147483647  
LABEL4:	.asciz	"this is a vaild string"  
LABEL5:	.asciz	"and here another few: "
		.asciz	"!@#$%^&*()_+"    
		.asciz	" "
		.asciz	"	"
		.asciz	"""
		.asciz	" " " " "

DoLABELl2:	jmp		DoLABELl
			bne		$15, $6, DoLABELl2 

thing2:	.extern		DoLABELl
thing:	.entry		DoLABELl2

.entry LABEL1
.entry LABEL2
.entry LABEL3
.entry LABEL4
.entry LABEL5

.entry LABEL5
.entry LABEL5

.extern EXT1
.extern EXT2
.extern EXT3
.extern EXT4

.extern EXT4
.extern EXT4

		la		EXT1   
		la		EXT2
		jmp		EXT3  
		jmp		EXT4


		la		EXT1
		la		EXT1
		call	EXT2
		call	EXT2		

LongButStilla1234567ValidLabel: stop
; a vaild Label:
Blt:	stop
; another vaild Label:
AsciZ:	stop

.asciz "A long but a valid line. she has only 80 characters. that the maximum!!"

jmp thing2


edited by Avraham Tsurel
; mistake in header: a comment without a comma ;

; an .as file to check if the asembler define all possible problems

; @ problems with input: Too long lines @ :
.asciz "This line is too long, and has too many chars in it. you supposed to make it a shorter one."
;this comment is too long, and has too many chars in it. you supposed to make it a shorter one.
.asciz "This line is 81 chars long, including white notes. please do something!" 
;This comment is 81 chars long, including white notes. please  be  aware  to that

; @ problems in illegal registers @ :

; must be without a + sign
        sub     $+0 ,$1 ,$2  
        subi    $26 ,   0   ,   $+27  
        move    $+15,$16
        sw      $30,-32768,$+31 


; out of range: vaild registers are 0-31
        sub     $3 ,$-4 ,$5
        andi    $28,    -30,    $32
        beq     $16,    $-6,     ValidLabel
        or      $9 ,$10,$154
        sub     $6 ,$7 ,$32
        mvlo    $21,$34
        addi    $-24  ,  123  ,  $25
        bne     $32 ,   $23,    ValidLabel 
        lb      $32  ,  123  ,  $25    
        lw      $-28,    -30,    $29   
        mvhi    $18,$-19


; invaild space in register.
        nor     $12,$1 3,$14 
        blt     $ 15,   $0,     ValidLabel  
        sb      $26 ,   0   ,   $ 27
        sh      26$ ,   0   ,   $27 5         
        jmp     $ 7        

; missed $ in before register
        and     $6 ,7 ,$5
        lh      $17,32767,20
        or      $9 ,$10,7
        nor     12,$13,$14 

; Added unnecessary $ sign
        or      $$9 ,$10,$11
        nor     $12,$13,$$14 
        move    $15,$$16
        mvhi    $$18,$19


; @ problems in comma @ :

; Added unnecessary comma sign
        add     $0 ,$1 ,$2 , 
        sub     $3 ,,$4 ,$5

        mvlo    $21,,$22
        move    $15,$16,
        addi    $24  ,  123 , ,  $25
        andi    $28,    -3,0,    $29,
        blt     $15,    $0,  ,   ValidLabel  
        bgt     $20,    $15,    ,ValidLabel
        sb      $26, ,   0   ,   $27
        lw      $28,    ,-30,    $29
        lh      $17,,32767,$20
        .dh     0,5,-3,+80, 1,,1 ,-32768,32767  
        jmp     $7,
        jmp     ,$7
        .dw     0,5,-3,+80, 1, ,1 ,-2147483648,2147483647  

; unnecessary comma sign in beginning of the line
        and     ,$6 ,$7 ,$8
        mvhi    ,$18,$19
        ori     ,$30,-32768,$31 
        beq     ,$16,    $6,     ValidLabel
        .db     ,0,5,-3,+80, 1,1 ,-128,127  


; missed comma sign
        nor     $12,$13$14 
        move    $15.$16
        .dh     0,5,-3,+80. 1,1 ,-128,127  


;too many parameters in line
        or      $9 ,$1,0,$11
        subi    $13 ,   0   ,   $1,4
        nori    $17,327,67,$20
        sw      $29 ,   0   ,   $1,2 

; unnecessary comma sign last in line
        bne     $23 ,   $23,    ValidLabel  , 
        lb      $24  ,  123  ,  $25 ,   
        sh      $14,-32768,$31 ,
        .db     0,5,-3,+80, 1,1 ,-128,127 , 
 

; @ problems in immed @ :

; immed with dot
        addi    $24  ,  12.3  ,  $25
        lb      $24  ,  1.23  ,  $25    

; immed with couple signs (+-) 
        subi    $26 ,   +-0   ,   $27  
        andi    $28,    --30,    $29
        sb      $26 ,   -+0   ,   $27

; immed with invaild char 
        lw      $28,    -3 0,    $29
        sh      $26 ,   !0   ,   $27
        sh      $26 ,   $0   ,   $2  


; @ problems in operands @ :

;mistaken number of operands 
        add     $0 ,$1 ,$2 ,$2 
        sub     $3 ,$4 
        and     $6 ,$7 ,$8,$3 ,$4 
        move    $15
        mvhi    $18,$19,$3 ,$4 
        mvlo    $21,$22,$0
        addi    $24   ,  $25
        subi    $26 ,   0   ,   $27  $3 
        andi    $28, 
        ori     -32768
        bne     $23 ,       ValidLabel   
        beq     $16,    $6,    
        blt     $15,    $0,   $3   ValidLabel  
        bgt     $20,    $15,    ValidLabel, ValidLabel 
        lb      $24  ,  123     
        sb      $26 ,   0   ,   $27,$3 ,$4
        lw      $28,    -30,    $29,$3 
        sw      $30,$31 
        jmp     ValidLabel, ValidLabel
        jmp     $7,$3
        la      $5
        call    $2
        stop $5

; @ problems in asciz string @
        .asciz  "without quotationmark int the end
        .asciz  without quotationmark in the beginning"
        .asciz  without quotationmark at all
        .asciz  "
        .asciz  "

; @ problems in vaild range of immed @ :
        ori     $30,-32769,$31 
        nori    $17,32768,$20
        sw      $30,-32769,$31 
        lh      $17,32768,$20

; @ problems in vaild range of data @ :
        .db     0,5,-3,+80, 1,1 ,-129,127  
        .db     0,5,-3,+80, 1,1 ,-128,128  
        .dh     0,5,-3,+80, 1,1 ,-32769,32767 
        .dh     0,5,-3,+80, 1,1 ,-32768,32768 
        .dw     0,5,-3,+80, 1,1 ,-2147483649,2147483647
        .dw     0,5,-3,+80, 1,1 ,-2147483648,2147483648

; @ problems in commands name:
        And     $6 ,$7 ,$8
        movE    $15,$16
        andii    $28,    -30,    $29
        !lh      $30,-32768,$31
        c-ll    meMaybe 
        .asci   "you forgot the 'z' letter"

; @ problems in labels names:

; label cannot start with number
4mylord:  add     $0 ,$1 ,$2

; missed colon in the end
unstoppable     move       $15,$16

; using saved words as label
move:  add     $0 ,$1 ,$2
asciz:  move    $15,$16

; missed label name
:       move    $15,$16


;commands cant accept extern label:
        bne     $23 ,   $23,    ExternLabel   
        beq     $16,    $6,     ExternLabel
        blt     $15,    $0,     ExternLabel  
        bgt     $20,    $15,    ExternLabel

;commands cant accept undefined labels
        jmp     UnDefinedLabel
        call    NotDefinedLabel
        bne     $22, $13, NotDefinedLabel

; cannot define an extern label as entry label
.extern ExternLabel
.entry  ExternLabel
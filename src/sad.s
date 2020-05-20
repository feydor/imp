; sad.s -- WORKING
default rel

    section .data
; all arrays are in row-major order
;~ template:   dq  2, 5, 5
            ;~ dq  4, 0, 7
            ;~ dq  7, 5, 9
 template:   dq  1, 1, 1
             dq  2, 2, 2
             dq  3, 3, 3

tmax_rows:   dq  3
tmax_cols:   dq  3

print_string:  
            db  "val: %d ", 0

print_comment:  
            db "printing: %d", 10, 0
format:     db "%d ", 0
newline:    db "", 10, 0

;~ frame:      dq  2, 7, 5, 8, 6
            ;~ dq  1, 7, 4, 2, 7
            ;~ dq  8, 4, 6, 8, 5
 ;~ frame:      dq  1, 1, 1, 1, 1
             ;~ dq  2, 2, 2, 2, 2
             ;~ dq  3, 3, 3, 3, 3
             ;~ dq  4, 4, 4, 4, 4
			 ;~ dq	 5, 5, 5, 5, 5
 frame:      dq  10, 10, 10, 10, 10
             dq  20, 2, 2, 2, 20
             dq  30, 2, 2, 2, 30
             dq  40, 2, 2, 2, 40
			 dq	 50, 50, 50, 50, 50

f_height:   dq  5
f_width:    dq  5

result:     dq  0, 0, 0
            dq  0, 0, 0
            dq  0, 0, 0
lowest_sad:   dq  255

    ;global main
    extern printf
    extern exit

    %define offset  8
    %define t_height 3
    %define t_width 3
main1:
    ; register usage:
	; 	Bytes Location  Description
    ;       8 r14       displacement between first element and current element
    ;       8 rbx       pointer to template
    ;       8 rcx       stores temporary template value per loop
    ;       8 rsi       stores temp loop var   

    ; args: %1 f_x, %2 f_y, %3 f_width 
    %macro  byte_displacement 3
        mov     rax, %2            ; rax = f_y
        mul     %3                 ; rax = f_y * f_width
        add     rax, %1            ; rax = f_y * f_width + f_x
        imul    rax, offset        ; rax = (f_y * f_width + f_x) * offset            
    %endmacro

    %macro sad_body 0
        ; call sad
        ; int** sad(int* template, int starting_row, int starting_col, int* frame, int f_height, int f_width )
        ; first push caller-saved registers
        push    r8
        push    r9
        push    r10
        push    r11
        ; load arguments
        lea     rdi, [template]      
        mov     rsi, r9      
        mov     rdx, r8
        lea     rcx, [frame]
        mov     r8, [f_height]
        mov     r9, [f_width]
        call    sad
        ; store rax if (rax < lowest_sad)
        mov     rdx, [lowest_sad]
        cmp     rax, rdx
        jge     %%CONTINUE_SAD_BODY
        mov     [lowest_sad], rax

        %%CONTINUE_SAD_BODY:
        pop     r11
        pop     r10
        pop     r9
        pop     r8
    %endmacro

    sub     rsp, 8                  ; align the sp by 16B
    
    ; print template
    lea     rdi, [template]
    mov     rsi, 9
    call    print_arr	
	
    lea     rdi, [newline]
    mov     al, byte 0
    call 	[printf wrt ..got]
    
    lea     rbx, [template]         ; template address
    lea     rcx, [frame]            ; frame address
    lea     rsi, [result]           ; result address
    mov     r8, 0                   ; f_x
    mov     r9, 0                   ; f_y
    mov     r10, [f_width]          ; frame width
    mov     r11, [f_height]         ; frame height
    mov     r14, 0                  ; frame displacement (f_d)
    mov		r12, 0					; i index
    mov		r13, 0					; j index


; x: from 0 to (f_width - t_width)
; y: 0
SAD_LOOP:
    ; calculate frame displacement, f_d = (f_y * f_width + f_x) * offset
    ;byte_displacement r8, r9, r10 

    ; call sad
    sad_body

    ; if tunr1, else if turn 2, else if turn 3
    ; turn1 = f_y = 0
    ; cmp     r9, 0
    ; je      TURN1_INCREMENT
    ; ; turn2 = f_x = (f_width - t_width
    ; mov     rax, r10
    ; sub     rax, t_width
    ; cmp     r8, rax
    ; je      TURN2_INCREMENT
    ; turn3 = 
    
; x: i to (f_width - t_width - i)
; y: j
TURN1_INCREMENT:
    ; increment f_x and loop if (f_x <= (f_width - t_width - i))
    add     r8, 1
    mov     rax, r10
    sub     rax, t_width        ; assuming 3x3 template
    sub		rax, r12
    cmp     r8, rax
	jle     SAD_LOOP

    ; f_x is (f_width - t_width) + 1 here, so subtract 1
    sub     r8, 1               ; f_x = (f_width - t_width)
    
; x: (f_width - t_width -i)
; y: j to (f_height - t_height -j)
TURN2_LOOP:

    ; call sad
    sad_body

    ; increment f_y and loop if (f_y <= (f_height - t_height))
    add     r9, 1
    mov     rax, r11
    sub     rax, t_height          ; assuming 3x3 template
    sub		rax, r13
    cmp     r9, rax
	jle     TURN2_LOOP

    ; f_y is (f_height - t_height) + 1 here, so subtract 1
    sub     r9, 1         
; x: (f_width - t_width -i) to i
; y: (f_height - t_height -j)
TURN3_LOOP:
    ; call sad
    sad_body

    ; decrement f_x and loop if (f_x >= i)
    sub     r8, 1
    cmp     r8, r12
	jge     TURN3_LOOP

    ; f_x is -1 here, so add 1
    add     r8, 1         
; x: i
; y: (f_height - t_height -j) to j
TURN4_LOOP:
    ; call sad
    sad_body

    ; decrement f_y and loop if (f_y >= 0)
    sub     r9, 1
    cmp     r9, r13
	jge     TURN4_LOOP
    ; f_y is -1 here, so add 1
    add     r9, 1   
	
	
	; increment i and j
	add		r12, 1
	add		r13, 1
	
	; if i == f_width - t_width
	; if j == f_height - t_height
	; exit loop
	mov		rax, r10		; f_width
	sub		rax, t_width	; f_width - t_width
	cmp		r12, rax
	je		PRINT_RESULTS
	mov		rax, r11		; f_height
	sub		rax, t_height	; f_height - t_height
	cmp		r13, rax
	je		PRINT_RESULTS
	
	
	
	
	; else, continue
	jmp		SAD_LOOP
	
	
PRINT_RESULTS:
    ; print result matrix
    ;~ lea     rdi, [result]
    ;~ mov     rsi, 9
    ;~ call    print_ar
    
    lea		rdi, [format]
    mov		rsi, [lowest_sad]
    mov		al, byte 0
    call	[printf wrt ..got]

    ; loop if (index_y < max_col)
    call [exit wrt ..got]

; NOTES
; - assuming 3x3 template
; - max frame size is 255x255 
;
; Sum of absolute differences of two mxn matrices
; int** sad(int* template, int starting_row, int starting_col, int* frame, int f_height, int f_width )
; rax contains the calculated sad value
; args:
; rdi: address of template
; rsi: starting frame row
; rdx: starting frame column
; rcx: address of frame
;  r8: frame height 
;  r9: frame width
global sad
sad:
    ; register usage:
	; 	Bytes Location  Description
    ;       8 rbx       template address
	;		1 r8b		template x index (zero extended to r8)
	;		1 r9b		template y index (zero extended to r9)
    ;       8 r10       temporary template byte displacement 
    ;       8 r11       frame address
    ;       1 r12b      frame height (zero extended to r12)
    ;       1 r13b      frame width (zero extended to r13)
    ;       8 r14       current template element
    ;       8 r15       current frame element

    ; push callee-saved registers
    push    rbx
    push    r12
    push    r13
    push    r14
    push    r15

    ; retrieve parameters from arguments
    mov     rbx, rdi                ; address of template
    mov     r11, rcx                ; address of frame
    mov     r12b, r8b               ; frame height
    mov     r13b, r9b               ; frame width

    ; prologue
    push    rbp                     ; pop old rbp value to stack
    sub     rsp, 70o                ; allocate space for 7 8B local variables
    lea     rbp, [rsp]              ; use rbp as frame pointer

    ; initialize index variables
    mov     r8b, 0                  ; template_x
    mov     r9b, 0                  ; template_y
    mov     r10, 0                  ; template byte displacement
    mov     qword [rbp-8], 0        ; frame_x
    mov     qword [rbp-16], rsi     ; frame_y, initialize to starting_frow
    mov     qword [rbp-24], 0       ; frame byte displacement
    mov     qword [rbp-32], 0       ; holds result
    mov     qword [rbp-40], 0       ; temp
    mov     qword [rbp-48], rsi     ; starting_frow
    mov     qword [rbp-56], rdx     ; starting_fcol

    ; iterates across template array
    LOOP_COL:
        ; reset current col counts
        mov     r8b, 0
        mov     rax, [rbp-56]
        mov     [rbp-8], rax            ; reset f_y to starting_fcol
        
        LOOP_ROW:
            ; calculate displacements & get elements
            ; d = byte_offset * (curr_row * cols_per_row + curr_col)
            movzx   rax, r9b            ; rax = t_y (clear upper 7 bytes of rax)
            imul    rax, t_width        ; rax = (t_y * t_width)                      
            lea     rax, [rax + r8]     ; rcx = (t_y * t_width + t_x)
            imul    rax, offset         ; rax = (t_y * t_width + t_x) * offset             
            ; get element
            mov     r10, rax            ; store template byte displacement
            mov     r14, [rbx + r10]    ; get template element

            ; f_d = byte_offset * (f_y * f_width + f_x)
            movzx   rax, r13b           ; rax = f_width (clear upper 7 bytes of rax) 
            imul    rax, [rbp-16]       ; rax = (f_y * f_width)
            add     rax, [rbp-8]        ; rcx = (f_y * f_width + f_x)
            imul    rax, offset         ; rax = (f_y * f_width + f_x) * offset          
            ; get element
            mov     [rbp-24], rax       ; store frame byte displacement
            mov     r15, [r11 + rax]    ; get frame element
            
            ; if (f_x > f_width) 
            ; add to frame byte offset of frame the amount of bytes to begin on the next row
            ; d += (f_width - f_x) * offset
            cmp     byte [rbp-8], r13b        
            jl     CONTINUE_ROW
            movzx   rax, r13b           ; rax = f_width (clear upper 56 bits of rax)
            sub     rax, [rbp-8]        ; rax = f_width - f_x
            imul    rax, offset         ; rax = (f_width - f_x) * offset       
            add     rax, [rbp-24]       ; rax = oldoffset + additional offset
            ; replace current element of m2
            mov     r15, [r11 + rax]

        CONTINUE_ROW:
            ; calculate difference
            sub     r14, r15            ; x = (template_element - frame_element)
            ; take the absolute value of the difference
            mov     [rbp-40], r14       ; memory for x
            sar     r14, 63             ; y = x >>> 63 bits
            mov     r15, r14
            xor     r15, [rbp-40]       ; r15 = (x ^ y)
            mov     [rbp-40], r15
            sub     [rbp-40], r14       ; (x ^ y) - y
            mov     r14, [rbp-40]       ; x = |template_element - frame_element|

            ; store absolute difference in result matrix
            lea     r15, [result]
            mov     [r15 + r10], r14    ; store absolute difference
            ; accumulate result in [rbp-32]
            add     [rbp-32], r14
           
            ; increment col (r8b) and loop if (t_x < t_width)
            add     r8b, 1                      ; t_x++           
            add     qword [rbp-8], 1            ; f_x++       
            cmp     r8b, t_width                
        jl LOOP_ROW
        
        ; increment row (r9b) and loop if (t_y < t_height)
        add     r9b, 1                          ; t_y++
        add     qword [rbp-16], 1               ; f_y++
        cmp     r9b, t_width
    jl LOOP_COL

    ; store result in rax
    mov     rax, [rbp-32]

    ; epilogue
    lea     rsp, [rbp+70o]          ; deallocate 7 8B local variables, restore rsp
    pop     rbp

    ; pop callee-saved registers off stack
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    ret

; void print_arr(int** arr, int n)
print_arr:
        ; register usage:
        ; 	Bytes Location  Description
        ;		8 r13		size of array, n
        ;		8 rbx		loop index, i
        ;       8 r12       aray address
        ; push callee-saved registers
        push    rbx             
        push    r12
        ; loop prologue
        mov     rcx, rsi        ; load n
        test    rcx, rcx        
        jng     LOOP_END        ; skip if n <= 0
        xor     rbx, rbx        ; i = 0
        mov     r12, rdi        ; store array address
    LOOP_TOP:   
        ; loop body
        mov     rsi, [r12 + offset*rbx]      ; dereference element
        lea     rdi, [format wrt rip]
        mov     al, 0
        push    rcx
        call    [printf wrt ..got]
        pop     rcx

        ; increment and loop
        add     rbx, 1          ; i = i + 1
        cmp     rbx, rcx        ; i < n
        jl      LOOP_TOP        ; loop back if i < n
    LOOP_END:
        ; pop callee-saved registers
        pop     r12
        pop     rbx
        ret


; int print_int(size_t i) 
; rax is 1 if succesful
print_int:	
	push    rbx			    ; will be used 
	mov     rbx, rdi		; store to-be-printed into rbx 
	
	lea     rdi, [print_comment wrt rip] 
	mov     rsi, rbx		; param from caller		
	mov     al, byte 0
	call    [printf wrt ..got]

	pop     rbx
	ret	

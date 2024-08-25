.ORIG x3000           ; Start the program at memory location x3000
LEA R0, HELLO_STR     ; Load the address of the string into R0
PUTS                  ; Output the string stored at the address in R0
HALT                  ; Halt the program

HELLO_STR .STRINGZ "Hello"  ; Define the string "Hello" (null-terminated)
.END                  ; End of the program

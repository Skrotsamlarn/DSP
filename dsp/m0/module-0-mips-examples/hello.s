	.data

msg:	.asciiz "Hello World!"

	.text
	
msgg:   .asciiz "hej"
	
	.text
main:
	li $v0, 4 	# Systemcall code print_str
	li $t0, 4 
	la $a0, msg
	la $a0, msgg
	syscall
	
	li $v0, 10	# Systemcall code exit
	syscall

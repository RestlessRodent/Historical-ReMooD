
.section .bootup
.global _start
.global main

_start:
	jal main
	
_loopy:
	j _loopy


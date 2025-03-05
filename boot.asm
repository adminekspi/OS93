[BITS 16]
[ORG 0x7C3E]

jmp boot_start
nop

;-----------------------------------------
; FAT16 BPB, for 128 MB image (compatible with mkfs.vfat)
OEM_ID             db "MSDOS5.0"
BytesPerSector     dw 512
SectorsPerCluster  db 4                ; 4 sectors = 2048 bytes
ReservedSectors    dw 1
NumFATs            db 2
RootEntryCount     dw 224
TotalSectors       dw 0                ; Set to 0, use TotalSectors32
MediaDescriptor    db 0xF8
SectorsPerFAT      dw 256
SectorsPerTrack    dw 63
NumberOfHeads      dw 255
HiddenSectors      dd 0
TotalSectors32     dd 262144           ; 128 MB / 512 = 262144 sectors

; Extended BPB
DriveNumber        db 0x80
Reserved1          db 0
BootSignature      db 0x29
VolumeID           dd 0x12345678
VolumeLabel        db "OS93       "   ; 11 characters (padded with spaces)
FileSystemType     db "FAT16   "      ; 8 characters

;-----------------------------------------
; Variables (used within the bootloader)
FirstDataSector    dw 0       ; First data sector after the root directory
RootDirStart       dw 0       ; Starting LBA of the root directory

; Kernel file name to be loaded (8+3, without dot, padded with spaces)
KernelName         db "KERNEL  BIN"   ; total 11 bytes

; Messages
boot_msg           db "Booting OS93...", 10, 13, 0
error_msg          db "Error loading kernel!", 10, 13, 0
notfound_msg       db "Kernel file not found!", 10, 13, 0

;-----------------------------------------
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

    ; (Pre-calculated) Number of root directory sectors: (224*32)/512 = 14 sectors
    mov cx, 14

    ; RootDirStart = ReservedSectors + (NumFATs * SectorsPerFAT)
    mov ax, SectorsPerFAT     ; AX = 256
    mov bl, [NumFATs]         ; BL = 2
    mov bh, 0                 ; BX = 0x0002
    mul bx                    ; AX = 256 * 2 = 512
    add ax, ReservedSectors   ; 512 + 1 = 513
    mov [RootDirStart], ax    ; Root directory starts at sector 513

    ; FirstDataSector = RootDirStart + RootDirSectors = 513 + 14 = 527
    mov ax, [RootDirStart]
    add ax, cx
    mov [FirstDataSector], ax

    ; Load the root directory (14 sectors) to address 0x9000
    mov si, [RootDirStart]    ; Starting LBA (513)
    mov ax, 0x0000
    mov es, ax
    mov di, 0x9000            ; Target memory address
    mov bx, 14                ; 14 sectors
.load_root_loop:
    push bx
    push si
    push di
    mov ax, si              ; LBA sector (e.g., 513)
    call read_sector        ; Read 1 sector to ES:DI
    pop di
    pop si
    pop bx
    add di, 512             ; Each sector is 512 bytes
    inc si                  ; Next LBA
    dec bx
    jnz .load_root_loop

    ; Search for "KERNEL  BIN" file in the root directory (224 entries)
    mov si, 0          ; Root directory data loaded in ES; ES:SI = offset 0 (i.e., 0x9000 physical address)
    mov cx, 224
.search_loop:
    cmp byte es:[si], 0    ; Check through ES: if 0, it's empty
    je kernel_not_found
    push cx
    push si
    mov di, KernelName     ; DS:KernelName
    mov cx, 11             ; Compare 11 bytes
    call compare_string2   ; Compare ES:SI with DS:DI
    pop si
    pop cx
    cmp ax, 0
    je found_kernel
    add si, 32             ; Next directory entry (each entry is 32 bytes)
    loop .search_loop

kernel_not_found:
    mov si, notfound_msg - 0x7C00
    call print_string
    jmp hang

found_kernel:
    ; In the found directory entry, starting cluster is at offset 26 (2 bytes)
    mov ax, [si+26]
    cmp ax, 2
    jb kernel_error         ; Must be a valid file (cluster >= 2)
    ; Calculate the first kernel sector:
    ; Kernel LBA = FirstDataSector + (cluster - 2) * SectorsPerCluster
    mov bx, ax
    sub bx, 2
    mov al, SectorsPerCluster   ; AL = 4
    cbw
    mul bx                     ; AX = (cluster - 2) * 4
    add ax, [FirstDataSector]  ; AX = first LBA sector of the kernel
    mov si, ax               ; Kernel's LBA start in SI

    ; Adjust the number of sectors to load based on the kernel file size.
    ; Here, 3 clusters (12 sectors) are read.
    mov bx, 12

    ; Kernel will be loaded to address 0x1000
    mov di, 0x1000
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
    inc si                 ; Next LBA sector
    dec bx
    jnz .load_kernel_loop

    ; Jump to the loaded kernel
    jmp 0x1000

kernel_error:
    mov si, error_msg - 0x7C00
    call print_string
    jmp hang

;-----------------------------------------
; read_sector:
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

;-----------------------------------------
; LBA2CHS:
; Input: AX = LBA sector number
; Output: CH = cylinder, DH = head, CL = sector
; Assumes: SectorsPerTrack = 63, NumberOfHeads = 255
LBA2CHS:
    push ax
    mov bx, 63
    xor dx, dx
    div bx              ; AX = quotient, DX = remainder
    ; Sector number = (remainder + 1)
    mov cl, dl
    inc cl
    ; Now, quotient (AX) contains head and cylinder info.
    mov bx, 255
    xor dx, dx
    div bx              ; AX = quotient / 255, DX = quotient mod 255
    mov ch, al        ; Cylinder = AL
    mov dh, dl        ; Head = DL
    pop ax
    ret

;-----------------------------------------
; compare_string:
; Compares strings at DS:SI and DS:DI for CX bytes.
; If equal, AX = 0; otherwise, AX != 0.
compare_string:
    push cx
.compare_loop:
    lodsb             ; DS:SI -> AL
    scasb             ; Compare with DS:DI (DI increments)
    jne .not_equal
    loop .compare_loop
    xor ax, ax
    pop cx
    ret
.not_equal:
    mov ax, 1
    pop cx
    ret

; compare_string2 (case-insensitive):
; ES:SI (root directory entry) with DS:DI (KernelName) comparison for CX bytes.
; Case-insensitive comparison.
; If equal, AX = 0; if not equal, AX ? 0.
compare_string2:
    push cx
.compare2_loop:
    mov al, es:[si]    ; Read from ES (directory entry character)
    mov bl, ds:[di]    ; Read from DS (KernelName character)
    ; Convert characters to uppercase (if between 'a' and 'z')
    cmp al, 'a'
    jb .skip_convert_al
    cmp al, 'z'
    ja .skip_convert_al
    sub al, 32         ; Convert to uppercase
.skip_convert_al:
    cmp bl, 'a'
    jb .skip_convert_bl
    cmp bl, 'z'
    ja .skip_convert_bl
    sub bl, 32
.skip_convert_bl:
    cmp al, bl
    jne .not_equal2
    inc si
    inc di
    loop .compare2_loop
    xor ax, ax         ; If equal, AX = 0
    pop cx
    ret
.not_equal2:
    mov ax, 1
    pop cx
    ret

;-----------------------------------------
; print_string:
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

times 510 - ($ - $$) db 0
dw 0xAA55

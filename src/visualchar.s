; Oric Screen Editor
; Screen editor for the Oric Atmos
; Written in 2022 by Xander Mol

; https://github.com/xahmol/PETScreenEdit
; https://www.idreamtin8bits.com/

; Data for visual charset mapping
; Inspired by Petmate and petscii.krissz.hu
; Suggested by jab / Artline Designs (Jaakko Luoto)              

    .segment "VISUALCHAR"

    ; Data
    
    .byte  $37,$4B,$33,$43,$3F,$4F,$27,$2B,$3C,$4C,$40,$30,$48,$34,$24,$28
    .byte  $55,$5A,$51,$52,$5D,$5E,$54,$58,$2D,$2E,$22,$21,$2A,$25,$53,$5F
    .byte  $36,$49,$26,$29,$56,$59,$57,$5B,$3D,$4E,$32,$41,$50,$5C,$2C,$2F
    .byte  $45,$3A,$44,$38,$47,$3B,$46,$39,$3E,$4D,$31,$42,$23,$35,$4A,$20
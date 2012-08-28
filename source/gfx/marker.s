
@{{BLOCK(marker)

@=======================================================================
@
@	marker, 16x16@4, 
@	Transparent color : 00,00,00
@	+ palette 16 entries, not compressed
@	+ 4 tiles Metatiled by 1x2 not compressed
@	Total size: 32 + 128 = 160
@
@	Time-stamp: 2012-07-06, 15:32:39
@	Exported by Cearn's GBA Image Transmogrifier, v0.8.10
@	( http://www.coranac.com/projects/#grit )
@
@=======================================================================

	.section .rodata
	.align	2
	.global markerTiles		@ 128 unsigned chars
	.hidden markerTiles
markerTiles:
	.word 0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11113311,0x11113311,0x11131131
	.word 0x11131131,0x11133331,0x11331133,0x11311113,0x11111111,0x11111111,0x11111111,0x11111111
	.word 0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11333311,0x12111311,0x13111311
	.word 0x11333311,0x13111311,0x13111311,0x11333311,0x11111111,0x11111111,0x11111111,0x11111111

	.section .rodata
	.align	2
	.global markerPal		@ 32 unsigned chars
	.hidden markerPal
markerPal:
	.hword 0x0000,0x4631,0x09E2,0x01C0,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000

@}}BLOCK(marker)

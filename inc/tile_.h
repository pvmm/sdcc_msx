#pragma once
#include <monospace.h>

/*
 * Graphics 2 tileset
 */


/* Standard POSIX-style types */
typedef EM2_RowPageFlags row_page_flags;
typedef U8x8 u8x8_t;


/* A 256 chars tileset represents a third of the screen in GRAPHICS 2 */
typedef u8x8_t tileset_t[256];


/**
 * Init tileset library and define if tile graphics and colors are interlaced.
 * Note: in interlaced mode, half of the 256 tiles are used for every other frame.
 */
void g2_init_tileset (bool interlaced);


/**
 * Check if library is in interlaced tile graphics and colors mode.
 * Note: in interlaced mode, half of the 256 tiles are used for every other frame.
 */
bool g2_is_interlaced (void);


/**
 * Wipe tileset memory
 */
void g2_wipe_tileset (tileset_t set);


/**
 * Set single tile in tileset.
 */
void g2_set_tile
    (
        row_page_flags pages,       // pages where this tileset is active
        uint8_t index,              // index in ASCII table
        const u8x8_t shape,         // shape of tile
        const u8x8_t color          // color of tile
    );


/**
 * Change color of tile in tileset. This affects only the next printed tiles.
 */
void g2_set_tile_color
    (
        row_page_flags pages,       // pages where this tileset is active
        uint8_t index,              // index in ASCII table
        const u8x8_t color          // color of tile
    );


/**
 * Change shape of tile in tileset. This affects only the next printed tiles.
 */
void g2_set_tile_shape
    (
        row_page_flags pages,       // pages where this tileset is active
        uint8_t index,              // index in ASCII table
        const u8x8_t shape          // shape of tile
    );


/**
 * Set a tile pair to be used in interlaced mode.
 */
void g2_set_tile_pairing
    (
        uint8_t assoc_code,
        uint8_t ascii_code1,
        uint8_t ascii_code2
    );


/**
 * Set all tiles at once with the sames colors (2 colors per line).
 *
 * Note: this is most useful when using tileset as font.
 */
INLINE void g2_set_font
    (
    	row_page_flags pages,       // pages where this tileset is active
        const tileset_t set,              // tileset to setup
    	const u8x8_t color          // colors to use for the entire tileset
    )
{
	for (uint16_t index = 0; index < 256; index++)
        g2_set_tile (pages, index, set[index], color);
}


/**
 * Set all tiles at once.
 */
INLINE void g2_set_tileset
    (
    	row_page_flags pages,          // pages where this font is active
    	const tileset_t shapes,   // tiles shapes
    	const tileset_t colors    // tiles colors
    )
{
	for (uint16_t index = 0; index < 256; index++)
        g2_set_tile (pages, index, shapes[index], colors[index]);
}


void g2_set_pos (const uint8_t x, const uint8_t y);


void g2_get_pos (uint8_t *x, uint8_t *y);


void g2_put_tile (uint8_t index);


void g2_put_interlaced (uint8_t index0, uint8_t index1);


void g2_put_buffer (const char *buf, uint16_t len);


void g2_put_at (uint8_t x, uint8_t y, const char *buf, uint16_t len);


void transform_null(const u8x8_t source, u8x8_t target);


void transform_flip_x(const u8x8_t source, u8x8_t target);

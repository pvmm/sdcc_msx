#include <tms99X8.h>
#include <tile_.h>
//#include <assert.h>


static uint8_t pos_x = 0, pos_y = 0;


static bool interlaced = false;


void g2_set_interlaced (bool b)
{
    interlaced = b;
}


bool g2_get_interlaced (void)
{
    return interlaced;
}


/* interlaced tileset */
void g2_wipe_tileset (tileset_t set)
{
	memset (set, 0, sizeof (tileset_t));
}


void g2_set_tile
(
    row_page_flags pages,       // pages where this tileset is active
    uint8_t index,              // index in ASCII table
    const u8x8_t shape,         // shape of tile
    const u8x8_t color          // color of tile
)
{
	uint16_t tile_address;

	for (uint8_t page = 0; page < 3; page++)
    {
        if ((pages & (1 << page)) == 0) continue;

		tile_address = (( (uint16_t) page) << 11) + (( (uint16_t) index) << 3);
		TMS99X8_memcpy (MODE2_ADDRESS_PG + tile_address, shape, 8);
		TMS99X8_memcpy (MODE2_ADDRESS_CT + tile_address, color, 8);
	}
}


void g2_set_tile_color
(
    row_page_flags pages,       // pages where this tileset is active
    uint8_t index,              // index in ASCII table
    const u8x8_t color          // color of tile
)
{
	uint16_t tile_address;

	for (uint8_t page = 0; page < 3; page++)
    {
        if ((pages & (1 << page)) == 0) continue;

		tile_address = (( (uint16_t) page) << 11) + (( (uint16_t) index) << 3);
		TMS99X8_memcpy (MODE2_ADDRESS_CT + tile_address, color, 8);
	}
}


void g2_set_tile_shape
(
    row_page_flags pages,       // pages where this tileset is active
    uint8_t index,              // index in ASCII table
    const u8x8_t shape          // shape of tile
)
{
	uint16_t tile_address;

	for (uint8_t page = 0; page < 3; page++)
    {
        if ((pages & (1 << page)) == 0) continue;

		tile_address = (( (uint16_t) page) << 11) + (( (uint16_t) index) << 3);
		TMS99X8_memcpy (MODE2_ADDRESS_CT + tile_address, shape, 8);
	}
}


void g2_set_pos (const uint8_t x, const uint8_t y) { pos_x = x; pos_y = y; }


void g2_get_pos (uint8_t *x, uint8_t *y) { *x = pos_x; *y = pos_y; }


void g2_put_tile (uint8_t index)
{
	uint16_t address = (( (uint16_t) pos_y) << 5) + pos_x;

	TMS99X8_memset (MODE2_ADDRESS_PN0 + address, index, 1);

    if (interlaced)
    {
        TMS99X8_memset (MODE2_ADDRESS_PN1 + address, index + 128, 1);
    }

	pos_x++;
	if (pos_x == 32) { pos_x = 0; pos_y++; };
	if (pos_y == 24) { pos_y = 0; };
}


void g2_put_buffer (const char *buf, uint16_t size)
{
	while (size--> 0)
		g2_put_tile (*buf++);
}


void g2_put_at (uint8_t x, uint8_t y, const char *buf, uint16_t size)
{
	g2_set_pos (x, y);
	g2_put_buffer (buf, size);
}


void transform_null(const u8x8_t source, u8x8_t target)
{
	for (uint8_t i = 0; i < 8; i++)
		target[i] = source[i];
}


void transform_flip_x(const u8x8_t source, u8x8_t target)
{
	for (int i = 0; i < 8; ++i)
    {
		target[i]  = (source[i] & 1)  << 7;   // 00000001 => 10000000
		target[i] |= (source[i] & 2)  << 5;   // 00000010 => 01000000
		target[i] |= (source[i] & 4)  << 3;   // 00000100 => 00100000
		target[i] |= (source[i] & 8)  << 1;   // 00001000 => 00010000
		target[i] |= (source[i] & 16) >> 1;   // 00010000 => 00001000
		target[i] |= (source[i] & 32) >> 3;   // 00100000 => 00000100
		target[i] |= (source[i] & 64) >> 5;   // 01000000 => 00000010
		target[i] |= (source[i] & 128) >> 7;  // 10000000 => 00000001
	}
}

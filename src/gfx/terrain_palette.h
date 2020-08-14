#ifndef terrain_palette_include_file
#define terrain_palette_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define terrain_palette_width 24
#define terrain_palette_height 12
#define terrain_palette_size 290
#define terrain_palette ((gfx_sprite_t*)terrain_palette_data)
extern unsigned char terrain_palette_data[290];

#ifdef __cplusplus
}
#endif

#endif

#ifndef cursor_include_file
#define cursor_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define cursor_width 10
#define cursor_height 10
#define cursor_size 102
#define cursor ((gfx_sprite_t*)cursor_data)
extern unsigned char cursor_data[102];

#ifdef __cplusplus
}
#endif

#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "core.h"
#include "platform.h"
#include "res.h"
#include "imui.h"
#include "video.h"

static void on_text_input(struct window* window, const char* text, void* udata) {
	ui_text_input_event(udata, text);
}

i32 main() {
	srand((u32)time(null));

	init_time();

	main_window = new_window(make_v2i(1366, 768), "Immediate Mode UI", true);

	video_init();
	res_init();

	struct ui_context* ui = new_ui_context(load_shader("res/shaders/sprite.glsl"), main_window, load_font("res/DejaVuSans.ttf", 14.0f));
	set_window_uptr(main_window, ui);
	set_on_text_input(main_window, on_text_input);

	struct texture* player_texture = load_texture("res/bmp/char.bmp");

	u64 now = get_time(), last = now; 
	f64 timestep = 0.0;

	char texts[256][256];
	u32 text_count = 0;
	char buffer[256] = "";

	while (!window_should_close(main_window)) {
		update_events(main_window);

		video_clear();

		ui_begin_frame(ui);

		if (ui_begin_window(ui, "Test Window A", make_v2i(0, 0))) {
			if (ui_text_input(ui, "Text Input", buffer, sizeof(buffer))) {
				strcpy(texts[text_count++], buffer);
				buffer[0] = '\0';
			}

			if (ui_button(ui, "Clear")) {
				text_count = 0;
			}

			for (u32 i = 0; i < text_count; i++) {
				ui_text(ui, texts[i]);
			}

			ui_end_window(ui);
		}

		if (ui_begin_window(ui, "Test Window B", make_v2i(0, 300))) {
			ui_image(ui, player_texture, make_rect(0, 0, 16, 16));

			for (u32 i = 0; i < 25; i++) {
				ui_button(ui, "Button");
			}

			ui_end_window(ui);
		}

		ui_end_frame(ui);

		swap_window(main_window);
	}

	free_ui_context(ui);

	res_deinit();

	free_window(main_window);
}

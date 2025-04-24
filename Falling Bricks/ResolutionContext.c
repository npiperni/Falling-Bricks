#include "ResolutionContext.h"
#include "Constants.h"

ResolutionContext get_resolution_context(int window_width, int window_height) {
	ResolutionContext context;
	context.scale_factor = MIN((float)window_width / WINDOW_WIDTH, (float)window_height / WINDOW_HEIGHT);
	context.x_offset = (window_width - WINDOW_WIDTH * context.scale_factor) / 2;
	context.y_offset = (window_height - WINDOW_HEIGHT * context.scale_factor) / 2;
	return context;
}

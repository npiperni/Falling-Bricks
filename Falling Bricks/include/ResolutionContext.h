#pragma once
typedef struct {
	float scale_factor;
	int x_offset;
	int y_offset;
} ResolutionContext;

ResolutionContext get_resolution_context(int window_width, int window_height);
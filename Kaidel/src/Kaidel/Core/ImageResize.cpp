#include "KDpch.h"
#include "ImageResize.h"


#define CHANNEL_POINTER(image_data, width, num_channels, i, j, k) \
    ((image_data) + ((i) * (width) + (j)) * (num_channels) + (k))


//i = Y-Axis
//j = X-Axis
//x = X-Axis
//y = Y-Axis

void resize_uint8_normalized(const uint8_t* org_image, uint32_t org_width, uint32_t org_height, uint8_t* new_image, uint32_t new_width, uint32_t new_height, int nr_channels) {
	
	//if input is 1x1
	if (org_width == 1 && org_height == 1) {

		uint8_t channels[4] = { 0 };
		std::memcpy(channels, org_image, sizeof(uint8_t) * nr_channels);
		
		for (uint32_t i = 0; i < new_height; ++i) {
			for (uint32_t j = 0; j < new_width; ++j) {
				for (uint32_t k = 0; k < nr_channels; ++k) {
					*CHANNEL_POINTER(new_image, new_width, nr_channels, i, j, k) = channels[k];
				}
			}
		}
		return;
	}
	
	float x_ratio = (float)org_width / (float)new_width;
	float y_ratio = (float)org_height / (float)new_height;

	//if input is 1xh
	if (org_width == 1) {
		for (uint32_t i = 0; i < new_height; ++i) {
			float org_image_y = ((float)i * y_ratio);
			for (uint32_t j = 0; j < new_width; ++j) {
				for (uint32_t k = 0; k < nr_channels; ++k) {
					*CHANNEL_POINTER(new_image, new_width, nr_channels, i, j, k) = *CHANNEL_POINTER(org_image,org_width,nr_channels,(int)org_image_y,0,k);
				}
			}
		}
		return;
	}

	//if input is wx1
	if (org_height == 1) {
		for (uint32_t i = 0; i < new_height; ++i) {
			for (uint32_t j = 0; j < new_width; ++j) {
				float org_image_x = ((float)j * x_ratio);
				for (uint32_t k = 0; k < nr_channels; ++k) {
					*CHANNEL_POINTER(new_image, new_width, nr_channels, i, j, k) = *CHANNEL_POINTER(org_image, org_width, nr_channels,0, (int)org_image_x, k);
				}
			}
		}
		return;
	}
	
	
	for (int i = 0; i < new_height; ++i) {
		float org_image_y = ((float)i * y_ratio);
		for (int j = 0; j < new_width; ++j) {
			float org_image_x = ((float)j * x_ratio);
			float dx = org_image_x - (int)org_image_x;
			float dy = org_image_y - (int)org_image_y;
			int x = (int)org_image_x;
			int y = (int)org_image_y;
			for (int k = 0; k < nr_channels; ++k) {
				uint8_t top_left = 0;
				uint8_t top_right = 0;
				uint8_t bottom_left = 0;
				uint8_t bottom_right = 0;

				//Top left
				if (y - 1 >= 0 && x - 1 >= 0) {
					top_left = *CHANNEL_POINTER(org_image, org_width, nr_channels, y - 1, x - 1, k);
				}
				//Top right
				if (y - 1 >= 0 && x + 1 < org_width) {
					top_right = *CHANNEL_POINTER(org_image, org_width, nr_channels, y - 1, x + 1, k);
				}
				//Bottom left
				if (y + 1 < org_height && x - 1 >= 0) {
					bottom_left = *CHANNEL_POINTER(org_image, org_width, nr_channels, y + 1, x - 1, k);
				}
				//Bottom right
				if (y + 1 < org_height && x + 1 < org_width) {
					bottom_right = *CHANNEL_POINTER(org_image, org_width, nr_channels, y + 1, x + 1, k);
				}

				float top_left_normalized = ((float)top_left) / 255.0f;
				float top_right_normalized = ((float)top_right) / 255.0f;
				float bottom_left_normalized = ((float)bottom_left) / 255.0f;
				float bottom_right_normalized = ((float)bottom_right) / 255.0f;

				float interpolated_normalized_value = (1 - dx) * (1 - dy) * top_left_normalized +
					dx * (1 - dy) * top_right_normalized +
					(1 - dx) * dy * bottom_left_normalized +
					dx * dy * bottom_right_normalized;
				*CHANNEL_POINTER(new_image, new_width, nr_channels, i, j, k) = (uint8_t)(interpolated_normalized_value * 255.0f);
			}
		}
	}
}




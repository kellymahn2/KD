#pragma once
#include <stb_image_resize.h>
namespace Kaidel {
	template<typename T>
	T* BilinearResize(const void* original_image,
		int original_height, int original_width,
		int new_height, int new_width) {

		const T* img = (const T*)original_image;

		auto interpolate = [](const T& p1, const T& p2, const T& p3, const T& p4, float x_ratio, float y_ratio) {
			return p1 * (1 - x_ratio) * (1 - y_ratio) +
				p2 * x_ratio * (1 - y_ratio) +
				p3 * y_ratio * (1 - x_ratio) +
				p4 * x_ratio * y_ratio;
			};

		T* resizedImage = new T[new_width * new_height];

		if (!resizedImage)
			return nullptr;

		for (int i = 0; i < new_height; ++i) {
			for (int j = 0; j < new_width; ++j) {
				float x = (j + 0.5) * original_width / new_width - 0.5;
				float y = (i + 0.5) * original_height / new_height - 0.5;

				int x1 = static_cast<int>(x);
				int y1 = static_cast<int>(y);
				int x2 = std::min(x1 + 1, original_width - 1);
				int y2 = std::min(y1 + 1, original_height - 1);

				float dx = x - x1;
				float dy = y - y1;

				T interpolated_value = interpolate(img[y1 * original_width + x1],
					img[y1 * original_width + x2],
					img[y2 * original_width + x1],
					img[y2 * original_width + x2],
					dx, dy);
				resizedImage[i * new_width + j] = interpolated_value;
			}
		}

		return resizedImage;
	}
	

#define RESIZE_ERR -1
#define RESIZE_COMPLETE 0


	static int resize_float(const float* org_image,int org_width,int org_height,float* new_image,int new_width,int new_height,int num_channels) {

		if (stbir_resize_float(org_image, org_width, org_height, 0, new_image, new_width, new_height, 0, num_channels) != 1) {
			return RESIZE_ERR;
		}
		return RESIZE_COMPLETE;
	}

	static int resize_uint8(const uint8_t* org_image, int org_width, int org_height, uint8_t* new_image, int new_width, int new_height, int num_channels) {


		if (stbir_resize_uint8(org_image, org_width, org_height, 0, new_image, new_width, new_height, 0, num_channels)!=1) {
			return RESIZE_ERR;
		}

		return RESIZE_COMPLETE;
	}

	static int resize_int8(const int8_t* org_image, int org_width, int org_height, int8_t* new_image, int new_width, int new_height, int num_channels) {
		return RESIZE_COMPLETE;
	}

	static int resize_uint8_normalized(const uint8_t* org_image, int org_width, int org_height, uint8_t* new_image, int new_width, int new_height, int num_channels) {

		float x_ratio = (float)org_width / (float)new_width;
		float y_ratio = (float)org_height / (float)new_height;

		for (int i = 0; i < new_height; ++i) {
			for (int j = 0; j < new_width; ++j) {
				float x = j * x_ratio;
				float y = i * y_ratio;
				for (int k = 0; k < num_channels; ++k) {
					float x_diff = x - (int)x;
					float y_diff = y - (int)y;

					int index = ((int)y * org_width + (int)x) * num_channels + k;

					float top_left = (float)(org_image[index]) / 255.0f;
					float top_right = (float)(org_image[index + num_channels]) / 255.0f;
					float bottom_left = (float)(org_image[index + org_width * num_channels]) / 255.0f;
					float bottom_right = (float)(org_image[index + (org_width + 1) * num_channels]) / 255.0f;

					float interpolated_value = top_left * (1 - x_diff) * (1 - y_diff) +
						top_right * x_diff * (1 - y_diff) +
						bottom_left * y_diff * (1 - x_diff) +
						bottom_right * x_diff * y_diff;

					float denormalized_interpolated_value = (interpolated_value) * 255.0f;

					new_image[(i * new_width + j) * num_channels + k] = (uint8_t)denormalized_interpolated_value;

				}
			}
		}

		return RESIZE_COMPLETE;
	}

	static int resize_int8_normalized(const int8_t* org_image, int org_width, int org_height, int8_t* new_image, int new_width, int new_height, int num_channels) {
		float x_ratio = (float)org_width / (float)new_width;
		float y_ratio = (float)org_height / (float)new_height;

		for (int i = 0; i < new_height; ++i) {
			for (int j = 0; j < new_width; ++j) {
				float x = j * x_ratio;
				float y = i * y_ratio;
				for (int k = 0; k < num_channels; ++k) {
					float x_diff = x - (int)x;
					float y_diff = y - (int)y;

					int index = ((int)y * org_width + (int)x) * num_channels + k;

					float top_left = (float)(org_image[index] + 128) / 255.0f;
					float top_right = (float)(org_image[index + num_channels] + 128) / 255.0f;
					float bottom_left = (float)(org_image[index + org_width * num_channels] + 128) / 255.0f;
					float bottom_right = (float)(org_image[index + (org_width + 1) * num_channels] + 128) / 255.0f;

					float interpolated_value = top_left * (1 - x_diff) * (1 - y_diff) +
						top_right * x_diff * (1 - y_diff) +
						bottom_left * y_diff * (1 - x_diff) +
						bottom_right * x_diff * y_diff;

					float denormalized_interpolated_value = (interpolated_value) * 255.0f - 128.0f;

					new_image[(i * new_width + j) * num_channels + k] = (int8_t)denormalized_interpolated_value;

				}
			}
		}

		return RESIZE_COMPLETE;
	}
	
	static int resize_int32(const int32_t* org_image, int org_width, int org_height, int32_t* new_image,int new_width,int new_height,int num_channels) {
		float x_ratio = (float)org_width / (float)new_width;
		float y_ratio = (float)org_height / (float)new_height;

		for (int i = 0; i < new_height; ++i) {
			for (int j = 0; j < new_width; ++j) {
				float x = j * x_ratio;
				float y = i * y_ratio;
				for (int k = 0; k < num_channels; ++k) {
					float x_diff = x - (int)x;
					float y_diff = y - (int)y;

					int index = ((int)y * org_width + (int)x) * num_channels + k;


					//coonvert from [-2^31,2^31-1] to [0,2^32-1] to [0,1.0]
					float top_left = (float)((float)org_image[index] - (float)INT_MIN) / ((float)UINT_MAX);
					float top_right = (float)((float)org_image[index + num_channels] - (float)INT_MIN) / ((float)UINT_MAX);
					float bottom_left = (float)((float)org_image[index + org_width * num_channels] - (float)INT_MIN) / ((float)UINT_MAX);
					float bottom_right = (float)((float)org_image[index + (org_width + 1) * num_channels] - (float)INT_MIN) / ((float)UINT_MAX);

					float interpolated_value = top_left * (1 - x_diff) * (1 - y_diff) +
						top_right * x_diff * (1 - y_diff) +
						bottom_left * y_diff * (1 - x_diff) +
						bottom_right * x_diff * y_diff;
					float denormalized_interpolated_value = (interpolated_value) * ((float)UINT_MAX) + (float)INT_MIN;

					new_image[(i * new_width + j) * num_channels + k] = (int32_t)denormalized_interpolated_value;
				}
			}
		}

		return RESIZE_COMPLETE;
	}

	static int resize_uint32(const uint32_t* org_image, int org_width, int org_height, uint32_t* new_image, int new_width, int new_height, int num_channels) {
		float x_ratio = (float)org_width / (float)new_width;
		float y_ratio = (float)org_height / (float)new_height;

		for (int i = 0; i < new_height; ++i) {
			for (int j = 0; j < new_width; ++j) {
				float x = j * x_ratio;
				float y = i * y_ratio;
				for (int k = 0; k < num_channels; ++k) {
					float x_diff = x - (int)x;
					float y_diff = y - (int)y;

					int index = ((int)y * org_width + (int)x) * num_channels + k;


					//coonvert from [0,2^32-1] to [0,1.0]
					float top_left = (float)(org_image[index]) / ((float)UINT_MAX);
					float top_right = (float)(org_image[index + num_channels]) / ((float)UINT_MAX);
					float bottom_left = (float)(org_image[index + org_width * num_channels]) / ((float)UINT_MAX);
					float bottom_right = (float)(org_image[index + (org_width + 1) * num_channels]) / ((float)UINT_MAX);

					float interpolated_value = top_left * (1 - x_diff) * (1 - y_diff) +
						top_right * x_diff * (1 - y_diff) +
						bottom_left * y_diff * (1 - x_diff) +
						bottom_right * x_diff * y_diff;

					float denormalized_interpolated_value = (interpolated_value) * ((float)UINT_MAX);

					new_image[(i * new_width + j) * num_channels + k] = (uint32_t)denormalized_interpolated_value;
				}
			}
		}

		return RESIZE_COMPLETE;
	}




}

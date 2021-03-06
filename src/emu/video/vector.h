// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Aaron Giles,Bernd Wiebelt,Allard van der Bas
#ifndef __VECTOR__
#define __VECTOR__

#define VECTOR_COLOR111(c) \
	rgb_t(pal1bit((c) >> 2), pal1bit((c) >> 1), pal1bit((c) >> 0))

#define VECTOR_COLOR222(c) \
	rgb_t(pal2bit((c) >> 4), pal2bit((c) >> 2), pal2bit((c) >> 0))

#define VECTOR_COLOR444(c) \
	rgb_t(pal4bit((c) >> 8), pal4bit((c) >> 4), pal4bit((c) >> 0))


/* The vertices are buffered here */
struct point
{
	point() :
		x(0),
		y(0),
		col(0),
		intensity(0),
		arg1(0),
		arg2(0),
		status(0) {}

	int x; int y;
	rgb_t col;
	int intensity;
	int arg1; int arg2; /* start/end in pixel array or clipping info */
	int status;         /* for dirty and clipping handling */
};

class vector_device : public device_t, public device_video_interface
{
public:
	// construction/destruction
	vector_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	vector_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void clear_list();

	void add_point(int x, int y, rgb_t color, int intensity);
	void add_clip(int minx, int miny, int maxx, int maxy);

	void set_flicker(float newval);
	float get_flicker();

	void set_beam_width_min(float newval);
	float get_beam_width_min();

	void set_beam_width_max(float newval);
	float get_beam_width_max();

	void set_beam_intensity_weight(float newval);
	float get_beam_intensity_weight();

	// device-level overrides
	virtual void device_start() override;

private:
	static float m_flicker;
	static float m_beam_width_min;
	static float m_beam_width_max;
	static float m_beam_intensity_weight;
	std::unique_ptr<point[]> m_vector_list;
	static int m_vector_index;
	int m_min_intensity;
	int m_max_intensity;

	// Serial output option for driving vector display hardware
	const char * m_serial;
	int m_serial_fd;
	float m_serial_scale_x;
	float m_serial_scale_y;
	float m_serial_offset_x;
	float m_serial_offset_y;
	int m_serial_rotate;
	int m_serial_bright;
	int m_serial_drop_frame;
	int m_serial_sort;
	unsigned m_vector_transit[3];
	unsigned char * m_serial_buf;
	size_t m_serial_offset;
	struct serial_segment_t * m_serial_segments;
	struct serial_segment_t * m_serial_segments_tail;

	void serial_reset();

	void
	serial_draw_point(
		unsigned x,
		unsigned y,
		int intensity
	);

	void
	serial_draw_line(
		float x0,
		float y0,
		float x1,
		float y1,
		int intensity
	);

	void
	serial_send();

	float normalized_sigmoid(float n, float k);
};


// device type definition
extern const device_type VECTOR;

#define MCFG_VECTOR_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, VECTOR, 0)

#endif

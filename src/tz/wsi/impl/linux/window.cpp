#ifdef __linux__
#define GLX_GLXEXT_PROTOTYPES
#include "tz/wsi/impl/linux/window.hpp"
#include <GL/glxext.h>
#include <X11/Xatom.h>
#include <cstring>

namespace tz::wsi::impl
{
	window_x11::window_x11(window_info info):
	flags(info.window_flags)
	{
		const x11_display_data x11d = impl::x11_display();
		this->wnd = XCreateSimpleWindow(x11d.display, RootWindow(x11d.display, x11d.screen), 0, 0, info.dimensions[0], info.dimensions[1], 1, BlackPixel(x11d.display, x11d.screen), WhitePixel(x11d.display, x11d.screen));
		this->set_title(info.title);
		XSelectInput(x11d.display, this->wnd, ExposureMask | KeyPressMask);
		XMapWindow(x11d.display, this->wnd);

		Atom protocols[] = {XInternAtom(x11d.display, "WM_DELETE_WINDOW", False)};
		XSetWMProtocols(x11d.display, this->wnd, protocols, sizeof(protocols) / sizeof(Atom));
		if(info.window_flags & window_flag::opengl)
		{
			this->impl_init_opengl();
		}
	}

	window_x11::~window_x11()
	{
		if(this->wnd != static_cast<unsigned int>(-1))
		{
			this->impl_request_close();
			this->wnd = static_cast<unsigned int>(-1);
		}
	}

	window_x11::native window_x11::get_native() const
	{
		return this->wnd;
	}

	void window_x11::request_close()
	{
		this->close_requested = true;
	}

	bool window_x11::is_close_requested() const
	{
		return this->close_requested;
	}

	tz::vec2ui window_x11::get_dimensions() const
	{
		XWindowAttributes attr{};
		XGetWindowAttributes(impl::x11_display().display, this->wnd, &attr);
		return static_cast<tz::vec2ui>(tz::vec2i{attr.width, attr.height});
	}

	void window_x11::set_dimensions([[maybe_unused]] tz::vec2ui dimensions)
	{
		tz::report("warning: NYI set_dimensions linux");
	}

	std::string window_x11::get_title() const
	{
		char* name = nullptr;
		[[maybe_unused]] Status res = XFetchName(impl::x11_display().display, this->wnd, &name);
		std::string ret = name;
		XFree(name);
		return ret;
	}

	void window_x11::set_title(std::string title)
	{
		XStoreName(impl::x11_display().display, this->wnd, title.c_str());
	}

	window_flag::flag_bit window_x11::get_flags() const
	{
		return this->flags;
	}

	void window_x11::update()
	{
		XEvent* evt = impl::get_current_event();
		if(evt == nullptr || evt->xclient.window != this->wnd || this->close_requested) return;
		XEvent e = *evt;
		if(e.type == Expose)
		{
			/*
			auto dims = this->get_dimensions();
			XFillRectangle(x11d.display, this->wnd, DefaultGC(x11d.display, x11d.screen), 0, 0, dims[0], dims[1]);
			*/
		}
		if(e.type == KeyPress)
		{
		}
		if(e.type == ClientMessage)
		{
			const Atom protocol = e.xclient.data.l[0];
			if(protocol != None)
			{
				if(protocol == XInternAtom(impl::x11_display().display, "WM_DELETE_WINDOW", False) && e.xclient.window == this->wnd)
				{
					this->impl_request_close();
				}
			}
		}
	}

	bool window_x11::make_opengl_context_current()
	{
		if(ctx == nullptr)
		{
			return false;
		}
		return glXMakeCurrent(impl::x11_display().display, this->wnd, this->ctx);
	}

	#if TZ_VULKAN
	VkSurfaceKHR window_x11::make_vulkan_surface(VkInstance vkinst) const
	{
		VkSurfaceKHR surf;
		VkXlibSurfaceCreateInfoKHR create
		{
			.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.dpy = impl::x11_display().display,
			.window = this->wnd
		};
		auto fn = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(vkGetInstanceProcAddr(vkinst, "vkCreateXlibSurfaceKHR"));
		tz::assert(fn != nullptr);
		VkResult res = fn(vkinst, &create, nullptr, &surf);
		tz::assert(res == VK_SUCCESS);
		return surf;
	}
	#endif // TZ_VULKAN

	const keyboard_state& window_x11::get_keyboard_state() const
	{
		return this->kb_state;
	}

	const mouse_state& window_x11::get_mouse_state() const
	{
		return this->m_state;
	}

	void* window_x11::get_user_data() const
	{
		return this->userdata;
	}

	void window_x11::set_user_data(void* udata)
	{
		this->userdata = udata;
	}

	void window_x11::impl_request_close()
	{
		XUnmapWindow(impl::x11_display().display, this->wnd);
		XDestroyWindow(impl::x11_display().display, this->wnd);
		XFlush(impl::x11_display().display);
		this->close_requested = true;
		this->wnd = -1;
	}

	static bool is_extension_supported(const char *extList, const char *extension)
	{
		const char *start;
		const char *where, *terminator;
	  
		/* Extension names should not have spaces. */
		where = strchr(extension, ' ');
		if (where || *extension == '\0')
			return false;

	  /* It takes a bit of care to be fool-proof about parsing the
		 OpenGL extensions string. Don't be fooled by sub-strings,
		 etc. */
		for (start=extList;;)
		{
			where = strstr(start, extension);

			if (!where)
				break;

			terminator = where + strlen(extension);

			if( where == start || *(where - 1) == ' ' )
				if ( *terminator == ' ' || *terminator == '\0' )
					return true;
			start = terminator;
		}

		return false;
	}

	void window_x11::impl_init_opengl()
	{
		const auto& x11d = impl::x11_display();
		constexpr int maj = 4, min = 6;
		int nelements;
		GLXFBConfig* fb_configs = glXChooseFBConfig(x11d.display, x11d.screen, nullptr, &nelements);
		tz::assert(fb_configs != nullptr, "Failed to retrieve any framebuffer configs for OpenGL context.");
		// Let's go through and choose one, instead of at random.
		GLXFBConfig best_config = fb_configs[0];
		int best_sample_count;
		glXGetFBConfigAttrib(x11d.display, best_config, GLX_SAMPLES, &best_sample_count);
		for(int i = 0; i < nelements; i++)
		{
			XVisualInfo* vi = glXGetVisualFromFBConfig(x11d.display, fb_configs[i]);
			if(vi != nullptr)
			{
				int samp_buf, samples;
				glXGetFBConfigAttrib(x11d.display, fb_configs[i], GLX_SAMPLE_BUFFERS, &samp_buf);
				glXGetFBConfigAttrib(x11d.display, fb_configs[i], GLX_SAMPLES, &samples);
				if(samp_buf && samples > best_sample_count)
				{
					best_sample_count = samples;
					best_config = fb_configs[i];
				}
			}
		}
		XFree(fb_configs);
		int attribs[] =
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, maj,
			GLX_CONTEXT_MINOR_VERSION_ARB, min,
			GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
			None
		};
		const char *glx_exts = glXQueryExtensionsString(x11d.display, x11d.screen);
		if(!is_extension_supported(glx_exts, "GLX_ARB_create_context"))
		{
			tz::error("glXCreateContextAttribsARB is not supported/available. Cannot create OpenGL context.");
		}
		typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

		glXCreateContextAttribsARBProc glXCreateContextAttribsARB = reinterpret_cast<glXCreateContextAttribsARBProc>(reinterpret_cast<void*>(glXGetProcAddressARB(reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB"))));
		tz::assert(glXCreateContextAttribsARB != nullptr);
		this->ctx = glXCreateContextAttribsARB(x11d.display, best_config, nullptr, True, attribs);
		tz::assert(this->ctx != nullptr, "Could create OpenGL context :(");
		this->make_opengl_context_current();	
	}

	void* get_opengl_proc_address_linux(const char* name)
	{
		return (void*)glXGetProcAddress((const GLubyte*)name);
	}
}

#endif // __linux__

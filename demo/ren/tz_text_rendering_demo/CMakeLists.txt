add_demo(
	TARGET tz_text_rendering_demo
	SOURCE_FILES
		main.cpp
)

add_text(
	TARGET tz_text_rendering_demo
	INPUT_DIR ${PROJECT_SOURCE_DIR}
	OUTPUT_DIR ${PROJECT_BINARY_DIR}
	TEXT_FILES
		demo/ren/tz_text_rendering_demo/res/ProggyClean.ttf
)

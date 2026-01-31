idf_component_register(SRCS main.c in.c out.c
                       INCLUDE_DIRS .
                       PRIV_REQUIRES esp_timer config pin c24k_8b)
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")

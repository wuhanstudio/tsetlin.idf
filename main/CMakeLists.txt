idf_component_register(SRCS "main.c" "sdcard.c" "mnist.c"
                    INCLUDE_DIRS "."
                    REQUIRES "fatfs")
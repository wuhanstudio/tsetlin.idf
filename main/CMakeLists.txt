idf_component_register(SRCS "main.c" "sdcard.c" "mnist.c" "protobuf-c/protobuf-c.c" "tsetlin.pb-c.c"
                    INCLUDE_DIRS "."
                    REQUIRES "fatfs")
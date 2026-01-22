idf_component_register(SRCS "main.c" "sdcard.c" "../../../mnist/mnist.c" "../../../tsetlin/tsetlin.c" "../../../tsetlin/clause.c" "../../../protobuf-c/protobuf-c.c" "../../../tsetlin/tsetlin.pb-c.c"
                    INCLUDE_DIRS "." "../../../" "../../../tsetlin" "../../../mnist"
                    REQUIRES "fatfs" "esp_psram")
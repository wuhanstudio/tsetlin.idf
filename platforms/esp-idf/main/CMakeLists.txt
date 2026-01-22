idf_component_register(SRCS "main.c" "sdcard.c" "../../../mnist/mnist.c" "../../../tsetlin/tsetlin.c" "../../../tsetlin/clause.c" "../../../protobuf-c/protobuf-c.c" "../../../protobuf/tsetlin.pb-c.c"
                    INCLUDE_DIRS "." "../../../" "../../../tsetlin" "../../../mnist" "../../../protobuf" "../../../utils"
                    REQUIRES "fatfs" "esp_psram")
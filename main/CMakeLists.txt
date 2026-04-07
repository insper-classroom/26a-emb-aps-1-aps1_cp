add_executable(pico_emb main.c genius.c audio.c)

set_target_properties(pico_emb PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# pull in common dependencies
target_link_libraries(pico_emb pico_stdlib pico_multicore hardware_pwm hardware_clocks hardware_sync)

# create map/bin/hex/uf2 file etc.
pico_enable_stdio_usb(pico_emb 1)
pico_enable_stdio_uart(pico_emb 0)

pico_add_extra_outputs(pico_emb)

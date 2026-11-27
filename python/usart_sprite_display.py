#!/usr/bin/env python3
"""
USART sprite-screen emulator.

Reads lines of hexadecimal bytes from a serial port. Each byte is a sprite
index into a black/white PNG sprite map. Sprites are placed left-to-right,
top-to-bottom into a 64x128 buffer. When '\n' is received the buffer is
shown on screen and then cleared.

Requires: pygame, pyserial
    pip install pygame pyserial
"""

import sys

import pygame
import serial

# ----------------------------------------------------------------------------
# Configuration
# ----------------------------------------------------------------------------
SERIAL_PORT = "COM3"   # e.g. "COM3" on Windows
BAUD_RATE = 115200
SPRITE_MAP_PATH = "./python/sprites.png"

# Incoming data format:
#   "hex" - ASCII hex characters (e.g. "0011ABCD"), messages end with '\n' (0x0A)
#   "raw" - raw bytes, each byte is a sprite index, messages end with 0xFF
#           (so sprite index 255 can't be used in this mode)
INPUT_MODE = "raw"

SCALE = 6                      # integer window scale factor

# Bytes sent after the key character on press / release.
# (Sent as raw bytes 0x01 and 0x00, i.e. C-style '\1' and '\0'.)
KEY_PRESS_BYTE = b"\x01"
KEY_RELEASE_BYTE = b"\x00"

# ----------------------------------------------------------------------------
# Fixed screen / sprite geometry
# ----------------------------------------------------------------------------
SPRITE_SIZE = 4                # sprites are 4x4 pixels
MAP_SPRITES_WIDE = 16          # sprite map is 16 sprites wide
SCREEN_SPRITES_W = 16          # 16 * 4 = 64 px
SCREEN_SPRITES_H = 32          # 32 * 4 = 128 px
SCREEN_W = SCREEN_SPRITES_W * SPRITE_SIZE
SCREEN_H = SCREEN_SPRITES_H * SPRITE_SIZE
SCREEN_SPRITE_COUNT = SCREEN_SPRITES_W * SCREEN_SPRITES_H

if INPUT_MODE == "hex":
    TERMINATOR = 0x0A                           # '\n'
    MAX_MESSAGE_LEN = SCREEN_SPRITE_COUNT * 2   # 2 hex characters per sprite
elif INPUT_MODE == "raw":
    TERMINATOR = 0xFF
    MAX_MESSAGE_LEN = SCREEN_SPRITE_COUNT       # 1 byte per sprite
else:
    raise ValueError(f"INPUT_MODE must be 'hex' or 'raw', got {INPUT_MODE!r}")

BLACK = (0, 0, 0)
WHITE = (255, 255, 255)


def load_sprite_map(path):
    """Load the PNG and threshold it to pure black/white at 50%.

    Returns (surface, sprite_rows).
    """
    img = pygame.image.load(path)
    w, h = img.get_size()
    bw = pygame.Surface((w, h))
    for y in range(h):
        for x in range(w):
            r, g, b, a = img.get_at((x, y))
            # Composite onto black so transparent pixels count as black.
            lum = (0.299 * r + 0.587 * g + 0.114 * b) * (a / 255.0)
            bw.set_at((x, y), WHITE if lum >= 127.5 else BLACK)
    return bw, h // SPRITE_SIZE


def build_frame(line_bytes, sprite_map, sprite_rows):
    """Render one line of sprite indexes into a fresh 64x128 surface."""
    frame = pygame.Surface((SCREEN_W, SCREEN_H))
    frame.fill(BLACK)  # blank background for any unfilled sprites

    max_index = sprite_rows * MAP_SPRITES_WIDE
    for pos, idx in enumerate(line_bytes[:SCREEN_SPRITE_COUNT]):
        if idx >= max_index:
            print(f"Warning: sprite index {idx} (0x{idx:02X}) is outside the "
                  f"sprite map ({sprite_rows} rows); leaving blank.",
                  file=sys.stderr)
            continue

        src = pygame.Rect(
            (idx % MAP_SPRITES_WIDE) * SPRITE_SIZE,
            (idx // MAP_SPRITES_WIDE) * SPRITE_SIZE,
            SPRITE_SIZE,
            SPRITE_SIZE,
        )
        dst = (
            (pos % SCREEN_SPRITES_W) * SPRITE_SIZE,
            (pos // SCREEN_SPRITES_W) * SPRITE_SIZE,
        )
        frame.blit(sprite_map, dst, src)

    return frame


def key_to_char(key):
    """Return the lowercase alphanumeric character for a pygame key code,
    or None if it isn't a-z / 0-9 (shift/caps lock are ignored)."""
    if pygame.K_a <= key <= pygame.K_z or pygame.K_0 <= key <= pygame.K_9:
        return chr(key)
    return None


def parse_line(line, overflowed):
    """Convert one received message into a bytes object of sprite indexes.

    In "hex" mode the message is ASCII hex; in "raw" mode it is already the
    sprite indexes. Returns None (after printing a warning) if the message is
    malformed, e.g. because the device restarted partway through it.
    """
    if overflowed or len(line) > MAX_MESSAGE_LEN:
        unit = "hex characters" if INPUT_MODE == "hex" else "bytes"
        print(f"Warning: message too long (max {MAX_MESSAGE_LEN} {unit}); "
              f"discarding.", file=sys.stderr)
        return None
    if INPUT_MODE == "raw":
        return line
    if len(line) % 2 != 0:
        print(f"Warning: odd number of hex characters ({len(line)}); "
              f"discarding.", file=sys.stderr)
        return None
    try:
        return bytes.fromhex(line.decode("ascii"))
    except (ValueError, UnicodeDecodeError):
        print("Warning: message contains non-hex characters; discarding.",
              file=sys.stderr)
        return None


def main():
    pygame.init()
    window = pygame.display.set_mode((SCREEN_W * SCALE, SCREEN_H * SCALE))
    pygame.display.set_caption("USART Sprite Display")
    clock = pygame.time.Clock()

    sprite_map, sprite_rows = load_sprite_map(SPRITE_MAP_PATH)

    # Raises serial.SerialException if the port can't be opened.
    ser = serial.Serial(
        port=SERIAL_PORT,
        baudrate=BAUD_RATE,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=0,  # non-blocking reads
    )

    shown_frame = pygame.Surface((SCREEN_W, SCREEN_H))
    shown_frame.fill(BLACK)
    rx = bytearray()    # bytes received since the last terminator
    overflowed = False  # True once the current message exceeds the max length

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type in (pygame.KEYDOWN, pygame.KEYUP):
                char = key_to_char(event.key)
                if char is not None:
                    suffix = (KEY_PRESS_BYTE if event.type == pygame.KEYDOWN
                              else KEY_RELEASE_BYTE)
                    ser.write(char.encode("ascii") + suffix)

        data = ser.read(ser.in_waiting or 1)
        for byte in data:
            if byte != TERMINATOR:
                # Stop storing once too long (so a runaway message can't
                # grow forever); just wait for the next terminator.
                if len(rx) > MAX_MESSAGE_LEN:
                    overflowed = True
                else:
                    rx.append(byte)
                continue

            # Terminator received: validate, draw if good, and always reset.
            sprite_indexes = parse_line(bytes(rx), overflowed)
            if sprite_indexes is not None:
                shown_frame = build_frame(sprite_indexes, sprite_map,
                                          sprite_rows)
            rx.clear()
            overflowed = False

        scaled = pygame.transform.scale(shown_frame, window.get_size())
        window.blit(scaled, (0, 0))
        pygame.display.flip()
        clock.tick(120)

    ser.close()
    pygame.quit()


if __name__ == "__main__":
    main()
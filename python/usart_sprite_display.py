#!/usr/bin/env python3
"""
USART sprite-screen emulator.

Reads lines of hexadecimal bytes from a serial port. Each byte is a sprite
index into a black/white PNG sprite map. Sprites are placed left-to-right,
top-to-bottom into a 64x128 buffer. When '\n' is received the buffer is
shown on screen and then cleared.

A menu bar at the top of the window has a serial port selector. At startup
the first available port is selected automatically; if there are none, the
program idles until a port is chosen from the menu.

Requires: pygame, pyserial
    pip install pygame pyserial
"""

import sys

import pygame
import serial
import serial.tools.list_ports

# ----------------------------------------------------------------------------
# Configuration
# ----------------------------------------------------------------------------
BAUD_RATE = 576000             # serial port is chosen from the menu bar
SPRITE_MAP_PATH = "./python/sprites.png"

# Incoming data format:
#   "hex" - ASCII hex characters (e.g. "0011ABCD"), messages end with '\n' (0x0A)
#   "raw" - raw bytes, each byte is a sprite index, messages end with 0xFF
#           (so sprite index 255 can't be used in this mode)
INPUT_MODE = "raw"

SCALE = 4                      # integer window scale factor

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

# Menu bar (drawn above the emulated screen, in window pixels)
MENU_BAR_HEIGHT = 28
MENU_ROW_HEIGHT = 24
MENU_BUTTON_WIDTH = 210
MIN_WINDOW_WIDTH = 360         # keeps the menu bar usable at small SCALE values

BG_COLOR = (20, 20, 20)
BAR_COLOR = (40, 40, 40)
BAR_LINE_COLOR = (90, 90, 90)
HOVER_COLOR = (75, 75, 75)
TEXT_COLOR = (230, 230, 230)
TEXT_DIM_COLOR = (140, 140, 140)


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


def list_ports():
    """Return a sorted list of available serial port names (never raises)."""
    try:
        return sorted(p.device for p in serial.tools.list_ports.comports())
    except Exception as exc:
        print(f"Warning: could not list serial ports: {exc}", file=sys.stderr)
        return []


def open_port(name):
    """Open the named port with the configured settings (8N1, non-blocking).

    Returns None (after printing a warning) if it can't be opened.
    """
    try:
        return serial.Serial(
            port=name,
            baudrate=BAUD_RATE,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0,  # non-blocking reads
        )
    except (serial.SerialException, OSError, ValueError) as exc:
        print(f"Warning: could not open {name}: {exc}", file=sys.stderr)
        return None


def close_port(ser):
    """Close a port if there is one, ignoring errors."""
    if ser is not None:
        try:
            ser.close()
        except (serial.SerialException, OSError):
            pass


class PortMenu:
    """Menu bar with a dropdown for choosing the serial port."""

    def __init__(self, font):
        self.font = font
        self.button = pygame.Rect(8, 4, MENU_BUTTON_WIDTH, MENU_BAR_HEIGHT - 8)
        self.ports = []
        self.selected = None   # name of the chosen port, or None
        self.is_open = False   # dropdown list visible?

    def _item_rect(self, index):
        return pygame.Rect(self.button.x,
                           MENU_BAR_HEIGHT + index * MENU_ROW_HEIGHT,
                           self.button.w, MENU_ROW_HEIGHT)

    def handle_click(self, pos):
        """Process a left click. Returns a port name if the user picked one."""
        if self.is_open:
            self.is_open = False
            for i, name in enumerate(self.ports):
                if self._item_rect(i).collidepoint(pos):
                    return name
            return None
        if self.button.collidepoint(pos):
            self.ports = list_ports()   # rescan every time the list is opened
            self.is_open = True
        return None

    def _blit_text(self, surface, text, color, left, centery):
        img = self.font.render(text, True, color)
        surface.blit(img, (left, centery - img.get_height() // 2))
        return img

    def draw(self, surface, status):
        width = surface.get_width()
        mouse = pygame.mouse.get_pos()

        # Bar background
        pygame.draw.rect(surface, BAR_COLOR, (0, 0, width, MENU_BAR_HEIGHT))
        pygame.draw.line(surface, BAR_LINE_COLOR,
                         (0, MENU_BAR_HEIGHT - 1), (width, MENU_BAR_HEIGHT - 1))

        # Selector button: current port + dropdown arrow
        hot = self.is_open or self.button.collidepoint(mouse)
        pygame.draw.rect(surface, HOVER_COLOR if hot else BG_COLOR,
                         self.button, border_radius=3)
        pygame.draw.rect(surface, BAR_LINE_COLOR, self.button, 1,
                         border_radius=3)
        self._blit_text(surface, self.selected or "No port selected",
                        TEXT_COLOR if self.selected else TEXT_DIM_COLOR,
                        self.button.x + 8, self.button.centery)
        ax, ay = self.button.right - 14, self.button.centery
        pygame.draw.polygon(surface, TEXT_COLOR,
                            [(ax - 4, ay - 2), (ax + 4, ay - 2), (ax, ay + 3)])

        # Status text, right-aligned
        img = self.font.render(status, True, TEXT_DIM_COLOR)
        surface.blit(img, (width - img.get_width() - 8,
                           self.button.centery - img.get_height() // 2))

        # Dropdown list (drawn last so it overlays the screen)
        if self.is_open:
            names = self.ports or [None]
            for i, name in enumerate(names):
                rect = self._item_rect(i)
                hovered = name is not None and rect.collidepoint(mouse)
                pygame.draw.rect(surface, HOVER_COLOR if hovered else BAR_COLOR,
                                 rect)
                self._blit_text(
                    surface,
                    name if name is not None else "No ports found",
                    TEXT_COLOR if name is not None else TEXT_DIM_COLOR,
                    rect.x + 8, rect.centery)
            pygame.draw.rect(
                surface, BAR_LINE_COLOR,
                pygame.Rect(self.button.x, MENU_BAR_HEIGHT, self.button.w,
                            MENU_ROW_HEIGHT * len(names)), 1)


def main():
    pygame.init()
    screen_px_w = SCREEN_W * SCALE
    screen_px_h = SCREEN_H * SCALE
    win_w = max(screen_px_w, MIN_WINDOW_WIDTH)
    window = pygame.display.set_mode((win_w, MENU_BAR_HEIGHT + screen_px_h))
    pygame.display.set_caption("USART Sprite Display")
    clock = pygame.time.Clock()
    font = pygame.font.Font(None, 22)
    frame_pos = ((win_w - screen_px_w) // 2, MENU_BAR_HEIGHT)

    sprite_map, sprite_rows = load_sprite_map(SPRITE_MAP_PATH)

    # Auto-select the first available port; if there are none, stay idle
    # until the user picks one from the menu.
    menu = PortMenu(font)
    ser = None
    ports = list_ports()
    if ports:
        menu.selected = ports[0]
        ser = open_port(ports[0])

    shown_frame = pygame.Surface((SCREEN_W, SCREEN_H))
    shown_frame.fill(BLACK)
    rx = bytearray()    # bytes received since the last terminator
    overflowed = False  # True once the current message exceeds the max length

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                choice = menu.handle_click(event.pos)
                if choice is not None:
                    close_port(ser)
                    menu.selected = choice
                    ser = open_port(choice)
                    rx.clear()
                    overflowed = False

            elif event.type in (pygame.KEYDOWN, pygame.KEYUP):
                char = key_to_char(event.key)
                if char is not None and ser is not None:
                    suffix = (KEY_PRESS_BYTE if event.type == pygame.KEYDOWN
                              else KEY_RELEASE_BYTE)
                    try:
                        ser.write(char.encode("ascii") + suffix)
                    except (serial.SerialException, OSError) as exc:
                        print(f"Warning: write to {menu.selected} failed: "
                              f"{exc}", file=sys.stderr)
                        close_port(ser)
                        ser = None

        data = b""
        if ser is not None:
            try:
                data = ser.read(ser.in_waiting or 1)
            except (serial.SerialException, OSError) as exc:
                print(f"Warning: lost connection to {menu.selected}: {exc}",
                      file=sys.stderr)
                close_port(ser)
                ser = None

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

        if ser is not None:
            status = "Connected"
        elif menu.selected is not None:
            status = "Not connected"
        else:
            status = "Idle (no ports)"

        window.fill(BG_COLOR)
        scaled = pygame.transform.scale(shown_frame,
                                        (screen_px_w, screen_px_h))
        window.blit(scaled, frame_pos)
        menu.draw(window, status)
        pygame.display.flip()
        clock.tick(120)

    close_port(ser)
    pygame.quit()


if __name__ == "__main__":
    main()
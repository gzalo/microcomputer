#!/usr/bin/env python3
"""Build credit-card-size references and a single A4 cut-and-fold sheet.

Usage: python docs/build_cheatsheets.py [--dpi 300]
Requires the packages in docs/requirements-pdf.txt.
"""

from __future__ import annotations

import argparse
import io
import re
from pathlib import Path

import pypdfium2 as pdfium
from PIL import Image, ImageDraw, ImageOps
from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import mm
from reportlab.lib.utils import ImageReader
from reportlab.pdfgen import canvas

from build_guia_pdf import (
    ROOT,
    SOURCE,
    WEAR_ASSETS,
    face,
    fax_filter,
    parse_blocks,
    register_fonts,
)


CARD_W, CARD_H = 85.6 * mm, 53.98 * mm
BINARY_SOURCE = ROOT / "docs" / "repaso-de-binario.md"
OUTPUT_DIR = ROOT / "output" / "pdf"
BINARY_OUTPUT = OUTPUT_DIR / "repaso-de-binario-85.6x53.98mm.pdf"
INSTRUCTION_OUTPUT = OUTPUT_DIR / "instrucciones-8080-85.6x53.98mm.pdf"
SHEET_OUTPUT = OUTPUT_DIR / "tarjetas-8080-hoja-a4.pdf"

# Compact wording for a card. The source guide remains authoritative; the
# opcode list is checked against its instruction table on every build.
INSTRUCTION_ROWS = [
    ("00", "00", "NOP", "NO CAMBIA NADA"),
    ("76", "76", "HLT", "DETIENE LA CPU"),
    ("3E nn", "3E nn", "MVI A,nn", "CARGA A CON nn"),
    ("06 nn / 0E nn", "06/0E nn", "MVI B/C,nn", "CARGA B O C CON nn"),
    ("21 lo hi", "21 lo hi", "LXI H,nnnn", "HL = DIRECCIÓN nnnn"),
    ("36 nn", "36 nn", "MVI M,nn", "RAM[HL] = nn"),
    ("77", "77", "MOV M,A", "RAM[HL] = A"),
    ("23", "23", "INX H", "HL = HL + 1"),
    ("3C", "3C", "INR A", "A = A + 1"),
    ("C6 nn", "C6 nn", "ADI nn", "A = A + nn"),
    ("C3 lo hi", "C3 lo hi", "JMP nnnn", "SALTA A nnnn"),
    ("C2 lo hi", "C2 lo hi", "JNZ nnnn", "SALTA SI Z = 0"),
]


def verify_guide_instructions() -> None:
    tables = [value for kind, value in parse_blocks(SOURCE.read_text(encoding="utf-8"))
              if kind == "table" and value[0][0] == "Bytes hexadecimales"]
    if len(tables) != 1:
        raise ValueError("Expected one instruction table in the Markdown guide")
    actual = [re.sub(r"`", "", row[0]) for row in tables[0][1:]]
    expected = [row[0] for row in INSTRUCTION_ROWS]
    if actual != expected:
        raise ValueError("The guide's opcode list changed; update the pocket card")


def binary_rows() -> list[list[str]]:
    tables = [value for kind, value in parse_blocks(BINARY_SOURCE.read_text(encoding="utf-8"))
              if kind == "table"]
    if len(tables) != 1 or len(tables[0]) != 9:
        raise ValueError("Expected an eight-row binary table in the standalone Markdown")
    rows = [[cell.replace("`", "") for cell in row] for row in tables[0][1:]]
    for index, row in enumerate(rows):
        if row != [f"{index:04b}", f"{index:X}",
                   f"{index + 8:04b}", f"{index + 8:X}"]:
            raise ValueError(f"Incorrect nibble mapping in row {index + 1}")
    return rows


def frame(c: canvas.Canvas, card_no: str) -> None:
    c.setFillColor(colors.white)
    c.rect(0, 0, CARD_W, CARD_H, stroke=0, fill=1)
    c.setFillColor(colors.black)
    c.setStrokeColor(colors.black)
    c.setFont(face(True), 5.5)
    c.drawString(9, CARD_H - 10, "EXPEDIENTE 8080")
    c.drawRightString(CARD_W - 9, CARD_H - 10, card_no)
    c.setLineWidth(0.5)
    c.line(9, CARD_H - 14, CARD_W - 9, CARD_H - 14)
    c.line(9, 12, CARD_W - 9, 12)
    c.setFont(face(), 5.2)
    c.drawRightString(CARD_W - 9, 5.5, "by gzalo")


def spaced_bits(c: canvas.Canvas, x: float, y: float, bits: str) -> None:
    """Line up each bit with the 8 4 2 1 positions above it."""
    text = c.beginText(x, y)
    text.setFont(face(True), 6.7)
    text.setCharSpace(3.18)
    text.textOut(bits)
    text.setCharSpace(0)
    c.drawText(text)


def binary_vector() -> bytes:
    rows = binary_rows()
    buffer = io.BytesIO()
    c = canvas.Canvas(buffer, pagesize=(CARD_W, CARD_H), invariant=1)
    c.setTitle("Repaso de binario - 85.6 x 53.98 mm")
    frame(c, "01")
    c.setFont(face(True), 9.5)
    c.drawString(9, CARD_H - 27, "REPASO DE BINARIO")
    c.setFont(face(True), 6.1)
    c.drawString(9, CARD_H - 39, "4 LLAVES = 1 HEX  /  ARRIBA = 1  /  ABAJO = 0")
    c.setLineWidth(0.45)
    c.line(9, CARD_H - 44, CARD_W - 9, CARD_H - 44)
    c.setFont(face(True), 6.0)
    c.drawString(14, CARD_H - 53, "8 4 2 1")
    c.drawString(76, CARD_H - 53, "HEX")
    c.drawString(128, CARD_H - 53, "8 4 2 1")
    c.drawString(190, CARD_H - 53, "HEX")
    c.line(9, CARD_H - 58, CARD_W - 9, CARD_H - 58)
    c.line(CARD_W / 2, CARD_H - 44, CARD_W / 2, 25)
    for row_index, (left_bits, left_hex, right_bits, right_hex) in enumerate(rows):
        y = CARD_H - 69 - row_index * 8.0
        c.setFont(face(True), 6.7)
        spaced_bits(c, 14, y, left_bits)
        c.drawString(76, y, left_hex)
        spaced_bits(c, 128, y, right_bits)
        c.drawString(190, y, right_hex)
    c.line(9, 25, CARD_W - 9, 25)
    c.setFont(face(True), 5.4)
    c.drawString(9, 17, "UN BYTE = DOS GRUPOS  /  3C = 0011 1100")
    c.showPage()
    c.save()
    return buffer.getvalue()


def instruction_vector() -> bytes:
    verify_guide_instructions()
    buffer = io.BytesIO()
    c = canvas.Canvas(buffer, pagesize=(CARD_W, CARD_H), invariant=1)
    c.setTitle("Instrucciones 8080 - 85.6 x 53.98 mm - dos caras")
    for side in range(2):
        frame(c, f"02 / {side + 1}")
        c.setFont(face(True), 9.5)
        c.drawString(9, CARD_H - 27, "INSTRUCCIONES")
        c.setFont(face(), 6.0)
        c.drawRightString(CARD_W - 9, CARD_H - 27,
                          f"8080 / CARA {side + 1} DE 2")
        c.line(9, CARD_H - 34, CARD_W - 9, CARD_H - 34)
        c.setFont(face(True), 5.3)
        c.drawString(10, CARD_H - 43, "HEX")
        c.drawString(73, CARD_H - 43, "ORDEN")
        c.drawString(145, CARD_H - 43, "EFECTO")
        c.line(9, CARD_H - 47, CARD_W - 9, CARD_H - 47)
        for row_index, (_, code, mnemonic, note) in enumerate(
                INSTRUCTION_ROWS[side * 6:(side + 1) * 6]):
            y = CARD_H - 59 - row_index * 13.4
            c.setFont(face(True), 6.2)
            c.drawString(10, y, code)
            c.drawString(73, y, mnemonic)
            c.setFont(face(), 6.0)
            c.drawString(145, y, note)
            c.setStrokeColor(colors.HexColor("#777777"))
            c.setLineWidth(0.3)
            c.line(9, y - 4, CARD_W - 9, y - 4)
            c.setStrokeColor(colors.black)
        c.setFont(face(), 5.3)
        c.drawString(9, 17,
                     "nn = BYTE" if side == 0 else "lo hi = BAJO, ALTO")
        c.showPage()
    c.save()
    return buffer.getvalue()


def light_card_wear(page: Image.Image, side: int) -> Image.Image:
    """A little edge wear, kept away from the small instruction text."""
    base = page.convert("RGBA")
    width, height = base.size
    with Image.open(WEAR_ASSETS / "foxing-smudges.png") as source:
        overlay = source.convert("RGBA").resize((width, height), Image.Resampling.LANCZOS)
    if side % 2 == 0:
        overlay = ImageOps.mirror(overlay)
    alpha = overlay.getchannel("A").point(lambda value: round(value * 0.075))
    overlay.putalpha(alpha)
    base.alpha_composite(overlay)
    return base.convert("RGB")


def processed_pages(vector: bytes, dpi: int) -> list[Image.Image]:
    pages = pdfium.PdfDocument(vector)
    processed = []
    for index in range(len(pages)):
        image = pages[index].render(scale=dpi / 72).to_pil()
        image = light_card_wear(image, index)
        image = fax_filter(image, index + 41, paper_wear=False).convert("RGB")
        border_width = max(2, round(dpi * 0.25 / 25.4))
        ImageDraw.Draw(image).rectangle(
            (0, 0, image.width - 1, image.height - 1),
            outline=(145, 145, 145), width=border_width,
        )
        processed.append(image)
    return processed


def image_reader(image: Image.Image) -> ImageReader:
    jpeg = io.BytesIO()
    image.save(jpeg, format="JPEG", quality=92, optimize=True)
    jpeg.seek(0)
    return ImageReader(jpeg)


def card_pdf(images: list[Image.Image], destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    c = canvas.Canvas(str(destination), pagesize=(CARD_W, CARD_H), invariant=1)
    for image in images:
        c.drawImage(image_reader(image), 0, 0, width=CARD_W, height=CARD_H)
        c.showPage()
    c.save()


def sheet_pdf(binary: Image.Image, instructions: list[Image.Image]) -> None:
    """Place the instruction faces flush together and the binary card 30 mm below."""
    page_w, page_h = A4
    pair_x = (page_w - 2 * CARD_W) / 2
    pair_y = page_h - 25 * mm - CARD_H
    binary_x = (page_w - CARD_W) / 2
    binary_y = pair_y - 30 * mm - CARD_H
    c = canvas.Canvas(str(SHEET_OUTPUT), pagesize=A4, invariant=1)
    c.setTitle("Tarjetas 8080 - hoja A4 para recortar y doblar")
    c.drawImage(image_reader(instructions[0]), pair_x, pair_y,
                width=CARD_W, height=CARD_H)
    c.drawImage(image_reader(instructions[1]), pair_x + CARD_W, pair_y,
                width=CARD_W, height=CARD_H)
    c.drawImage(image_reader(binary), binary_x, binary_y,
                width=CARD_W, height=CARD_H)
    c.showPage()
    c.save()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dpi", type=int, default=300,
                        help="Raster resolution (default: 300)")
    args = parser.parse_args()
    if not 200 <= args.dpi <= 600:
        parser.error("--dpi must be between 200 and 600")
    register_fonts()
    binary = processed_pages(binary_vector(), args.dpi)
    instructions = processed_pages(instruction_vector(), args.dpi)
    card_pdf(binary, BINARY_OUTPUT)
    card_pdf(instructions, INSTRUCTION_OUTPUT)
    sheet_pdf(binary[0], instructions)
    print(BINARY_OUTPUT)
    print(INSTRUCTION_OUTPUT)
    print(SHEET_OUTPUT)


if __name__ == "__main__":
    main()

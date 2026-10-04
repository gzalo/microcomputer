#!/usr/bin/env python3
"""Turn guia-fiesta-hacker.md into an A4, image-only photocopy dossier.

Usage: python docs/build_guia_pdf.py [--landscape] [--output path.pdf] [--dpi 220]
Dependencies: reportlab, Pillow, pypdfium2 (see requirements-pdf.txt).
"""

from __future__ import annotations

import argparse
import html
import io
import random
import re
import zlib
from pathlib import Path

import pypdfium2 as pdfium
from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageOps
from reportlab.lib import colors
from reportlab.lib.enums import TA_LEFT
from reportlab.lib.pagesizes import A4, landscape
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.utils import ImageReader
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.pdfgen import canvas
from reportlab.platypus import (
    BaseDocTemplate,
    Flowable,
    Frame,
    NextPageTemplate,
    PageBreak,
    PageTemplate,
    Paragraph,
    Spacer,
    Table,
    TableStyle,
)


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "docs" / "guia-fiesta-hacker.md"
PHOTO = ROOT / "images" / "photo_new.jpg"
WEAR_ASSETS = ROOT / "docs" / "assets" / "paper-wear"
DEFAULT_OUTPUT = ROOT / "output" / "pdf" / "expediente-8080-impresion.pdf"
LANDSCAPE_OUTPUT = ROOT / "output" / "pdf" / "expediente-8080-apaisado.pdf"
PAGE_W, PAGE_H = A4
MARGIN_X = 47
CONTENT_W = PAGE_W - 2 * MARGIN_X
INK = colors.black
RULE = colors.HexColor("#8b8b8b")
PAPER = colors.white


def set_page_orientation(horizontal: bool) -> None:
    global PAGE_W, PAGE_H, MARGIN_X, CONTENT_W
    PAGE_W, PAGE_H = landscape(A4) if horizontal else A4
    MARGIN_X = 95 if horizontal else 47
    CONTENT_W = PAGE_W - 2 * MARGIN_X


def register_fonts() -> None:
    """Use a real monospaced TTF when present, with PDF Courier as fallback."""
    candidates = [
        (Path("C:/Windows/Fonts/cour.ttf"), Path("C:/Windows/Fonts/courbd.ttf")),
        (Path("/usr/share/fonts/truetype/liberation2/LiberationMono-Regular.ttf"),
         Path("/usr/share/fonts/truetype/liberation2/LiberationMono-Bold.ttf")),
        (Path("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"),
         Path("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf")),
    ]
    for regular, bold in candidates:
        if regular.exists() and bold.exists():
            pdfmetrics.registerFont(TTFont("Dossier", str(regular)))
            pdfmetrics.registerFont(TTFont("Dossier-Bold", str(bold)))
            pdfmetrics.registerFontFamily("Dossier", normal="Dossier", bold="Dossier-Bold")
            return
    pdfmetrics.registerFontFamily("Courier", normal="Courier", bold="Courier-Bold")


def face(bold: bool = False) -> str:
    prefix = "Dossier" if "Dossier" in pdfmetrics.getRegisteredFontNames() else "Courier"
    return prefix + ("-Bold" if bold else "")


def inline_markup(value: str, jitter: bool = False) -> str:
    """Convert the guide's small inline Markdown subset to ReportLab markup."""
    tokens = re.split(r"(`[^`]+`|\*\*[^*]+\*\*)", value)
    out = []
    rng = random.Random(zlib.crc32(value.encode("utf-8")))
    for token in tokens:
        if token.startswith("`") and token.endswith("`"):
            out.append(f"<b>{html.escape(token[1:-1])}</b>")
        elif token.startswith("**") and token.endswith("**"):
            out.append(f"<b>{html.escape(token[2:-2])}</b>")
        else:
            if jitter:
                for part in re.split(r"(\s+)", token):
                    if part and not part.isspace():
                        size = rng.choice((9.0, 9.15, 9.2, 9.25, 9.4))
                        out.append(f'<font size="{size}">{html.escape(part)}</font>')
                    else:
                        out.append(part)
            else:
                out.append(html.escape(token))
    return "".join(out)


def parse_blocks(markdown: str):
    lines = markdown.splitlines()
    blocks = []
    idx = 0
    while idx < len(lines):
        line = lines[idx].strip()
        if not line:
            idx += 1
            continue
        if line.startswith("# "):
            blocks.append(("title", line[2:]))
            idx += 1
        elif line.startswith("### "):
            blocks.append(("subsection", line[4:]))
            idx += 1
        elif line.startswith("## "):
            blocks.append(("section", line[3:]))
            idx += 1
        elif line.startswith("| "):
            table = []
            while idx < len(lines) and lines[idx].strip().startswith("|"):
                row = [cell.strip() for cell in lines[idx].strip().strip("|").split("|")]
                if not all(re.fullmatch(r":?-{3,}:?", cell) for cell in row):
                    table.append(row)
                idx += 1
            blocks.append(("table", table))
        else:
            paragraph = [line]
            idx += 1
            while idx < len(lines) and lines[idx].strip() and not lines[idx].startswith(("#", "|")):
                paragraph.append(lines[idx].strip())
                idx += 1
            blocks.append(("paragraph", " ".join(paragraph)))
    return blocks


class WornParagraph(Paragraph):
    """Slight baseline and alignment variation, like re-fed typewritten sheets."""

    def drawOn(self, canv, x, y, _sW=0):
        rng = random.Random(zlib.crc32(self.text.encode("utf-8")))
        canv.saveState()
        canv.translate(x + rng.uniform(-0.43, 0.43), y + rng.uniform(-0.38, 0.38))
        canv.rotate(rng.uniform(-0.075, 0.075))
        super().drawOn(canv, 0, 0, _sW)
        canv.restoreState()


class TypewrittenHeading(Flowable):
    def __init__(self, label: str, small: bool = False):
        super().__init__()
        self.label = label.upper()
        self.small = small
        self.width = CONTENT_W
        self.height = 32 if small else 42
        self.keepWithNext = True

    def draw(self):
        c = self.canv
        rng = random.Random(zlib.crc32(self.label.encode("utf-8")))
        size = 9.6 if self.small else 12.2
        c.setFont(face(True), size)
        c.setFillColor(INK)
        x = 0.0
        baseline = 12 if self.small else 13
        for char in self.label:
            c.drawString(x + rng.uniform(-0.25, 0.25),
                         baseline + rng.uniform(-0.55, 0.55), char)
            x += pdfmetrics.stringWidth(char, face(True), size)
        c.setStrokeColor(RULE)
        c.setLineWidth(0.5)
        c.line(0, 2, CONTENT_W, 2)


class EvidencePhoto(Flowable):
    def __init__(self, image_path: Path):
        super().__init__()
        self.width = CONTENT_W if PAGE_H > PAGE_W else 450
        self.height = 184 if PAGE_H > PAGE_W else 166
        source = Image.open(image_path).convert("RGB")
        source = ImageOps.grayscale(source)
        source = ImageOps.autocontrast(source, cutoff=1)
        source = ImageEnhance.Contrast(source).enhance(1.2)
        # Crop the room away, retaining the LCD and the upper bank of switches.
        source = source.crop((0, 125, source.width, 540))
        source = ImageOps.fit(source, (880, 290), centering=(0.5, 0.5))
        source = source.point(lambda value: min(255, round(value / 51) * 51))
        draw = ImageDraw.Draw(source)
        rng = random.Random(8080)
        for _ in range(11):
            y = rng.randrange(source.height)
            x = rng.randrange(0, source.width // 2)
            draw.line((x, y, min(source.width, x + rng.randrange(70, 360)), y),
                      fill=255, width=1)
        data = io.BytesIO()
        source.save(data, format="PNG")
        data.seek(0)
        self.image = ImageReader(data)

    def draw(self):
        c = self.canv
        c.setStrokeColor(RULE)
        c.setLineWidth(0.7)
        c.rect(0, 22, self.width, self.height - 22, stroke=1, fill=0)
        c.drawImage(self.image, 1, 23, width=self.width - 2,
                    height=self.height - 24, mask="auto")
        c.setFillColor(INK)
        c.setFont(face(), 7.2)
        c.drawString(0, 7, "EVIDENCIA 01  /  PANEL DE LA MÁQUINA  /  ARCHIVO FOTOGRÁFICO")


class FieldNotes(Flowable):
    """Writing space turns the final folio into a useful lab record."""

    def __init__(self):
        super().__init__()
        self.width = CONTENT_W
        self.height = 665 if PAGE_H > PAGE_W else PAGE_H - 150

    def draw(self):
        c = self.canv
        if PAGE_W > PAGE_H:
            title_y = self.height - 23
            rule_y = title_y - 8
            heading_y = rule_y - 21
            first_rule_y = heading_y - 10
            address_edge, byte_edge = 150, 260
            c.setFillColor(INK)
            c.setFont(face(True), 10)
            c.drawString(0, title_y, "BITÁCORA DE CAMPO / SECUENCIA CARGADA")
            c.setStrokeColor(INK)
            c.setLineWidth(0.7)
            c.line(0, rule_y, self.width, rule_y)
            c.setFont(face(True), 7.5)
            c.drawString(6, heading_y, "DIRECCIÓN")
            c.drawString(address_edge + 10, heading_y, "BYTE")
            c.drawString(byte_edge + 12, heading_y, "OBSERVACIÓN / RESPUESTA")
            c.setStrokeColor(RULE)
            c.setLineWidth(0.35)
            for y in range(round(first_rule_y), 38, -22):
                c.line(0, y, self.width, y)
            c.line(address_edge, first_rule_y, address_edge, 43)
            c.line(byte_edge, first_rule_y, byte_edge, 43)
            c.setFont(face(), 7)
            c.setFillColor(INK)
            c.drawString(0, 19, "CONSERVAR ESTE REGISTRO JUNTO AL EXPEDIENTE")
            return
        c.setFillColor(INK)
        c.setFont(face(True), 10)
        c.drawString(0, 642, "BITÁCORA DE CAMPO / SECUENCIA CARGADA")
        c.setStrokeColor(INK)
        c.setLineWidth(0.7)
        c.line(0, 634, self.width, 634)
        c.setFont(face(True), 7.5)
        c.drawString(6, 613, "DIRECCIÓN")
        c.drawString(132, 613, "BYTE")
        c.drawString(224, 613, "OBSERVACIÓN / RESPUESTA")
        c.setStrokeColor(RULE)
        c.setLineWidth(0.35)
        for y in range(603, 40, -26):
            c.line(0, y, self.width, y)
        c.line(119, 603, 119, 57)
        c.line(210, 603, 210, 57)
        c.setFont(face(), 7)
        c.setFillColor(INK)
        c.drawString(0, 27, "CONSERVAR ESTE REGISTRO JUNTO AL EXPEDIENTE")


def seal(c, x: float, y: float, radius: float = 43) -> None:
    """Vector rubber-stamp effect. Coordinates are the seal center."""
    c.saveState()
    c.translate(x, y)
    c.rotate(-11)
    c.setStrokeColor(INK)
    c.setFillColor(INK)
    c.setLineWidth(1.7)
    c.circle(0, 0, radius, stroke=1, fill=0)
    c.circle(0, 0, radius - 5, stroke=1, fill=0)
    c.setFont(face(True), 8.1)
    c.drawCentredString(0, 12, "EX UMBRIS")
    c.setLineWidth(0.9)
    c.line(-27, 4, 27, 4)
    c.setFont(face(True), 10.5)
    c.drawCentredString(0, -9, "by gzalo")
    c.line(-27, -14, 27, -14)
    c.setFont(face(), 6.1)
    c.drawCentredString(0, -25, "AD LUCEM")
    # Tiny pale interruptions evoke an imperfect stamp impression.
    c.setStrokeColor(PAPER)
    c.setLineWidth(0.55)
    for ax, ay, bx, by in [(-34, 22, -27, 24), (31, -22, 36, -18), (-19, -34, -13, -35)]:
        c.line(ax, ay, bx, by)
    c.restoreState()


def page_wear(c, page: int) -> None:
    """The remaining vector marks are only light fax noise; damage uses scans."""
    rng = random.Random(1980 + page)
    c.saveState()
    c.setStrokeColor(colors.black)
    # Weak fax scan lines and occasional toner flecks.
    c.setStrokeAlpha(0.045)
    for _ in range(8):
        y = rng.uniform(80, PAGE_H - 75)
        c.setLineWidth(rng.uniform(0.25, 0.65))
        c.line(30, y, PAGE_W - 30, y + rng.uniform(-0.3, 0.3))
    c.setFillAlpha(0.12)
    for _ in range(65):
        x = rng.uniform(20, PAGE_W - 20)
        y = rng.uniform(30, PAGE_H - 30)
        c.circle(x, y, rng.uniform(0.12, 0.42), stroke=0, fill=1)
    c.restoreState()


def page_art(c, doc) -> None:
    c.saveState()
    page_wear(c, doc.page)
    c.setStrokeColor(RULE)
    c.setLineWidth(0.55)
    c.line(MARGIN_X, 51, PAGE_W - MARGIN_X, 51)
    c.setFillColor(INK)
    c.setFont(face(), 7)
    c.drawString(MARGIN_X, 37, "ARCHIVO 8080  /  COPIA DE CAMPO")
    if doc.pageTemplate.id != "notes":
        c.drawRightString(PAGE_W - MARGIN_X, 37, f"{doc.page:02d}")
    if doc.page == 1:
        c.setFillColor(INK)
        c.setFont(face(True), 24)
        c.drawString(MARGIN_X, PAGE_H - 83, "EXPEDIENTE 8080")
        c.setFont(face(), 8.3)
        c.setFillColor(INK)
        c.drawString(MARGIN_X + 2, PAGE_H - 104, "PROTOCOLO DE ENCENDIDO / TRES TRANSMISIONES")
        seal(c, PAGE_W - MARGIN_X - 44, PAGE_H - 87, 42)
        c.setStrokeColor(RULE)
        c.line(MARGIN_X, PAGE_H - 130, PAGE_W - MARGIN_X, PAGE_H - 130)
    else:
        c.setFont(face(True), 8)
        c.setFillColor(INK)
        c.drawString(MARGIN_X, PAGE_H - 41, "EXPEDIENTE 8080")
        c.line(MARGIN_X, PAGE_H - 49, PAGE_W - MARGIN_X, PAGE_H - 49)
    c.restoreState()


def table_widths(rows):
    headers = rows[0]
    if len(headers) == 4:
        return [173, 77, 173, CONTENT_W - 423]
    if len(headers) == 2:
        return [165, CONTENT_W - 165]
    if headers[0] == "Registro":
        return [89, 77, CONTENT_W - 166]
    if headers[0] == "Bytes hexadecimales":
        return [126, 126, CONTENT_W - 252]
    return [119, 92, CONTENT_W - 211]


def build_table(rows, table_head, table_body):
    widths = table_widths(rows)
    data = []
    for row_idx, row in enumerate(rows):
        style = table_head if row_idx == 0 else table_body
        data.append([Paragraph(inline_markup(cell), style) for cell in row])
    table = Table(data, colWidths=widths, repeatRows=1, hAlign="LEFT", splitByRow=1)
    table.setStyle(TableStyle([
        ("VALIGN", (0, 0), (-1, -1), "TOP"),
        ("LEFTPADDING", (0, 0), (-1, -1), 7),
        ("RIGHTPADDING", (0, 0), (-1, -1), 7),
        ("TOPPADDING", (0, 0), (-1, -1), 5),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 5),
        ("LINEABOVE", (0, 0), (-1, 0), 0.9, INK),
        ("LINEBELOW", (0, 0), (-1, 0), 0.9, INK),
        ("LINEBELOW", (0, 1), (-1, -1), 0.3, RULE),
    ]))
    return table


def make_vector_pdf() -> bytes:
    register_fonts()
    body = ParagraphStyle("body", fontName=face(), fontSize=9.2, leading=14.1,
                          textColor=INK, spaceAfter=10, alignment=TA_LEFT)
    table_head = ParagraphStyle("table-head", parent=body, fontName=face(True),
                                fontSize=8.2, leading=11, spaceAfter=0, textColor=INK)
    table_body = ParagraphStyle("table-body", parent=body, fontSize=8.2,
                                leading=11.5, spaceAfter=0)
    story = []
    photo_added = False
    paragraph_count = 0
    for kind, content in parse_blocks(SOURCE.read_text(encoding="utf-8")):
        if kind == "title":
            continue  # The first-page masthead is drawn in page_art.
        if kind == "section":
            if PAGE_H > PAGE_W and content.lower().startswith("los bolsillos"):
                story.append(PageBreak())
            story.append(TypewrittenHeading(content))
        elif kind == "subsection":
            story.append(TypewrittenHeading(content, small=True))
        elif kind == "paragraph":
            story.append(WornParagraph(inline_markup(content, jitter=True), body))
            paragraph_count += 1
            if paragraph_count == 4 and not photo_added and PHOTO.exists():
                story.extend([Spacer(1, 5), EvidencePhoto(PHOTO), Spacer(1, 11)])
                photo_added = True
        elif kind == "table":
            story.append(Spacer(1, 4))
            story.append(build_table(content, table_head, table_body))
            story.append(Spacer(1, 9))

    story.extend([NextPageTemplate("notes"), PageBreak(), FieldNotes()])

    vector = io.BytesIO()
    doc = BaseDocTemplate(vector, pagesize=(PAGE_W, PAGE_H), leftMargin=MARGIN_X,
                          rightMargin=MARGIN_X, topMargin=0, bottomMargin=0,
                          title="Expediente 8080", author="gzalo",
                          subject="Guía de campo para la fiesta hacker")
    first_frame = Frame(MARGIN_X, 63, CONTENT_W, PAGE_H - 213,
                        leftPadding=0, rightPadding=0, topPadding=0, bottomPadding=0)
    later_frame = Frame(MARGIN_X, 63, CONTENT_W, PAGE_H - 128,
                        leftPadding=0, rightPadding=0, topPadding=0, bottomPadding=0)
    doc.addPageTemplates([
        PageTemplate(id="first", frames=[first_frame],
                     onPage=page_art, autoNextPageTemplate="later"),
        PageTemplate(id="later", frames=[later_frame],
                     onPage=page_art),
        PageTemplate(id="notes",
                     frames=[Frame(MARGIN_X, 63, CONTENT_W, PAGE_H - 128,
                                   leftPadding=0, rightPadding=0,
                                   topPadding=0, bottomPadding=0)],
                     onPage=page_art),
    ])
    doc.build(story)
    return vector.getvalue()


def composite_wear(page_image: Image.Image, page_number: int,
                   notes_page: bool = False) -> Image.Image:
    """Place real scanned blemishes before raster fax degradation."""
    base = page_image.convert("RGBA")
    width, height = base.size

    def add_asset(filename: str, box: tuple[int, int, int, int],
                  opacity: float, rotation: int = 0, flip: bool = False) -> None:
        path = WEAR_ASSETS / filename
        if not path.exists():
            raise FileNotFoundError(f"Missing paper-wear asset: {path}")
        overlay = Image.open(path).convert("RGBA")
        if flip:
            overlay = ImageOps.mirror(overlay)
        if rotation:
            overlay = overlay.rotate(rotation, expand=False)
        x, y, image_width, image_height = box
        overlay = overlay.resize((image_width, image_height), Image.Resampling.LANCZOS)
        gray = ImageOps.grayscale(overlay)
        alpha = overlay.getchannel("A").point(lambda value: round(value * opacity))
        overlay = Image.merge("RGBA", (gray, gray, gray, alpha))
        left, top = max(0, x), max(0, y)
        right, bottom = min(width, x + image_width), min(height, y + image_height)
        if right > left and bottom > top:
            cropped = overlay.crop((left - x, top - y, right - x, bottom - y))
            base.alpha_composite(cropped, (left, top))

    # Foxing stays near the page perimeter; alternate its orientation by folio.
    add_asset("foxing-smudges.png", (0, 0, width, height),
              opacity=0.10 if notes_page else 0.12,
              rotation=180 if page_number % 3 == 0 else 0,
              flip=page_number % 2 == 0)
    ring_size = round(width * 0.36)
    if page_number == 1:
        add_asset("coffee-ring.png",
                  (width - round(ring_size * 0.46), round(height * 0.51),
                   ring_size, ring_size), opacity=0.21, rotation=20)
    elif notes_page:
        add_asset("coffee-ring.png",
                  (-round(ring_size * 0.56), round(height * 0.83),
                   ring_size, ring_size), opacity=0.16, rotation=95)
    elif page_number == 4:
        add_asset("coffee-ring.png",
                  (width - round(ring_size * 0.82), round(height * 0.57),
                   ring_size, ring_size), opacity=0.33, rotation=165)

    fold_width = round(width * 0.69)
    fold_height = round(fold_width / 3)
    if page_number in (2, 3):
        add_asset("fold-crease.png",
                  (width - round(fold_width * 0.70), height - fold_height - 30,
                   fold_width, fold_height), opacity=0.12,
                  flip=page_number == 3)
    return base.convert("RGB")


def fax_filter(page_image: Image.Image, page_number: int,
               paper_wear: bool = True, notes_page: bool = False) -> Image.Image:
    """Apply one deterministic scan/photocopy pass to the *whole* page."""
    rng = random.Random(8080 + page_number)
    aged = (composite_wear(page_image, page_number, notes_page)
            if paper_wear else page_image)
    gray = ImageEnhance.Contrast(aged.convert("L")).enhance(1.16)
    gray = gray.point(lambda v: 0 if v < 58 else 42 if v < 116 else
                      105 if v < 166 else 182 if v < 219 else 235 if v < 246 else 255)
    # A one-pixel gray echo around type and rules resembles a copied fax.
    ghost = gray.point(lambda v: 230 if v < 125 else 255)
    gray = ImageChops.darker(gray, ImageChops.offset(ghost, 1, 1))
    width, height = gray.size
    shifted = Image.new("L", (width, height), 255)
    y = 0
    while y < height:
        band_height = rng.randrange(17, 56)
        offset = rng.choices((-2, -1, 0, 1, 2), weights=(1, 4, 20, 4, 1))[0]
        shifted.paste(gray.crop((0, y, width, min(height, y + band_height))),
                      (offset, y))
        y += band_height
    draw = ImageDraw.Draw(shifted)
    for _ in range(max(9, height // 170)):
        y = rng.randrange(40, height - 40)
        x = rng.randrange(35, width // 2)
        draw.line((x, y, min(width - 35, x + rng.randrange(width // 3, width)), y),
                  fill=rng.choice((215, 232, 242)), width=1)
    # Sparse toner grains and pinhole dropouts retain readable letter cores.
    for _ in range(width * height // 1900):
        x, y = rng.randrange(25, width - 25), rng.randrange(25, height - 25)
        if shifted.getpixel((x, y)) < 90:
            draw.point((x, y), fill=255)
        elif rng.random() < 0.28:
            draw.point((x, y), fill=rng.choice((140, 195, 225)))
    return shifted


def make_pdf(output: Path, dpi: int) -> None:
    vector = make_vector_pdf()
    source_pdf = pdfium.PdfDocument(vector)
    output.parent.mkdir(parents=True, exist_ok=True)
    final = canvas.Canvas(str(output), pagesize=(PAGE_W, PAGE_H), pageCompression=1,
                          invariant=1)
    final.setTitle("Expediente 8080 - copia de fax")
    final.setAuthor("gzalo")
    for index in range(len(source_pdf)):
        rendered = source_pdf[index].render(scale=dpi / 72).to_pil()
        processed = fax_filter(rendered, index + 1,
                               notes_page=index == len(source_pdf) - 1)
        jpeg = io.BytesIO()
        processed.save(jpeg, format="JPEG", quality=88, optimize=True)
        jpeg.seek(0)
        final.drawImage(ImageReader(jpeg), 0, 0, width=PAGE_W, height=PAGE_H)
        final.showPage()
    final.save()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--landscape", action="store_true",
                        help="Render an A4 landscape variant")
    parser.add_argument("--output", type=Path,
                        help="PDF path (defaults to the selected orientation's output)")
    parser.add_argument("--dpi", type=int, default=220,
                        help="Raster resolution for the image-only PDF (default: 220)")
    args = parser.parse_args()
    if not 150 <= args.dpi <= 600:
        parser.error("--dpi must be between 150 and 600")
    set_page_orientation(args.landscape)
    output = args.output or (LANDSCAPE_OUTPUT if args.landscape else DEFAULT_OUTPUT)
    make_pdf(output.resolve(), args.dpi)
    print(output.resolve())


if __name__ == "__main__":
    main()

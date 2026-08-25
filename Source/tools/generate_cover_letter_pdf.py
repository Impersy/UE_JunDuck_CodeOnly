from __future__ import annotations

from pathlib import Path

from docx import Document
from reportlab.lib.colors import HexColor
from reportlab.lib.enums import TA_CENTER, TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import Paragraph, SimpleDocTemplate, Spacer


ROOT = Path(__file__).resolve().parents[1]
SOURCE_DOCX = ROOT / "output" / "docx" / "LeeJongHyuk_CoverLetter_Updated.docx"
OUT_DIR = ROOT / "output" / "pdf"
OUT_PATH = OUT_DIR / "LeeJongHyuk_CoverLetter_Updated.pdf"

FONT_REGULAR = Path(r"C:\Windows\Fonts\malgun.ttf")
FONT_BOLD = Path(r"C:\Windows\Fonts\malgunbd.ttf")


def register_fonts() -> None:
    pdfmetrics.registerFont(TTFont("Malgun", str(FONT_REGULAR)))
    pdfmetrics.registerFont(TTFont("Malgun-Bold", str(FONT_BOLD)))


def read_docx_text() -> tuple[str, list[str]]:
    doc = Document(SOURCE_DOCX)
    texts = [paragraph.text.strip() for paragraph in doc.paragraphs if paragraph.text.strip()]
    if not texts:
        raise RuntimeError("No text found in source DOCX.")
    return texts[0], texts[1:]


def build_pdf() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    register_fonts()

    title, paragraphs = read_docx_text()

    pdf = SimpleDocTemplate(
        str(OUT_PATH),
        pagesize=A4,
        leftMargin=21 * mm,
        rightMargin=21 * mm,
        topMargin=16 * mm,
        bottomMargin=16 * mm,
        title="자기소개서",
        author="Lee Jong Hyuk",
    )

    title_style = ParagraphStyle(
        "CoverLetterTitle",
        fontName="Malgun-Bold",
        fontSize=18,
        leading=25,
        alignment=TA_CENTER,
        textColor=HexColor("#111827"),
        spaceAfter=8 * mm,
    )

    body_style = ParagraphStyle(
        "CoverLetterBody",
        fontName="Malgun",
        fontSize=9.9,
        leading=15.7,
        alignment=TA_LEFT,
        firstLineIndent=10,
        textColor=HexColor("#111827"),
        spaceAfter=4.0,
    )

    story = [Paragraph(title, title_style)]
    for text in paragraphs:
        story.append(Paragraph(text, body_style))
        story.append(Spacer(1, 0.7))

    pdf.build(story)
    print(OUT_PATH)


if __name__ == "__main__":
    build_pdf()

from __future__ import annotations

import os
from dataclasses import dataclass
import hashlib
from pathlib import Path
from typing import Iterable, Sequence

from PIL import Image
from reportlab.lib import colors
from reportlab.lib.pagesizes import landscape
from reportlab.lib.units import inch
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.pdfgen import canvas


ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "output" / "pdf"
TMP_DIR = ROOT / "tmp" / "pdfs"
PDF_PATH = OUT_DIR / "JunDuck_Combat_Portfolio.pdf"
COMPRESSED_IMAGE_DIR = TMP_DIR / "compressed_images"
PDF_IMAGE_MAX_DIMENSION = 1700
PDF_IMAGE_JPEG_QUALITY = 82

CARD_IMAGE_DIR = Path(r"C:\Users\USER\Desktop\portfolio\이미지")
CARD_IMAGES = [
    CARD_IMAGE_DIR / "1.png",
    CARD_IMAGE_DIR / "2.png",
    CARD_IMAGE_DIR / "3.png",
    CARD_IMAGE_DIR / "4.png",
    CARD_IMAGE_DIR / "5.png",
]

MALGUN = Path(r"C:\Windows\Fonts\malgun.ttf")
MALGUN_BOLD = Path(r"C:\Windows\Fonts\malgunbd.ttf")

PAGE_W = 1200
PAGE_H = 900

COL_BG = colors.HexColor("#0b0f14")
COL_PANEL = colors.HexColor("#121821")
COL_PANEL_2 = colors.HexColor("#171d26")
COL_LINE = colors.HexColor("#2a3442")
COL_TEXT = colors.HexColor("#e7edf4")
COL_MUTED = colors.HexColor("#9aa8b8")
COL_DIM = colors.HexColor("#6f7d8d")
COL_BLUE = colors.HexColor("#58a6ff")
COL_CYAN = colors.HexColor("#2dd4bf")
COL_GREEN = colors.HexColor("#7ee787")
COL_PURPLE = colors.HexColor("#b18cff")
COL_RED = colors.HexColor("#ff7b72")
COL_YELLOW = colors.HexColor("#ffd166")


@dataclass(frozen=True)
class Chapter:
    number: str
    title: str
    subtitle: str
    image: Path
    points: Sequence[str]
    problem: str
    solution: str
    result: str


def setup_fonts() -> None:
    pdfmetrics.registerFont(TTFont("Malgun", str(MALGUN)))
    pdfmetrics.registerFont(TTFont("MalgunBold", str(MALGUN_BOLD)))


def draw_bg(c: canvas.Canvas) -> None:
    c.setFillColor(COL_BG)
    c.rect(0, 0, PAGE_W, PAGE_H, fill=1, stroke=0)


def text_width(text: str, font: str, size: float) -> float:
    return pdfmetrics.stringWidth(text, font, size)


def wrap_text(text: str, font: str, size: float, max_width: float) -> list[str]:
    lines: list[str] = []
    for para in text.split("\n"):
        if not para.strip():
            lines.append("")
            continue
        current = ""
        for token in para.split(" "):
            candidate = token if not current else f"{current} {token}"
            if text_width(candidate, font, size) <= max_width:
                current = candidate
                continue

            if current:
                lines.append(current)
                current = token
            else:
                chunk = ""
                for ch in token:
                    candidate = chunk + ch
                    if text_width(candidate, font, size) <= max_width:
                        chunk = candidate
                    else:
                        if chunk:
                            lines.append(chunk)
                        chunk = ch
                current = chunk
        if current:
            lines.append(current)
    return lines


def draw_wrapped(
    c: canvas.Canvas,
    text: str,
    x: float,
    y: float,
    max_width: float,
    font: str = "Malgun",
    size: float = 16,
    leading: float | None = None,
    color=COL_TEXT,
) -> float:
    c.setFillColor(color)
    c.setFont(font, size)
    if leading is None:
        leading = size * 1.55
    for line in wrap_text(text, font, size, max_width):
        c.drawString(x, y, line)
        y -= leading
    return y


def draw_title(c: canvas.Canvas, title: str, subtitle: str | None = None) -> None:
    c.setFillColor(COL_BLUE)
    c.setFont("MalgunBold", 18)
    c.drawString(60, PAGE_H - 72, "JunDuck Combat Architecture")
    c.setFillColor(COL_TEXT)
    c.setFont("MalgunBold", 38)
    c.drawString(60, PAGE_H - 130, title)
    if subtitle:
        c.setFillColor(COL_MUTED)
        c.setFont("Malgun", 18)
        c.drawString(62, PAGE_H - 166, subtitle)


def draw_footer(c: canvas.Canvas, page_no: int) -> None:
    c.setStrokeColor(COL_LINE)
    c.line(60, 42, PAGE_W - 60, 42)
    c.setFillColor(COL_DIM)
    c.setFont("Malgun", 11)
    c.drawString(60, 24, "UE5 · C++ · Action Combat Portfolio")
    c.drawRightString(PAGE_W - 60, 24, f"{page_no:02d}")


def rounded_panel(c: canvas.Canvas, x: float, y: float, w: float, h: float, fill=COL_PANEL, stroke=COL_LINE, radius=16) -> None:
    c.setFillColor(fill)
    c.setStrokeColor(stroke)
    c.roundRect(x, y, w, h, radius, fill=1, stroke=1)


def draw_chip(c: canvas.Canvas, x: float, y: float, text: str, fill, stroke=None, font_size=13) -> float:
    if stroke is None:
        stroke = fill
    pad_x = 15
    w = text_width(text, "MalgunBold", font_size) + pad_x * 2
    c.setFillColor(fill)
    c.setStrokeColor(stroke)
    c.roundRect(x, y, w, 32, 8, fill=1, stroke=1)
    c.setFillColor(COL_TEXT)
    c.setFont("MalgunBold", font_size)
    c.drawCentredString(x + w / 2, y + 9, text)
    return x + w + 10


def draw_flow(
    c: canvas.Canvas,
    x: float,
    y: float,
    labels: Sequence[str],
    colors_: Sequence,
    box_w: float = 225,
    box_h: float = 42,
    gap: float = 18,
    font_size: float = 13,
) -> None:
    for i, label in enumerate(labels):
        rounded_panel(c, x, y - i * (box_h + gap), box_w, box_h, fill=colors_[i], stroke=colors_[i], radius=8)
        c.setFillColor(COL_TEXT)
        c.setFont("MalgunBold", font_size)
        c.drawCentredString(x + box_w / 2, y - i * (box_h + gap) + (box_h - font_size) / 2 - 1, label)
        if i < len(labels) - 1:
            c.setFillColor(COL_DIM)
            c.setFont("MalgunBold", 14)
            arrow_y = y - i * (box_h + gap) - (gap / 2) - 5
            c.drawCentredString(x + box_w / 2, arrow_y, "↓")


def image_for_pdf(path: Path) -> Path:
    """Create a compressed JPEG copy for PDF embedding without touching source images."""
    COMPRESSED_IMAGE_DIR.mkdir(parents=True, exist_ok=True)
    key = hashlib.sha1(str(path.resolve()).encode("utf-8")).hexdigest()[:16]
    out_path = COMPRESSED_IMAGE_DIR / f"{path.stem}_{key}.jpg"
    if out_path.exists() and out_path.stat().st_mtime >= path.stat().st_mtime:
        return out_path

    with Image.open(path) as img:
        img = img.convert("RGB")
        img.thumbnail((PDF_IMAGE_MAX_DIMENSION, PDF_IMAGE_MAX_DIMENSION), Image.Resampling.LANCZOS)
        img.save(out_path, "JPEG", quality=PDF_IMAGE_JPEG_QUALITY, optimize=True, progressive=True)
    return out_path


def draw_image_fit(c: canvas.Canvas, path: Path, x: float, y: float, w: float, h: float) -> None:
    pdf_path = image_for_pdf(path)
    with Image.open(pdf_path) as img:
        iw, ih = img.size
    scale = min(w / iw, h / ih)
    dw, dh = iw * scale, ih * scale
    c.drawImage(str(pdf_path), x + (w - dw) / 2, y + (h - dh) / 2, dw, dh, preserveAspectRatio=True, mask="auto")


def new_page(c: canvas.Canvas, page_no: int, title: str, subtitle: str | None = None) -> int:
    if page_no > 0:
        c.showPage()
    draw_bg(c)
    draw_title(c, title, subtitle)
    draw_footer(c, page_no + 1)
    return page_no + 1


def page_cover(c: canvas.Canvas, page_no: int) -> int:
    draw_bg(c)
    c.setFillColor(COL_BLUE)
    c.setFont("MalgunBold", 18)
    c.drawString(70, PAGE_H - 95, "JunDuck")
    c.setFillColor(COL_TEXT)
    c.setFont("MalgunBold", 54)
    c.drawString(70, PAGE_H - 175, "Action Combat")
    c.drawString(70, PAGE_H - 238, "Architecture Portfolio")
    c.setFillColor(COL_MUTED)
    c.setFont("Malgun", 20)
    c.drawString(74, PAGE_H - 292, "Sekiro-like 3rd Person Action Boss Battle · UE5 C++")

    meta_items = [("Development", "Solo"), ("Period", "2 Months"), ("Engine", "UE5 C++")]
    x = 74
    for label, value in meta_items:
        c.setFillColor(COL_DIM)
        c.setFont("Malgun", 10)
        c.drawString(x, PAGE_H - 334, label)
        c.setFillColor(COL_TEXT)
        c.setFont("MalgunBold", 15)
        c.drawString(x, PAGE_H - 356, value)
        x += 142

    c.setFillColor(COL_BLUE)
    c.setFont("MalgunBold", 10.5)
    c.drawString(330, PAGE_H - 382, "Demo Video")
    c.setFillColor(COL_MUTED)
    c.setFont("Malgun", 11.5)
    c.drawString(330, PAGE_H - 402, "YouTube link will be added after the demo video is ready.")

    rounded_panel(c, 70, 125, 510, 265, fill=COL_PANEL)
    c.setFillColor(COL_YELLOW)
    c.setFont("MalgunBold", 15)
    c.drawString(100, 350, "Project Goal")
    summary = (
        "개별 공격을 하드코딩하는 방식이 아니라, 플레이어 입력부터 공격 판정, "
        "방어/패링, 보스 공격 선택, 처형 결과까지 재사용 가능한 액션 전투 구조로 묶는 것을 목표로 했습니다."
    )
    draw_wrapped(c, summary, 100, 316, 450, size=16, leading=28, color=COL_TEXT)

    rounded_panel(c, 685, 125, 390, 560, fill=COL_PANEL_2)
    c.setFillColor(COL_BLUE)
    c.setFont("MalgunBold", 16)
    c.drawString(720, 645, "Combat Flow")
    draw_flow(
        c,
        780,
        585,
        [
            "Player Input",
            "Action FSM",
            "Cancel / Buffer Policy",
            "Attack / Defense / Counter",
            "Common Attack Trace",
            "Target Reaction",
            "VFX / SFX / Time Effect / UI",
        ],
        [
            colors.HexColor("#12304a"),
            colors.HexColor("#12304a"),
            colors.HexColor("#281f45"),
            colors.HexColor("#451b21"),
            colors.HexColor("#2a2144"),
            colors.HexColor("#17352c"),
            colors.HexColor("#1d2937"),
        ],
        box_w=200,
    )
    draw_footer(c, 1)
    return 1


def page_overview(c: canvas.Canvas, page_no: int) -> int:
    page_no = new_page(c, page_no, "Project Overview", "전투 시스템을 설명하기 위한 5개 핵심 챕터와 부록 구성")
    left_x = 70
    y = PAGE_H - 270
    overview = [
        ("01", "Player Action FSM + Cancel / Buffer", "입력을 즉시 실행하지 않고 요청으로 변환한 뒤, 상태와 캔슬 정책을 통과시킵니다."),
        ("02", "Defense / Parry System", "우클릭 하나를 탭, 홀드, 체인 패리, 공중 패리, 미키리 준비로 해석합니다."),
        ("03", "Boss Combat FSM + Attack Link", "거리, 페이즈, 쿨타임, 확률을 기반으로 공격을 고르고 Notify에서 다음 공격을 링크합니다."),
        ("04", "Common Attack Trace + Hit Reaction", "플레이어, 보스, NPC가 같은 공격 요청과 피격 인터페이스를 공유합니다."),
        ("05", "Reusable Execution System", "처형하는 쪽과 당하는 쪽을 인터페이스로 분리해 보스와 NPC가 다른 결과를 처리합니다."),
    ]
    for no, title, desc in overview:
        rounded_panel(c, left_x, y - 7, 1060, 76, fill=COL_PANEL)
        c.setFillColor(COL_BLUE)
        c.setFont("MalgunBold", 18)
        c.drawString(left_x + 24, y + 23, no)
        c.setFillColor(COL_TEXT)
        c.setFont("MalgunBold", 18)
        c.drawString(left_x + 82, y + 27, title)
        draw_wrapped(c, desc, left_x + 82, y + 2, 910, size=13.5, leading=20, color=COL_MUTED)
        y -= 96

    rounded_panel(c, 70, 74, 1060, 110, fill=colors.HexColor("#111b24"))
    c.setFillColor(COL_YELLOW)
    c.setFont("MalgunBold", 14)
    c.drawString(100, 154, "Appendix Direction")

    appendix_groups = [
        ("전투 / 판정", "Special Counter", colors.HexColor("#2a2144")),
        ("연출 / 피드백", "VFX · TimeEffect · BGM", colors.HexColor("#17352c")),
        ("플레이 보조", "Lock-On Camera · Tutorial · Potion · Monster HUD · Niagara Trail · Foot IK", colors.HexColor("#12304a")),
    ]
    gx = 100
    gy = 92
    group_widths = [245, 285, 455]
    for index, (label, body, fill) in enumerate(appendix_groups):
        gw = group_widths[index]
        rounded_panel(c, gx, gy, gw, 44, fill=fill, stroke=colors.HexColor("#334155"), radius=8)
        c.setFillColor(COL_TEXT)
        c.setFont("MalgunBold", 11.5)
        c.drawString(gx + 14, gy + 25, label)
        c.setFillColor(COL_MUTED)
        c.setFont("Malgun", 10.8)
        c.drawString(gx + 14, gy + 10, body)
        gx += gw + 18
    return page_no


def page_chapter_image(c: canvas.Canvas, page_no: int, chapter: Chapter) -> int:
    page_no = new_page(c, page_no, f"{chapter.number}. {chapter.title}", chapter.subtitle)
    psr = [
        ("Problem", chapter.problem, COL_RED),
        ("Solution", chapter.solution, COL_BLUE),
        ("Result", chapter.result, COL_GREEN),
    ]
    x = 60
    y = 525
    w = 300
    h = 170
    for label, body, accent in psr:
        rounded_panel(c, x, y, w, h, fill=COL_PANEL, radius=12)
        c.setFillColor(accent)
        c.setFont("MalgunBold", 13)
        c.drawString(x + 20, y + h - 32, label)
        draw_wrapped(c, body, x + 20, y + h - 58, w - 40, size=11.2, leading=16, color=COL_MUTED)
        y -= h + 18

    rounded_panel(c, 390, 62, 760, 645, fill=colors.HexColor("#0f141b"), radius=14)
    draw_image_fit(c, chapter.image, 405, 78, 730, 613)
    return page_no


def page_appendix_special_counter(c: canvas.Canvas, page_no: int) -> int:
    page_no = new_page(c, page_no, "Appendix A. Special Counter System", "Mikiri와 Jump Counter는 공격자와 대응자의 세트 플레이로 구성")
    rounded_panel(c, 70, 145, 500, 545, fill=COL_PANEL)
    rounded_panel(c, 630, 145, 500, 545, fill=COL_PANEL)
    c.setFillColor(COL_PURPLE)
    c.setFont("MalgunBold", 22)
    c.drawString(110, 640, "Mikiri Counter")
    c.drawString(670, 640, "Jump Counter")
    mikiri = [
        "Thrust Danger Attack",
        "Danger Marker 표시",
        "Mikiri Command Window Notify",
        "Dash + Right Click",
        "Mikiri Ready Window",
        "Attack Collision",
        "Player Success Montage",
        "Attacker Countered React",
    ]
    stomp = [
        "Low Sweep Danger Attack",
        "Danger Marker 표시",
        "Jump로 회피 판정",
        "Stomp Chance 부여",
        "위치 보정",
        "Bounce Notify",
        "Posture Damage",
        "후속 행동 가능",
    ]
    draw_flow(c, 205, 575, mikiri, [colors.HexColor("#281f45")] * len(mikiri), box_h=38, gap=13, font_size=12.2)
    draw_flow(c, 765, 575, stomp, [colors.HexColor("#17352c")] * len(stomp), box_h=38, gap=13, font_size=12.2)
    body = (
        "위험공격은 일반 방어나 무적 회피로 끝내지 않고, 공격 타입에 맞는 대응을 요구하도록 분리했습니다. "
        "Mikiri는 공격자의 가능 구간과 플레이어의 입력 윈도우가 동시에 맞아야 성공하며, "
        "\nJump Counter는 하단 공격을 점프로 피한 뒤 밟기 후속 행동으로 이어집니다. "
        "보스와 TutorialNPC가 같은 인터페이스 기반 대응 구조를 재사용합니다."
    )
    rounded_panel(c, 70, 60, 1060, 58, fill=colors.HexColor("#111b24"))
    draw_wrapped(c, body, 95, 94, 1010, size=11.8, leading=16, color=COL_MUTED)
    return page_no


def page_appendix_cards(c: canvas.Canvas, page_no: int, title: str, subtitle: str, cards: Sequence[tuple[str, str]]) -> int:
    page_no = new_page(c, page_no, title, subtitle)
    x_positions = [70, 430, 790]
    y_positions = [510, 300, 90]
    idx = 0
    for y in y_positions:
        for x in x_positions:
            if idx >= len(cards):
                break
            card_title, desc = cards[idx]
            rounded_panel(c, x, y, 310, 170, fill=COL_PANEL)
            c.setFillColor(COL_BLUE if idx % 3 == 0 else COL_CYAN if idx % 3 == 1 else COL_PURPLE)
            c.setFont("MalgunBold", 15)
            c.drawString(x + 22, y + 130, card_title)
            draw_wrapped(c, desc, x + 22, y + 100, 266, size=11.6, leading=17, color=COL_MUTED)
            idx += 1
    return page_no


def build_pdf() -> None:
    setup_fonts()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    TMP_DIR.mkdir(parents=True, exist_ok=True)

    chapters = [
        Chapter(
            "01",
            "Player Action FSM / Cancel Buffer",
            "입력 요청, 캔슬 정책, 입력 버퍼를 중앙 파이프라인으로 관리",
            CARD_IMAGES[0],
            [
                "Input Handler는 액션을 직접 실행하지 않고 Action Request로 변환합니다.",
                "현재 ActionState, LocomotionState, DefenseState를 보고 즉시 실행 또는 PendingAction으로 분기합니다.",
                "CancelRule의 OpenTime과 BlendOutTime으로 후딜 캔슬과 몽타주 전환을 통제합니다.",
            ],
            "액션이 늘어날수록 입력, 캔슬, 후속 버퍼가 함수 곳곳에 흩어져 조작감 튜닝이 어려웠습니다.",
            "모든 입력을 ActionRequest로 변환하고, 현재 상태와 CancelRule을 통과한 요청만 실행하게 했습니다.",
            "공격, 회피, 점프, 패링 후속 행동을 같은 흐름에서 처리해 액션 추가와 튜닝 부담을 줄였습니다.",
        ),
        Chapter(
            "02",
            "Defense / Parry System",
            "우클릭 입력을 현재 상태와 타이밍에 따라 패링, 가드, 체인 패리로 해석",
            CARD_IMAGES[1],
            [
                "짧은 입력은 Parry Window, 유지 입력은 Guard Hold 의도로 처리합니다.",
                "공격 충돌 시 방향, 공격 타입, 윈도우 시간을 검사해 PerfectParry, NormalParry, GuardHit로 분기합니다.",
                "패링 성공 중 재입력, 피격 캔슬, 공중 패링까지 같은 방어 파이프라인 안에서 다룹니다.",
            ],
            "같은 우클릭 입력이 상황에 따라 패링, 가드, 체인 패리, 피격 캔슬로 달라져 입력 해석이 꼬이기 쉬웠습니다.",
            "DefenseComponent가 입력 유지 시간, 현재 상태, 패링 윈도우, 공격 방향과 타입을 함께 판단합니다.",
            "탭/홀드/체인/공중 방어가 안정적으로 분기되어 타이밍 기반 공방 흐름을 만들 수 있었습니다.",
        ),
        Chapter(
            "03",
            "Boss FSM / Attack Link",
            "상태 전환, 공격 후보 선정, 몽타주 Notify 기반 링크로 보스 전투 흐름 구성",
            CARD_IMAGES[2],
            [
                "공격 선택은 거리, 페이즈, 쿨타임, 변형 패턴 조건을 통과한 후보에서 가중치로 결정합니다.",
                "몽타주 내부 Notify가 CodeMove, Facing, AttackTrace, Link 시점을 열어준다.",
                "NormalAttackLink는 다음 공격을 재평가하고, 조건 실패 시 Approach나 Reposition으로 복귀합니다.",
            ],
            "보스를 고정 콤보나 단순 랜덤으로 만들면 거리, 페이즈, 쿨타임에 맞춘 공방 흐름을 만들기 어려웠습니다.",
            "Combat FSM과 AttackSelectionStrategy로 후보를 필터링하고, Notify 시점에서 다음 공격을 링크했습니다.",
            "보스가 접근, 재배치, 공격, 연계를 상황에 맞게 선택해 전투가 덜 반복적으로 느껴지게 했습니다.",
        ),
        Chapter(
            "04",
            "Common Attack Trace / Hit Reaction",
            "공격자는 공통 HitRequest만 만들고, 피격자는 인터페이스로 자기 반응을 선택",
            CARD_IMAGES[3],
            [
                "AttackTrace NotifyState가 무기, 킥, 투사체의 Trace 구간을 열고 중복 Hit를 방지합니다.",
                "HitRequest에는 Damage, PostureDamage, HitReactType, DangerAttackType, DefenseRule이 포함됩니다.",
                "피격자는 Parry, Guard, Dodge, Damage, HitReact, SuperArmor PhysicalHit를 자기 상태에 맞게 해석합니다.",
            ],
            "플레이어, 보스, NPC, 투사체 공격을 따로 처리하면 공격 타입과 피격 리액션 코드가 계속 중복됐습니다.",
            "공격자는 공통 HitRequest를 만들고, 피격자는 인터페이스로 받아 자신의 상태에 맞게 해석하게 했습니다.",
            "새 공격이나 새 몬스터도 같은 판정 파이프라인에 연결할 수 있어 재사용성을 높였습니다.",
        ),
        Chapter(
            "05",
            "Reusable Execution System",
            "처형 실행자와 처형 대상의 결과 처리를 인터페이스로 분리",
            CARD_IMAGES[4],
            [
                "플레이어는 처형 가능한 대상을 찾고 ExecutionComponent를 통해 실행 흐름만 담당합니다.",
                "Boss와 TutorialNPC는 IJunExecutionTargetInterface를 구현해 서로 다른 처형 결과를 처리합니다.",
                "Boss는 Life 감소와 페이즈 전환, NPC는 죽지 않고 회복과 튜토리얼 완료로 이어집니다.",
            ],
            "처형을 보스 전용으로 만들면 TutorialNPC나 일반 몬스터에 재사용하기 어렵고 의존성이 커졌습니다.",
            "플레이어는 처형 실행만 담당하고, 대상은 ExecutionTarget 인터페이스로 자기 처형 결과를 처리합니다.",
            "같은 처형 입력을 공유하면서도 보스는 Life 감소, NPC는 회복/Task 완료처럼 다른 결과를 낼 수 있습니다.",
        ),
    ]

    c = canvas.Canvas(str(PDF_PATH), pagesize=(PAGE_W, PAGE_H))
    c.setTitle("JunDuck Combat Architecture Portfolio")
    c.setAuthor("JunDuck")
    page_no = page_cover(c, 0)
    page_no = page_overview(c, page_no)
    for chapter in chapters:
        page_no = page_chapter_image(c, page_no, chapter)
    page_no = page_appendix_special_counter(c, page_no)

    appendix_systems = [
        ("VFX / SFX Subsystem", "Parry, GuardHit, GuardBreak, Blood, HitStop 요청을 캐릭터 코드에서 분리하고 Subsystem이 Niagara와 사운드를 처리합니다."),
        ("Time Effect Subsystem", "HitStop과 SlowMotion을 전역/부분 타임 효과로 관리해 처형, 피격, 공격 타격감을 독립적으로 조정합니다."),
        ("Lock-On Camera", "거리 기반 Pitch와 SpringArm Length, 히스테리시스, 처형/대화/죽음 전용 카메라 값을 별도로 두었습니다."),
        ("Tutorial NPC / Task", "대화, 암전 이동, 단계별 학습 Task, 더미 모드와 공격 모드를 가진 튜토리얼 NPC를 구현했습니다."),
        ("Potion System", "충전형 물약, 점진 회복, 무기 숨김/복구, 튜토리얼 회복 완료 이벤트를 PotionComponent로 분리했습니다."),
        ("BGM Manager", "맵 BGM, 전투 BGM, 처형/죽음 특수 상황의 볼륨 덕킹과 페이드 전환을 관리합니다."),
        ("Monster HUD", "보스가 아닌 몬스터/NPC용 머리 위 체력/체간/Life UI를 WidgetComponent 기반으로 구현했습니다."),
        ("Weapon Niagara", "Trail, Aura, Jigen, BloodTrail 이펙트를 Notify에서 선택해 켜고 끄도록 공통화했습니다."),
        ("Foot Placement IK", "플레이어, 보스, NPC의 경사면 접지를 위해 AnimInstance에서 FootPlacementAlpha를 상황별로 제어합니다."),
    ]
    page_no = page_appendix_cards(c, page_no, "Appendix B. Supporting Systems", "본문 5개 챕터 밖에 있는 구현 기능들", appendix_systems)

    appendix_gameplay = [
        ("Hit Stop / Combat Feedback", "피격, 공격 성공, 처형 상황에 따라 HitStop, SlowMotion, CameraShake, SFX/VFX를 분리해 전투 피드백을 조정했습니다."),
        ("Physical Hit Reaction", "라이트/슈퍼아머 피격에서 몽타주 위에 PhysicalAnimation을 얹어 타격감을 보강합니다."),
        ("Jump Attack / Air Reaction", "공중 피격, 공중 데스, 점프 패링, 점프 공격을 별도 리액션과 입력 흐름으로 분리했습니다."),
        ("Boss Parry Cycle", "보스의 일반/완벽 패리 확률, 공방 탈출, 카운터, 공격권 전환 흐름을 별도 로직으로 관리합니다."),
        ("Facing / Turn", "공격, 회피, 피격, 턴 몽타주별로 코드 Facing과 Notify 기반 Facing을 분리해 회전 튐을 줄였습니다."),
        ("Attack Trace Overrides", "몽타주 Notify에서 Trace End 연장/축소, Radius, SampleCount, 패링/가드 불가 여부를 설정합니다."),
        ("Arrow / Projectile", "화살과 번개 검기처럼 Trace가 아닌 투사체 공격도 같은 HitRequest 흐름으로 연결합니다."),
        ("Death / Respawn", "Fake Death, Real Death, 공중 Death, 부활, 리스폰, 카메라/화면 페이드/락온 복구를 처리합니다."),
        ("Packaging Polish", "SFX Preload, Niagara warmup 정리, 해상도 설정, Pause/Restart fade, Map Notice UI를 패키징 기준으로 다듬었습니다."),
    ]
    page_no = page_appendix_cards(c, page_no, "Appendix C. Gameplay Details", "전투 완성도와 플레이 감각을 보강한 세부 시스템", appendix_gameplay)

    page_no = new_page(c, page_no, "Source Structure Snapshot", "포트폴리오 설명과 연결되는 주요 코드 모듈")
    structure = [
        ("Player Components", "ActionState, Defense, CombatReaction, Equipment, Execution, Potion"),
        ("Player Partials", "ActionRequest, ActionPolicy, BasicAttack, HeavyAttack, Jigen, Dodge, Jump, Mikiri, JumpCounter, Camera"),
        ("Monster / Boss", "Monster StateMachine, CombatHit, CodeMove, HUD, Execution / Boss StateMachine, AttackSelection, AttackLink, Parry"),
        ("Interfaces", "AttackTarget, CombatHitTarget, DefenseReactionTarget, ExecutionTarget, MikiriCounterTarget, JumpCounterTarget, LockOnTarget"),
        ("Animation Notifies", "AttackTrace, SwordTrail, WeaponNiagaraToggle, CodeMoveWindow, NormalAttackLink, MontagePlayRateWindow"),
        ("Subsystems / UI", "CombatVFXSubsystem, TimeEffectSubsystem, BGMManager, CombatHUD, MonsterHUD, DangerMarker, LockOnMarker"),
    ]
    y = PAGE_H - 260
    for title, desc in structure:
        rounded_panel(c, 90, y - 10, 1020, 78, fill=COL_PANEL)
        c.setFillColor(COL_CYAN)
        c.setFont("MalgunBold", 16)
        c.drawString(120, y + 26, title)
        draw_wrapped(c, desc, 350, y + 26, 720, size=13, leading=19, color=COL_MUTED)
        y -= 96

    c.save()


if __name__ == "__main__":
    build_pdf()
    print(PDF_PATH)

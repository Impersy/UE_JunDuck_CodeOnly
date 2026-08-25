from __future__ import annotations

from pathlib import Path

from reportlab.lib import colors
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.pdfgen import canvas


ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "output" / "pdf"
PDF_PATH = OUT_DIR / "LeeJongHyuk_Resume_Updated.pdf"
PHOTO_PATH = Path("C:/Users/USER/Downloads/블루.jpg")

PAGE_W, PAGE_H = 595, 842
MX = 44

COL_TEXT = colors.HexColor("#1D2939")
COL_MUTED = colors.HexColor("#667085")
COL_DIM = colors.HexColor("#98A2B3")
COL_NAVY = colors.HexColor("#1E3A8A")
COL_BLUE = colors.HexColor("#2563EB")
COL_LINE = colors.HexColor("#D0D5DD")
COL_SOFT = colors.HexColor("#F8FAFC")
COL_PANEL = colors.HexColor("#F2F4F7")


def register_fonts() -> None:
    font_dir = Path("C:/Windows/Fonts")
    pdfmetrics.registerFont(TTFont("Malgun", str(font_dir / "malgun.ttf")))
    pdfmetrics.registerFont(TTFont("MalgunBold", str(font_dir / "malgunbd.ttf")))


def fn(bold: bool = False) -> str:
    return "MalgunBold" if bold else "Malgun"


def draw(c: canvas.Canvas, x: float, y: float, s: str, size: float = 10, color=COL_TEXT, bold: bool = False) -> None:
    c.setFillColor(color)
    c.setFont(fn(bold), size)
    c.drawString(x, y, s)


def right(c: canvas.Canvas, x: float, y: float, s: str, size: float = 9.5, color=COL_MUTED) -> None:
    c.setFillColor(color)
    c.setFont("Malgun", size)
    c.drawRightString(x, y, s)


def text_w(s: str, size: float, bold: bool = False) -> float:
    return pdfmetrics.stringWidth(s, fn(bold), size)


def wrap(s: str, max_w: float, size: float = 10, bold: bool = False) -> list[str]:
    lines: list[str] = []
    line = ""
    for token in s.split(" "):
        candidate = token if not line else f"{line} {token}"
        if text_w(candidate, size, bold) <= max_w:
            line = candidate
        else:
            if line:
                lines.append(line)
            line = token
    if line:
        lines.append(line)
    return lines


def para(c: canvas.Canvas, x: float, y: float, s: str, max_w: float, size: float = 10, leading: float = 15, color=COL_TEXT) -> float:
    for line in wrap(s, max_w, size):
        draw(c, x, y, line, size=size, color=color)
        y -= leading
    return y


def line(c: canvas.Canvas, y: float) -> None:
    c.setStrokeColor(COL_LINE)
    c.setLineWidth(0.75)
    c.line(MX, y, PAGE_W - MX, y)


def link(c: canvas.Canvas, label: str, display: str, target: str, x: float, y: float, w: float) -> None:
    if target.startswith("github.com/"):
        target = f"https://{target}"
    elif "@" in target and not target.startswith("mailto:"):
        target = f"mailto:{target}"
    draw(c, x, y + 13, label, size=8.5, color=COL_DIM, bold=True)
    draw(c, x, y - 1, display, size=9.2, color=COL_TEXT)
    c.linkURL(target, (x, y - 4, x + w, y + 28), relative=0, thickness=0)


def section(c: canvas.Canvas, y: float, title: str) -> float:
    draw(c, MX, y, title, size=15.5, color=COL_NAVY, bold=True)
    line(c, y - 8)
    return y - 28


def card(c: canvas.Canvas, x: float, y: float, w: float, h: float, fill=COL_SOFT) -> None:
    c.setFillColor(fill)
    c.setStrokeColor(colors.HexColor("#E4E7EC"))
    c.roundRect(x, y, w, h, 9, fill=1, stroke=1)


def bullet(c: canvas.Canvas, x: float, y: float, s: str, max_w: float, size: float = 9.3) -> float:
    c.setFillColor(COL_BLUE)
    c.circle(x + 2.8, y + 3.2, 2, fill=1, stroke=0)
    return para(c, x + 13, y, s, max_w - 13, size=size, leading=14.2, color=COL_TEXT)


def tag(c: canvas.Canvas, x: float, y: float, s: str) -> float:
    w = text_w(s, 8.7, True) + 18
    c.setFillColor(colors.HexColor("#EFF6FF"))
    c.setStrokeColor(colors.HexColor("#BFDBFE"))
    c.roundRect(x, y, w, 22, 8, fill=1, stroke=1)
    draw(c, x + 9, y + 6.3, s, size=8.7, color=COL_NAVY, bold=True)
    return x + w + 7


def page_base(c: canvas.Canvas, page: int) -> None:
    c.setFillColor(colors.white)
    c.rect(0, 0, PAGE_W, PAGE_H, fill=1, stroke=0)
    c.setFillColor(COL_SOFT)
    c.rect(0, PAGE_H - 88, PAGE_W, 88, fill=1, stroke=0)
    draw(c, MX, 26, "Game Client Programmer Resume", size=8.4, color=COL_DIM)
    right(c, PAGE_W - MX, 26, f"{page:02d}", size=9, color=COL_DIM)


def draw_photo(c: canvas.Canvas, x: float, y: float) -> None:
    w, h = 84, 108
    c.setFillColor(colors.white)
    c.setStrokeColor(COL_LINE)
    c.roundRect(x, y, w, h, 6, fill=1, stroke=1)
    if PHOTO_PATH.exists():
        c.drawImage(str(PHOTO_PATH), x + 4, y + 4, w - 8, h - 8, preserveAspectRatio=True, mask="auto")
    else:
        draw(c, x + 28, y + 58, "PHOTO", size=10, color=COL_DIM, bold=True)
        draw(c, x + 14, y + 42, "profile.jpg", size=7.6, color=COL_DIM)


def header(c: canvas.Canvas) -> None:
    page_base(c, 1)
    draw(c, MX, PAGE_H - 47, "이종혁", size=25, color=COL_TEXT, bold=True)
    draw(c, MX + 86, PAGE_H - 43, "Game Client Programmer", size=13, color=COL_NAVY, bold=True)
    draw(c, MX, PAGE_H - 68, "UE5 C++ 액션 전투 시스템과 DirectX gameplay/rendering system 구현 경험", size=9.7, color=COL_MUTED)
    draw_photo(c, PAGE_W - MX - 84, PAGE_H - 147)

    info_x = MX
    info_y = PAGE_H - 123
    infos = [("출생", "1999년생"), ("병역", "면제"), ("희망 직무", "게임 클라이언트 프로그래머")]
    for label, value in infos:
        draw(c, info_x, info_y, label, size=8.5, color=COL_DIM, bold=True)
        draw(c, info_x + 48, info_y, value, size=9.5, color=COL_TEXT)
        info_y -= 23

    link_y = PAGE_H - 123
    link(c, "UE5 Project Video", "YouTube - UE5 Project Video", "https://youtu.be/WXY8d4P0Uec", MX + 230, link_y, 130)
    link(c, "DX Projects Video", "YouTube - DX Projects Video", "https://youtu.be/MyqkYV1b1fc", MX + 230, link_y - 42, 130)
    link(c, "GitHub", "github.com/Impersy", "github.com/Impersy", MX + 230, link_y - 84, 120)
    link(c, "Email", "jhorn3927@gmail.com", "jhorn3927@gmail.com", MX + 380, link_y - 42, 130)
    draw(c, MX + 380, link_y - 71, "Phone", size=8.5, color=COL_DIM, bold=True)
    draw(c, MX + 380, link_y - 85, "010-3727-8335", size=9.2, color=COL_TEXT)


def page_one(c: canvas.Canvas) -> None:
    header(c)
    y = PAGE_H - 220

    y = section(c, y, "Profile")
    card(c, MX, y - 90, PAGE_W - MX * 2, 90, fill=colors.HexColor("#F5F8FF"))
    para(
        c,
        MX + 16,
        y - 22,
        "C++ 기반 게임 클라이언트 개발자를 목표로, 기능 구현뿐 아니라 시스템의 동작 원리와 확장 구조를 함께 고민해왔습니다. DirectX 프로젝트에서는 로우 레벨 렌더링/엔진 구조를 직접 다루며 카메라, 충돌, 파티클, 미니맵 등 게임 시스템을 구현했습니다. UE5 프로젝트에서는 엔진 클래스를 상속/확장해 액션 전투 구조를 직접 설계하고, 유지보수성과 확장성을 고려해 리팩토링했습니다. LLM/Agent는 반복 작업과 로직 점검을 빠르게 처리하는 보조 도구로 활용하되, 최종 판단과 적용은 직접 검수했습니다.",
        PAGE_W - MX * 2 - 32,
        size=9.25,
        leading=14.2,
    )
    y -= 118

    y = section(c, y, "Core Strengths")
    strengths = [
        ("문제 분석과 해결", "현상을 로그와 재현 조건으로 좁히고, 임시방편보다 원인을 찾아 구조적으로 수정하는 방식을 지향합니다."),
        ("시스템 구조화", "기능이 커질수록 상태 전이, 인터페이스, 컴포넌트, 서브시스템으로 책임을 나누며 유지보수성을 개선합니다."),
        ("구현 완성도", "조작감, 카메라, 사운드, 이펙트, UI까지 실제 플레이 감각을 기준으로 반복 테스트하고 조정합니다."),
        ("AI 활용과 검수", "LLM/Agent를 반복 작업과 로직 점검의 보조 도구로 활용하되, 설계 판단과 최종 코드는 직접 검수합니다."),
    ]
    box_w = (PAGE_W - MX * 2 - 14) / 2
    for i, (title, body) in enumerate(strengths):
        x = MX + (i % 2) * (box_w + 14)
        by = y - (i // 2) * 94
        card(c, x, by - 78, box_w, 78, fill=COL_SOFT)
        draw(c, x + 13, by - 24, title, size=10.8, color=COL_NAVY, bold=True)
        para(c, x + 13, by - 43, body, box_w - 26, size=8.7, leading=12.8, color=COL_MUTED)
    y -= 205

    y = section(c, y, "Skills") - 8
    groups = [
        ("Programming", ["C++", "STL", "Data Structure", "Algorithm"]),
        ("Engine / Graphics", ["Unreal Engine 5", "DirectX11", "DirectX9", "HLSL", "WinAPI", "ImGui"]),
        ("CS / Math", ["Data Structure", "Algorithm", "Vector / Matrix"]),
        ("Design Pattern", ["Singleton", "Template Method", "Bridge", "Component", "Factory"]),
    ]
    for group, values in groups:
        draw(c, MX, y, group, size=10.3, color=COL_NAVY, bold=True)
        x = MX + 126
        for value in values:
            if x + text_w(value, 8.7, True) + 28 > PAGE_W - MX:
                y -= 28
                x = MX + 126
            x = tag(c, x, y - 5, value)
        y -= 35


def project_card(c: canvas.Canvas, y: float, title: str, meta: str, tech: str, desc: str, bullets: list[str]) -> float:
    h = 104
    card(c, MX, y - h, PAGE_W - MX * 2, h, fill=COL_SOFT)
    draw(c, MX + 15, y - 24, title, size=12.1, color=COL_TEXT, bold=True)
    right(c, PAGE_W - MX - 15, y - 22, meta, size=9, color=COL_MUTED)
    draw(c, MX + 15, y - 42, tech, size=9.1, color=COL_BLUE)
    para(c, MX + 15, y - 61, desc, PAGE_W - MX * 2 - 30, size=9.2, leading=13.5, color=COL_TEXT)
    by = y - 82
    for b in bullets[:2]:
        by = bullet(c, MX + 16, by, b, PAGE_W - MX * 2 - 32, size=8.8)
    return y - h - 17


def page_two(c: canvas.Canvas) -> None:
    c.showPage()
    page_base(c, 2)
    y = PAGE_H - 120
    y = section(c, y, "Projects")

    y = project_card(
        c,
        y,
        "[UE5 C++] Sekiro-like Combat System",
        "Personal · 60 Days",
        "UE5 C++ · Tutorial + One Boss Fight · Packaged Shipping Build",
        "3인칭 액션 보스전. 조작감과 전투 흐름 중심의 플레이어 액션, 방어, 보스 AI, 공격 판정 구현.",
        [
            "Player Action FSM, Input Buffer, Cancel Rule 기반 액션 흐름 구성",
            "Boss FSM, Attack Link, Common AttackTrace / HitReaction, Execution 시스템 구현",
        ],
    )
    y = project_card(
        c,
        y,
        "[DX11 3D] Demon Slayer Recreation",
        "Team 6 · 60 Days",
        "C++ · DirectX11 · HLSL · ImGui",
        "3D 액션 전투 프로젝트. 전투 카메라와 UI/연출 시스템을 담당.",
        [
            "전투축 기반 Combat Camera System 구현",
            "Minimap System, Awakening Cutscene System 구현",
        ],
    )
    y = project_card(
        c,
        y,
        "[DX11 3D] Thymesia Recreation",
        "Personal · 43 Days",
        "C++ · DirectX11 · HLSL",
        "패링 중심의 3D 액션 전투 프로젝트. 충돌 판정과 반복 렌더링 최적화를 구현.",
        [
            "Parry System, Weapon OBB Collision System 구현",
            "Particle Instancing 기반 반복 파티클 렌더링 최적화",
        ],
    )
    y = project_card(
        c,
        y,
        "[DX9 2.5D] IRA",
        "Team 4 · 30 Days",
        "C++ · DirectX9",
        "2.5D 전투 프로젝트. 활 무기와 시간 정지 스킬 흐름을 담당.",
        [
            "Bow Weapon System 구현",
            "Layer 단위 Time Stop Skill System 구현",
        ],
    )
    y = project_card(
        c,
        y,
        "[WinAPI 2D] Skul Recreation",
        "Personal · 16 Days",
        "C++ · WinAPI",
        "2D 액션 프로젝트. 기본 전투와 보스 패턴, 객체 관리 로직을 구현.",
        [
            "2D Sprite 기반 전투 및 스킬 시스템 구현",
            "보스 패턴 및 객체 관리 로직 제작",
        ],
    )


def page_three(c: canvas.Canvas) -> None:
    c.showPage()
    page_base(c, 3)
    y = PAGE_H - 120
    y = section(c, y, "Study Focus")
    rows = [
        ("2026.07 - Present", "UEFN / Verse", "UEFN 제작 흐름과 Verse 기반 게임 로직 학습"),
        ("2026.07 - Present", "Unreal Framework", "GC, Reflection, UObject/Actor 생명주기 학습"),
        ("2026.03 - Present", "CS / C++ 학습", "알고리즘 & 자료구조, C++ 기본 및 심화"),
        ("2025.12 - 2026.07", "Unreal Engine 5 C++", "UE5 C++ 기반 3인칭 액션 전투 포트폴리오 제작"),
        ("2022.09 - 2023.10", "쥬신게임아카데미", "게임 프로그래밍 정규반 커리큘럼 수료"),
        ("2021.10 - 2022.06", "게임 개발 학습", "C++, DirectX12, 알고리즘 과정 수료"),
        ("2021.02 - 2021.06", "C++ 학습", "객체지향 프로그래밍 독학"),
        ("2020.11 - 2021.01", "C 언어 기초", "C 언어 기초 독학"),
    ]
    for period, name, desc in rows:
        draw(c, MX, y, period, size=8.8, color=COL_DIM)
        draw(c, MX + 120, y, name, size=9.5, color=COL_TEXT, bold=True)
        draw(c, MX + 260, y, desc, size=9.1, color=COL_MUTED)
        y -= 24

    y -= 14
    y = section(c, y, "Education / Experience")
    rows = [
        ("2018.03 - 2021.06", "충북대학교", "지구환경과학과 / 3학년 1학기 중퇴"),
        ("2021.08 - 2022.08", "대치명인학원", "수학 조교"),
    ]
    for period, name, desc in rows:
        draw(c, MX, y, period, size=8.8, color=COL_DIM)
        draw(c, MX + 120, y, name, size=9.5, color=COL_TEXT, bold=True)
        draw(c, MX + 260, y, desc, size=9.1, color=COL_MUTED)
        y -= 24

    y -= 20
    y = section(c, y, "Portfolio Note")
    note = (
        "프로젝트별 상세 구조, 코드 카드, 문제-해결-결과 정리는 별도 포트폴리오 PDF에 정리했습니다. "
        "이력서에서는 지원자 정보와 핵심 역량, 프로젝트 요약만 빠르게 확인할 수 있도록 구성했습니다."
    )
    card(c, MX, y - 76, PAGE_W - MX * 2, 76, fill=colors.HexColor("#F5F8FF"))
    para(c, MX + 16, y - 25, note, PAGE_W - MX * 2 - 32, size=9.8, leading=15.5)


def build_pdf() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    register_fonts()
    c = canvas.Canvas(str(PDF_PATH), pagesize=(PAGE_W, PAGE_H))
    c.setTitle("Lee Jong Hyuk Resume")
    page_one(c)
    page_two(c)
    page_three(c)
    c.save()


if __name__ == "__main__":
    build_pdf()
    print(PDF_PATH)

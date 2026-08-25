from __future__ import annotations

from pathlib import Path

from reportlab.lib import colors
from reportlab.pdfgen import canvas

import generate_junduck_portfolio_pdf as jun


ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "output" / "pdf"
PDF_PATH = OUT_DIR / "LeeJongHyuk_Integrated_Portfolio.pdf"
DX_PAGE_DIR = ROOT / "tmp" / "pdfs"
UNREAL_DEMO_URL = "https://youtu.be/WXY8d4P0Uec"

PROJECT_SCREENSHOTS = {
    "demon_slayer": Path(r"C:\Users\USER\Desktop\portfolio\이미지\화면 캡처 2026-07-03 154809.png"),
    "thymesia": Path(r"C:\Users\USER\Desktop\portfolio\이미지\화면 캡처 2026-07-03 154858.png"),
    "ira": Path(r"C:\Users\USER\Desktop\portfolio\이미지\화면 캡처 2026-07-03 154937.png"),
    "ue5": Path(r"C:\Users\USER\Desktop\portfolio\이미지\화면 캡처 2026-07-03 202242.png"),
}


def dx_page(num: int) -> Path:
    return DX_PAGE_DIR / f"dx_full_page-{num:02d}.png"


def add_clickable_link(c: canvas.Canvas, url: str, x: float, y: float, w: float, h: float) -> None:
    target = url
    if url.startswith("github.com/"):
        target = f"https://{url}"
    elif "@" in url and not url.startswith(("http://", "https://", "mailto:")):
        target = f"mailto:{url}"
    c.linkURL(target, (x, y, x + w, y + h), relative=0, thickness=0)


def draw_footer(c: canvas.Canvas, page_no: int) -> None:
    c.setStrokeColor(jun.COL_LINE)
    c.line(60, 42, jun.PAGE_W - 60, 42)
    c.setFillColor(jun.COL_DIM)
    c.setFont("Malgun", 11)
    c.drawString(60, 24, "UE5 · DirectX · C++ · Game Client Portfolio")
    c.drawRightString(jun.PAGE_W - 60, 24, f"{page_no:02d}")


def draw_title(c: canvas.Canvas, header: str, title: str, subtitle: str | None = None) -> None:
    c.setFillColor(jun.COL_BLUE)
    c.setFont("MalgunBold", 18)
    c.drawString(60, jun.PAGE_H - 72, header)
    c.setFillColor(jun.COL_TEXT)
    c.setFont("MalgunBold", 38)
    c.drawString(60, jun.PAGE_H - 130, title)
    if subtitle:
        c.setFillColor(jun.COL_MUTED)
        c.setFont("Malgun", 18)
        c.drawString(62, jun.PAGE_H - 166, subtitle)


def draw_unreal_title(c: canvas.Canvas, title: str, subtitle: str | None = None) -> None:
    draw_title(c, "Sekiro-like Combat System", title, subtitle)


def draw_unreal_footer(c: canvas.Canvas, page_no: int) -> None:
    c.setStrokeColor(jun.COL_LINE)
    c.line(60, 42, jun.PAGE_W - 60, 42)
    c.setFillColor(jun.COL_DIM)
    c.setFont("Malgun", 11)
    c.drawString(60, 24, "UE5 · C++ · Sekiro-like Action Combat Portfolio")
    c.drawRightString(jun.PAGE_W - 60, 24, f"{page_no:02d}")


def new_page(c: canvas.Canvas, page_no: int, title: str, subtitle: str | None = None, header: str = "Game Client Programmer Portfolio") -> int:
    if page_no > 0:
        c.showPage()
    jun.draw_bg(c)
    draw_title(c, header, title, subtitle)
    draw_footer(c, page_no + 1)
    return page_no + 1


def draw_metric(c: canvas.Canvas, x: float, y: float, label: str, value: str) -> None:
    c.setFillColor(jun.COL_DIM)
    c.setFont("Malgun", 10)
    c.drawString(x, y + 23, label)
    c.setFillColor(jun.COL_TEXT)
    c.setFont("MalgunBold", 16)
    c.drawString(x, y, value)


def page_combined_cover(c: canvas.Canvas, page_no: int) -> int:
    jun.draw_bg(c)
    c.setFillColor(jun.COL_TEXT)
    c.setFont("MalgunBold", 54)
    c.drawString(70, jun.PAGE_H - 160, "Game Client")
    c.drawString(70, jun.PAGE_H - 223, "Programmer Portfolio")

    c.setFillColor(jun.COL_MUTED)
    c.setFont("Malgun", 20)
    c.drawString(74, jun.PAGE_H - 277, "UE5 Action Combat Architecture + DirectX Gameplay / Rendering Systems")

    draw_metric(c, 74, jun.PAGE_H - 350, "Primary Focus", "Action Combat")
    draw_metric(c, 255, jun.PAGE_H - 350, "Engine / API", "UE5 · DX11 · DX9")
    draw_metric(c, 455, jun.PAGE_H - 350, "Portfolio Scope", "4 Projects")

    links = [
        ("UE5 Project Video", UNREAL_DEMO_URL),
        ("DX Projects Video", "https://youtu.be/MyqkYV1b1fc"),
    ]
    link_y = 448
    for label, url in links:
        jun.rounded_panel(c, 70, link_y, 500, 42, fill=colors.HexColor("#101923"), stroke=jun.COL_LINE, radius=9)
        c.setFillColor(jun.COL_BLUE)
        c.setFont("MalgunBold", 12.5)
        c.drawString(100, link_y + 14, label)
        c.setFillColor(jun.COL_TEXT)
        c.setFont("Malgun", 10.8)
        c.drawString(250, link_y + 14, url)
        add_clickable_link(c, url, 70, link_y, 500, 42)
        link_y -= 50

    jun.rounded_panel(c, 70, 115, 500, 245, fill=jun.COL_PANEL)
    c.setFillColor(jun.COL_YELLOW)
    c.setFont("MalgunBold", 16)
    c.drawString(105, 318, "Portfolio Direction")
    body = (
        "이 포트폴리오는 로우 레벨 구현 경험과 엔진 기반 시스템 설계 경험을 함께 보여주는 것을 목표로 합니다. "
        "DirectX 프로젝트에서는 렌더링/엔진 구조를 직접 다루며 카메라, 충돌, 파티클, 미니맵 등 게임 시스템을 구현했습니다. "
        "UE5 프로젝트에서는 엔진 클래스를 상속/확장해 액션 전투 구조를 직접 설계하고, 유지보수성과 확장성을 고려해\u00A0리팩토링했습니다. "
        "LLM/Agent는 반복 작업과 로직 점검을 빠르게 처리하는 보조 도구로 활용했으며, 최종 판단과 적용은 직접 검수했습니다."
    )
    jun.draw_wrapped(c, body, 105, 284, 450, size=13.2, leading=21.2, color=jun.COL_TEXT)

    jun.rounded_panel(c, 630, 120, 500, 430, fill=jun.COL_PANEL_2)
    c.setFillColor(jun.COL_BLUE)
    c.setFont("MalgunBold", 16)
    c.drawString(670, 505, "Portfolio Sections")
    jun.draw_flow(
        c,
        760,
        445,
        [
            "Profile / Skills",
            "UE5 Combat System",
            "Action Combat Systems",
            "DirectX Projects",
            "Gameplay / Rendering Systems",
        ],
        [
            colors.HexColor("#12304a"),
            colors.HexColor("#451b21"),
            colors.HexColor("#281f45"),
            colors.HexColor("#17352c"),
            colors.HexColor("#1d2937"),
        ],
        box_w=210,
        box_h=46,
        gap=24,
    )

    draw_footer(c, 1)
    return 1


def page_profile(c: canvas.Canvas, page_no: int) -> int:
    page_no = new_page(c, page_no, "Profile / Skills", "전투 시스템 중심의 게임 클라이언트 프로그래머")

    jun.rounded_panel(c, 70, 500, 680, 175, fill=jun.COL_PANEL)
    c.setFillColor(jun.COL_CYAN)
    c.setFont("MalgunBold", 18)
    c.drawString(105, 635, "Profile")
    body = (
        "전투 시스템 중심의 Gameplay Programming에 관심이 있으며, UE5 C++ 프로젝트와 DirectX 개인/팀 프로젝트를 통해 "
        "전투, 카메라, UI, 렌더링 시스템을 구현했습니다."
    )
    jun.draw_wrapped(c, body, 105, 600, 610, size=15, leading=26, color=jun.COL_TEXT)

    jun.rounded_panel(c, 70, 300, 680, 155, fill=jun.COL_PANEL)
    c.setFillColor(jun.COL_YELLOW)
    c.setFont("MalgunBold", 18)
    c.drawString(105, 415, "Focus Area")
    focus = "Action Combat · Gameplay Programming · Camera Systems · Rendering Systems · Tools for Feedback"
    jun.draw_wrapped(c, focus, 105, 378, 610, size=15, leading=24, color=jun.COL_TEXT)

    jun.rounded_panel(c, 790, 300, 340, 375, fill=jun.COL_PANEL)
    c.setFillColor(jun.COL_BLUE)
    c.setFont("MalgunBold", 18)
    c.drawString(825, 635, "Skills")
    skills = [
        ("Language", "C++"),
        ("Engine / API", "Unreal Engine 5\nDirectX11 / DirectX9"),
        ("Systems", "Action FSM\nCombat Systems\nOBB Collision\nParticle Instancing\nLock-On Camera"),
    ]
    y = 590
    for label, value in skills:
        c.setFillColor(jun.COL_YELLOW)
        c.setFont("MalgunBold", 12)
        c.drawString(825, y, label)
        y = jun.draw_wrapped(c, value, 825, y - 24, 260, size=13.5, leading=21, color=jun.COL_TEXT)
        y -= 18

    jun.rounded_panel(c, 70, 120, 1060, 120, fill=colors.HexColor("#111b24"))
    c.setFillColor(jun.COL_GREEN)
    c.setFont("MalgunBold", 16)
    c.drawString(105, 200, "Strength")
    strength = (
        "문제를 시스템 단위로 분해하고, 플레이 감각에 필요한 기능을 상태, 정책, 인터페이스, Subsystem으로 구조화합니다. "
        "DirectX 프로젝트에서는 수학/좌표계 기반 구현과 렌더링 최적화 포인트를 직접 다뤘습니다."
    )
    jun.draw_wrapped(c, strength, 105, 170, 990, size=14, leading=23, color=jun.COL_MUTED)
    return page_no


def page_projects_overview(c: canvas.Canvas, page_no: int) -> int:
    page_no = new_page(c, page_no, "Projects Overview", "Unreal 메인 프로젝트와 DirectX 기반 프로젝트 요약")
    projects = [
        ("Sekiro-like Combat System", "UE5 C++ Personal Project · 60 Days", "Action FSM / Defense & Parry / Boss FSM / Common Attack Trace / Execution System"),
        ("Demon Slayer Recreation", "DirectX11 Team Project · 6 Members · 60 Days", "Combat Camera / Awakening Cutscene / Minimap UI"),
        ("Thymesia Recreation", "DirectX11 Personal Project · 43 Days", "Parry System / Particle Instancing / OBB Weapon Hit Detection"),
        ("IRA Recreation", "DirectX9 Team Project · 4 Members · 30 Days", "Time Stop Skill / Player Combat / Camera System"),
    ]
    y = 555
    for index, (title, meta, desc) in enumerate(projects):
        jun.rounded_panel(c, 80, y, 1040, 125, fill=jun.COL_PANEL)
        accent = [jun.COL_BLUE, jun.COL_GREEN, jun.COL_CYAN, jun.COL_PURPLE][index]
        c.setFillColor(accent)
        c.setFont("MalgunBold", 20)
        c.drawString(115, y + 81, title)
        c.setFillColor(jun.COL_TEXT)
        c.setFont("MalgunBold", 15.5)
        c.drawString(115, y + 51, meta)
        jun.draw_wrapped(c, desc, 115, y + 23, 930, size=14.5, leading=20, color=jun.COL_MUTED)
        y -= 150
    return page_no


def unreal_chapters() -> list[jun.Chapter]:
    return [
        jun.Chapter("01", "Player Action FSM / Cancel Buffer", "입력 요청, 캔슬 정책, 입력 버퍼를 중앙 파이프라인으로 관리", jun.CARD_IMAGES[0], [],
                    "액션이 늘어날수록 입력, 캔슬, 후속 버퍼가 함수 곳곳에 흩어져 조작감 튜닝이 어려웠습니다.",
                    "모든 입력을 ActionRequest로 변환하고, 현재 상태와 CancelRule을 통과한 요청만 실행하게 했습니다.",
                    "공격, 회피, 점프, 패링 후속 행동을 같은 흐름에서 처리해 액션 추가와 튜닝 부담을 줄였습니다."),
        jun.Chapter("02", "Defense / Parry System", "우클릭 입력을 현재 상태와 타이밍에 따라 패링, 가드, 체인 패리로 해석", jun.CARD_IMAGES[1], [],
                    "같은 우클릭 입력이 상황에 따라 패링, 가드, 체인 패리, 피격 캔슬로 달라져 입력 해석이 꼬이기 쉬웠습니다.",
                    "DefenseComponent가 입력 유지 시간, 현재 상태, 패링 윈도우, 공격 방향과 타입을 함께 판단합니다.",
                    "탭/홀드/체인/공중 방어가 안정적으로 분기되어 타이밍 기반 공방 흐름을 만들 수 있었습니다."),
        jun.Chapter("03", "Boss FSM / Attack Link", "상태 전환, 공격 후보 선정, 몽타주 Notify 기반 링크로 보스 전투 흐름 구성", jun.CARD_IMAGES[2], [],
                    "보스를 고정 콤보나 단순 랜덤으로 만들면 거리, 페이즈, 쿨타임에 맞춘 공방 흐름을 만들기 어려웠습니다.",
                    "Combat FSM과 AttackSelectionStrategy로 후보를 필터링하고, Notify 시점에서 다음 공격을 링크했습니다.",
                    "보스가 접근, 재배치, 공격, 연계를 상황에 맞게 선택해 전투가 덜 반복적으로 느껴지게 했습니다."),
        jun.Chapter("04", "Common Attack Trace / Hit Reaction", "공격자는 공통 HitRequest만 만들고, 피격자는 인터페이스로 자기 반응을 선택", jun.CARD_IMAGES[3], [],
                    "플레이어, 보스, NPC, 투사체 공격을 따로 처리하면 공격 타입과 피격 리액션 코드가 계속 중복됐습니다.",
                    "공격자는 공통 HitRequest를 만들고, 피격자는 인터페이스로 받아 자신의 상태에 맞게 해석하게 했습니다.",
                    "새 공격이나 새 몬스터도 같은 판정 파이프라인에 연결할 수 있어 재사용성을 높였습니다."),
        jun.Chapter("05", "Reusable Execution System", "처형 실행자와 처형 대상의 결과 처리를 인터페이스로 분리", jun.CARD_IMAGES[4], [],
                    "처형을 보스 전용으로 만들면 TutorialNPC나 일반 몬스터에 재사용하기 어렵고 의존성이 커졌습니다.",
                    "플레이어는 처형 실행만 담당하고, 대상은 ExecutionTarget 인터페이스로 자기 처형 결과를 처리합니다.",
                    "같은 처형 입력을 공유하면서도 보스는 Life 감소, NPC는 회복/Task 완료처럼 다른 결과를 낼 수 있습니다."),
    ]


def page_section(c: canvas.Canvas, page_no: int, title: str, subtitle: str, bullets: list[str]) -> int:
    page_no = new_page(c, page_no, title, subtitle)
    jun.rounded_panel(c, 120, 250, 960, 320, fill=jun.COL_PANEL)
    y = 500
    for bullet in bullets:
        c.setFillColor(jun.COL_BLUE)
        c.setFont("MalgunBold", 16)
        c.drawString(165, y, "•")
        jun.draw_wrapped(c, bullet, 195, y, 820, size=16, leading=28, color=jun.COL_TEXT)
        y -= 76
    return page_no


def page_unreal_section(c: canvas.Canvas, page_no: int) -> int:
    page_no = new_page(
        c,
        page_no,
        "Part 1. Unreal - Sekiro-like Combat System",
        "UE5 C++ 기반 3인칭 액션 보스전 포트폴리오",
    )
    bullets = [
        "실제 조작감을 고려해 입력, 캔슬, 패링, 회피, 점프, 스킬 흐름을 중앙 액션 파이프라인으로 정리했습니다.",
        "보스 전투는 상태 전환, 공격 선택, 몽타주 Notify 기반 링크를 통해 상황에 맞는 공방 흐름을 구성했습니다.",
        "AttackTrace, HitReaction, Execution, VFX/TimeEffect를 인터페이스와 컴포넌트 중심으로 분리해 재사용성을 높였습니다.",
    ]
    jun.rounded_panel(c, 70, 180, 485, 470, fill=jun.COL_PANEL)
    c.setFillColor(jun.COL_YELLOW)
    c.setFont("MalgunBold", 15)
    c.drawString(110, 605, "System Focus")
    y = 555
    for bullet in bullets:
        c.setFillColor(jun.COL_BLUE)
        c.setFont("MalgunBold", 16)
        c.drawString(110, y, "•")
        jun.draw_wrapped(c, bullet, 140, y, 365, size=15.3, leading=26, color=jun.COL_TEXT)
        y -= 102

    c.setFillColor(jun.COL_GREEN)
    c.setFont("MalgunBold", 13.5)
    c.drawString(110, 235, "Captured from packaged Shipping build")
    c.setFillColor(jun.COL_MUTED)
    c.setFont("Malgun", 13)
    c.drawString(110, 208, "Tutorial + one boss fight · UE5 C++")

    jun.rounded_panel(c, 590, 165, 540, 485, fill=colors.HexColor("#0f141b"), radius=14)
    ue_image = PROJECT_SCREENSHOTS["ue5"]
    if ue_image.exists():
        jun.draw_image_fit(c, ue_image, 610, 195, 500, 410)
    c.setFillColor(jun.COL_MUTED)
    c.setFont("Malgun", 11.5)
    c.drawCentredString(860, 142, "Gameplay screenshot used as the UE5 project visual reference")
    return page_no


def page_directx_section(c: canvas.Canvas, page_no: int) -> int:
    page_no = new_page(
        c,
        page_no,
        "Part 2. DirectX Projects",
        "엔진 기능을 직접 구현하며 쌓은 로우레벨 시스템 경험",
    )
    bullets = [
        "DirectX11 팀 프로젝트에서는 전투 카메라, 각성 컷신, 미니맵 UI처럼 플레이 경험을 보조하는 시스템을 구현했습니다.",
        "DirectX11 개인 프로젝트에서는 KeyFrame 기반 패링, OBB 무기 충돌, Point Instancing 파티클을 직접 구성했습니다.",
        "DirectX9 팀 프로젝트에서는 Layer 단위 TimeDelta 제어로 Time Stop 스킬을 구현했습니다.",
    ]
    jun.rounded_panel(c, 70, 140, 455, 515, fill=jun.COL_PANEL)
    c.setFillColor(jun.COL_YELLOW)
    c.setFont("MalgunBold", 15)
    c.drawString(110, 610, "DirectX Implementation Focus")
    y = 560
    for bullet in bullets:
        c.setFillColor(jun.COL_BLUE)
        c.setFont("MalgunBold", 16)
        c.drawString(110, y, "•")
        jun.draw_wrapped(c, bullet, 140, y, 335, size=14.4, leading=24, color=jun.COL_TEXT)
        y -= 108

    images = [
        ("Demon Slayer", PROJECT_SCREENSHOTS["demon_slayer"]),
        ("Thymesia", PROJECT_SCREENSHOTS["thymesia"]),
        ("IRA", PROJECT_SCREENSHOTS["ira"]),
    ]
    x = 560
    y = 505
    for label, image in images:
        jun.rounded_panel(c, x, y, 545, 150, fill=colors.HexColor("#0f141b"), radius=10)
        if image.exists():
            jun.draw_image_fit(c, image, x + 14, y + 12, 310, 126)
        c.setFillColor(jun.COL_TEXT)
        c.setFont("MalgunBold", 16.5)
        c.drawString(x + 345, y + 92, label)
        c.setFillColor(jun.COL_MUTED)
        c.setFont("Malgun", 12.5)
        c.drawString(x + 345, y + 62, "DirectX gameplay system sample")
        y -= 172
    return page_no


def page_dx_feature(c: canvas.Canvas, page_no: int, title: str, subtitle: str, problem: str, solution: str, result: str, image: Path | None = None) -> int:
    page_no = new_page(c, page_no, title, subtitle, header="DirectX Systems Portfolio")
    psr = [("Problem", problem, jun.COL_RED), ("Solution", solution, jun.COL_BLUE), ("Result", result, jun.COL_GREEN)]
    x = 55
    y = 520
    for label, body, accent in psr:
        jun.rounded_panel(c, x, y, 265, 168, fill=jun.COL_PANEL, radius=12)
        c.setFillColor(accent)
        c.setFont("MalgunBold", 15)
        c.drawString(x + 20, y + 128, label)
        jun.draw_wrapped(c, body, x + 18, y + 98, 226, size=12.3, leading=18.2, color=jun.COL_MUTED)
        y -= 184

    jun.rounded_panel(c, 340, 70, 810, 625, fill=colors.HexColor("#0f141b"), radius=14)
    if image and image.exists():
        jun.draw_image_fit(c, image, 358, 96, 774, 575)
    return page_no


def build_pdf() -> None:
    jun.setup_fonts()
    jun.draw_title = draw_unreal_title
    jun.draw_footer = draw_unreal_footer
    original_page_chapter_image = jun.page_chapter_image

    def page_chapter_image_readable_psr(c: canvas.Canvas, page_no: int, chapter: jun.Chapter) -> int:
        page_no = jun.new_page(c, page_no, f"{chapter.number}. {chapter.title}", chapter.subtitle)
        psr = [
            ("Problem", chapter.problem, jun.COL_RED),
            ("Solution", chapter.solution, jun.COL_BLUE),
            ("Result", chapter.result, jun.COL_GREEN),
        ]
        x = 60
        y = 512
        w = 300
        h = 180
        for label, body, accent in psr:
            jun.rounded_panel(c, x, y, w, h, fill=jun.COL_PANEL, radius=12)
            c.setFillColor(accent)
            c.setFont("MalgunBold", 15)
            c.drawString(x + 20, y + h - 34, label)
            jun.draw_wrapped(c, body, x + 20, y + h - 64, w - 40, size=12.7, leading=18.5, color=jun.COL_MUTED)
            y -= h + 14

        jun.rounded_panel(c, 390, 62, 760, 645, fill=colors.HexColor("#0f141b"), radius=14)
        jun.draw_image_fit(c, chapter.image, 405, 78, 730, 613)
        return page_no

    jun.page_chapter_image = page_chapter_image_readable_psr
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    c = canvas.Canvas(str(PDF_PATH), pagesize=(jun.PAGE_W, jun.PAGE_H))
    c.setTitle("Lee Jong Hyuk Integrated Game Client Portfolio")
    c.setAuthor("Lee Jong Hyuk")

    page_no = page_combined_cover(c, 0)
    page_no = page_profile(c, page_no)
    page_no = page_projects_overview(c, page_no)

    page_no = page_unreal_section(c, page_no)
    page_no = jun.page_overview(c, page_no)
    for chapter in unreal_chapters():
        page_no = jun.page_chapter_image(c, page_no, chapter)
    page_no = jun.page_appendix_special_counter(c, page_no)

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
    page_no = jun.page_appendix_cards(c, page_no, "Appendix B. Supporting Systems", "본문 5개 챕터 밖에 있는 구현 기능들", appendix_systems)

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
    page_no = jun.page_appendix_cards(c, page_no, "Appendix C. Gameplay Details", "전투 완성도와 플레이 감각을 보강한 세부 시스템", appendix_gameplay)

    page_no = page_directx_section(c, page_no)
    page_no = page_dx_feature(
        c, page_no,
        "DX 01. Combat Camera System",
        "Demon Slayer Project - 전투축 기반 카메라 전환",
        "플레이어가 적을 관통하거나 콤보 상황이 바뀔 때 카메라 축이 급격히 뒤집히는 문제가 있었습니다.",
        "Player-Enemy 전투축과 BattleCenter를 기준으로 카메라 방향을 계산하고, 상황별 Battle/Side Camera 상태를 분리했습니다.",
        "전투 대상 가시성을 유지하면서 콤보 상황에서는 Side View로 공격 흐름을 강조했습니다.",
        dx_page(5),
    )
    page_no = page_dx_feature(
        c, page_no,
        "DX 02. Data-Driven Cutscene Camera",
        "Demon Slayer Project - 각성 연출 컷 시퀀스",
        "캐릭터별 각성 연출을 하드코딩하면 컷 추가와 수정이 어렵고 카메라 연산 코드가 복잡해졌습니다.",
        "CutInCamDesc에 각도, 거리, 오프셋, 지속시간을 정의하고 단일 컷 연산과 시퀀스 제어를 분리했습니다.",
        "데이터 조합만으로 캐릭터별 컷신을 확장하고, 종료 후 전투 카메라로 복귀할 수 있게 했습니다.",
        dx_page(8),
    )
    page_no = page_dx_feature(
        c, page_no,
        "DX 03. Minimap UV Mapping",
        "Demon Slayer Project - 월드 좌표 기반 미니맵",
        "맵마다 월드 크기가 달라 플레이어 위치를 UI 좌표로 일관되게 표시하기 어려웠습니다.",
        "월드 좌표를 맵별 기준 범위로 0-1 UV에 정규화하고, 셰이더에서 표시 범위 밖을 Alpha Cut 처리했습니다.",
        "플레이어 주변 영역만 보여주는 동적 미니맵으로 탐색 가독성을 높였습니다.",
        dx_page(12),
    )
    page_no = page_dx_feature(
        c, page_no,
        "DX 04. KeyFrame Parry / OBB Hit",
        "Thymesia Project - 타이밍 패링과 무기 충돌",
        "공격 애니메이션마다 패링 가능 타이밍과 무기 충돌 판정이 달라 단순 충돌만으로는 전투 감각을 맞추기 어려웠습니다.",
        "보스 공격 KeyFrame과 충돌 상태를 함께 검사하고, 무기 Bone Transform 기반 OBB를 SAT로 판정했습니다.",
        "공격별 패링 타이밍과 회전 무기 궤적이 반영된 근접 히트 판정을 구현했습니다.",
        dx_page(21),
    )
    page_no = page_dx_feature(
        c, page_no,
        "DX 05. Particle Instancing",
        "Thymesia Project - 공용 Point Instance Buffer",
        "Parry, Spark, Dust 이펙트를 개별 오브젝트로 생성하면 반복 생성 비용과 관리 비용이 커졌습니다.",
        "위치, 방향, 크기, 수명 정보를 InstanceDesc로 만들고 공용 Point Instance Buffer에서 일괄 갱신/렌더링했습니다.",
        "반복 파티클을 데이터 기반으로 관리하고 DrawIndexedInstanced로 렌더링 비용을 줄였습니다.",
        dx_page(18),
    )
    page_no = page_dx_feature(
        c, page_no,
        "DX 06. Time Stop Skill",
        "IRA Project - Layer 단위 시간 제어",
        "시간 정지 스킬에서 모든 오브젝트를 멈추면 플레이어와 스킬 예외 객체까지 같이 멈추는 문제가 생겼습니다.",
        "Layer가 관리하는 오브젝트에 0.f TimeDelta를 전달하고, 예외 객체는 실제 TimeDelta를 참조하도록 분리했습니다.",
        "적, 투사체, 일반 오브젝트만 정지시키고 플레이어와 스킬 흐름은 유지하는 특수 전투 스킬을 구현했습니다.",
        dx_page(24),
    )

    page_no = new_page(c, page_no, "Links", "영상과 코드 확인 경로")
    links = [
        ("UE5 Project Video", UNREAL_DEMO_URL),
        ("DX Projects Video", "https://youtu.be/MyqkYV1b1fc"),
        ("GitHub", "github.com/Impersy"),
        ("Email", "jhorn3927@gmail.com"),
    ]
    y = 545
    for title, value in links:
        jun.rounded_panel(c, 155, y, 890, 88, fill=jun.COL_PANEL)
        c.setFillColor(jun.COL_BLUE)
        c.setFont("MalgunBold", 17)
        c.drawString(195, y + 52, title)
        c.setFillColor(jun.COL_TEXT)
        c.setFont("Malgun", 15.5)
        c.drawString(430, y + 52, value)
        add_clickable_link(c, value, 155, y, 890, 88)
        y -= 108

    c.save()
    jun.page_chapter_image = original_page_chapter_image


if __name__ == "__main__":
    build_pdf()
    print(PDF_PATH)

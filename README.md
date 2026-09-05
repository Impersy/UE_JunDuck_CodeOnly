# UE5 Action Combat

Unreal Engine 5와 C++로 제작한 3인칭 액션 전투 개인 프로젝트입니다.

플레이어 액션, 방어·패링, 공격 판정, 보스 AI와 처형 시스템을 구현했습니다. 기능을 추가하는 과정에서 발생한 액션 전환 복잡도와 보스 판단 중단 문제를 분석하고, 공통 처리 흐름과 책임이 드러나는 구조로 개선했습니다.

## Project Information

- Engine: Unreal Engine 5.6
- Language: C++
- Genre: Third-person Action Combat
- Development: Personal Project
- Period: 2026.04.17 - 2026.06.15
- Scope: Tutorial + One Boss Fight
- Build: Packaged Shipping Build

## Demo

- [전체 게임 플레이 영상](https://youtu.be/WXY8d4P0Uec)
- [실행 빌드 다운로드](https://drive.google.com/file/d/1s0zmTS2GU0LTUhpN9wbjDqIf98-mnwey/view?usp=drive_link)

## Core Features

### Player Action System

플레이어의 공격·방어·회피·점프·스킬 상태와 전환 관계를 관리합니다.

- `EJunPlayerActionState` 기반 액션 상태 관리
- 출발 액션과 도착 액션 사이의 전환 허용 관계 검사
- `OpenTime`과 `BlendOutTime`을 포함한 액션별 Cancel Rule
- Recovery·Defense·Parry 요청의 공통 처리 흐름
- 전환 유형의 특성에 맞춘 허용 조건 검사
- 콤보와 일부 특수 전환에만 선택적 입력 보관 적용

전환이 허용되면 기존 액션의 공격 판정, Montage, Gameplay Tag와 런타임 상태를 정리한 뒤 다음 액션을 실행합니다.

### Defense and Parry

입력 시점과 공격 방향에 따라 방어와 패링 결과를 구분합니다.

- Guard와 Perfect Parry 판정
- 정면 각도 검사
- 연속 패링 처리
- Guard Block과 Guard Break
- 패링 성공 후 후속 액션 전환
- Posture Damage와 Hit Reaction 처리

### Boss Combat AI

보스의 전체 상태, 현재 전투 실행 단계와 다음 행동 계획을 구분해 관리합니다.

- `CurrentState`: 추적·전투·복귀 등 전체 상태
- `CurrentCombatSubState`: 접근·공격·회피 등 현재 실행 단계
- `CurrentCombatPlan`: 다음 공격 또는 이동 계획
- `TransitionBossCombatState()`를 통한 전투 상태 전이
- 거리·Phase·Cooldown·최근 행동을 이용한 공격 후보 선택
- Montage Notify 기반 Combo와 공격 연계
- 공격 후보가 없을 때 접근 후 다시 판단하는 Fallback

공격 가능한 후보가 없는 상황에서는 `TryPlanNoAttackFallback()`이 접근 경로를 시작하고 계획을 갱신해 보스의 판단이 이어지도록 처리합니다.

### Combat Interaction

공격자는 공통 Hit Request를 만들고, 피격 대상이 자신의 상태에 맞는 결과를 결정합니다.

- 공통 공격 Trace와 Hit Request
- Player·Monster별 Hit Reaction 분기
- Damage·Parry·Guard·Posture 처리
- Super Armor와 물리 반응
- 공격 방향과 Hit Type에 따른 Montage 선택
- Character Team 관계를 이용한 적대 여부 판별

### Execution System

처형 대상 Interface를 통해 플레이어와 대상의 책임을 분리했습니다.

- 플레이어는 처형 입력과 Montage 실행 담당
- Boss·Monster·Tutorial NPC가 자신의 처형 결과 처리
- 보스의 Phase 전환과 생명 소모
- Tutorial NPC의 복구와 Tutorial 진행
- 구체 클래스 직접 참조 감소

## Additional Systems

- Tutorial Task와 NPC Dialogue
- Lock-on Camera
- Mikiri Counter
- Jump Counter
- Potion System
- Monster Overhead HUD
- Danger Attack UI
- Niagara Sword Trail
- Combat VFX Subsystem
- Hit Stop과 Slow Motion
- Foot Placement IK

## Code Structure

Source/JunDuck
├─ Character
│  ├─ Player
│  │  ├─ PlayerComponent
│  │  └─ PlayerPartials
│  └─ Monster
│     ├─ Boss
│     └─ MonsterPartials
├─ Combat
├─ Animation
├─ AI
├─ Interface
├─ Weapon
├─ Camera
├─ UI
└─ System

기능별 코드는 Component와 Partial 파일로 나누고, 공통 전투 기능은 Interface와 Subsystem을 통해 연결했습니다.

## Repository Scope

이 저장소는 C++ Source 저장소입니다.
Unreal Engine 기본 생성 파일, 대용량 Asset과 외부 Marketplace 콘텐츠는 포함하지 않습니다. 전체 프로젝트 실행이 필요한 경우 위의 Packaged Shipping Build를 이용해 주세요.

제외 항목:
- Binaries/
- Intermediate/
- Saved/
- DerivedDataCache/
- .vs/
- 대용량 콘텐츠 및 외부 라이선스 Asset

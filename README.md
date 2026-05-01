[![DroneXpress](/Images/Drone_X-press.PNG)](https://www.youtube.com/watch?v=zQOUfBSoa4g)
# Drone X-press
* 드론을 몰고 마을에 빠르게 배달을 목표로 하는 캐쥬얼 비행 아케이드 게임
* 난이도가 달라지는 3개의 스테이지의 배달시간 기록 갱신

## 프로젝트 정보(Project Info)
* 장르 : 캐주얼 비행 아케이트 / 타임 어택
* 플랫폼 : PC
* 엔진 : Unreal Engine 5.5
* 진행 기간: 26/04/21 - 26/05/01

## 팀원(Participants)
* 박주홍: https://github.com/markpark65
    * 게임 기획, 레벨 디자인, 테스팅
* 이채호: https://github.com/asdf5849067
    * UX/UI, 게임 플로우
* 이은진: https://github.com/ljin0x0
    * 레벨 디자인, 에셋 리서치, 테스팅
* 김재성: https://github.com/3924js
    * 게임 프로그래밍, 에셋 리서치

## 프로젝트 설명(Project Description)
* 카툰풍의 마을을 드론으로 다니며 정해진 장소에 택배 배달
    * 3축으로 이동 및 회전이 가능 드론의 움직임
    * 서로 충돌시 배달을 인식하는 택배상자와 배달지점
* 배달을 다채롭게 만드는 여러 장애물과 버프
    * 플레이어를 추적해 슬로우, 암전, 조종불가등을 충돌하며 부여하는 운석들
    * 회전 장애물, 이동 장애물, 전선등 곳곳에 배치된 방해요소
    * 텔레포트, 소형화등 배달에 도움되는 아이템
* 스테이지에 따른 장애물, 방해요소, 아이템의 변화와 기록 측정
    * 3단계 스테이지 구성
        * 1스테이지 - Rookie Flight: 가속 게이트와 전선등 기초적인 기능과 함께 맵을 익히는 스테이지
        * 2스테이지 - Intermediate Flight: 아이템과 유성등 추가적인 기능을 접하는 스테이지
        * 3스테이지 - Pro Racer: 모든 기믹이 존재하는 가장 높은 난이도 스테이지
    * 시간 기록의 측정과 리더보드 시스템

## 사용 에셋(Assets used)
* 그래픽
    * Assetsbille Town:
        * https://www.fab.com/listings/fd558d8c-bd7e-461f-8449-a7cc9c277078
* SFX
    * thegreatbelow (Freesound) - Security alarm:
        * https://pixabay.com/sound-effects/film-special-effects-security-alarm-63578/
    * olenchic - ELECTRIC:
        * https://pixabay.com/sound-effects/film-special-effects-electric-155027/
    * Universfield - Automobile Horn 02:
        * https://pixabay.com/sound-effects/film-special-effects-automobile-horn-02-352065/
    * ALEXIS_GAMING_CAM - Accepter 1:
        * https://pixabay.com/sound-effects/film-special-effects-accepter-1-394922/
    * freesound_community - teleport:
        * https://pixabay.com/sound-effects/film-special-effects-teleport-90324/
    * EvelX - charging machine.wav:
        * https://freesound.org/people/EvelX/sounds/144129/
    * u_sdjy4zhhbd - car sound effect:
        * https://pixabay.com/sound-effects/city-car-sound-effect-126709/
* VFX
    * Basic VFX Pack:
        * https://www.fab.com/listings/75698e52-edfc-4f76-a86c-b4f26fcf5a29
* font
    * Greenhouse Gas:
        * https://www.1001fonts.com/greenhouse-gas-font.html
    * 7 Segment Bold:
        * https://www.dafont.com/seven-segment.font

## 트러블 슈팅(Troubleshooting)
* 드론 움직임 개선
    * 초기 좌표 직접 수정과 라인트레이스에 기반한 기초적인 이동/충돌 구현
        * 자연스럽지 못한 충돌, 드론의 관성/회전등 자연스러움 부족
    * FloatingPawnMovement로 이동 시스템 변경, 이동과 회전에 보간 설정 추가
    * 각종 충돌 및 이벤트 로직, 이동 자연스러움 개선
* 충돌 이벤트 조건 개선
    * 기존 아이템 사용을 위한 충돌 조건 검사에서 NotifyActorBeginOverlap()를 사용.
        * 드론 외곽에 상자를 인식하기 위한 별도의 더 큰 SphereComponent가 존재, 어떤 컴포넌트가 충돌해도 이벤트가 발생하기에 본체에 충돌하기 전에 아이템 습득, 사용판정.
    * 대신 OnComponentBeginOverlap에 바인드할 함수를 만들고 AddDynamic으로 연결
    * 또한 기획 변경에 따라 기존 외곽 감지용 충돌체 삭제
    * 더 세밀한 충돌 이벤트 처리 가능, 정상적인 거리에서의 아이템 사용/운석 충돌 구현
* 배달지점 시인성 문제
    * 배달지점이 하나의 넓은 박스로만 되어있어 맵 사이즈에 비해 작아 플레이어가 어디에 배달지점이 있는지 인식하기 난해
    * 미니맵 추가, 배달지점 방향 표시, 빛 기둥 이펙트 추가
    * 어디있든 배달지점 방향을 파악할 수 있도록 개선
* 속도감 문제
    * 드론 조종에 있어서 관성으로 부드럽게 움직이긴 하지만 속도에 대한 체감, 몰입도 부족
    * 속도에 비례해 달라지는 카메라 FOV, 드론 비행 사운드 추가
    * 속도계, 시간타이머 디지털 폰트 사용으로 레이싱게임스러운 디자인 추가
    * 플레이중 느껴지는 속도감과 몰입감 개선.
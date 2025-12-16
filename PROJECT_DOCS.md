# 项目综合文档

## 一、需求说明书
### 1. 系统背景
本项目为 Qt 桌面端实现的“欢乐斗地主”卡牌游戏，支持单机模式下的人机对战。游戏围绕发牌、抢地主、出牌、计分与结算等核心流程展开，包含动画与音乐资源以提升体验。

### 2. 系统目标
- 提供完整的斗地主一局流程（发牌、抢地主、出牌、结算、重新开局）。
- 支持 1 名用户与 2 名机器人对战，具备基础 AI 决策（抢地主、出牌）。
- 提供直观的图形界面、音效与倒计时提示，保障操作反馈。

### 3. 角色
- **玩家（user）**：真人用户，操作 UI 发起抢地主、出牌或“不要”。
- **机器人（robot）**：左右两名 AI，对应 player::ROOT 类型，自动决策抢地主与出牌。
- **游戏控制器（gamecontrol）**：调度全局状态与信号，负责发牌、轮转、结算与状态通知。

### 4. 功能列表
- **加载与启动**：启动页 `Loadprocess` 显示加载动画，随后进入主界面。
- **发牌/抢地主**：`gamecontrol::SetAllCards` 创建完整牌堆；`StartPrepareLord` 触发叫分信号；`BecomeLord` 确定地主并发放底牌。
- **出牌流程**：`gamecontrol::GamePlayhand` 校验/更新出牌记录、处理“要不起”、驱动下一位玩家决策。
- **AI 决策**：`Strategy` 评估牌力并生成出牌；`robotgetloard`、`robotplayhand` 负责具体 AI 行为。
- **界面呈现**：`Maingame` 渲染背景、卡牌、按钮、倒计时、动画、音效；`CardPanel` 展示单张卡牌与交互。
- **计分与结算**：`GamePlayhand` 完成倍数计算与分数增减，触发 `EndPanel` 结算界面与音效。
- **重新开局**：`Maingame::RePlayGame` 调用 `RetCardDate`/`InitGameScene` 等重置状态，重新发牌。

### 5. 非功能需求
- **性能**：单机桌面应用，需保证动画、发牌、拖拽等操作流畅；倒计时基于 `QTimer`。
- **可用性**：界面固定 1000x650，提供角色/状态标签、音效提示、超时自动出牌。
- **稳定性**：重新开局需清理上一局状态，避免空指针及状态泄漏。

### 6. 约束条件
- 必须使用 Qt（.pro 工程，依赖 Widgets、多媒体、动画模块）。
- 资源文件通过 `res.qrc` 管理，路径受 Qt 资源系统约束。
- 仅单机环境，无网络与数据库。

### 7. 运行环境
- Qt 5/6（含 Widgets、Multimedia、Core 模块）。
- C++17 编译器（如 MSVC、MinGW、Clang、GCC）。
- 支持 Windows / Linux / macOS 桌面环境。

### 8. 用户需求描述
- 用户能够开始一局游戏，看到发牌动画与倒计时提示。
- 用户可选择抢/不抢地主，系统根据最高叫分判定地主并发底牌。
- 用户可出牌、拖拽选择或点击选择卡牌，若无法压制可选择“不要”。
- 游戏展示机器人手牌张数、出牌区与提示标签（如“要不起”）。
- 结算后用户可查看胜负与得分，并能重新开始下一局。

## 二、概要设计文档
### 1. 系统总体架构
- **表示层**：`Maingame` 窗口、`Loadprocess` 启动画面、`EndPanel` 结算窗口、`CardPanel`/`MyButtonGroup`/`Timecount` UI 控件。
- **业务控制层**：`gamecontrol` 负责全局状态机（发牌/抢地主/出牌）、信号槽、计分与流程推进。
- **领域层**：`player` 及其子类 `user`、`robot`；`Card`/`Cards` 表示牌值集合；`PlayHand` 判型；`Strategy`/`robotgetloard`/`robotplayhand` 实现 AI 决策。
- **资源与音效层**：`Bgmcontrol` 管理背景音乐/音效；`res.qrc` 管理图片、音频与 JSON 配置。

### 2. 模块划分与关系
- `Maingame` 持有 `gamecontrol` 实例，订阅其信号（状态变更、出牌、叫分、倒计时、结算）。
- `gamecontrol` 维护三名 `player` 对象，分发牌堆、调用玩家接口触发 AI/用户操作，并通过信号与 UI 通信。
- `player`/`robot`/`user` 通过信号 `notifygrablordbet`、`notifyplayhand` 将操作回传给 `gamecontrol`。
- `Strategy` 读取 `Cards`/`PlayHand` 信息计算最优出牌或是否压制。
- 资源文件与界面布局通过 Qt Designer 生成的 `.ui` 与 `res.qrc` 绑定。

### 3. 主要流程
- **启动**：`main.cpp` 创建 `QApplication`，显示 `Loadprocess`；加载完毕后进入 `Maingame`。
- **发牌**：`gamecontrol::SetAllCards` 初始化牌堆；`Maingame::SatrtPend` 调度 `PendCardplayer` 动画发牌至三名玩家。
- **抢地主**：`gamecontrol::StartPrepareLord` -> 玩家 `PrepareGetLord` -> `Onbet` 统计叫分；三轮结束后 `BecomeLord` 发放底牌并切换到出牌状态。
- **出牌**：`gamecontrol::GamePlayhand` 校验“要不起”与正常出牌，更新 `_PlayHandplayer/_PlayHandCards`，通知 UI 与下家；`Maingame::OndisPosePlayhand` 渲染出牌区与音效。
- **结算**：当某玩家手牌为空，按倍数结算分数，发射 `S_PlayResult` 与 `USERWIN` 状态，`EndPanel` 展示结果。
- **重新开局**：`Maingame::RePlayGame` -> `gamecontrol::RetCardDate` 重置牌堆、玩家状态、UI 选中状态，重新发牌。

### 4. 界面结构
- 主窗口 1000x650：背景图、三方出牌区、手牌区、头像/身份标签、提示标签、按钮组（抢地主、出牌、不要、重新开始）、倒计时、分数面板。
- 启动画面：进度条式加载条与背景图。
- 结算面板：显示胜负、分数、继续游戏按钮。

### 5. 关键数据结构
- `Card`：点数 `cardpoint`、花色 `cardsuit`；比较/哈希重载支持 `QSet` 存储。
- `Cards`：`QSet<Card*>` 存储手牌集合，支持增删、排序、随机抽取、点数统计。
- `_Playercontext`（Maingame 内部）：记录每个玩家的手牌区域、出牌区域、头像/提示标签位置及最后出牌缓存。
- `Betrest`：跟踪抢地主轮次与最高叫分。

## 三、详细设计文档
### 1. 主要类与成员
- **Loadprocess**：持有主背景与进度图，`paintEvent` 绘制加载动画，`QTimer` 驱动进度。
- **Maingame**：
  - 成员：图像资源 `_IMage_Map/_IMage_Cards/_Card_back`，`gamecontrol* _Gamecontrol`，玩家上下文映射 `_Playercontexts`，发牌/移动卡片 `CardPanel` 指针，计时器 `_Timer_PlayHand`，音效控制 `_Bgmcontrol`，动画 `_MyAnmation`，倒计时 `_Timecount` 等。
  - 核心方法：
    - `InitGamecontrol` 连接信号、初始化玩家与牌堆。【F:maingame.cpp†L57-L86】
    - `InitCardpanelMap` 裁剪卡面图片并生成 `CardPanel` 映射。【F:maingame.cpp†L177-L200】
    - `InitGameScene`/`InitGroupbtn` 设置 UI 区域与按钮。
    - `SatrtPend`、`PendCardplayer`、`PendCardpos` 负责发牌动画与落位。
    - `PlayerStateChange` 响应状态：抢地主按钮显示、出牌按钮状态、倒计时启动等。
    - `OndisPosePlayhand` 根据 `Cards*` 渲染出牌；空牌表示“要不起”并显示标签/音效。【F:maingame.cpp†L481-L518】
    - `UserPlayHand` / `UserNoPlayer` 触发用户出牌或不出牌信号。
    - `RePlayGame` 清理状态、重新开始；注意 `_Playercontexts` 中 `_Last_Cards` 需判空。【F:maingame.cpp†L703-L733】
- **gamecontrol**：
  - 状态枚举 `GameState`（发牌/抢地主/出牌）与 `USERSTATE`（抢地主/出牌/胜利）。
  - 流程方法：`InitPlayer` 构建三名玩家并连接信号；`SetAllCards` 初始化牌堆；`StartPrepareLord` 开启叫分；`Onbet` 统计最高分并决定重新发牌或 `BecomeLord`；`GamePlayhand` 处理出牌/要不起、倍数、胜负判定；`RetCardDate` 重新开局清理状态。【F:gamecontrol.cpp†L8-L347】
- **player**：
  - 管理手牌集合 `Cards _Cards`，记录前后家链表，角色/性别/位置等元信息；
  - 提供 `PrepareGetLord`、`PreparePlayCard` 作为用户/机器人行为入口；信号 `notifygrablordbet`、`notifyplayhand` 反馈操作。
- **user**：
  - 继承 `player`，重写 `PrepareGetLord`、`PreparePlayCard`，由 UI 按钮驱动信号发送。【F:user.h†L1-L13】
- **robot**：
  - 继承 `player`，在 `RobotGetLard`、`RobotThinkPlayhand` 中使用 `Strategy` 计算叫分/出牌，触发相应信号。【F:robot.h†L1-L20】
- **Strategy**：
  - 根据当前手牌与对手牌型评估（`EvaluateHandStrength`、`ShouldPressCard`），生成可行牌组（`GetrootPlayhand`、`FirstPlayhand`、`Getbigplayhand` 等），支撑机器人决策。【F:strategy.h†L1-L49】
- **PlayHand**：
  - 将 `Cards` 分类并识别牌型；提供 `CanBeat` 判定能否压制上一手；暴露牌型、关键点数、张数等信息。【F:playhand.h†L1-L57】
- **UI 及辅助类**：
  - `CardPanel` 表示单张卡牌的图元与选中状态；`MyButton`/`MyButtonGroup` 管理交互按钮；`Timecount` 显示倒计时；`Bgmcontrol` 控制背景音乐与音效；`AnmationPixmap` 用于连胜/出牌动画；`EndPanel` 展示胜负信息。

### 2. 关键流程与状态
- **出牌状态流转**（`gamecontrol::GamePlayhand`）：
  1. 接收玩家与牌组；若为空则视为“要不起”，不更新出牌记录，仅切换到下家并通知 UI。【F:gamecontrol.cpp†L176-L205】
  2. 正常出牌：更新 `_PlayHandplayer/_PlayHandCards/_CurrentPlayer`，移除玩家手牌，广播信号。【F:gamecontrol.cpp†L209-L223】
  3. 若出炸弹翻倍；若手牌清空则计算分数与胜负，发射结果信号。【F:gamecontrol.cpp†L233-L275】
  4. 否则切换下家并调用其 `PreparePlayCard`。【F:gamecontrol.cpp†L277-L287】
- **叫分/抢地主**（`Onbet`）：统计最高叫分与首叫标记，三轮后若无人叫分则重新发牌，否则调用 `BecomeLord` 发底牌并进入出牌阶段。【F:gamecontrol.cpp†L307-L347】
- **重新开局**：`RetCardDate` 重置牌堆、玩家手牌、出牌记录、倍数与倒计时，防止状态泄漏。【F:gamecontrol.cpp†L110-L138】

### 3. 接口说明
- `gamecontrol::S_gamePlayHand(player*, Cards*)`：通知界面刷新出牌或“要不起”。
- `gamecontrol::S_PlayerStateChange(player*, USERSTATE)`：驱动 UI 更新按钮与提示。
- `player::notifygrablordbet(player*, int)`：玩家叫分信号。
- `player::notifyplayhand(player*, Cards*)`：玩家出牌信号。
- `Maingame` 槽函数 `gamenotifyGetLoard`、`OndisPosePlayhand`、`PlayerStateChange` 等接收上述信号。

### 4. 关键算法
- **牌型识别**：`PlayHand::classify/judeHandType` 将手牌按数量分组，判断顺子、连对、飞机、炸弹、王炸等牌型。
- **AI 出牌策略**：`Strategy::EvaluateHandStrength` 等评分函数评估手牌强度，`ShouldPressCard` 判断是否压制上一手，`FindHand_*` 系列搜索满足条件的牌组（如三带、飞机、顺子）。

## 四、数据库设计文档
- 项目未使用数据库。数据全部保存在内存结构中：
  - 牌堆/手牌由 `Cards`（`QSet<Card*>`）管理，随机发牌与排序在内存完成。
  - 游戏配置（音效列表）存放在 `conf/playList.json` 并通过 Qt 资源系统加载。
  - 分数等状态变量存在 `player` 与 `gamecontrol` 成员中，游戏结束后不会持久化。

## 五、源代码说明
### 1. 目录结构
- `main.cpp`：应用入口，创建 `Loadprocess` 窗口。
- `maingame.*` / `maingame.ui`：主界面窗口与逻辑。
- `gamecontrol.*`：核心状态机与流程调度。
- `player.*` `user.*` `robot.*`：玩家基类与人机实现。
- `strategy.*` `playhand.*`：AI 策略与牌型判断。
- `card.*` `cards.*` `cardpanel.*`：牌数据结构与 UI 呈现。
- `robotgetloard.*` `robotplayhand.*`：机器人叫分/出牌实现。
- `bgmcontrol.*` `timecount.*` `anmationpixmap.*` `endpanel.*`：音效、倒计时、动画、结算界面。
- `images/` `music/` `conf/` `res.qrc`：资源与资源清单。
- `LandFarmer.pro`：Qt 工程文件。

### 2. 函数/类用途速查
- `Maingame::InitGamecontrol`：初始化 `gamecontrol`、玩家与信号连接。【F:maingame.cpp†L57-L86】
- `Maingame::OndisPosePlayhand`：渲染出牌/要不起、更新提示与音效。【F:maingame.cpp†L481-L518】
- `Maingame::RePlayGame`：清理状态并重新开局。【F:maingame.cpp†L703-L733】
- `gamecontrol::SetAllCards`：构建 54 张牌堆并重置计数。【F:gamecontrol.cpp†L71-L94】
- `gamecontrol::GamePlayhand`：处理出牌逻辑、倍数、胜负判定。【F:gamecontrol.cpp†L176-L287】
- `gamecontrol::Onbet`：处理抢地主流程，决定重新发牌或进入出牌阶段。【F:gamecontrol.cpp†L307-L347】
- `player::RemoveCards` / `ClearCards`：管理玩家手牌；`SetPendinfo` 记录上一手。
- `Strategy::GetrootPlayhand` / `Getbigplayhand`：返回 AI 选择的出牌组合。【F:strategy.h†L17-L40】
- `PlayHand::CanBeat`：比较两副牌能否压制。【F:playhand.h†L21-L42】

### 3. 程序执行流程
- **初始化**：`main` -> `Loadprocess`（计时绘制） -> `Maingame` 初始化资源与控制器。
- **发牌阶段**：`gamecontrol::SetAllCards` 创建牌堆；`Maingame::SatrtPend` 循环调用 `PendCardplayer` 将牌分发到各玩家 `_Playercontext`。
- **抢地主阶段**：`StartPrepareLord` 通知当前玩家叫分；`Onbet` 统计；若无人叫分则 `RetCardDate` 重洗牌。
- **出牌阶段**：`PreparePlayCard` -> UI/AI 选择牌型 -> `GamePlayhand` 判定与轮转；`Timecount` 提示倒计时，超时可自动出牌。
- **结算与重开**：手牌为空时结算分数与胜负，`EndPanel` 显示；`RePlayGame` 触发新一局。

### 4. 关键逻辑说明
- **“要不起”处理**：在 `GamePlayhand` 中空牌直接进入下家，同时向 UI 发 `S_gamePlayHand` 以展示“要不起”标签与音效。【F:gamecontrol.cpp†L176-L205】
- **倍数逻辑**：炸弹 `Hand_Bomb` 将 `_Bet` 倍数翻倍，影响结算分数。【F:gamecontrol.cpp†L229-L274】
- **计分**：根据地主/农民身份调整胜负双方分数；`_Bet` 由叫分与炸弹叠加而成。【F:gamecontrol.cpp†L233-L269】
- **AI 策略**：`Strategy` 通过牌力评估与搜索组合决定是否压制或走最优出牌；`robot` 调用相应策略函数完成自动操作。
- **动画与音效**：`Bgmcontrol` 在状态变化（如出牌、要不起、结果）时播放对应音效；`AnmationPixmap` 展示出牌动画；`res.qrc` 统一管理资源路径。

### 5. 框架/技术说明
- 使用 **Qt Widgets** 构建 UI，事件处理（鼠标、定时器、动画）均基于 Qt 信号槽。
- **Qt Multimedia**（通过 `Bgmcontrol`）播放背景音乐与音效。
- **Qt 资源系统**（`res.qrc`）嵌入图片、音频、JSON 配置。
- 无网络、数据库、AI 框架外部依赖，AI 逻辑完全自研于 `Strategy`。

## 六、测试说明文档
### 1. 自带测试代码
仓库未提供自动化测试或单元测试文件，主要依赖手动运行进行验证。

### 2. 测试策略
- **功能测试**：验证发牌、抢地主、出牌、倍数翻倍、结算、重新开局流程。
- **逻辑测试**：检查“要不起”处理、倍数计算、身份变更、倒计时触发。
- **异常/边界测试**：
  - 无人叫分时是否重新洗牌；
  - 结算后重新开局是否清理状态（避免空指针 `_Last_Cards` 等）；
  - 超时自动出牌是否正常；
  - 炸弹/王炸等特殊牌型是否正确识别与倍数翻倍。
- **资源测试**：音效与图片路径是否能从 `res.qrc` 正常加载。

### 3. 测试用例示例
- **抢地主轮空**：三人均选“不要”后应重新发牌且分数不变。
- **要不起表现**：任意一方无法压制时，UI 显示“要不起”标签且播放对应音效。
- **炸弹倍数**：任意炸弹出牌后 `_Bet` 翻倍，最终结算分数随倍数变化。
- **胜负结算**：地主/农民胜利时分数增减符合规则，并触发 `S_PlayResult` 音效。

### 4. 测试执行结果
当前仓库未包含自动化测试结果；需通过手动运行游戏进行验证。

## 七、可执行文件与部署说明
### 1. 编译
- 使用 Qt Creator 打开 `LandFarmer.pro`；确保 Qt 多媒体与 Widgets 模块可用。
- 选择合适的工具链（MSVC/MinGW/GCC/Clang）并构建。
- 资源由 `res.qrc` 自动编译入可执行文件，无需额外拷贝。

### 2. 运行
- **Windows/Linux/macOS**：构建后直接运行生成的可执行文件。若使用动态链接版本 Qt，需要确保 Qt 库、`plugins/platforms` 等插件在同目录或已配置 `QT_PLUGIN_PATH`。

### 3. 资源部署
- 图片、音效、JSON 配置均打包在资源系统中，无需额外部署；若改为外部资源，需保持 `images/`、`music/`、`conf/` 路径与 `res.qrc` 一致。

### 4. 环境/输入要求
- 需鼠标操作（点击/拖拽卡牌、按钮）；
- 若开启音效需可用的系统音频输出；
- 无网络、数据库或额外服务依赖。

## 八、期末大作业报告（Markdown）

### 1. 选题背景
本项目选题为经典三人扑克牌游戏“斗地主”的桌面端实现。通过 Qt 框架完成 GUI、动画、音频与交互，结合简易 AI 实现人机对战，适合作为课程设计或期末大作业。

### 2. 开发环境说明
- 语言：C++17
- 框架：Qt Widgets + Qt Multimedia
- 工程：`LandFarmer.pro`
- 资源：`res.qrc` 管理的图片、音频、JSON 配置
- 运行平台：Windows / Linux / macOS 桌面

### 3. 系统功能说明
- 启动/加载界面：展示加载进度后进入主界面。
- 发牌与抢地主：随机洗牌、发牌并进行三轮叫分，最高分者成为地主获取底牌。
- 出牌与提示：支持用户点击/拖拽选牌，出牌或“不要”；机器人自动决策；倒计时与音效提示。
- 倍数与计分：根据叫分与炸弹翻倍，游戏结束时依据身份加减分。
- 结算与重开：结算面板显示胜负与分数，允许重新开始新一局。

### 4. 系统设计说明
- **架构**：三层（UI 层 `Maingame`、控制层 `gamecontrol`、领域层 `player`/`Strategy` 等）。
- **数据流**：`gamecontrol` 产生状态信号 -> `Maingame` 更新 UI；用户/机器人操作通过 `notifygrablordbet`/`notifyplayhand` 反馈给控制层。
- **状态机**：`PENDCARD`（发牌）→`GETLORD`（抢地主）→`GIVECARD`（出牌）→`USERWIN`（结算）→重新开局。
- **AI 设计**：`Strategy` 评估牌力并搜索可行牌型；`robot` 在适当时机调用策略输出叫分/出牌结果。

### 5. 技术点说明
- Qt 信号槽驱动的事件流；
- `QPainter`/`QPixmap` 渲染背景与卡牌裁剪；
- `QTimer` 控制发牌/倒计时/动画节奏；
- `PlayHand` 牌型判定与比较；
- `Strategy` 牌力评估与搜索；
- Qt 资源系统整合图片与音效。

### 6. 关键代码说明
- `Maingame::InitGamecontrol`：完成控制器初始化与信号绑定，确保 UI 能响应状态变化。【F:maingame.cpp†L57-L86】
- `gamecontrol::GamePlayhand`：集中处理出牌、要不起、倍数、结算与轮转，是核心状态流转函数。【F:gamecontrol.cpp†L176-L287】
- `gamecontrol::Onbet`：实现抢地主流程与重新发牌分支，决定是否进入出牌阶段。【F:gamecontrol.cpp†L307-L347】
- `PlayHand` / `Strategy`：提供牌型识别与 AI 决策的关键逻辑。【F:playhand.h†L21-L57】【F:strategy.h†L17-L49】

### 7. 系统运行截图说明
仓库未附运行截图，可预期界面包括：顶部/左右头像与身份标签、中央出牌区、底部用户手牌与按钮、倒计时与分数面板；背景图为 `images/background-*.png` 随机选取。

### 8. 遇到的问题与解决方案
- **要不起未显示/无音效**：`GamePlayhand` 早期未向 UI 发信号，导致 `OndisPosePlayhand` 不触发。现已在空牌分支发送 `S_gamePlayHand` 以更新标签与音效。【F:gamecontrol.cpp†L176-L205】【F:maingame.cpp†L481-L518】
- **重新开局空指针风险**：`RePlayGame` 需判空 `_Last_Cards` 后再清理，避免上一局置空导致崩溃。【F:maingame.cpp†L703-L733】
- **状态残留**：`RetCardDate` 重置牌堆、出牌记录与倒计时，防止上一局信息泄漏。【F:gamecontrol.cpp†L110-L138】

### 9. 测试说明
- 手动场景：验证发牌/抢地主/出牌/倍数/结算；
- 边界：全员不叫分、连续“要不起”、炸弹/王炸、重新开局。
- 自动化：仓库暂无，需要后续补充。

### 10. 项目总结
项目实现了完整的斗地主流程与基础 AI，界面与音效资源丰富。核心控制器 `gamecontrol` 将状态机、计分与信号统一管理，UI 通过 `Maingame` 渲染并提供用户交互。后续可扩展网络对战、持久化战绩、更多 AI 策略与单元测试。

### 11. 参考文献
- Qt 官方文档（Widgets、Multimedia、资源系统）
- 斗地主规则与常见牌型策略

### 12. 附录：大模型协作过程记录
- 阅读与梳理仓库结构，确认无数据库与网络依赖。
- 解析核心文件 `maingame.cpp`、`gamecontrol.cpp`、`player/robot/strategy/playhand` 等，提炼状态机与流程。
- 根据源码逻辑编写需求、概要设计、详细设计、测试与部署说明。
- 汇总关键问题与解决方案，形成期末报告。

